#include "evaluate.h"
#include "bt_string.h"
#include "type.h"
#include "number.h"
#include "consts.h"
#include "utils.h"
#include "res_wrds.h"
#include "code_output.h"
#include "variable.h"
#include "code_stream.h"
#include "envelope.h"
#include "expression_tree.h"
#include "parameter.h"
#include "compiler.h"

#include <string.h>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

/* forward declarations */
void deliver_ccidx_dest(variable **target_var, nap_compiler *_compiler, const expression_tree* node, int level, const method* the_method, call_context* cc, int reqd_type, int& idxc, const variable* var, int forced_mov, bool &psuccess);

/**
 * Returns true if the type that's passed in is an atomic type (string, number, variable, env.var)
 */
bool is_atomic_type(int the_type)
{
    switch (the_type)
    {
    case BASIC_TYPE_STRING:
    case BASIC_TYPE_VARIABLE:
    case BASIC_TYPE_CLASS_VAR:
    case BASIC_TYPE_INT:
    case BASIC_TYPE_REAL:
    case BASIC_TYPE_CHAR:
    case BASIC_TYPE_BOOL:
    case ENVIRONMENT_VARIABLE:
    case TEMPLATED_VARIABLE:
    case KEYWORD_FALSE:
    case KEYWORD_TRUE:
    case BASIC_TYPE_EXTERN_VARIABLE:
        return true;
    }
    return false;
}

static bool is_post_pre_op(int opt)
{
    return opt == OPERATOR_POSTINC || opt == OPERATOR_PREINC || opt == OPERATOR_POSTDEC || opt == OPERATOR_PREDEC;
}

/**
 * Returns true if the node contains data that is an "lvalue" (ie. you can assign into it)
 */
static bool is_assign_destination(expression_tree* node)
{
    return BASIC_TYPE_CLASS_VAR == node->op_type
            || BASIC_TYPE_VARIABLE == node->op_type
            || BASIC_TYPE_INDEXED == node->op_type
            || MULTI_DIM_INDEX == node->op_type
            || BASIC_TYPE_EXTERN_VARIABLE == node->op_type;
}

static bool is_variable(expression_tree* node)
{
    return BASIC_TYPE_VARIABLE == node->op_type
            || BASIC_TYPE_CLASS_VAR == node->op_type
            || BASIC_TYPE_EXTERN_VARIABLE == node->op_type;
}

/**
 * Deals with the post/pre increment/decrement.
 * The result is returned in reg(level) so the callers need to be aware of this.
 * There is code generated automatically for handling the increment of the variable
 */
static void deal_with_post_pre_node(variable** target_var, nap_compiler* _compiler,
                                    const expression_tree* pp_node,
                                    int level,
                                    const method* the_method,
                                    call_context* cc,
                                    int forced_mov, bool& psuccess)
{
    if(is_variable(pp_node->left))    /* a normal variable is being post/pre'd */
    {
        variable* var = (variable*)pp_node->left->reference->to_interpret;    /* this is the variable */
        if(pp_node->op_type == OPERATOR_POSTDEC || pp_node->op_type == OPERATOR_POSTINC) /*post increment/decrement?*/
        {
            mov_target_register_source_var(target_var, _compiler, cc, var, level);                   /* post operation: firstly "use" the original value*/
            unary_operation_variable(_compiler, cc, pp_node->op_type, var);    /* then update it*/
        }
        else    /* must be pre increment/decrement */
        {
            unary_operation_variable(_compiler, cc, pp_node->op_type, var);    /* firstly update the variable*/
            mov_target_register_source_var(target_var, _compiler, cc, var, level);                   /* then "use" it*/
        }
    }
    else
    if(pp_node->left->op_type == MULTI_DIM_INDEX) /* this is an array value post/pre inc/decd*/
    {
        int idxc = 0;
        variable* var = (variable*)(((envelope*)pp_node->left->left->reference)->to_interpret);
        deliver_ccidx_dest(target_var, _compiler,pp_node->left, level + 1, the_method, cc, var->i_type, idxc, var, forced_mov, psuccess);            /* firstly initialize the "return" value */
        SUCCES_OR_RETURN;

        if(pp_node->op_type == OPERATOR_POSTDEC || pp_node->op_type == OPERATOR_POSTINC)
        {
            mov_target_register_source_indexed(_compiler, cc, var, level, idxc);            /* reg(level) goes out to the caller*/
            operation_on_indexed(_compiler, cc, pp_node->op_type, var, idxc);        /* then increment it */
        }
        else
        {
            operation_on_indexed(_compiler, cc, pp_node->op_type, var, idxc);        /* update it */
            mov_target_register_source_indexed(_compiler, cc, var, level, idxc);            /* then initialize the return value (use it) in the caller*/
        }

        clidx(_compiler);                                            /* and finally liberate the indexes for the next operation*/
    }
    else
    {
        cc->compiler->throw_error("Can not post/pre increment this value");
        psuccess = false;
    }
}

/**
 * Deals with the += -= /= *= etc= operations.
 */
static void resolve_op_equal(variable** target_var, nap_compiler* _compiler,
                             const expression_tree* node,
                             const method* the_method,
                             call_context* cc,
                             int level,
                             int reqd_type,
                             int forced_mov, bool& psuccess)
{
    if(node->left)
    {
        if(node->left->op_type == BASIC_TYPE_VARIABLE || node->left->op_type == BASIC_TYPE_CLASS_VAR)
        {
            variable* var = (variable*)node->left->reference->to_interpret;
            if(is_atomic_type(node->right->op_type))
            {
                mov_start_register(_compiler, var->i_type, level);
                compile(target_var, _compiler,node->right, the_method, cc, level, var->i_type, forced_mov, psuccess);    /* filled up the 'incby' */
                if(!psuccess)
                {
                    return;
                }

                operation_target_var_source_reg(_compiler, var->cc, node->op_type, var, level);
            }
            else
            {
                if(is_post_pre_op(node->right->op_type))    /* we post/pre inc/dec with "something++" */
                {
                    deal_with_post_pre_node(target_var, _compiler, node->right, level, the_method, cc, forced_mov, psuccess); /* ??? var.cc ??*/
                    SUCCES_OR_RETURN;

                    operation_target_var_source_reg(_compiler, cc, node->op_type, var, level);
                }
                else    /* the value with which we incr/decr is a normal "complex" operation, compile it for the current level and add (sub...) it to the variable */
                {
                    compile(target_var, _compiler,node->right, the_method, cc, level, var->i_type, forced_mov, psuccess);
                    SUCCES_OR_RETURN;

                    operation_target_var_source_reg(_compiler, cc, node->op_type, var, level);
                }
            }
        }
        else
        if(MULTI_DIM_INDEX == node->left->op_type) /* getting here with z[1] += xxx */
        {
            /* let's put in reg(level+1) what this indexed will be incremented with*/
            variable* var = (variable*)(((envelope*)node->left->left->reference)->to_interpret);
            int index;
            if(is_atomic_type(node->right->op_type))    /* getting here with z[1] += 4*/
            {
                deliver_ccidx_dest(target_var, _compiler, node->left, level, the_method, cc, reqd_type, index, var, forced_mov, psuccess);    /* fisrtly calculating the index since this might mess up the registers*/
                SUCCES_OR_RETURN;

                mov_start_register(_compiler, var->i_type, level);
                compile(target_var, _compiler,node->right, the_method, cc, level, var->i_type, forced_mov, psuccess);            /* printing right's content*/
                SUCCES_OR_RETURN;

                operation_target_indexed_source_reg(_compiler, cc, node->op_type, var, index, level);        /* and finally updating the indexed value*/
                clidx(_compiler);
            }
            else
            {
                if(is_post_pre_op(node->right->op_type))    /* we post/pre inc/dec with "something++" */
                {
                    deliver_ccidx_dest(target_var, _compiler,node->left, level, the_method, cc, reqd_type, index, var, forced_mov, psuccess);    /* firstly calculating the index since this might mess up the registers*/
                    SUCCES_OR_RETURN;

                    deal_with_post_pre_node(target_var, _compiler, node->right, level, the_method, cc, forced_mov, psuccess);    /* then initializing the result of the post/pre operation */
                    SUCCES_OR_RETURN;

                    operation_target_indexed_source_reg(_compiler, cc, node->op_type, var, index, level);     /* and finally updating the indexed value*/
                    clidx(_compiler);                /* and clearing the indexes*/
                }
                else    /* the value with which we incr/decr is a normal "complex" operation, compile it for the current level and add (sub...) it to the variable */
                {
                    compile(target_var, _compiler,node->right, the_method, cc, level+1, var->i_type, forced_mov, psuccess);
                    SUCCES_OR_RETURN;

                    operation_target_register_source_register(_compiler, var->i_type, level, var->i_type, level + 1);

                    deliver_ccidx_dest(target_var, _compiler,node->left, level, the_method, cc, reqd_type, index, var, forced_mov, psuccess);    /* fisrtly calculating the index since this might mess up the registers*/
                    SUCCES_OR_RETURN;

                    operation_target_indexed_source_reg(_compiler, cc, node->op_type, var, index, level);     /* and finally updating the indexed value*/
                    clidx(_compiler);
                }
            }
        }
    }
    else
    {
        cc->compiler->throw_error("Internal: op= not having a variable to increment");
        psuccess = false;
    }
}


/**
 * Generates code to deliver the result of an indexed operation.
 * @param node - is the node which is holding the MULTI_DIM_INDEX op type
 * @param level - the current level on which we're working
 * @param the_method - the method in which this is happening
 * @param cc - the call context in which this is happening
 * @param reqd_type - the required type
 */
void deliver_ccidx_dest(variable** target_var, nap_compiler* _compiler, const expression_tree* node, int level,
                        const method* the_method, call_context* cc,
                        int /*reqd_type*/, int& idxc, const variable* /*var*/,
                        int forced_mov, bool& psuccess)
{
    multi_dimension_index* mdi = (multi_dimension_index*)node->right->reference->to_interpret;
    std::vector<expression_tree*>::iterator indxs = mdi->dimension_values->begin();
    idxc = 0;
    while(indxs != mdi->dimension_values->end())
    {
        if(is_atomic_type((*indxs)->op_type))    /* put the indexes into reg(current level)*/
        {
            mov_target_index_register_source_atomic(target_var, _compiler, idxc, *indxs, the_method, cc, level, forced_mov, psuccess);
            SUCCES_OR_RETURN;

        }
        else
        {
            if(is_post_pre_op((*indxs)->op_type))    /* put the index in the next level, modify the variable*/
            {
                deal_with_post_pre_node(target_var, _compiler, *indxs, level + 1, the_method, cc, forced_mov, psuccess);    /* dealing with the post/pre operation*/
                SUCCES_OR_RETURN;

                /* and here updating the current index register to hold the value from the reg(level) */
                mov_target_index_register_source_int_register(_compiler, idxc, level + 1);

            }
            else    /* compile everything in the next level and finally assign the next reg to current reg*/
            {
                int int_type = BASIC_TYPE_INT;
                compile(target_var, _compiler,*indxs, the_method, cc, level + 1, int_type, forced_mov, psuccess);
                SUCCES_OR_RETURN;

                mov_target_index_register_source_int_register(_compiler, idxc, level+1);
            }
        }
        idxc ++;
        indxs ++;
    }
    /* now into the current register move the variable x[reg_idxi(0), reg_idxi(1), ...] */
}



/**
 * Resolves the code generation for the operation found in the given node
 */
void resolve_operation(variable** target_var, nap_compiler* _compiler,
                       const expression_tree* node,
                       int reqd_type,
                       int level,
                       const method* the_method,
                       call_context* cc,
                       int forced_mov, bool& psuccess)
{
    if(node->left)    /* the first operand is in the left node */
    {
        if(is_atomic_type(node->left->op_type))    /* the evaluation of the left will go into the current level reg*/
        {                                        /* and to this we'll add the evaluation of the right */
            if(node->right)                        /* binary operation with two sides */
            {
                mov_start_register(_compiler, reqd_type, level);
                compile(target_var, _compiler,node->left, the_method, cc, level, reqd_type, forced_mov, psuccess);    /* the stuff contained in the 'atomic' left branch */
                SUCCES_OR_RETURN;

                if(is_atomic_type(node->right->op_type))    /* number / variable */
                {
                    operation_start_register(_compiler, node, reqd_type, level);
                    compile(target_var, _compiler,node->right, the_method, cc, level, reqd_type, forced_mov, psuccess);    /* make the operations with the right side*/
                    SUCCES_OR_RETURN;
                }
                else
                {
                    if(is_post_pre_op(node->right->op_type)) /* in case the second operand is post/pre inc/dec the handling is different*/
                    {
                        deal_with_post_pre_node(target_var, _compiler, node->right, level + 1, the_method, cc, forced_mov, psuccess);
                        SUCCES_OR_RETURN;

                        operation_register_level_register_next_level(_compiler, node, reqd_type, level);

                    }
                    else    /* this is a "normal" operation or similar */
                    {
                        compile(target_var, _compiler,node->right, the_method, cc, level + 1, reqd_type, forced_mov, psuccess);
                        SUCCES_OR_RETURN;

                        operation_register_level_register_next_level(_compiler, node, reqd_type, level);
                    }
                }
            }
            else
            {
                cc->compiler->throw_error("Internal error in an operation");
                psuccess = false;
                return;
            }
        }
        else    /* node->left is not atomic */
        {
            /* firstly initialize the left side (first operand) to the current level variable */
            if(is_post_pre_op(node->left->op_type))    /* post/pre inc*/
            {
                deal_with_post_pre_node(target_var, _compiler, node->left, level, the_method, cc, forced_mov, psuccess);    /* deals with the post/pre in the left node*/
                SUCCES_OR_RETURN;

            }
            else
            {
                compile(target_var, _compiler,node->left, the_method, cc, level + 1, reqd_type, forced_mov, psuccess);
                SUCCES_OR_RETURN;

                move_register_level_register_next_level(_compiler,reqd_type, level);
            }

            if(node->right) /* this branch will put the actual operation in the operations list with reg(level)*/
            {
                if(is_atomic_type(node->right->op_type))
                {
                    operation_start_register(_compiler, node, reqd_type, level);
                    compile(target_var, _compiler,node->right, the_method, cc, level, reqd_type, forced_mov, psuccess);
                    SUCCES_OR_RETURN;

                }
                else
                {
                    if(is_post_pre_op(node->right->op_type))
                    {
                        deal_with_post_pre_node(target_var, _compiler, node->right, level + 1, the_method, cc, forced_mov, psuccess); /* initialize the next level register to the result of the post/pre operation*/
                        SUCCES_OR_RETURN;

                        operation_register_level_register_next_level(_compiler, node, reqd_type, level);
                    }
                    else
                    {
                        compile(target_var, _compiler,node->right, the_method, cc, level + 1, reqd_type, forced_mov, psuccess);
                        SUCCES_OR_RETURN;

                        operation_register_level_register_next_level(_compiler, node, reqd_type, level);
                    }
                }
            }
        }
    }
}


static void resolve_class_member(variable** target_var, nap_compiler* _compiler, const expression_tree* node,
                                 int level, const method* the_method, call_context* cc, int forced_mov, bool& psuccess)
{
    variable* dest = (variable*)(((envelope*)node->left->reference)->to_interpret);
    if(node->father->right)
    {
        if(! is_post_pre_op(node->father->right->op_type))
        {
            if(is_atomic_type(node->father->right->op_type))
            {
                mov_start_register(_compiler, dest->i_type, level);
            }
            compile(target_var, _compiler,node->father->right, the_method, cc, level, dest->i_type, forced_mov, psuccess);
            SUCCES_OR_RETURN;

        }
        else        /* indexed = postinc/dec / preinc/dec*/
        {
            deal_with_post_pre_node(target_var, _compiler, node->right, level, the_method, cc, forced_mov, psuccess);
            SUCCES_OR_RETURN;

            /* when the above ended, reg(level) contains the variable I need to assign */
        }
    }

}

static void do_list_assignment(variable** target_var, nap_compiler* _compiler,
                               envelope* rvalue, variable* var, int level,
                               const method* the_method, call_context* cc,
                               int reqd_type, bool& psuccess )
{
    std::vector<envelope*> *lstvec = (std::vector<envelope*>*)rvalue->to_interpret;
    std::vector<envelope*>::iterator lst = lstvec->begin();
    int indxctr = 0;
    while(lst != lstvec->end())
    {
        code_stream(_compiler) << mov()

                      << reg() << "idx"  << '0'

                      << indxctr
                      ;

        if(((expression_tree*)(*lst)->to_interpret)->op_type <= var->i_type)
        {
            code_stream(_compiler) << mov()

                          << reg() << get_reg_type(var->i_type)  << level
                          ;
        }
        compile(target_var, _compiler,(expression_tree*)(*lst)->to_interpret, the_method, cc, level + 1, reqd_type, 0, psuccess);
        SUCCES_OR_RETURN;

        code_stream(_compiler) << mov()

                      << ccidx()  << (std::string(cc->name) + STR_DOT + var->name)  << '1'

                      << reg() << get_reg_type(var->i_type)  << level
                      ;

        clidx(_compiler);

        lst ++;
        indxctr ++;
    }
}

/**
 * Deals with code generation for the assignment
 */
static void resolve_assignment(variable** target_var, nap_compiler* _compiler, const expression_tree* node,
                        int level, const method* the_method, call_context* cc,
                        int reqd_type , int forced_mov, bool& psuccess, bool& array_init_not_needed)
{
    /* what will be calculated will be put all the time in the register on the current level */
    if(node->left)    /* contains the destination */
    {
        if(is_assign_destination(node->left))    /* can assign only into a variable */
        {
            variable* dest = NULL;
            if(is_variable(node->left))    /* assignment goes into a variable */
            {
                dest = (variable*)(((envelope*)node->left->reference)->to_interpret);
                if(!dest)
                {
                    cc->compiler->throw_error("cannot find a variable");
                    psuccess = false;
                    return;
                }
                if(node->right)
                {
                    if(! is_post_pre_op(node->right->op_type))    /* maybe here check if the right side is logical/comparison */
                    {
                        if(node->right->op_type == LIST_VALUE)
                        {
                            do_list_assignment(target_var, _compiler, node->right->reference, dest, level, the_method, cc, reqd_type, psuccess);
                            SUCCES_OR_RETURN;

                        }
                        else
                        {
                            /* start a move into the current register */
                            if(is_atomic_type(node->right->op_type))
                            {
                                mov_start_register(_compiler, dest->i_type, level);
                            }
                            compile(target_var, _compiler,node->right, the_method, cc, level, dest->i_type, forced_mov, psuccess);
                            SUCCES_OR_RETURN;

                            if(dest->func_par)
                            {
                                mov_target_variable_source_register(_compiler, dest->func_par->the_method->main_cc, dest, level);
                            }
                            else
                            {
                                if(node->right->op_type == FUNCTION_CALL)
                                {
                                    // see if the function returns an array or a class type
                                    call_frame_entry* cfe = (call_frame_entry*)node->right->reference->to_interpret;
                                    if(! (cfe->the_method->dynamic_ret_type_array || !(cfe->the_method->ret_type_array_dim.empty())))
                                    {
                                        mov_target_variable_source_register(_compiler, cc, dest, level);
                                    }
                                    else
                                    {
                                        // method returns array or object. Deal with the later, ignore the first
                                        array_init_not_needed = true;
                                    }
                                }
                                else
                                {
                                    mov_target_variable_source_register(_compiler, cc, dest, level);
                                }

                            }
                        }
                    }
                    else    /* postinc/dec / preinc/dec*/
                    {
                        deal_with_post_pre_node(target_var, _compiler, node->right, level, the_method, cc, forced_mov, psuccess);
                        SUCCES_OR_RETURN;

                        mov_start(_compiler);
                        int forced_type = -2;
                        compile(target_var, _compiler,node->left, the_method, cc, level, forced_type, forced_mov, psuccess); /* this will print the "mov to dest" */
                        SUCCES_OR_RETURN;

                        second_part_target_unknown_source_register(_compiler, dest, level);
                    }
                }
            }
            else
            if(OPERATOR_DOT == node->left->op_type)      // class member access
            {
                resolve_class_member(target_var, _compiler, node->left, level, the_method, cc, forced_mov, psuccess);
                SUCCES_OR_RETURN;

            }
            else    /* this is indexed*/
            {
                dest = (variable*)(((envelope*)node->left->left->reference)->to_interpret);
                if(node->right)
                {
                    if(! is_post_pre_op(node->right->op_type))
                    {
                        if(is_atomic_type(node->right->op_type))
                        {
                            mov_start_register(_compiler, dest->i_type, level);
                        }
                        compile(target_var, _compiler,node->right, the_method, cc, level, dest->i_type, forced_mov, psuccess);
                        SUCCES_OR_RETURN;

                    }
                    else    /* indexed = postinc/dec / preinc/dec*/
                    {
                        deal_with_post_pre_node(target_var, _compiler, node->right, level, the_method, cc, forced_mov, psuccess);
                        SUCCES_OR_RETURN;

                        /* when the above ended, reg(level) contains the variable I need to assign */
                    }
                }
                int idxc = 0;
                deliver_ccidx_dest(target_var, _compiler,node->left, level + 1, the_method, cc, reqd_type, idxc, dest, forced_mov, psuccess);
                SUCCES_OR_RETURN;

                /* the above one prepares the ccidx*/
                mov_target_indexed_variable_source_register(_compiler, dest, idxc, level);

                clidx(_compiler);
            }
        }
    }
}

/**
 * The logic of this function is to determine in an expression the maximum value
 * of foundreq, ie. which is the type this expression will evaluate to.
 */
static void populate_maximal_type(const expression_tree* node, int& foundreq)
{
    if(node)
    {
        if(node->right && node->right->op_type == FUNCTION_STRING_LEN) // this can go only in an integer register
        {
            foundreq = BASIC_TYPE_INT;
        }

        switch(node->op_type)
        {
        case BASIC_TYPE_BOOL:
        case BASIC_TYPE_CHAR:
        case BASIC_TYPE_INT:
        case BASIC_TYPE_REAL:
        case BASIC_TYPE_STRING:
            if((int)node->op_type > foundreq)
            {
                foundreq = node->op_type;
            }
            break;
        case BASIC_TYPE_VARIABLE:
        case BASIC_TYPE_CLASS_VAR:
            {
                variable* var = (variable*)node->reference->to_interpret;
                if(var->i_type > foundreq) foundreq = var->i_type;
            }
            break;
        case MULTI_DIM_INDEX:    /* left: contains a variable that will be indexed. right->reference = env with multi_dimension_index:*/
            if(node->left && node->right && node->left->reference && (BASIC_TYPE_VARIABLE == node->left->reference->type || BASIC_TYPE_CLASS_VAR == node->left->reference->type) )
            {
                variable* var = (variable*)node->left->reference->to_interpret;
                if(var->i_type > foundreq) foundreq = var->i_type;
            }
            break;
        default:
            if(node->left)
            {
                populate_maximal_type(node->left, foundreq);
            }
            if(node->right)
            {
                populate_maximal_type(node->right, foundreq);
            }
            break;
        }
    }
}

bool is_logical_operation(expression_tree* node)
{
    if(node->op_type > COMP_START_IDX && node->op_type < COMP_LAST) return true;
    return false;
}

static std::vector<std::string> chop_up(const std::string& s)
{
    const char* q = s.c_str();
    std::vector<std::string> result;
    while(*q)
    {
        std::string cpart;
        while(*q && *q != ' ' && *q != ',' && *q != '(' && *q != ')' && *q != '\n')
        {
            cpart += *q;
            q++;
        }
        cpart = strim(cpart);
        if(cpart.length())
        {
            result.push_back(cpart);
        }
        if(*q) q ++;
    }

    return result;
}


void compile_a_block(variable** target_var, nap_compiler* _compiler, const std::vector<expression_tree*>& container,
                     int level,
                     call_context* cc,
                     const method* the_method, int forced_mov, bool& psuccess)
{
    std::string if_hash = cc->hash;
    std::vector<expression_tree*>::const_iterator q = container.begin();
    push_cc_start_marker(_compiler, if_hash.c_str()); /* push a marker onto the stack so that the end of the if's CC will know till where to delete*/
    int last_opcode = -1;
    while(q != container.end())
    {
        int local_req = -1;
        compile(target_var, _compiler,*q, the_method, cc, level + 1, local_req, forced_mov, psuccess);
        SUCCES_OR_RETURN;

        if((*q)->op_type != STATEMENT_CLOSE_CC)
        {
            last_opcode = (*q)->op_type;
        }
        q ++;
    }
    push_cc_end_marker(_compiler, if_hash.c_str());
    if(last_opcode == RETURN_STATEMENT)
    {
        call_context* parentccs = cc->father;
        while(parentccs)
        {
            code_stream(_compiler) << "clrsn" << parentccs->hash;
            if(parentccs && parentccs->stop_backward)
            {
                break;
            }

            parentccs = parentccs->father;
        }
        code_stream(_compiler) << "popall" ;
        code_stream(_compiler) << "leave" ;
    }
}

/**
 * Generates bytecode for the If handling.
 * At the end the instruction flow should look like:
 * condition-bytecodes
 * jlbf :if_1
 * if we have "else" branch in the if jmp if_2
 * else jmp if_end
 * if_1:
 * bytecodes for the if_branch
 * [if-have-else jmp if_end ]
 * if_2:
 * bytecodes for the else branch
 * if_end:
*/
static void resolve_if_keyword(variable** target_var, nap_compiler* _compiler,
                               const expression_tree* node,
                               const method* the_method, call_context* cc,
                               int level, int reqd_type,
                               int forced_mov, bool& psuccess)
{
    resw_if* my_if = (resw_if*)node->reference->to_interpret;

    /*check if the info is a logical operation or not.
    If it's go to the else (of the C++ if statement below, not of the if we are working on), if it's not calculate everything on the next level, mov the result in current level and
    check if it's zero or not
    */
    if(!is_logical_operation(my_if->logical_expr))    /* testing the 0- ness of a variable */
    {
        if(reqd_type == -1)
        {
            populate_maximal_type(my_if->logical_expr, reqd_type);
        }
        if(is_atomic_type(my_if->logical_expr->op_type))
        {
            mov_start_register(_compiler, reqd_type, level);
            compile(target_var, _compiler,my_if->logical_expr, the_method, cc, level, reqd_type, forced_mov, psuccess);
            SUCCES_OR_RETURN;

        }
        else    // this automatically deals with the post/pre inc/dec too
        {
            compile(target_var, _compiler,my_if->logical_expr, the_method, cc, level + 1, reqd_type, forced_mov, psuccess);
            SUCCES_OR_RETURN;

            move_register_level_register_next_level(_compiler,reqd_type, level );
        }
        cmp_register_with_zero(_compiler, reqd_type, level);
    }
    else        /* here the logical expr contains a normal logical operation or similar, this populates automatically the last boolean flag*/
    {
        compile(target_var, _compiler,my_if->logical_expr, the_method, cc, level, reqd_type, forced_mov, psuccess);    /* first step: compile the logical expression*/
        SUCCES_OR_RETURN;

    }

    if(my_if->if_branch)    /* if we have an if branch*/
    {

        std::stringstream ss;
        ss << my_if->if_branch->name << C_UNDERLINE << cc->labels.size() << C_UNDERLINE << my_if->if_branch->hash;
        std::string if_label_name = ss.str();
        ss.str(std::string());

        std::string else_label_name;

        ss << cc->name << C_UNDERLINE << cc->labels.size() << C_UNDERLINE << cc->hash;
        std::string end_label_name = ss.str();
        ss.str(std::string());

        jlbf(_compiler, if_label_name);                    /* jump to the 'if' call context if the logical expression evaluated to true */

        if(my_if->else_branch) /* if there's an else branch */
        {
            ss << my_if->else_branch->name << C_UNDERLINE << cc->labels.size() << C_UNDERLINE << my_if->else_branch->hash;
            else_label_name = ss.str();
            ss.str(std::string());
            jmp(_compiler, else_label_name);    /* jump to the else branch if the logical operation did no evaluate to true*/
        }
        else
        {
            jmp(_compiler, end_label_name);    /* if there's no else branch jump to the first operation after this is*/
        }

        code_stream(_compiler) << fully_qualified_label(if_label_name) ;            /* the label for the if branch */
        cc->add_label(-1, if_label_name);    /* for now added with dummy data to the CC*/
        if(! my_if->if_branch->expressions.empty())    /*to solve: if(1); else ...*/
        {
            compile_a_block(target_var, _compiler, my_if->if_branch->expressions, level,
                            my_if->if_branch, the_method, forced_mov, psuccess);
            SUCCES_OR_RETURN;

        }

        if(my_if->else_branch)        /* if we have an else branch */
        {
            jmp(_compiler, end_label_name);    /* make sure, that after executing the 'if' branch we don't end up here */
            code_stream(_compiler) << fully_qualified_label(else_label_name) ;    /* here place the label so the bytecode will know where to jump */
            cc->add_label(-1, else_label_name);    /* for now added with dummy data to the CC*/

            compile_a_block(target_var, _compiler, my_if->else_branch->expressions,
                            level,
                            my_if->else_branch, the_method, forced_mov, psuccess);
            SUCCES_OR_RETURN;

        }
        code_stream(_compiler) << fully_qualified_label(end_label_name) ;        /* finally, in this case this would be the last label regarding this if */
        cc->add_label(-1, end_label_name);    /* adding it to the call context */
    }
    else    /* if we don't have an if branch */
    {
        if(my_if->else_branch)    /* but we have an else branch */
        {
            /* generating a name for the end of the 'if' */
            std::stringstream ss;
            ss << cc->name.c_str() << C_UNDERLINE << (int)cc->labels.size();
            std::string s = ss.str();
            const char* end_label_name = s.c_str();

            jlbf(_compiler, end_label_name);                /* jump to the end of the if, if the logical expression evaluated to true */
            compile_a_block(target_var, _compiler, my_if->else_branch->expressions, level,
                            my_if->else_branch, the_method, forced_mov, psuccess);
            SUCCES_OR_RETURN;

            /* label after else */
            code_stream(_compiler) << fully_qualified_label(end_label_name) ;
            cc->add_label(-1, end_label_name);

        }
    }
}

/**
 * Generates bytecode for the while handling.
 * At the end the instruction flow should look like:
 * while_start:
 * condition-bytecodes
 * jnlbf :while_end
 * bytecodes for the while body
 * jmp while_start
 * while_end:
*/
static void resolve_while_keyword(variable** target_var, nap_compiler* _compiler,
                                  const expression_tree* node,
                                  const method* the_method,
                                  call_context* cc,
                                  int level, int reqd_type,
                                  int forced_mov, bool& psuccess)
{
    resw_while* my_while = (resw_while*)node->reference->to_interpret;
    /* as a first step we should print the label of the while start and end*/
    std::stringstream ss;
    ss << cc->name << C_UNDERLINE << (int)cc->labels.size();
    std::string s = ss.str();
    const char* end_label_name = s.c_str();

    my_while->break_label = my_while->operations->add_break_label(-1, end_label_name);    /* adding the default break label location of the while to the call context */

    std::stringstream ss1;
    ss << my_while->operations->name << C_UNDERLINE << (int)cc->labels.size();
    std::string s1 = ss1.str();
    const char* while_label_name = s1.c_str();

    /* print the while start label */
    code_stream(_compiler) << fully_qualified_label(while_label_name) ;
    cc->add_label(-1, while_label_name);

    /*check if the info is a logical operation or not.
    If it's go to the else (of the if below), if it's not calculate everything on the next level, mov the result in current level and
    check if it's zero or not
    */
    if(!is_logical_operation(my_while->logical_expr))    /* testing the 0- ness of a variable */
    {
        if(reqd_type == -1)
        {
            populate_maximal_type(my_while->logical_expr, reqd_type);
        }
        if(is_atomic_type(my_while->logical_expr->op_type))
        {
            mov_start_register(_compiler, reqd_type, level);
            compile(target_var, _compiler,my_while->logical_expr, the_method, cc, level, reqd_type, forced_mov, psuccess);
            SUCCES_OR_RETURN;
        }
        else    // this automatically deals with the post/pre inc/dec too
        {
            if(my_while->logical_expr->op_type == NT_VARIABLE_DEF_LST)
            {
                cc->compiler->throw_error("ERROR: You cannot declare a variable here\n", NULL);
                psuccess = false;
                return;
            }
            else
            {
                compile(target_var, _compiler,my_while->logical_expr, the_method, cc, level + 1, reqd_type, forced_mov, psuccess);
                SUCCES_OR_RETURN;

                move_register_level_register_next_level(_compiler,reqd_type, level );
            }

        }
        cmp_register_with_zero(_compiler, reqd_type, level);
    }
    else        /* here the logical expr contains a normal logical operation or similar, this populates automatically the last boolean flag*/
    {
        compile(target_var, _compiler,my_while->logical_expr, the_method, cc, level, reqd_type, forced_mov, psuccess);    /* first step: compile the logical expression*/
        SUCCES_OR_RETURN;

    }

    /* now print the jnlbf to the end of the while*/
    jnlbf(_compiler, end_label_name);

    if(my_while->operations)    /* if we have operations in the while */
    {
        compile_a_block(target_var, _compiler, my_while->operations->expressions, level, my_while->operations, the_method, forced_mov, psuccess);
        SUCCES_OR_RETURN;

        jmp(_compiler, while_label_name);
    }

    code_stream(_compiler) << fully_qualified_label(end_label_name) ;        /* finally, in this case this would be the last label regarding this while */
    cc->add_label(-1, end_label_name);    /* adding it to the call context */
}

/**
 * Generates bytecode for the for handling. for(init statement; condition; operation) {statements}
 * At the end the instruction flow should look like:
 * bytecode for variable initialization
 * for_start:
 * statement-bytecodes
 * operation bytecodes
 * evaluate condition
 * jlbf :for_start
*/
static void resolve_for_keyword(variable** target_var,nap_compiler* _compiler,
                                const expression_tree* node,
                                const method* the_method,
                                call_context* cc, int level,
                                int reqd_type, int forced_mov, bool& psuccess)
{
    resw_for* my_for = (resw_for*)node->reference->to_interpret;

    push_cc_start_marker(_compiler,my_for->unique_hash.c_str());
    /* as a first step we should compile the init statement of the for*/
    compile(target_var, _compiler,my_for->tree_init_stmt, the_method, cc, level + 1, reqd_type, forced_mov, psuccess);
    SUCCES_OR_RETURN;

    /* then we should print the label of the for start*/
    bytecode_label start_label = cc->provide_label();
    bytecode_label end_label = cc->provide_label();

    code_stream(_compiler) << fully_qualified_label(start_label.name) ;

    /* Now, the statements */
    if(my_for->operations)    /* if we have operations in the body */
    {
        my_for->operations->add_break_label(-1, end_label.name);
        compile_a_block(target_var, _compiler, my_for->operations->expressions, level, my_for->operations, the_method, forced_mov, psuccess);
        SUCCES_OR_RETURN;

    }

    /* now execute the for "operation" ie. usually the i++ */
    compile(target_var, _compiler,my_for->tree_expr, the_method, cc, level, reqd_type, forced_mov, psuccess);    /* first step: compile the logical expression*/
    SUCCES_OR_RETURN;

    /* and the condition */
    if(!is_logical_operation(my_for->tree_condition))    /* testing the 0- ness of a variable or something else that evaluates to a number */
    {
        if(reqd_type == -1)
        {
            populate_maximal_type(my_for->tree_condition, reqd_type);
        }
        if(is_atomic_type(my_for->tree_condition->op_type))
        {
            mov_start_register(_compiler,reqd_type, level);
            compile(target_var, _compiler,my_for->tree_condition, the_method, cc, level, reqd_type, forced_mov, psuccess);
            SUCCES_OR_RETURN;

        }
        else    // this automatically deals with the post/pre inc/dec too
        {
            move_register_level_register_next_level(_compiler,reqd_type, level );
        }
        cmp_register_with_zero(_compiler, reqd_type, level);
    }
    else        /* here the logical expr contains a normal logical operation or similar, this populates automatically the last boolean flag*/
    {
        compile(target_var, _compiler,my_for->tree_condition, the_method, cc, level, reqd_type, forced_mov, psuccess);    /* first step: compile the logical expression*/
        SUCCES_OR_RETURN;

    }

    // do we still need to loop?

    jlbf(_compiler,start_label.name);
    code_stream(_compiler) << fully_qualified_label(end_label.name) ;

    push_cc_end_marker(_compiler,my_for->unique_hash.c_str());
}


/**
 * Generates code for defining the variable.
 * The idea behind this is:
 *  - add the variable to the stack (by using a push operation) In fact this will add only the envelope of the variable
 *  - in case this variable has dimensions generate code to update the dimensions of the variable
 *  - in case this is initialized generate code to initialize the variable
 *
 */
void resolve_variable_definition(variable** target_var, nap_compiler* _compiler,
                                 const expression_tree* node,
                                 const method* the_method,
                                 call_context* cc, int level, int reqd_type,
                                 int forced_mov, bool& psuccess)
{
    {
        std::vector<variable_definition*>* vdlist = (std::vector<variable_definition*>*)node->reference->to_interpret;
        if(!vdlist)
        {
            return;
        }

        std::vector<variable_definition*>::const_iterator vdl = vdlist->begin();
        while(vdl != vdlist->end())
        {
            variable_definition* vd = *vdl;
            bool array_init_not_needed = false;

            if(!vd->the_variable)
            {
                cc->compiler->throw_error("Internal: A variable declaration is not having an associated variable object", NULL);
                psuccess = false;
                return;
            }

            /* warning!!! Only :
             *  1. NON static variables are being pushed.
             *  2. class variables are NOT pushed
             *  3. UserDef variables (ie. classes) are being pushed specially
             */
            if(vd->the_variable->i_type  == BASIC_TYPE_USERDEF)
            {
                /* search if we have a definition for it too, such as: Base t = new Derived()*/
                if(vd->the_value)
                {
                    code_stream(_compiler) << push() << ref() << (std::string(cc->name) + STR_DOT + vd->the_variable->name) ;

                    expression_tree* tempassign = new expression_tree(node->expwloc);

                    expression_tree* tempvar = new expression_tree(node->expwloc);
                    tempvar->left = tempvar->right = 0;
                    tempvar->op_type = BASIC_TYPE_VARIABLE;
                    tempvar->reference = new_envelope(vd->the_variable, BASIC_TYPE_VARIABLE, _compiler);

                    tempassign->left = tempvar;
                    tempassign->right = vd->the_value;
                    bool array_init_not_needed = false;
                    resolve_assignment(target_var, _compiler, tempassign, level, the_method, cc, reqd_type, forced_mov, psuccess, array_init_not_needed);
                    SUCCES_OR_RETURN;

                    // the constructor call is NOT pushing this
                    // TODO: This was here ... push_cc_end_marker();
                }
                else
                {
                    push_usertype_variable(_compiler,cc, vd->the_variable);
                }
            }
            else
            {
                push_variable(_compiler,cc, vd->the_variable);

                if(vd->the_value)
                {
                    expression_tree* tempassign = node->expwloc->new_expression();
                    expression_tree* tempvar = node->expwloc->new_expression();

                    tempvar->left = tempvar->right = 0;
                    tempvar->op_type = BASIC_TYPE_VARIABLE;
                    tempvar->reference = new_envelope(vd->the_variable, BASIC_TYPE_VARIABLE, _compiler);

                    tempassign->left = tempvar;
                    tempassign->right = vd->the_value;
                    resolve_assignment(target_var, _compiler, tempassign, level, the_method, cc, reqd_type, forced_mov, psuccess, array_init_not_needed);
                    SUCCES_OR_RETURN;
                }
            }

            if(! array_init_not_needed)
            {
                /* now check if there are multiple dimensions to this variable that were not already initialized */
                multi_dimension_def* q = NULL;
                q = vd->md_def;
                while(q && q->next)
                {
                    if(q->dimension > 0)    /* this dimension definition is a simple number */
                    {
                        call_internal_grow_with_immediate(_compiler,cc, vd->the_variable, q->dimension);
                    }
                    else
                    {
                        if(q->dynamic)
                        {
                            resolve_variable_add_dynamic_dimension(_compiler,cc, vd->the_variable);
                        }
                        else
                        {
                            if(!q->expr_def)
                            {
                                cc->compiler->throw_error("Internal: Multi-dim initialization is not having an associated expression", NULL);
                                psuccess = false;
                                return;
                            }

                            if(q->expr_def->op_type != BASIC_TYPE_INT)
                            {
                                code_stream(_compiler) << mov() << reg() << int_() << level  + 1;
                            }

                            // well.... the line above was not commented out and it shuld be like that for simple values
                            // however for complex ones array[t] needs to be there.
                            compile(target_var, _compiler,q->expr_def, the_method, cc, level + 1, reqd_type, 1, psuccess);
                            SUCCES_OR_RETURN;

                            call_internal_grow_with_register(_compiler,cc, vd->the_variable, level + 1);

                        }
                    }
                    q = q->next;
                }
            }
            vdl ++;
        }
    }
}

/**
 * This will generate code to find the location of the break label of the enclosing while/for statement and create a jump to it.
 * TODO: what to do with the stack marker ???
 */
static void resolve_break_keyword(nap_compiler* _compiler, call_context* cc, bool&psuccess)
{
    /* find the first while or for call context. Also consider switch statements*/
    call_context* qcc = cc;
    if(cc == 0)
    {
        return;
    }

    /* hold the number of stack rollbacks we need to put in here*/
    while(qcc && qcc->type != call_context::CC_WHILE && qcc->type != call_context::CC_FOR)
    {
        qcc = qcc->father;
    }
    if(!qcc)    /* means this has reached to the highest level and the break was not placed in a for/while/switch*/
    {
        cc->compiler->throw_error(E0012_SYNTAXERR);
        psuccess = false;
        return;
    }

    jmp(_compiler,qcc->break_label.name);
}

/**
 * This method compiles the given node into a series of assembly opcodes / bytecode
 */
void compile(variable** target_var, nap_compiler* _compiler, const expression_tree* node,
             const method* the_method, call_context* cc,
             int level, int& reqd_type, int forced_mov, bool& psuccess)
{
    populate_maximal_type(node, reqd_type);

    if(node == 0)
    {
        return;
    }

    if(node && node->right && node->right->op_type == FUNCTION_STRING_LEN) // this length operation can go only in an integer register
    {
        reqd_type = BASIC_TYPE_INT;
    }

    cc->compiler->set_location(node->expwloc);

    // funny check to check for empty string:
    if(node && node->info.empty() && node->op_type == BASIC_TYPE_STRING)
    {
        code_stream(_compiler) << "\"\"";
        return;
    }

    if( node && (!node->info.empty() || node->op_type == STATEMENT_NEW_CC
                 || node->op_type == STATEMENT_CLOSE_CC
                 || node->op_type == BASIC_TYPE_VARIABLE
                 || node->op_type == BASIC_TYPE_CLASS_VAR || node->op_type == ASM_BLOCK)
            )// added here to solve: int a; if(a&1) { int x = a++; } was not printing the x in the mov ...
    {
        switch(node->op_type)
        {
        case BASIC_TYPE_STRING:
            if(node->reference)
            {
                bt_string* bts = (bt_string*)node->reference->to_interpret;
                code_stream(_compiler) <<std::string("\"") + bts->m_the_string + "\"" ;
            }
            break;
        case BASIC_TYPE_INT:
            if(node->reference)
            {
                number* nr = (number*)node->reference->to_interpret;
                if(!forced_mov)
                {
                    code_stream(_compiler) << *(NUMBER_INTEGER_TYPE*)(nr->location());
                }
                else
                {
                    code_stream(_compiler) << mov()
                                  << reg() << STR_INT  << level
                                  << *(NUMBER_INTEGER_TYPE*)(nr->location());
                }
            }
            break;
        case BASIC_TYPE_REAL:
            if(node->reference)
            {
                number* nr = (number*)node->reference->to_interpret;
                code_stream(_compiler) << *(NUMBER_REAL_TYPE*)nr->location();
            }
            break;

        case BASIC_TYPE_VARIABLE:
        case BASIC_TYPE_CLASS_VAR:
        case BASIC_TYPE_EXTERN_VARIABLE:
            if(node->reference)
            {
                variable* var = (variable*)node->reference->to_interpret;
                *target_var = var;
                if(var->i_type != reqd_type && reqd_type != -2)
                {
                    /* this is a conversion operation */
                    code_stream(_compiler) << "@c"
                                  << get_reg_type(var->i_type) << get_reg_type(reqd_type)
                                  << fully_qualified_varname(cc, var);
                }
                else
                {
                    if(var->peek_index == -1)
                    {
                        std::string s = fully_qualified_varname(cc, var);
                        code_stream(_compiler)  << s ;
                    }
                    else
                    {
                        code_stream(_compiler) << "peek" << get_reg_type(var->i_type) << var->peek_index;
                    }
                }
            }
            break;

        case MULTI_DIM_INDEX:    /* left: contains a variable that will be indexed. right->reference = env with multi_dimension_index:*/
            if(node->left && node->right && node->left->reference && (BASIC_TYPE_VARIABLE == node->left->reference->type || BASIC_TYPE_CLASS_VAR == node->left->reference->type) )
            {
                int idxc = 0;
                variable* var = (variable*)node->left->reference->to_interpret;
                deliver_ccidx_dest(target_var, _compiler,node, level, the_method, cc, reqd_type, idxc, var, forced_mov, psuccess);
                SUCCES_OR_RETURN;

                code_stream(_compiler) << mov()
                              << reg() << get_reg_type(reqd_type)  << level
                              << ccidx()
                                << fully_qualified_varname(cc, var)
                                << idxc;
                clidx(_compiler);
            }
            break;

        case STATEMENT_IF:
            resolve_if_keyword(target_var, _compiler, node, the_method, cc, level, reqd_type, forced_mov, psuccess);
            SUCCES_OR_RETURN;

            break;

        case STATEMENT_WHILE:
            resolve_while_keyword(target_var, _compiler, node, the_method, cc, level, reqd_type, forced_mov, psuccess);
            SUCCES_OR_RETURN;

            break;

        case STATEMENT_FOR:
            resolve_for_keyword(target_var, _compiler, node, the_method, cc, level, reqd_type, forced_mov, psuccess);
            SUCCES_OR_RETURN;

            break;

        case STATEMENT_BREAK:
            resolve_break_keyword(_compiler,cc, psuccess);
            SUCCES_OR_RETURN;

            break;


        case COMP_EQUALEQUAL:                /* these operation will modify the lbf register (last boolean flag) */
        case COMP_NEQ:
        case COMP_LT:
        case COMP_GT:
        case COMP_LTE:
        case COMP_GTE:
        case OPERATOR_ADD:                    /* in this case the current level register will be initialized with the */
        case OPERATOR_MINUS:                /* result of the operations between left and right*/
        case OPERATOR_DIVIDE:
        case OPERATOR_MULTIPLY:
        case OPERATOR_BITWISE_AND:
        case OPERATOR_BITWISE_OR:
        case OPERATOR_BITWISE_XOR:
        case OPERATOR_MODULO:
        case OPERATOR_SHIFT_LEFT:
        case OPERATOR_SHIFT_RIGHT:
            resolve_operation(target_var, _compiler, node, reqd_type, level, the_method, cc, forced_mov, psuccess);
            SUCCES_OR_RETURN;

            break;

        case OPERATOR_UNARY_PLUS:
        case OPERATOR_BITWISE_COMP:
        case OPERATOR_NOT:
        case OPERATOR_UNARY_MINUS:
            if(node->left)
            {
                if(is_atomic_type(node->left->op_type))
                {
                    /* start a mov operation into the given register type*/
                    code_stream(_compiler) << mov()
                                  << reg() << get_reg_type(reqd_type)  << level;

                    /* compile what is after the unary operation */
                    compile(target_var, _compiler,node->left, the_method, cc, level, reqd_type, forced_mov, psuccess);
                    SUCCES_OR_RETURN;


                    /* move from the given register */
                    code_stream(_compiler)
                                  << get_opcode(node->op_type)
                                  << reg() << get_reg_type(reqd_type)  << level;
                }
                else    /* this automatically deals with the post/pre inc/dec too */
                {
                    compile(target_var, _compiler,node->left, the_method, cc, level + 1, reqd_type, forced_mov, psuccess);
                    SUCCES_OR_RETURN;

                    code_stream(_compiler) << mov()
                                  << reg() << get_reg_type(reqd_type)  << level
                                  << reg() << get_reg_type(reqd_type)  << level + 1;

                    code_stream(_compiler)
                                  << get_opcode(node->op_type)
                                  << reg() << get_reg_type(reqd_type)  << level;
                }
            }
            break;

        case OPERATOR_ASSIGN:
            {
            bool array_init_not_needed = false;
            resolve_assignment(target_var, _compiler, node, level, the_method, cc, reqd_type, forced_mov, psuccess, array_init_not_needed);
            SUCCES_OR_RETURN;

            break;
            }

        case OPERATOR_POSTINC:
        case OPERATOR_POSTDEC:
        case OPERATOR_PREINC:
        case OPERATOR_PREDEC:
            if(node->left)
            {
                if(node->left->op_type == BASIC_TYPE_VARIABLE || node->left->op_type == BASIC_TYPE_CLASS_VAR)
                {
                    deal_with_post_pre_node(target_var, _compiler, node, level, the_method, cc, forced_mov, psuccess);
                    SUCCES_OR_RETURN;

                }
                else
                if(node->left->op_type == MULTI_DIM_INDEX)
                {
                    deal_with_post_pre_node(target_var, _compiler, node, level, the_method, cc, forced_mov, psuccess);
                    SUCCES_OR_RETURN;

                }
                else
                {
                    cc->compiler->throw_error("cannot pre/post inc/dec this");
                    psuccess = false;
                    return;
                }
            }
            break;
        case OPERATOR_DIV_EQUAL:
        case OPERATOR_MUL_EQUAL:
        case OPERATOR_MINUS_EQUAL:
        case OPERATOR_PLUS_EQUAL:
        case OPERATOR_AND_EQUAL:
        case OPERATOR_OR_EQUAL:
        case OPERATOR_XOR_EQUAL:
        case OPERATOR_SHR_EQUAL:
        case OPERATOR_SHL_EQUAL:
            resolve_op_equal(target_var, _compiler, node, the_method, cc, level, reqd_type, forced_mov, psuccess);
            SUCCES_OR_RETURN;

            break;

        case NT_VARIABLE_DEF_LST:
            resolve_variable_definition(target_var, _compiler, node, the_method, cc, level, reqd_type, forced_mov, psuccess);
            SUCCES_OR_RETURN;

            return;

        case STATEMENT_CLOSE_CC:    /* in this case put a marker on the stack to show the interpreter till which point to purge the variables on the stack when closingthe CC*/
            // push_cc_end_marker();
            break;

        case STATEMENT_NEW_CC:
            {
                if(node->reference)
                {
                    call_context* new_cc = (call_context*)node->reference->to_interpret;    // Here we don't get a reference ... :(

                    std::string cc_start_hash = new_cc->hash;

                    push_cc_start_marker(_compiler,cc_start_hash.c_str());
                    new_cc->compile_standalone(_compiler, level, reqd_type, forced_mov, psuccess);
                    SUCCES_OR_RETURN;

                    push_cc_end_marker(_compiler,cc_start_hash.c_str());
                }
                break;
            }

        case OPERATOR_DOT:
            /* do something like the constructor call but with the specific thing that is on the left side and the other specific thing on the right side */
            if(node->right->op_type == FUNCTION_CALL_OF_OBJECT)
            {
                if(node->left->op_type == BASIC_TYPE_VARIABLE)
                {
                    variable* vd = (variable* )node->left->reference->to_interpret;

                    call_frame_entry* cfe = (call_frame_entry*)node->right->reference->to_interpret;
                    method* m = cfe->the_method;
                    std::vector<parameter*>::iterator ingoing_parameters = cfe->parameters.begin();
                    int pc = 0;
                    std::string dot_determiner_hash = cfe->the_method->main_cc->hash;

                    push_cc_start_marker(_compiler,dot_determiner_hash.c_str());
                    while(ingoing_parameters != cfe->parameters.end())
                    {
                        parameter* p = *ingoing_parameters;
                        parameter* fp = m->get_parameter(pc);
                        if(!fp)
                        {
                            cc->compiler->throw_error("parameter not found");
                            psuccess = false;
                            return;
                        }
                        expression_tree* t = p->expr;
                        if(t->op_type <= BASIC_TYPE_CLASS_VAR)
                        {
                            code_stream(_compiler) << mov() << reg() << get_reg_type(fp->type)  << level  ;
                        }
                        compile(target_var, _compiler,t, the_method, cc, level, fp->type, forced_mov, psuccess);
                        SUCCES_OR_RETURN;

                        code_stream(_compiler) << push() << reg() << get_reg_type(fp->type)  << level  ;

                        ingoing_parameters ++;
                        pc ++;
                    }
                    code_stream(_compiler) << call() << '$'
                                  << (std::string(cc->name) + STR_DOT + vd->name) << '@' << m->method_name ;
                    if(m->ret_type) // pop something only if the method was defined to return something
                    {
                        code_stream(_compiler) << pop() << reg() << get_reg_type(m->ret_type)  << level  ;
                        // this might also pop the stack marker. In that case leaves the stack intact
                        // this solves the problem when the function does not return anything
                        // but is required to. We might raise a runtime exception in this case
                    }
                    push_cc_end_marker(_compiler,dot_determiner_hash.c_str());
                    break;

                }
            }
            else
            if(node->right->op_type == MEMBER_ACCESS_OF_OBJECT)
            {
                variable* var_of_class = (variable*)node->right->reference->to_interpret;
                variable* the_class_var = (variable* )node->left->reference->to_interpret;
                code_stream(_compiler) << mov() << reg() << get_reg_type(var_of_class->i_type)  << level   ;
                if(var_of_class->i_type != reqd_type && reqd_type != -2)
                {
                    code_stream(_compiler) << "@" << "#c" << get_reg_type(var_of_class->i_type) << get_reg_type(reqd_type)  ;
                }
                code_stream(_compiler) << '&' << fully_qualified_varname(cc, the_class_var) << '@' << var_of_class->name;
            }
            else
            if(node->right->op_type == FUNCTION_STRING_LEN) // this can go only in an integer register
            {
                variable* vd = (variable* )node->left->reference->to_interpret;
                code_stream(_compiler) << mov()

                              << reg() << get_reg_type(BASIC_TYPE_INT)  << level

                              << "@#len"  << fully_qualified_varname(0, vd)
                              ;
            }
            break;
        case FUNCTION_CALL_OF_OBJECT:

            break;

        case FUNCTION_CALL_CONSTRUCTOR:
        {
            call_frame_entry* cfe = (call_frame_entry*)node->reference->to_interpret;
            constructor_call* m = (constructor_call*)cfe->the_method;
            std::vector<parameter*>::iterator ingoing_parameters = cfe->parameters.begin();
            int pc = 0;

            while(ingoing_parameters != cfe->parameters.end())
            {
                parameter* p = *ingoing_parameters;
                parameter* fp = m->get_parameter(pc);
                if(!fp)
                {
                    cc->compiler->throw_error("parameter not found");
                    psuccess = false;
                    return;
                }
                expression_tree* t = p->expr;
                if(t->op_type <= BASIC_TYPE_CLASS_VAR)
                {
                    code_stream(_compiler) << mov() << reg() << get_reg_type(fp->type)  << level  ;
                }
                compile(target_var, _compiler,t, the_method, cc, level, fp->type, forced_mov, psuccess);
                SUCCES_OR_RETURN;

                code_stream(_compiler) << push() << reg() << get_reg_type(fp->type)  << level  ;

                ingoing_parameters ++;
                pc ++;
            }
            code_stream(_compiler) << push() << ref() << (std::string(cc->name) + STR_DOT + "this") ;
            code_stream(_compiler) << call() << "@crea"  << m->the_class->name  << (std::string(cc->name) + STR_DOT + "this").c_str() ;
            code_stream(_compiler) << call()

                          << '$' << (std::string(cc->name) + STR_DOT + "this")
                          << '@' << (std::string(m->the_class->name) +  STR_DOT + m->method_name).c_str()
                          ;
            code_stream(_compiler) << pop() << reg() << 'g'  << level  ;

            break;
        }

        case FUNCTION_CALL:                       /* Fall through */
        case FUNCTION_CALL_NAP_PRINT:
        {

            std::map<parameter*, variable*> reference_par_target_var;
            int pc = 0;
            call_frame_entry* cfe = (call_frame_entry*)node->reference->to_interpret;
            std::string func_call_hash = generate_unique_hash();
            method* m = cfe->the_method; // the called method
            std::vector<parameter*>::const_iterator ingoing_parameters = cfe->parameters.begin();
            std::vector<parameter*>::const_iterator last = cfe->parameters.end();
            push_cc_start_marker(_compiler, func_call_hash.c_str());
            while(ingoing_parameters != last)
            {
                parameter* p = *ingoing_parameters;
                parameter* fp = m->get_parameter(pc);
                int required_compilation_type = -1;
                bool served = false;

                if(!fp && node->op_type != FUNCTION_CALL_NAP_PRINT)
                {
                    cc->compiler->throw_error("parameter not found");
                    psuccess = false;
                    return;
                }

                if(fp)
                {
                    required_compilation_type = fp->type;
                }

                expression_tree* t = p->expr;
                if(t->op_type <= BASIC_TYPE_CLASS_VAR && fp)
                {
                    if(t->op_type == BASIC_TYPE_VARIABLE)
                    {
                        variable* v = (variable*)t->reference->to_interpret;
                        // is this a multi dim def variable?
                        if(v->mult_dim_def)
                        {
                            code_stream(_compiler) << "push";
                            served = true;
                        }
                        else
                        {
                            code_stream(_compiler) << mov() << reg() << get_reg_type(fp->type) << level;
                        }
                    }
                    else
                    {
                        code_stream(_compiler) << mov() << reg() << get_reg_type(fp->type) << level;
                    }
                }
                else
                {
                    if(p->expr->op_type == BASIC_TYPE_VARIABLE)
                    {
                        variable* v = (variable*)t->reference->to_interpret;
                        code_stream(_compiler) << mov()
                               << reg() << get_reg_type(v->i_type) // this was fp->type before introducing the PRINT function
                                << level
                               ;
                        required_compilation_type = v->i_type;
                    }
                    else
                    {
                        if(p->expr->op_type < BASIC_TYPE_VARIABLE && p->expr->op_type > 0)
                        {
                            // a plain constant
                            code_stream(_compiler) << mov() << reg()
                                                   << get_reg_type(p->expr->op_type) // this was fp->type before introducing the PRINT function
                                                   << level;
                        }
                    }
                }
                variable *backup_target_var = *target_var;
                compile(const_cast<variable**>(&node->target_var), _compiler,t, the_method, cc, level, required_compilation_type, forced_mov, psuccess);
                if(node->op_type != FUNCTION_CALL_NAP_PRINT && const_cast<variable**>(&node->target_var))
                {
                    variable* v = * (const_cast<variable**>(&node->target_var));
                    if(fp && fp->is_reference)
                    {
                        reference_par_target_var.insert(std::make_pair(fp,v));
                    }
                }
                *target_var = backup_target_var;

                SUCCES_OR_RETURN;

                if(node->op_type == FUNCTION_CALL_NAP_PRINT)
                {
                    // if this is actually a hardcoded character push a %c format specifier
                    if(p->expr->op_type == BASIC_TYPE_INT && p->expr->info[0] == '\'')
                    {
                        code_stream(_compiler) << push() << "\"%c\"" ;
                        pc ++;
                    }

                    if(required_compilation_type == -1)
                    {
                        // see if this is a function call or not?
                        if(t->op_type == FUNCTION_CALL)
                        {
                            call_frame_entry* cfe2 = (call_frame_entry*)t->reference->to_interpret;
                            if(cfe2->the_method->ret_type)
                            {
                                required_compilation_type = cfe2->the_method->ret_type;
                            }
                        }
                        else
                        {
                            _compiler->throw_error("Syntax error: ", t->expwloc->expression);
                            psuccess = false;
                            return;
                        }
                    }

                    // hopefully here the required_compilation_type is not -1
                    if(!served) code_stream(_compiler) << push()
                                   << reg() << get_reg_type(required_compilation_type)
                                    << level
                                   ;
                }
                else
                {
                    if(!served) code_stream(_compiler) << push()
                                       << reg() << get_reg_type(required_compilation_type)
                                        << level
                                       ;
                }

                ingoing_parameters ++;
                pc ++;
            }
            if(node->op_type != FUNCTION_CALL_NAP_PRINT)
            {
                code_stream(_compiler) << call()
                          << '@' << std::string(m->main_cc->father->name) +'.' + m->method_name ;
            }
            else
            {
                code_stream(_compiler) << push() << pc
                          << "intr" << 1 ;
            }

            if(m->ret_type && m->ret_type != BASIC_TYPE_VOID)
            {
                // check if this method returns array, in that case use the stored value
                if(!m->ret_type_array_dim.empty() || m->dynamic_ret_type_array)
                {
                    code_stream(_compiler) << "restore" << fully_qualified_varname(node->target_var->cc, node->target_var);
                }
                else
                {
                    code_stream(_compiler) << mov()
                              << reg() << get_reg_type(m->ret_type)  << level
                              << "rv" << get_reg_type(m->ret_type) ;
                }
            }
            push_cc_end_marker(_compiler,func_call_hash.c_str());

            /* Now do some steps in order to fetch the reference type parameters from the function
             * and put them in the correct variables */
            for(std::map<parameter*, variable*>::const_iterator it = reference_par_target_var.begin(); it != reference_par_target_var.end(); ++it)
            {
                //std::cout << it->first->fully_qualified_name() << " goes into " << fully_qualified_varname(cc, it->second);
                if(it->second->mult_dim_def)
                {
                    code_stream(_compiler) << "serve" << fully_qualified_varname(cc, it->second) << it->first->fully_qualified_name();
                }
                else
                {
                    code_stream(_compiler) << mov() << reg() << get_reg_type(it->first->type) << level << it->first->fully_qualified_name();
                    code_stream(_compiler) << mov() << fully_qualified_varname(cc, it->second) << reg() << get_reg_type(it->first->type) << level;
                }
            }

            // and now do some stuff to pop the parameters of the function
            for(size_t i=0; i<m->parameters.size(); i++)
            {
                // code_stream(_compiler) << "poke" << fully_qualified_varname(m->main_cc, m->parameters[i]->name.c_str());
            }
            break;
        }

        case RETURN_STATEMENT:
            {
            if(node->reference)
            {
                expression_tree* t = (expression_tree*)node->reference->to_interpret;
                if(the_method)
                {
                    int ret_type = the_method->ret_type;
                    if(t->op_type <= BASIC_TYPE_INDEXED)
                    {
                        // does this return an array?
                        if(!the_method->ret_type_array_dim.empty() || the_method->dynamic_ret_type_array)
                        {
                            code_stream(_compiler) << "store";
                        }
                        else // just a plain variable
                        {
                            code_stream(_compiler) << mov() << reg() << get_reg_type(ret_type) << level;
                        }
                    }

                    // now if the method returns an array expect target_var to be the variable
                    compile(target_var, _compiler,t, the_method, cc, level, ret_type, forced_mov, psuccess);
                    SUCCES_OR_RETURN;
                    *target_var = 0;

                    if(!(!the_method->ret_type_array_dim.empty() || the_method->dynamic_ret_type_array))
                    {
                        code_stream(_compiler) << "return" << reg() << get_reg_type(ret_type)  << level;
                    }
                }
                else
                {
                    // returning from the main call context
                    code_stream(_compiler) << mov() << reg() << int_()  << level;
                    int rt = BASIC_TYPE_INT;
                    compile(target_var, _compiler,t, the_method, cc, level, rt, forced_mov, psuccess);
                    SUCCES_OR_RETURN;

                    code_stream(_compiler) << "exit" << reg() << int_()  << level;
                    _compiler->already_exited = true;
                }
            }
            break;
            }

        case KEYWORD_TRUE:
            code_stream(_compiler) << "true";
            break;

        case KEYWORD_FALSE:
            code_stream(_compiler) << "false";
            break;

        case ASM_BLOCK:
            {
            std::vector<std::string> asm_commands = chop_up(node->expwloc->expression);
            for(size_t i=0; i<asm_commands.size(); i++)
            {
                code_stream(_compiler) << asm_commands[i];
            }
            break;
            }

        case FUNCTION_CALL_NAP_EXEC:
        {
            // generate code to call the interrupt 2 to compile regs(0)
            // and then interrupt 3 to execute the code
            int ty = -2;
            if(is_atomic_type(node->left->op_type))
            {
                /* start a mov operation into the string register o fnext level */
                code_stream(_compiler) << mov()

                              << reg() << "string"  << level + 1
                              ;

                /* compile what is in the node */
                compile(target_var, _compiler,node->left, the_method, cc, level + 1, ty, forced_mov, psuccess);
                SUCCES_OR_RETURN;

                /* move it to reg string 0 */
                code_stream(_compiler) << mov()
                                       << reg() << "string"  << 0

                                       << reg() << "string"  << level + 1 ;

            }
            else /* compiles the node, places in the next level string register */
            {
                compile(target_var, _compiler,node->left, the_method, cc, level + 1, ty, forced_mov, psuccess);
                SUCCES_OR_RETURN;

                code_stream(_compiler) << mov()
                                       << reg() << "string"  << 0

                                       << reg() << "string"  << level + 1 ;
            }

            /* and call the required interrupts */
            code_stream(_compiler)
                                   << "intr" << 2

                                   << "intr" << 3
                                   ;
            break;
        }
        default:
            printf("Something funny:%d\n", node->op_type);
            exit(2);
        }
    }
}



#include "evaluate.h"
#include "bt_string.h"
#include "throw_error.h"
#include "type.h"
#include "parametr.h"
#include "number.h"
#include "consts.h"
#include "operations.h"
#include "indexed.h"
#include "operations.h"
#include "bsd_indx.h"
#include "notimpl.h"
#include "utils.h"
#include "res_wrds.h"
#include "code_output.h"
#include "variable.h"
#include "code_stream.h"

#include <string.h>

/* forward declarations */
void deliver_ccidx_dest(const expression_tree* node, int level, const method* the_method, call_context* cc, int reqd_type, int& idxc, const variable* var, int forced_mov);

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
    return BASIC_TYPE_CLASS_VAR == node->op_type || BASIC_TYPE_VARIABLE == node->op_type || BASIC_TYPE_INDEXED == node->op_type || MULTI_DIM_INDEX == node->op_type;
}

/**
 * Deals with the post/pre increment/decrement.
 * The result is returned in reg(level) so the callers need to be aware of this.
 * There is code generated automatically for handling the increment of the variable
 */
static void deal_with_post_pre_node(const expression_tree* pp_node, int level, const method* the_method, call_context* cc, int forced_mov)
{
    if(pp_node->left->op_type == BASIC_TYPE_VARIABLE || pp_node->left->op_type == BASIC_TYPE_CLASS_VAR)	/* a normal variable is being post/pre'd */
    {
    variable* var = (variable*)pp_node->left->reference->to_interpret;	/* this is the variable */
        if(pp_node->op_type == OPERATOR_POSTDEC || pp_node->op_type == OPERATOR_POSTINC)
        {
            mov_var_into_reg(var, level);			/* post operation: firstly "use" the original value*/
            operation_on_variable(pp_node->op_type, var);		/* then update it*/
        }
        else
        {
            operation_on_variable(pp_node->op_type, var);		/* firstly update the variable*/
            mov_var_into_reg(var, level);			/* then "use" it*/
        }
    }
    else
    if(pp_node->left->op_type == MULTI_DIM_INDEX) /* this is an array value post/pre inc/decd*/
    {
    int idxc = 0;
    variable* var = (variable*)(((envelope*)pp_node->left->left->reference)->to_interpret);
        deliver_ccidx_dest(pp_node->left, level + 1, the_method, cc, var->i_type, idxc, var, forced_mov);			/* firstly initialize the "return" value */
        if(pp_node->op_type == OPERATOR_POSTDEC || pp_node->op_type == OPERATOR_POSTINC)
        {
            mov_indexed_into_reg(var, level, idxc);			/* reg(level) goes out to the caller*/
            operation_on_indexed(pp_node->op_type, var, idxc);		/* then increment it */
        }
        else
        {
            operation_on_indexed(pp_node->op_type, var, idxc);		/* update it */
            mov_indexed_into_reg(var, level, idxc);			/* then initialize the return value (use it) in the caller*/
        }

        clear_indexes(cc);											/* and finally liberate the indexes for the next operation*/
    }
    else
    {
        throw_error("Can not post/pre increment this value");
    }
}

/**
 * Deals with the += -= /= *= etc= operations.
 */
static void resolve_op_equal(const expression_tree* node, const method* the_method, call_context* cc, int level, int reqd_type, int forced_mov)
{
    if(node->left)
    {
        if(node->left->op_type == BASIC_TYPE_VARIABLE || node->left->op_type == BASIC_TYPE_CLASS_VAR)
        {
        variable* var = (variable*)node->left->reference->to_interpret;
            if(is_atomic_type(node->right->op_type))
            {
                init_reg_with_atomic(node->left, node->right, var->i_type, level);
                compile(node->right, the_method, cc, level, var->i_type, forced_mov);	/* filled up the 'incby' */

                operation_target_var_source_reg(node->op_type, var, level);
            }
            else
            {
                if(is_post_pre_op(node->right->op_type))	/* we post/pre inc/dec with "something++" */
                {
                    deal_with_post_pre_node(node->right, level, the_method, cc, forced_mov);
                    operation_target_var_source_reg(node->op_type, var, level);
                }
                else	/* the value with which we incr/decr is a normal "complex" operation, compile it for the current level and add (sub...) it to the variable */
                {
                    compile(node->right, the_method, cc, level, var->i_type, forced_mov);
                    operation_target_var_source_reg(node->op_type, var, level);
                }
            }
        }
        else
        if(MULTI_DIM_INDEX == node->left->op_type) /* getting here with z[1] += xxx */
        {
            /* let's put in reg(level+1) what this indexed will be incremented with*/
        variable* var = (variable*)(((envelope*)node->left->left->reference)->to_interpret);
        int index;
            if(is_atomic_type(node->right->op_type))	/* getting here with z[1] += 4*/
            {
                deliver_ccidx_dest(node->left, level, the_method, cc, reqd_type, index, var, forced_mov);	/* fisrtly calculating the index since this might mess up the registers*/
                init_reg_with_atomic(node->left, node->right, var->i_type, level);
                compile(node->right, the_method, cc, level, var->i_type, forced_mov);			/* printing right's content*/
                operation_target_indexed_source_reg(node->op_type, var, index, level);		/* and finally updating the indexed value*/
                clear_indexes(cc);
            }
            else
            {
                if(is_post_pre_op(node->right->op_type))	/* we post/pre inc/dec with "something++" */
                {
                    deliver_ccidx_dest(node->left, level, the_method, cc, reqd_type, index, var, forced_mov);	/* firstly calculating the index since this might mess up the registers*/
                    deal_with_post_pre_node(node->right, level, the_method, cc, forced_mov);	/* then initializing the result of the post/pre operation */
                    operation_target_indexed_source_reg(node->op_type, var, index, level);	 /* and finally updating the indexed value*/
                    clear_indexes(cc);				/* and clearing the indexes*/
                }
                else	/* the value with which we incr/decr is a normal "complex" operation, compile it for the current level and add (sub...) it to the variable */
                {
                    compile(node->right, the_method, cc, level+1, var->i_type, forced_mov);
                    operation_target_reg_source_reg(var->i_type, level, var->i_type, level + 1);

                    deliver_ccidx_dest(node->left, level, the_method, cc, reqd_type, index, var, forced_mov);	/* fisrtly calculating the index since this might mess up the registers*/
                    operation_target_indexed_source_reg(node->op_type, var, index, level);	 /* and finally updating the indexed value*/
                    clear_indexes(cc);
                }
            }
        }
    }
    else
    {
        throw_error("Internal: op= not having a variable to increment");
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
void deliver_ccidx_dest(const expression_tree* node, int level, const method* the_method, call_context* cc, int reqd_type, int& idxc, const variable* var, int forced_mov)
{
multi_dimension_index* mdi = (multi_dimension_index*)node->right->reference->to_interpret;
expression_tree_list* indxs = mdi->dimension_values;
    idxc = 0;
    while(indxs)
    {
        if(is_atomic_type(indxs->root->op_type))	/* put the indexes into reg(current level)*/
        {
            move_atomic_into_index_register(idxc, indxs, the_method, cc, level, forced_mov);
        }
        else
        {
            if(is_post_pre_op(indxs->root->op_type))	/* put the index in the next level, modify the variable*/
            {
                deal_with_post_pre_node(indxs->root, level + 1, the_method, cc, forced_mov);	/* dealing with the post/pre operation*/
                /* and here updating the current index register to hold the value from the reg(level) */
                move_int_register_into_index_register(idxc, level);

            }
            else	/* compile everything in the next level and finally assign the next reg to current reg*/
            {
                compile(indxs->root, the_method, cc, level + 1, BASIC_TYPE_INT, forced_mov);
                move_int_register_into_index_register(idxc, level+1);
            }
        }
        idxc ++;
        indxs = indxs->next;
    }
    /* now into the current register move the variable x[reg_idxi(0), reg_idxi(1), ...] */
}



/**
 * Resolves the code generation for the operation found in the given node
 */
void resolve_operation( const expression_tree* node, int reqd_type, int level, const method* the_method, call_context* cc, int forced_mov)
{
    if(node->left)	/* the first operand is in the left node */
    {
        if(is_atomic_type(node->left->op_type))	/* the evaluation of the left will go into the current level reg*/
        {										/* and to this we'll add the evaluation of the right */
            if(node->right)						/* binary operation with two sides */
            {
                init_reg_with_atomic(node->left, node->right, reqd_type, level);
                compile(node->left, the_method, cc, level, reqd_type, forced_mov);	/* the stuff contained in the 'atomic' left branch */
                print_newline();

                if(is_atomic_type(node->right->op_type))	/* number / variable */
                {
                    operation_start_register_atomic(node, reqd_type, level);
                    compile(node->right, the_method, cc, level, reqd_type, forced_mov);	/* make the operations with the right side*/
                    print_newline();
                }
                else
                {
                    if(is_post_pre_op(node->right->op_type)) /* in case the second operand is post/pre inc/dec the handling is different*/
                    {
                        deal_with_post_pre_node(node->right, level + 1, the_method, cc, forced_mov);
                        operation_register_level_register_next_level(node, reqd_type, level);

                    }
                    else	/* this is a "normal" operation or similar */
                    {
                        compile(node->right, the_method, cc, level + 1, reqd_type, forced_mov);
                        operation_register_level_register_next_level(node, reqd_type, level);
                    }
                }
            }
            else
            {
                throw_error("Internal error in an operation");
            }
        }
        else	/* node->left is not atomic */
        {
            /* firstly initialize the left side (first operand) to the current level variable */
            if(is_post_pre_op(node->left->op_type))	/* post/pre inc*/
            {
                deal_with_post_pre_node(node->left, level, the_method, cc, forced_mov);	/* deals with the post/pre in the left node*/
            }
            else
            {
                compile(node->left, the_method, cc, level + 1, reqd_type, forced_mov);
                move_register_level_register_next_level(reqd_type, level);
            }

            if(node->right) /* this branch will put the actual operation in the operations list with reg(level)*/
            {
                if(is_atomic_type(node->right->op_type))
                {
                    operation_start_register_atomic(node, reqd_type, level);
                    compile(node->right, the_method, cc, level, reqd_type, forced_mov);
                    print_newline();
                }
                else
                {
                    if(is_post_pre_op(node->right->op_type))
                    {
                        deal_with_post_pre_node(node->right, level + 1, the_method, cc, forced_mov); /* initialize the next level register to the result of the post/pre operation*/
                        operation_register_level_register_next_level(node, reqd_type, level);
                    }
                    else
                    {
                        compile(node->right, the_method, cc, level + 1, reqd_type, forced_mov);
                        operation_register_level_register_next_level(node, reqd_type, level);
                    }
                }
            }
        }
    }
}





/**
 * Deals with code generation for the assignment
 */
void resolve_assignment( const expression_tree* node, int level, const method* the_method, call_context* cc, int reqd_type , int forced_mov)
{
    /* what will be calculated will be put all the time in the register on the current level */
    if(node->left)	/* contains the destination */
    {
        if(is_assign_destination(node->left))	/* can assign only into a variable */
        {
        variable* dest = NULL;
            if(BASIC_TYPE_VARIABLE == node->left->op_type || node->left->op_type == BASIC_TYPE_CLASS_VAR)	/* assignment goes into a variable */
            {
                dest = (variable*)(((envelope*)node->left->reference)->to_interpret);
                if(node->right)
                {
                    if(! is_post_pre_op(node->right->op_type))	/* maybe here check if the right side is logical/comparison */
                    {
                        if(node->right->op_type == LIST_VALUE)
                        {
                            do_list_assignment(node->right->reference, dest, level, the_method, cc, reqd_type);
                        }
                        else
                        {
                            if(is_atomic_type(node->right->op_type))
                            {
                                move_start_register_atomic(dest, level);
                            }
                            compile(node->right, the_method, cc, level, dest->i_type, forced_mov);
                            if(is_atomic_type(node->right->op_type))
                            {
                                print_newline();
                            }
                            move_reg_into_var(dest, level);
                        }
                    }
                    else	/* postinc/dec / preinc/dec*/
                    {
                        deal_with_post_pre_node(node->right, level, the_method, cc, forced_mov);
                        output_mov_instruction();
                        compile(node->left, the_method, cc, level, -2, forced_mov); /* this will print the "mov to dest" */
                        second_operand_register_level(dest, level);
                    }
                }
            }
            else	/* this is indexed*/
            {
                dest = (variable*)(((envelope*)node->left->left->reference)->to_interpret);
                if(node->right)
                {
                    if(! is_post_pre_op(node->right->op_type))
                    {
                        if(is_atomic_type(node->right->op_type))
                        {
                            move_start_register_atomic(dest, level);
                        }
                        compile(node->right, the_method, cc, level, dest->i_type, forced_mov);
                        if(is_atomic_type(node->right->op_type))
                        {
                            print_newline();
                        }
                    }
                    else	/* indexed = postinc/dec / preinc/dec*/
                    {
                        deal_with_post_pre_node(node->right, level, the_method, cc, forced_mov);
                        /* when the above ended, reg(level) contains the variable I need to assign */
                    }
                }
            int idxc = 0;
                deliver_ccidx_dest(node->left, level + 1, the_method, cc, reqd_type, idxc, dest, forced_mov);
                /* the above one prepares the ccidx*/
                move_register_level_into_indexe_variable(dest, idxc, level);

                clear_indexes(cc);
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
        switch(node->op_type)
        {
        case BASIC_TYPE_BOOL:
        case BASIC_TYPE_CHAR:
        case BASIC_TYPE_INT:
        case BASIC_TYPE_REAL:
        case BASIC_TYPE_STRING:
            if((int)node->op_type > foundreq) foundreq = node->op_type;
            break;
        case BASIC_TYPE_VARIABLE:
        case BASIC_TYPE_CLASS_VAR:
            {
            variable* var = (variable*)node->reference->to_interpret;
                if(var->i_type > foundreq) foundreq = var->i_type;
            }
            break;
        case MULTI_DIM_INDEX:	/* left: contains a variable that will be indexed. right->reference = env with multi_dimension_index:*/
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
static void resolve_if_keyword( const expression_tree* node, const method* the_method, call_context* cc, int level, int reqd_type, int forced_mov)
{
resw_if* my_if = (resw_if*)node->reference->to_interpret;

    /*check if the info is a logical operation or not.
    If it's go to the else (of the C++ if statement below, not of the if we are working on), if it's not calculate everything on the next level, mov the result in current level and
    check if it's zero or not
    */
    if(!is_logical_operation(my_if->logical_expr))	/* testing the 0- ness of a variable */
    {
        if(reqd_type == -1)
        {
            populate_maximal_type(my_if->logical_expr, reqd_type);
        }
        if(is_atomic_type(my_if->logical_expr->op_type))
        {
            move_start_register_atomic_with_type(reqd_type, level);
            compile(my_if->logical_expr, the_method, cc, level, reqd_type, forced_mov);
        }
        else	// this automatically deals with the post/pre inc/dec too
        {
            compile(my_if->logical_expr, the_method, cc, level + 1, reqd_type, forced_mov);
            move_register_level_register_next_level(reqd_type, level );
        }
        cmp_register_with_zero(reqd_type, level);
    }
    else		/* here the logical expr contains a normal logical operation or similar, this populates automatically the last boolean flag*/
    {
        compile(my_if->logical_expr, the_method, cc, level, reqd_type, forced_mov);	/* first step: compile the logical expression*/
    }

    /*
     - in ideal cases this should have a gt/lt ... etc operator but if it hasn't got any no problem
       later we'll check the final result of the expression and based on that we will evaluate the true-ness of the if
     - second step: add a jump based on the last boolean result populated by gt/lt/eq ... to the "if" call context
       and if there is no else branch a normal jump to the first operation after the if (xx)
     - compile the if branch
     - add a jump at the end of the "if" branch code the the first statement after this if
     - third step: if there is an else branch
     - add an uncoditional jump to the beginning of that instead of the (xx) above
     - compile the else branch code

    the solution for the labeling (in the bytecode version):
    - there's a list of labels maintained in a table associated with each call context (1, 2, 3, 4, ...)
    - each label has an associated address in the bytecode sequence of this function.
    - these entries are created in the code when required	and are saved in a special location in the output file

    - the code below automatically deals with the labeling. The jumps simply define to which label (index) they want to
      jump and the label definition defines them, thus providing the address. When executing the code, the interpreter
      will simply fetch the address from the labels

    - Latest decision: There will be a global table of labels and the jumps all wil index into that ...
    */

    if(my_if->if_branch)	/* if we have an if branch*/
    {
    char* if_label_name = alloc_mem(char, strlen(my_if->if_branch->name) + 32);
        sprintf(if_label_name, "%s_%d", my_if->if_branch->name, (int)cc->labels->size());	/* generating a name for the 'if' branch */
    char* else_label_name = NULL;
    char* end_label_name = alloc_mem(char, strlen(cc->name) + 32);
        sprintf(end_label_name, "%s_%d", cc->name, (int)cc->labels->size());	/* generating a name for the end of the 'if' */
        jlbf(if_label_name);					/* jump to the 'if' call context if the logical expression evaluated to true */

        if(my_if->else_branch) /* if there's an else branch */
        {
            else_label_name = alloc_mem(char, strlen(my_if->else_branch->name) + 32);
            sprintf(else_label_name, "%s_%d", my_if->else_branch->name, (int)cc->labels->size()); /* generating a name for the 'else' branch, will be placed in this variable */
            jmp(else_label_name);	/* jump to the else branch if the logical operation did no evaluate to true*/
        }
        else
        {
            jmp(end_label_name);	/* if there's no else branch jump to the first operation after this is*/
        }

        code_stream() << ':' << if_label_name << ':' << NEWLINE;			/* the label for the if branch */
        call_context_add_label(cc, -1, if_label_name);	/* for now added with dummy data to the CC*/
    expression_tree_list* q = my_if->if_branch->expressions;	/* and here compile the instructions in the 'if' branch*/
        if(q && !q->next)			/* one line if, no parantheses*/
        {
            compile(q->root, the_method, my_if->if_branch, level + 1, reqd_type, forced_mov);
        }
        else
        {
            if(q)	/*to solve: if(1); else ...*/
            {
                push_cc_start_marker();						/* push a marker onto the stack so that the end of the if's CC will know till where to delete*/
                while(q->next)
                {
                    compile(q->root, the_method, my_if->if_branch, level + 1, reqd_type, forced_mov);
                    q=q->next;
                }
            }
        }

        //push_cc_end_marker;
        if(my_if->else_branch)		/* if we have an else branch */
        {
            jmp(end_label_name);	/* make sure, that after executing the 'if' branch we don't end up here */
            code_stream() << ':' << else_label_name << ':' << NEWLINE;	/* here place the label so the bytecode will know where to jump */
            call_context_add_label(cc, -1, else_label_name);	/* for now added with dummy data to the CC*/
        expression_tree_list* q = my_if->else_branch->expressions;			/* and compile the instructions in the else branch too */
            if(q && !q->next)			/* one line if, no parantheses*/
            {
                compile(q->root, the_method, my_if->if_branch, level + 1, reqd_type, forced_mov);
            }
            else
            {
                push_cc_start_marker();						/* push a marker onto the stack so that the end of the if's CC will know till where to delete*/
                while(q->next)
                {
                    compile(q->root, the_method, my_if->if_branch, level + 1, reqd_type, forced_mov);
                    q=q->next;
                }
            }

        }
        code_stream() << ':' << end_label_name << ':' << NEWLINE ;		/* finally, in this case this would be the last label regarding this if */
        call_context_add_label(cc, -1, end_label_name);	/* adding it to the call context */
    }
    else	/* if we don't have an if branch */
    {
        if(my_if->else_branch)	/* but we have an else branch */
        {
            /* generating a name for the end of the 'if' */
        char* end_label_name = alloc_mem(char, strlen(cc->name) + 32);
            sprintf(end_label_name, "%s_%d", cc->name, (int)cc->labels->size());
            jlbf(end_label_name);				/* jump to the end of the if, if the logical expression evaluated to true */
        expression_tree_list* q = my_if->else_branch->expressions;			/* compile the instructions in the else branch too */
            if(q && !q->next)			/* one line if, no parantheses*/
            {
                compile(q->root, the_method, my_if->if_branch, level + 1, reqd_type, forced_mov);
            }
            else
            {
                push_cc_start_marker();						/* push a marker onto the stack so that the end of the if's CC will know till where to delete*/
                while(q->next)
                {
                    compile(q->root, the_method, my_if->if_branch, level + 1, reqd_type, forced_mov);
                    q=q->next;
                }
            }
            /* label after else */
            code_stream() << ':' << end_label_name << ':' << NEWLINE;
            call_context_add_label(cc, -1, end_label_name);

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
static void resolve_while_keyword(const expression_tree* node, const method* the_method, call_context* cc, int level, int reqd_type, int forced_mov)
{
    resw_while* my_while = (resw_while*)node->reference->to_interpret;

    /* as a first step we should print the label of the while start and end*/

    char* end_label_name = alloc_mem(char, strlen(cc->name) + 32);
    sprintf(end_label_name, "%s_%d", cc->name, (int)cc->labels->size());	/* generating a name for the end of the while */

    my_while->break_label = call_context_add_break_label(my_while->operations, -1, end_label_name);	/* adding the default break label location of the while to the call context */

    char* while_label_name = alloc_mem(char, strlen(my_while->operations->name) + 32);
    sprintf(while_label_name, "%s_%d", my_while->operations->name, (int)cc->labels->size());	/* generating a name for the content */

    /* print the while start label */
    code_stream() << ':' << while_label_name << ':' << NEWLINE ;
    call_context_add_label(cc, -1, while_label_name);

    /*check if the info is a logical operation or not.
    If it's go to the else (of the if below), if it's not calculate everything on the next level, mov the result in current level and
    check if it's zero or not
    */
    if(!is_logical_operation(my_while->logical_expr))	/* testing the 0- ness of a variable */
    {
        if(reqd_type == -1)
        {
            populate_maximal_type(my_while->logical_expr, reqd_type);
        }
        if(is_atomic_type(my_while->logical_expr->op_type))
        {
            move_start_register_atomic_with_type(reqd_type, level);
            compile(my_while->logical_expr, the_method, cc, level, reqd_type, forced_mov);
        }
        else	// this automatically deals with the post/pre inc/dec too
        {
            if(my_while->logical_expr->op_type == NT_VARIABLE_DEF_LST)
            {
                throw_error("ERROR: You cannot declare a variable here\n", NULL);
            }
            else
            {
                compile(my_while->logical_expr, the_method, cc, level + 1, reqd_type, forced_mov);
                move_register_level_register_next_level(reqd_type, level );
            }

        }
        cmp_register_with_zero(reqd_type, level);
    }
    else		/* here the logical expr contains a normal logical operation or similar, this populates automatically the last boolean flag*/
    {
        compile(my_while->logical_expr, the_method, cc, level, reqd_type, forced_mov);	/* first step: compile the logical expression*/
    }

    /* now print the jnlbf to the end of the while*/
    jnlbf(end_label_name);

    if(my_while->operations)	/* if we have operations in the while */
    {
        expression_tree_list* q = my_while->operations->expressions;	/* and here compile the instructions */
        if(q && !q->next)			/* one line while, no parantheses*/
        {
            compile(q->root, the_method, my_while->operations, level + 1, reqd_type, forced_mov);
        }
        else
        {
            if(q)	/*to resolve: while(1); trallala */
            {
                push_cc_start_marker();						/* push a marker onto the stack so that the end of the if's CC will know till where to delete*/
                while(q->next)
                {
                    compile(q->root, the_method, my_while->operations, level + 1, reqd_type, forced_mov);
                    q=q->next;
                }
            }
        }
        jmp( while_label_name);
    }

    code_stream() << ':' << end_label_name << ':' << NEWLINE ;		/* finally, in this case this would be the last label regarding this while */
    call_context_add_label(cc, -1, end_label_name);	/* adding it to the call context */
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
static void resolve_for_keyword(const expression_tree* node, const method* the_method, call_context* cc, int level, int reqd_type, int forced_mov)
{
    struct resw_for* my_for = (struct resw_for*)node->reference->to_interpret;

    /* as a first step we should compile the init statement of the for*/
    compile(my_for->tree_init_stmt, the_method, cc, level + 1, reqd_type, forced_mov);

    /* then we should print the label of the for start*/
    struct bytecode_label* start_label = call_context_provide_label(cc, 0);
    struct bytecode_label* end_label = call_context_provide_label(cc, 1);

    code_stream() << ':' << start_label->name << ':' << NEWLINE;

    /* Now, the statements */
    if(my_for->operations)	/* if we have operations in the body */
    {
        call_context_add_break_label(my_for->operations, -1, end_label->name);

        expression_tree_list* q = my_for->operations->expressions;	/* and here compile the instructions in the for body*/
        if(q && !q->next)			/* one line if, no parantheses*/
        {
            compile(q->root, the_method, my_for->operations, level + 1, reqd_type, forced_mov);
        }
        else
        {
            push_cc_start_marker();						/* push a marker onto the stack so that the end of the if's CC will know till where to delete*/
            while(q->next)
            {
                compile(q->root, the_method, my_for->operations, level + 1, reqd_type, forced_mov);
                q=q->next;
            }
        }
    }

    /* now execute the for "operation" ie. usually the i++ */
    compile(my_for->tree_expr, the_method, cc, level, reqd_type, forced_mov);	/* first step: compile the logical expression*/

    /* and the condition */
    if(!is_logical_operation(my_for->tree_condition))	/* testing the 0- ness of a variable or something else that evaluates to a number */
    {
        if(reqd_type == -1)
        {
            populate_maximal_type(my_for->tree_condition, reqd_type);
        }
        if(is_atomic_type(my_for->tree_condition->op_type))
        {
            move_start_register_atomic_with_type(reqd_type, level);
            compile(my_for->tree_condition, the_method, cc, level, reqd_type, forced_mov);
        }
        else	// this automatically deals with the post/pre inc/dec too
        {
            move_register_level_register_next_level(reqd_type, level );
        }
        cmp_register_with_zero(reqd_type, level);
    }
    else		/* here the logical expr contains a normal logical operation or similar, this populates automatically the last boolean flag*/
    {
        compile(my_for->tree_condition, the_method, cc, level, reqd_type, forced_mov);	/* first step: compile the logical expression*/
    }

    jlbf(start_label->name);
    code_stream() << ':' << end_label->name << ':' << NEWLINE ;
}


/**
 * Generates code for defining the variable.
 * The idea behind this is:
 *  - add the variable to the stack (by using a push operation) In fact this will add only the envelope of the variable
 *  - in case this variable has dimensions generate code to update the dimensions of the variable
 *  - in case this is initialized generate code to initialize the variable

 * The generated code will be something like:
 *
 */
void resolve_variable_definition(const expression_tree* node, const method* the_method, call_context* cc, int level, int reqd_type, int forced_mov)
{
    {
    variable_definition_list* vdl = (variable_definition_list*)node->reference->to_interpret;
        while(vdl)
        {
            if(vdl->the_definition)
            {
            variable_definition* vd = vdl->the_definition;
                if(!vd->the_variable)
                {
                    throw_error("Internal: A variable declaration is not having an associated variable object", NULL);
                }

                /* warning!!! Only :
                 *  1. NON static variables are being pushed.
                 *  2. class variables are NOT pushed   
                 *  3. UserDef variables (ie. classes) are being pushed specially
                 */
                if(!vd->the_variable->static_var)
                {
                    if(vd->the_variable->i_type  == BASIC_TYPE_USERDEF)
                    {
                        /* search if we have a definition for it too, such as: Base t = new Derived()*/
                        if(vd->the_value)
                        {
                            code_stream() << "pushref" << SPACE << vd->the_variable->name << NEWLINE;
                            
                            expression_tree* tempassign = alloc_mem(expression_tree, 1);
                            expression_tree* tempvar = alloc_mem(expression_tree, 1);
                            tempvar->left = tempvar->right = 0;
                            tempvar->op_type = BASIC_TYPE_VARIABLE;
                            tempvar->reference = new_envelope(vd->the_variable, BASIC_TYPE_VARIABLE);

                            tempassign->left = tempvar;
                            tempassign->right = vd->the_value;
                            resolve_assignment(tempassign, level, the_method, cc, reqd_type, forced_mov);
                            
                            // the constructor call is NOT pushing this
                            push_cc_end_marker();

                            
                        }
                        else
                        {
                            push_usertype_variable(vd->the_variable);
                        }
                    }
                    else
                    {
                        push_variable(vd->the_variable);
                        
                        if(vd->the_value)
                        {
                            expression_tree* tempassign = alloc_mem(expression_tree, 1);
                            expression_tree* tempvar = alloc_mem(expression_tree, 1);
                            tempvar->left = tempvar->right = 0;
                            tempvar->op_type = BASIC_TYPE_VARIABLE;
                            tempvar->reference = new_envelope(vd->the_variable, BASIC_TYPE_VARIABLE);

                            tempassign->left = tempvar;
                            tempassign->right = vd->the_value;
                            resolve_assignment(tempassign, level, the_method, cc, reqd_type, forced_mov);
                        }
                    }

                }
                else
                {
                    notimpl("static variables");
                }

                /* now check if there are multiple dimensions to this variable */
            multi_dimension_def* q = NULL;
                q = vd->md_def;
                while(q && q->next)
                {
                    if(q->dimension > 0)	/* this dimension definition is a simple number */
                    {
                        resolve_variable_add_dimension_number(vd->the_variable, q->dimension);
                    }
                    else
                    {
                        if(q->dynamic)
                        {
                            vd->the_variable->dynamic_dimension = 1;
                        }
                        else
                        {
                            if(!q->expr_def)
                            {
                                throw_error("Internal: Multi-dim initialization is not having an associated expression", NULL);
                            }
                            compile(q->expr_def, the_method, cc, level + 1, reqd_type, 1);
                            resolve_variable_add_dimension_regis(vd->the_variable, level + 1);

                        }
                    }
                    q = q->next;
                }
                /* and see if there is a value for this variable to assign to it */
                if(vd->the_value)
                {
                    // Do something about this shit ....
                    //envelope* def = evaluate(vd->the_value, the_method, cc);
                    //do_assignment(new_envelope(vd->the_variable, BASIC_TYPE_VARIABLE), def, -1, -1, -1, 0, the_method, cc, reqd_type, 0);
                }
                
            }
            else
            {
                throw_error("Internal: A Variable definition is not having a definition object", NULL);
            }
            vdl = vdl->next;
        }
    }
}

/**
 * This will generate code to find the location of the break label of the enclosing while/for statement and create a jump to it.
 * TODO: what to do with the stack marker ???
 */
static void resolve_break_keyword(const expression_tree* node, const method* the_method, call_context* cc, int level, int reqd_type, int forced_mov)
{
    /* find the first while or for call context. Also consider switch statements*/
    struct call_context* qcc = cc;
    /* hold the number of stack rollbacks we need to put in here*/
    int ccsc = 0;
    while(qcc && qcc->type != CALL_CONTEXT_TYPE_WHILE && qcc->type != CALL_CONTEXT_TYPE_FOR)
    {
        qcc = qcc->father;
        ccsc ++;
    }
    if(!qcc)	/* means this has reached to the highest level and the break was not placed in a for/while/switch*/
    {
        throw_error(E0012_SYNTAXERR);
    }

    for(int i=0; i< ccsc; i++)
    {
        push_cc_end_marker();
    }
    ujmp(qcc->break_label->name);
}

/**
 * This method compiles the given node into a series of assembly opcodes / bytecode
 */
void compile(const expression_tree* node, const method* the_method, call_context* cc, int level, int reqd_type, int forced_mov)
{
    if(reqd_type == -1)
    {
        populate_maximal_type(node, reqd_type);
    }
    set_location(node->expwloc);
    if( node && (node->info || node->op_type == STATEMENT_NEW_CC || node->op_type == STATEMENT_CLOSE_CC || node->op_type == BASIC_TYPE_VARIABLE || node->op_type == BASIC_TYPE_CLASS_VAR) )// added here to solve: int a; if(a&1) { int x = a++; } was not printing the x in the mov ...
    {
        switch(node->op_type)
        {
        case BASIC_TYPE_STRING:
            if(node->reference)
            {
                bt_string* bts = (bt_string*)node->reference->to_interpret;
                code_stream() <<'\"' << bts->the_string << '\"' ;
            }
            break;
        case BASIC_TYPE_INT:
            if(node->reference)
            {
            number* nr = (number*)node->reference->to_interpret;
                if(!forced_mov)
                {
                    code_stream() << *(long*)nr->location;
                }
                else
                {
                    code_stream() << "mov" << SPACE << "regi" << '(' << level << ')' << ',' << *(long*)nr->location << NEWLINE;
                }
            }
            break;
        case BASIC_TYPE_REAL:
            if(node->reference)
            {
            number* nr = (number*)node->reference->to_interpret;
                code_stream() << *(double*)nr->location;
            }
            break;

        case BASIC_TYPE_VARIABLE:
        case BASIC_TYPE_CLASS_VAR:
            if(node->reference)
            {
            variable* var = (variable*)node->reference->to_interpret;
                if(var->i_type != reqd_type && reqd_type != -2)
                {
                    code_stream() << "@c" << get_reg_type(var->i_type) << get_reg_type(reqd_type) << '(' << var->name << ')';
                }
                else
                {
                    code_stream() << var->name;
                }
            }
            break;

        case MULTI_DIM_INDEX:	/* left: contains a variable that will be indexed. right->reference = env with multi_dimension_index:*/
            if(node->left && node->right && node->left->reference && (BASIC_TYPE_VARIABLE == node->left->reference->type || BASIC_TYPE_CLASS_VAR == node->left->reference->type) )
            {
            int idxc = 0;
            variable* var = (variable*)node->left->reference->to_interpret;
                deliver_ccidx_dest(node, level, the_method, cc, reqd_type, idxc, var, forced_mov);
                code_stream() << "mov" << SPACE << "reg" << get_reg_type(reqd_type) << '(' << level << ')' << ',' <<
                                 "@ccidx" << '(' << var->name << ',' << idxc << ')' << NEWLINE;
                clear_indexes(cc);
            }
            break;

        case STATEMENT_IF:
            resolve_if_keyword(node, the_method, cc, level, reqd_type, forced_mov);
            break;

        case STATEMENT_WHILE:
            resolve_while_keyword(node, the_method, cc, level, reqd_type, forced_mov);
            break;

        case STATEMENT_FOR:
            resolve_for_keyword(node, the_method, cc, level, reqd_type, forced_mov);
            break;

        case STATEMENT_BREAK:
            resolve_break_keyword(node, the_method, cc, level, reqd_type, forced_mov);
            break;


        case COMP_EQUALEQUAL:				/* these operation will modify the lbf register (last boolean flag) */
        case COMP_NEQ:
        case COMP_LT:
        case COMP_GT:
        case COMP_LTE:
        case COMP_GTE:
        case OPERATOR_ADD:					/* in this case the current level register will be initialized with the */
        case OPERATOR_MINUS:				/* result of the operations between left and right*/
        case OPERATOR_DIVIDE:
        case OPERATOR_MULTIPLY:
        case OPERATOR_BITWISE_AND:
        case OPERATOR_BITWISE_OR:
        case OPERATOR_BITWISE_XOR:
        case OPERATOR_MODULO:
        case OPERATOR_SHIFT_LEFT:
        case OPERATOR_SHIFT_RIGHT:
            resolve_operation(node, reqd_type, level, the_method, cc, forced_mov);
            break;

        case OPERATOR_UNARY_PLUS:
        case OPERATOR_BITWISE_COMP:
        case OPERATOR_NOT:
        case OPERATOR_UNARY_MINUS:
            if(node->left)
            {
                if(is_atomic_type(node->left->op_type))
                {
                    code_stream() << "mov" << SPACE << "reg" << get_reg_type(reqd_type) << '(' << level << ')' << ',';
                    compile(node->left, the_method, cc, level, reqd_type, forced_mov);
                    code_stream() << NEWLINE << get_opcode(node->op_type) << SPACE << "reg"
                                  << get_reg_type(reqd_type) << '(' << level << ')' << NEWLINE;
                }
                else	/* this automatically deals with the post/pre inc/dec too */
                {
                    compile(node->left, the_method, cc, level + 1, reqd_type, forced_mov);
                    code_stream() << "mov" << SPACE << "reg" << get_reg_type(reqd_type) << '(' << level << ')'
                                  << ',' << "reg" << get_reg_type(reqd_type) << '(' << level + 1 << ')';
                    code_stream() << NEWLINE << get_opcode(node->op_type) << SPACE << "reg" <<
                                     get_reg_type(reqd_type) << '(' << level << ')' << NEWLINE;
                }
            }
            break;

        case OPERATOR_ASSIGN:
            resolve_assignment(node, level, the_method, cc, reqd_type, forced_mov);
            break;

        case OPERATOR_POSTINC:
        case OPERATOR_POSTDEC:
        case OPERATOR_PREINC:
        case OPERATOR_PREDEC:
            if(node->left)
            {
                if(node->left->op_type == BASIC_TYPE_VARIABLE || node->left->op_type == BASIC_TYPE_CLASS_VAR)
                {
                    deal_with_post_pre_node(node, level, the_method, cc, forced_mov);
                }
                else
                if(node->left->op_type == MULTI_DIM_INDEX)
                {
                    deal_with_post_pre_node(node, level, the_method, cc, forced_mov);
                }
                else
                {
                    throw_error("cannot pre/post inc/dec this");
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
            resolve_op_equal(node, the_method, cc, level, reqd_type, forced_mov);
            break;

        case NT_VARIABLE_DEF_LST:
            resolve_variable_definition(node, the_method, cc, level, reqd_type, forced_mov);
            return;

        case STATEMENT_CLOSE_CC:	/* in this case put a marker on the stack to show the interpreter till which point to purge the variables on the stack when closingthe CC*/
            push_cc_end_marker();
            break;

        case STATEMENT_NEW_CC:
            {
                if(node->reference)
                {
                    push_cc_start_marker();
                call_context* new_cc = (call_context*)node->reference->to_interpret;	// Here we don't get a reference ... :(
                    call_context_run_inner(new_cc, level, reqd_type, forced_mov);
                }
                break;
            }

        case FUNCTION_CALL_CONSTRUCTOR:
        {
            call_frame_entry* cfe = (call_frame_entry*)node->reference->to_interpret;
            constructor_call* m = (constructor_call*)cfe->the_method;
            parameter_list* ingoing_parameters = cfe->parameters;
            int pc = 0;
            push_cc_start_marker();
            while(ingoing_parameters)
            {
                parameter* p = ingoing_parameters->param;
                parameter* fp = method_get_parameter(m, pc);
                if(!fp)
                {
                    throw_error("parameter not found");
                }
                expression_tree* t = p->expr;
                if(t->op_type <= BASIC_TYPE_CLASS_VAR)
                {
                    code_stream() << "mov" << SPACE << "reg" << get_reg_type(fp->type) << '(' << level << ')' << ',';
                }
                compile(t, the_method, cc, level, fp->type, forced_mov);
                code_stream() << NEWLINE << "push" << SPACE << "reg" << get_reg_type(fp->type) << '(' << level << ')' << NEWLINE;

                ingoing_parameters = ingoing_parameters->next;
                pc ++;
            }
            code_stream() << "pushref" << SPACE << "this" << NEWLINE;
            code_stream() << "call" << SPACE << "@crea" << '(' << m->the_class->name << ',' << "this" << ')'<< NEWLINE;
            code_stream() << "call" << SPACE << '@' << m->the_class->name << "." << m->name << NEWLINE;
            code_stream() << "pop" << SPACE << "reg" << 'g' << '(' << level << ')' << NEWLINE;

            break;
        }

        case FUNCTION_CALL:
        {
            call_frame_entry* cfe = (call_frame_entry*)node->reference->to_interpret;
            method* m = cfe->the_method;
            parameter_list* ingoing_parameters = cfe->parameters;
            int pc = 0;
            push_cc_start_marker();
            while(ingoing_parameters)
            {
                parameter* p = ingoing_parameters->param;
                parameter* fp = method_get_parameter(m, pc);
                if(!fp)
                {
                    throw_error("parameter not found");
                }
                expression_tree* t = p->expr;
                if(t->op_type <= BASIC_TYPE_CLASS_VAR)
                {
                    code_stream() << "mov" << SPACE << "reg" << get_reg_type(fp->type) << '(' << level << ')' << ',';
                }
                compile(t, the_method, cc, level, fp->type, forced_mov);
                code_stream() << NEWLINE << "push" << SPACE << "reg" << get_reg_type(fp->type) << '(' << level << ')' << NEWLINE;

                ingoing_parameters = ingoing_parameters->next;
                pc ++;
            }
            code_stream() << "call" << SPACE << '@' << m->name << NEWLINE;
            if(m->ret_type) // pop something only if the method was defined to return something
            {
                code_stream() << "pop" << SPACE << "reg" << get_reg_type(m->ret_type) << '(' << level << ')' << NEWLINE;
                // this might also pop the stack marker. In that case leaves the stack intact
                // this solves the problem when the function does not return anything
                // but is required to. We might raise a runtime exception in this case
            }
            push_cc_end_marker();
            break;
        }

        case RETURN_STATEMENT:
            if(node->reference)
            {
                expression_tree* t = (expression_tree*)node->reference->to_interpret;
                int ret_type = the_method->ret_type;
                if(t->op_type <= BASIC_TYPE_INDEXED)
                {
                    code_stream() << "mov" << SPACE << "reg" << get_reg_type(ret_type) << '(' << level << ')' << ',';
                }
                compile(t, the_method, cc, level, ret_type, forced_mov);
                code_stream() << NEWLINE << get_opcode(node->op_type) << SPACE << "reg"
                              << get_reg_type(ret_type) << '(' << level << ')' << NEWLINE;
            }
            break;

        default:
            printf("Something funny:%d\n", node->op_type);
            break;
        }
    }
}



#include "code_output.h"
#include "consts.h"
#include "type.h"
#include "evaluate.h"
#include "variable.h"
#include "code_stream.h"
#include "utils.h"
#include "expression_tree.h"

#include <stdio.h>
#include <string>

#ifdef _WINDOWS
#pragma warning(disable:4244)
#endif

/*
 * Various methods to return strings which are used very often
 */

const char* get_reg_type(int req_type)
{
    switch(req_type)
    {
    case BASIC_TYPE_INT:
        return "int";
    case BASIC_TYPE_BYTE:
        return "byte";
    case BASIC_TYPE_REAL:
        return "real";
    case BASIC_TYPE_BOOL:
        return "bool";
    case BASIC_TYPE_CHAR:
        return "char";
    case BASIC_TYPE_STRING:
        return "string";
    }
    return "generic";
}

const char* get_opcode(int opt)
{
    switch(opt)
    {
    case OPERATOR_ADD:
    case OPERATOR_PLUS_EQUAL:
        return "add";
    case OPERATOR_ASSIGN:
        return mov();
    case OPERATOR_MINUS_EQUAL:
    case OPERATOR_MINUS:
        return "sub";
    case OPERATOR_MUL_EQUAL:
    case OPERATOR_MULTIPLY:
        return "mul";
    case OPERATOR_DIV_EQUAL:
    case OPERATOR_DIVIDE:
        return "div";
    case OPERATOR_AND_EQUAL:
    case OPERATOR_BITWISE_AND:
        return "and";
    case OPERATOR_OR_EQUAL:
    case OPERATOR_BITWISE_OR:
        return "or";
    case OPERATOR_XOR_EQUAL:
    case OPERATOR_BITWISE_XOR:
        return "xor";
    case OPERATOR_MODULO:
        return "mod";
    case OPERATOR_SHIFT_LEFT:
    case OPERATOR_SHL_EQUAL:
        return "shl";
    case OPERATOR_SHIFT_RIGHT:
    case OPERATOR_SHR_EQUAL:
        return "shr";
    case OPERATOR_UNARY_MINUS:
        return "neg";
    case OPERATOR_UNARY_PLUS:
        return "pos";
    case OPERATOR_BITWISE_COMP:
        return "bcom";
    case OPERATOR_NOT:
        return "not";
    case OPERATOR_POSTINC:
    case OPERATOR_PREINC:
        return "inc";
    case OPERATOR_POSTDEC:
    case OPERATOR_PREDEC:
        return "dec";
    case COMP_EQUALEQUAL:
        return "eq";
    case COMP_NEQ:
        return "neq";
    case COMP_LT:
        return "lt";
    case COMP_GT:
        return "gt";
    case COMP_LTE:
        return "lte";
    case COMP_GTE:
        return "gte";
    case RETURN_STATEMENT:
        return "return";
    }

    return "not implemented";
}

const char* leave()
{
    return "leave";
}

const char* pushall()
{
    return "pushall";
}

const char* popall()
{
    return "popall";
}

const char* mov()
{
    return "mov";
}

const char *int_()
{
    return "int";
}


const char* reg()
{
    return "reg";
}

const char* idx()
{
    return "idx";
}

const char* call()
{
    return "call";
}

const char* ccidx()
{
    return "@#ccidx";
}

static const char* grow()
{
    return "@#grow";
}

const char* push()
{
    return "push";
}

const char* pop()
{
    return "pop";
}

const char* ref()
{
    return "ref";
}

/*
 * Incomplete operations. These methods start an assembly operation but do not finish it, usually they are called
 * as being part of a bigger expression.
 */

void operation_start_register(nap_compiler* _compiler, const expression_tree* node, int reqd_type, int level )
{
    code_stream(_compiler)
            << get_opcode(node->op_type)

            << reg() << get_reg_type(reqd_type)  << level
            ;
}

void mov_start_register(nap_compiler* _compiler, int reqd_type, int level)
{
    code_stream(_compiler)
            << mov()
            << reg() << get_reg_type(reqd_type)  << level
            ;
}

void mov_start(nap_compiler* _compiler)
{
    code_stream(_compiler)
            << mov()
            ;
}

void second_part_target_unknown_source_register(nap_compiler* _compiler, variable* dest, int level)
{
    code_stream(_compiler)
            << reg() << get_reg_type(dest->i_type)  << level
            ;
}

/* Complete operations. These operations output full assembly code. */

void clidx(nap_compiler* _compiler)
{
    code_stream(_compiler) << "clidx" ;
}

void op_reg_reg(nap_compiler* _compiler, int op_type, char reg1_ty, int reg1_level, char reg2_ty, int reg2_level)
{
    code_stream(_compiler)
            << get_opcode(op_type)
            << reg() << get_reg_type(reg1_ty)  << reg1_level
            << reg() << get_reg_type(reg2_ty)  << reg2_level
            ;
}

void move_register_level_register_next_level(nap_compiler* _compiler,  int reqd_type, int level )
{
    code_stream(_compiler)
            << mov()
            << reg() << get_reg_type(reqd_type)  << level
            << reg() << get_reg_type(reqd_type)  << level + 1
            ;
}

void operation_register_level_register_next_level(nap_compiler* _compiler, const expression_tree* node, int reqd_type, int level )
{
    code_stream(_compiler)
            << get_opcode(node->op_type)
            << reg() << get_reg_type(reqd_type)  << level
            << reg() << get_reg_type(reqd_type)  << level + 1
            ;
}

void mov_target_register_source_var(variable** target_var, nap_compiler* _compiler, expression_tree* var_node, int reqd_type, int level,
                      const method* the_method, call_context* cc, int forced_mov, bool& psuccess)
{
    mov_start_register(_compiler, reqd_type, level);
    compile(target_var, _compiler, var_node, the_method, cc, level, reqd_type, forced_mov, psuccess);
}

void unary_operation_variable(nap_compiler* _compiler, call_context* cc, int opr, variable* var )
{
    code_stream(_compiler)
            << get_opcode(opr)
            << fully_qualified_varname(cc, var)
            ;
}

void operation_on_indexed(nap_compiler* _compiler, call_context* cc, int opr, const variable* var, int idxc )
{
    code_stream(_compiler)
            << get_opcode(opr)
            << ccidx()
                << fully_qualified_varname(cc, var)
                << idxc
            ;
}

void operation_target_var_source_reg(nap_compiler* _compiler, call_context* cc, int opr, variable* var, int level )
{
    code_stream(_compiler)
            << get_opcode(opr)
            << fully_qualified_varname(cc, var)
            << reg() << get_reg_type(var->i_type)  << level
            ;
}

void operation_target_indexed_source_reg(nap_compiler* _compiler,call_context* cc, int opr, variable* var, int index, int level )
{
    code_stream(_compiler)
            << get_opcode(opr)
            << ccidx()
                << fully_qualified_varname(cc, var)
                << index
            << reg() << get_reg_type(var->i_type)  << level
            ;
}

void cmp_register_with_zero(nap_compiler* _compiler, int reqd_type, int level )
{
    code_stream(_compiler)
            << get_opcode(COMP_NEQ)
            << reg() << get_reg_type(reqd_type) << level
            << '0'
            ;
}

void mov_target_register_source_var(variable** /*target_var*/, nap_compiler* _compiler, call_context* cc, variable* var, int level)
{
    code_stream(_compiler)
            << mov()
            << reg() << get_reg_type(var->i_type)  << level
            << fully_qualified_varname(cc, var) ;
}

void mov_target_register_source_indexed(nap_compiler* _compiler,call_context* cc, variable* var, int level, int idxc )
{
    code_stream(_compiler)
            << mov()
            << reg() << get_reg_type(var->i_type)  << level
            << ccidx()
                << fully_qualified_varname(cc, var)
                << idxc
            ;
}

void operation_target_register_source_register(nap_compiler* _compiler, int req_type_1, int level_1, int req_type_2, int level_2 )
{
    code_stream(_compiler)
            << mov()
            << reg() << get_reg_type(req_type_1)  << level_1
            << reg() << get_reg_type(req_type_2)  << level_2
            ;
}

void call_internal_grow_with_immediate(nap_compiler* _compiler,call_context* cc, variable* var, long dimension)
{
    code_stream(_compiler)
            << call()
            << grow()
                << fully_qualified_varname(cc, var)
                << dimension
            ;
}

void call_internal_grow_with_register(nap_compiler* _compiler, call_context* cc, variable* var, int level)
{
    code_stream(_compiler)
            << call()
            << grow()
                << fully_qualified_varname(cc, var)
                << reg() << int_()  << level
            ;
}

void push_cc_start_marker(nap_compiler* _compiler, const char* marker_name)
{
    if(marker_name)
    {
        code_stream(_compiler)
                << "marksn"
                << marker_name ;
    }
    else
    {
        code_stream(_compiler)
                << "marks" ;
    }
}

void push_cc_end_marker(nap_compiler* _compiler, const char* marker_name)
{
    if(marker_name)
    {
        code_stream(_compiler)
                << "clrsn"
                << marker_name;
    }
    else
    {
        code_stream(_compiler)
                << "clrs";
    }
}

void mov_target_index_register_source_atomic(variable** target_var, nap_compiler* _compiler, int& idxc,
                                     expression_tree* indxs,
                                     const method* the_method,
                                     call_context* cc, int level, int forced_mov, bool& psuccess )
{
    code_stream(_compiler)
            << mov()
            << reg() << idx()  << idxc
            ;

    int int_type = BASIC_TYPE_INT;
    compile(target_var, _compiler,indxs, the_method, cc, level, int_type, forced_mov, psuccess);
}

void mov_target_index_register_source_int_register(nap_compiler* _compiler, int& idxc, int level )
{
    code_stream(_compiler)
            << mov()
            << reg() << idx()  << idxc
            << reg() << get_reg_type(BASIC_TYPE_INT)  << level
            ;
}

void mov_target_variable_source_register(nap_compiler* _compiler,call_context* cc,  variable* dest, int level )
{
    code_stream(_compiler) << mov();

    if(dest->peek_index == -1)
    {
        std::string s = fully_qualified_varname(dest->cc ? dest->cc : cc, dest);
        code_stream(_compiler)  << s ;
    }
    else
    {
        code_stream(_compiler) << "peek" << "bp" << get_reg_type(dest->i_type) << dest->peek_index;
    }

     code_stream(_compiler) << reg() << get_reg_type(dest->i_type)  << level ;
}

void mov_target_indexed_variable_source_register(nap_compiler* _compiler, variable* dest, int idxc, int level )
{
    code_stream(_compiler)
            << mov()
            << ccidx()
                << fully_qualified_varname(0, dest)
                << idxc
            << reg() << get_reg_type(dest->i_type) <<C_PAR_OP << level
            ;
}

void push_variable(nap_compiler* _compiler,call_context* cc, variable* var)
{
    code_stream(_compiler)
            << push()
            << var->c_type
            << fully_qualified_varname(cc, var)
            ;
}

void push_usertype_variable(nap_compiler* _compiler,call_context* cc, variable* var)
{
    code_stream(_compiler) << call() << "@crea"  << var->c_type  << fully_qualified_varname(cc, var) ;
    code_stream(_compiler) << push() << "ref" << fully_qualified_varname(cc, var) ;
}

void exit_app(nap_compiler* _compiler)
{
    code_stream(_compiler) << "exit" ;
}

void peek(nap_compiler* _compiler,call_context* cc, const std::string& type, int idx, const char *dest)
{
    code_stream(_compiler) << "peek" << type
                  << idx
                 << fully_qualified_varname(cc, dest) ;
}

void jmp(nap_compiler* _compiler,const std::string& label)
{
    code_stream(_compiler) << "jmp" <<label ;
}

void jlbf(nap_compiler* _compiler,const std::string& label)
{
    code_stream(_compiler) << "jlbf" << label ;
}

void jnlbf(nap_compiler* _compiler,const char *label)
{
    code_stream(_compiler) << "jnlbf" << label ;
}


void resolve_variable_add_dynamic_dimension(nap_compiler *_compiler, call_context *cc, variable *var)
{
    code_stream(_compiler)
            << call()
            << grow()
                << fully_qualified_varname(cc, var)
                << -1
            ;
}

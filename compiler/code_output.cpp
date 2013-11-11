#include "code_output.h"
#include "consts.h"
#include "type.h"
#include "evaluate.h"
#include "tree.h"
#include "variable.h"
#include "code_stream.h"
#include "utils.h"
#include "expression_tree.h"

#include <stdio.h>
#include <string>

const char* get_reg_type(int req_type)
{
    switch(req_type)
    {
    case BASIC_TYPE_INT:
        return "int";
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

const char* mov()
{
    return "mov";
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

void clidx(nap_compiler* _compiler)
{
    code_stream(_compiler) << "clidx" << NEWLINE;
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

void op_reg_reg(nap_compiler* _compiler, int op_type, char reg1_ty, int reg1_level, char reg2_ty, int reg2_level)
{
    code_stream(_compiler) << get_opcode(op_type)
                  << SPACE
                  << reg() << get_reg_type(reg1_ty) << C_PAR_OP << reg1_level << C_PAR_CL
                  << C_COMMA
                  << reg() << get_reg_type(reg2_ty) << C_PAR_OP << reg2_level << C_PAR_CL
                  << NEWLINE;
}

void move_register_level_register_next_level(nap_compiler* _compiler,  int reqd_type, int level )
{
    code_stream(_compiler) << mov()
                  << SPACE
                  << reg() << get_reg_type(reqd_type) << C_PAR_OP << level << C_PAR_CL
                  << C_COMMA
                  << reg() << get_reg_type(reqd_type) << C_PAR_OP << level + 1 << C_PAR_CL
                  << NEWLINE;
}

void operation_register_level_register_next_level(nap_compiler* _compiler, const expression_tree* node, int reqd_type, int level )
{
    code_stream(_compiler) << get_opcode(node->op_type)
                  << SPACE
                  << reg() << get_reg_type(reqd_type) << C_PAR_OP << level << C_PAR_CL
                  << C_COMMA
                  << reg() << get_reg_type(reqd_type) << C_PAR_OP << level + 1 << C_PAR_CL
                  << NEWLINE;
}

void mov_var_into_reg(nap_compiler* _compiler, expression_tree* var_node, int reqd_type, int level,
                      const method* the_method, call_context* cc, int forced_mov)
{
    /* initialize the command output to move something into the register */
    code_stream(_compiler) << mov()
                  << SPACE
                  << reg() << get_reg_type(reqd_type) << C_PAR_OP << level << C_PAR_CL;

    /* put in the next register the variable*/
    compile(_compiler, var_node, the_method, cc, level, reqd_type, forced_mov);

    code_stream(_compiler) << NEWLINE;
}

void operation_on_variable(nap_compiler* _compiler, call_context* cc, int opr, variable* var )
{
    code_stream(_compiler) << get_opcode(opr)
                  << SPACE
                  << fully_qualified_varname(cc, var)
                  << NEWLINE;
}

void operation_on_indexed(nap_compiler* _compiler, call_context* cc, int opr, const variable* var, int idxc )
{
    code_stream(_compiler) << get_opcode(opr) << SPACE << ccidx() << C_PAR_OP << (std::string(cc->get_name()) +STR_DOT + var->name).c_str()
    << C_COMMA << idxc << C_PAR_CL << NEWLINE;
}

void operation_target_var_source_reg(nap_compiler* _compiler, call_context* cc, int opr, variable* var, int level )
{
    code_stream(_compiler) << NEWLINE << get_opcode(opr) << SPACE << (std::string(cc->get_name()) + STR_DOT  + var->name).c_str()
    << C_COMMA << reg() << get_reg_type(var->i_type) << C_PAR_OP << level << C_PAR_CL << NEWLINE;
}

void operation_target_indexed_source_reg(nap_compiler* _compiler,call_context* cc, int opr, variable* var, int index, int level )
{
    code_stream(_compiler) << NEWLINE << get_opcode(opr) << SPACE << ccidx() << C_PAR_OP<< (std::string(cc->get_name()) + STR_DOT  + var->name).c_str()
    << C_COMMA << index << C_PAR_CL << C_COMMA << reg() << get_reg_type(var->i_type) << C_PAR_OP << level << C_PAR_CL << NEWLINE;
}

void operation_start_register_atomic(nap_compiler* _compiler, const expression_tree* node, int reqd_type, int level )
{
    code_stream(_compiler) << get_opcode(node->op_type) << SPACE << reg() << get_reg_type(reqd_type) << C_PAR_OP << level << C_PAR_CL << C_COMMA;
}

void cmp_register_with_zero(nap_compiler* _compiler, int reqd_type, int level )
{
    code_stream(_compiler) << NEWLINE << "neq" << SPACE << reg() << get_reg_type(reqd_type) << C_PAR_OP<< level << C_PAR_CL << C_COMMA << '0' << NEWLINE;
}

void mov_var_into_reg(nap_compiler* _compiler, call_context* cc, variable* var, int level)
{
    code_stream(_compiler) << mov()
                  << SPACE
                  << reg() << get_reg_type(var->i_type) << C_PAR_OP << level << C_PAR_CL
                  << C_COMMA << fully_qualified_varname(cc, var) << NEWLINE;
}

void mov_indexed_into_reg(nap_compiler* _compiler,call_context* cc, variable* var, int level, int idxc )
{
    code_stream(_compiler) << mov() << SPACE << reg() << get_reg_type(var->i_type) << C_PAR_OP << level << C_PAR_CL << C_COMMA << ccidx() << C_PAR_OP << (std::string(cc->get_name()) + STR_DOT + var->name).c_str() << C_COMMA << idxc << C_PAR_CL << NEWLINE;
}

void mov_reg(nap_compiler* _compiler,int reqd_type, int level)
{
    code_stream(_compiler) <<  mov() << SPACE << reg() << get_reg_type(reqd_type) << C_PAR_OP << level << C_PAR_CL << C_COMMA;
}

void operation_target_reg_source_reg(nap_compiler* _compiler, int req_type_1, int level_1, int req_type_2, int level_2 )
{
    code_stream(_compiler) << mov() << SPACE << reg() << get_reg_type(req_type_1) << C_PAR_OP << level_1 << C_PAR_CL << reg() << get_reg_type(req_type_2) << C_PAR_OP << level_2 << C_PAR_CL << NEWLINE;
}

void resolve_variable_add_dimension_number(nap_compiler* _compiler,call_context* cc, variable* var, long dimension)
{
    code_stream(_compiler) << call() << SPACE << grow() << C_PAR_OP << fully_qualified_varname(cc, var) << C_COMMA << dimension << C_PAR_CL << NEWLINE ;
}

void resolve_variable_add_dimension_regis(nap_compiler* _compiler,call_context* cc, variable* var, int level)
{
    code_stream(_compiler) << call() << SPACE << grow() << C_PAR_OP << fully_qualified_varname(cc, var)<< C_COMMA  << reg() << "int" << C_PAR_OP << level << C_PAR_CL << C_PAR_CL << NEWLINE;
}

void push_cc_start_marker(nap_compiler* _compiler,const char* marker_name)
{
    if(marker_name)
    {
        code_stream(_compiler) << "marksn" ;
        code_stream(_compiler) << marker_name;
    }
    else
    {
        code_stream(_compiler) << "marks" ;
    }
    code_stream(_compiler) << NEWLINE;
}

void push_cc_end_marker(nap_compiler* _compiler,const char* marker_name)
{
    if(marker_name)
    {
        code_stream(_compiler) << "clrsn";
        code_stream(_compiler) << marker_name;
    }
    else
    {
        code_stream(_compiler) << "clrs";
    }
    code_stream(_compiler) << NEWLINE;

}

void move_atomic_into_index_register(nap_compiler* _compiler, int& idxc, expression_tree* indxs, const method* the_method, call_context* cc, int level, int forced_mov )
{
    code_stream(_compiler) << NEWLINE << mov() << SPACE << reg() << idx() << C_PAR_OP << idxc << C_PAR_CL << C_COMMA;
    int int_type = BASIC_TYPE_INT;
    compile(_compiler,indxs, the_method, cc, level, int_type, forced_mov);
    code_stream(_compiler) << NEWLINE;
}

void move_int_register_into_index_register(nap_compiler* _compiler, int& idxc, int level )
{
    code_stream(_compiler) << mov() << SPACE << reg() << idx() << "int" << C_PAR_OP << idxc << C_PAR_CL << C_COMMA << reg() << get_reg_type(BASIC_TYPE_INT) << C_PAR_OP<< level + 1 << C_PAR_CL << NEWLINE;
}

void move_start_register_atomic(nap_compiler* _compiler, variable* dest, int level )
{
    move_start_register_atomic_with_type(_compiler, dest->i_type, level);
}

void move_start_register_atomic_with_type(nap_compiler* _compiler, int reqd_type, int level )
{
    code_stream(_compiler) <<mov() << SPACE << reg() << get_reg_type(reqd_type) << C_PAR_OP << level << C_PAR_CL << C_COMMA;
}

void move_reg_into_var(nap_compiler* _compiler,call_context* cc,  variable* dest, int level )
{
    if(dest->cc)
    {
        code_stream(_compiler) << mov() << SPACE << fully_qualified_varname(dest->cc, dest) << C_COMMA << reg() << get_reg_type(dest->i_type) << C_PAR_OP << level << C_PAR_CL << NEWLINE;
    }
    else
    {
        code_stream(_compiler) << mov() << SPACE << fully_qualified_varname(cc, dest) << C_COMMA << reg() << get_reg_type(dest->i_type) << C_PAR_OP << level << C_PAR_CL << NEWLINE;
    }
}


void output_mov_instruction(nap_compiler* _compiler)
{
    code_stream(_compiler) << mov() << SPACE;
}

void second_operand_register_level(nap_compiler* _compiler, variable* dest, int level)
{
    code_stream(_compiler) <<C_COMMA <<reg() <<get_reg_type(dest->i_type) << C_PAR_OP << level << C_PAR_CL << NEWLINE;
}


void move_register_level_into_indexe_variable(nap_compiler* _compiler, variable* dest, int idxc, int level )
{
    code_stream(_compiler) << mov() << SPACE << ccidx() << C_PAR_OP << fully_qualified_varname(0, dest) << C_COMMA << idxc << C_PAR_CL << C_COMMA << reg() << get_reg_type(dest->i_type) <<C_PAR_OP << level << C_PAR_CL << NEWLINE ;
}

void push_variable(nap_compiler* _compiler,call_context* cc, variable* var)
{
    code_stream(_compiler) << push() << SPACE << var->c_type << SPACE << fully_qualified_varname(cc, var) << NEWLINE;
}

void push_usertype_variable(nap_compiler* _compiler,call_context* cc, variable* var)
{
    code_stream(_compiler) << call() << SPACE << "@crea" << C_PAR_OP << var->c_type << C_COMMA << fully_qualified_varname(cc, var) << C_PAR_CL<< NEWLINE;
    code_stream(_compiler) << push() << SPACE << "ref" << SPACE << fully_qualified_varname(cc, var) << NEWLINE;
}

void exit_app(nap_compiler* _compiler)
{
    code_stream(_compiler) <<"exit" << NEWLINE;
}

void peek(nap_compiler* _compiler,call_context* cc, const char *type, int idx, const char *dest)
{
    code_stream(_compiler) <<"peek" << type
                 << C_PAR_OP << idx << C_PAR_CL
                 << C_COMMA
                 << SPACE << fully_qualified_varname(cc, dest) << NEWLINE;
}

void jmp(nap_compiler* _compiler,const std::string& label)
{
    code_stream(_compiler) << "jmp" << SPACE <<label << NEWLINE;
}

void jlbf(nap_compiler* _compiler,const std::string& label)
{
    code_stream(_compiler) << "jlbf" << SPACE << label << NEWLINE;
}

void jnlbf(nap_compiler* _compiler,const char *label)
{
    code_stream(_compiler) << "jnlbf" << SPACE << label << NEWLINE;
}

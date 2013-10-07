#include "code_output.h"
#include "consts.h"
#include "type.h"
#include "evaluate.h"
#include "tree.h"
#include "variable.h"
#include "code_stream.h"
#include "utils.h"

#include <stdio.h>
#include <string>

char get_reg_type(int req_type)
{
    switch(req_type)
    {
    case BASIC_TYPE_INT:
        return 'i';
    case BASIC_TYPE_REAL:
        return 'f';
    case BASIC_TYPE_BOOL:
        return 'b';
    case BASIC_TYPE_CHAR:
        return 'c';
    case BASIC_TYPE_STRING:
        return 's';
    }
    return 'g';
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

static const char* cmp()
{
    return "cmp";
}

const char* call()
{
    return "call";
}

const char* ccidx()
{
    return "@ccidx";
}

static const char* grow()
{
    return "@grow";
}

void clidx()
{
    code_stream() << "clidx" << NEWLINE;
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

void op_reg_reg(int op_type, char reg1_ty, int reg1_level, char reg2_ty, int reg2_level)
{
    code_stream() << get_opcode(op_type)
                  << SPACE
                  << reg() << get_reg_type(reg1_ty) << C_PAR_OP << reg1_level << C_PAR_CL
                  << C_COMMA
                  << reg() << get_reg_type(reg2_ty) << C_PAR_OP << reg2_level << C_PAR_CL
                  << NEWLINE;
}

void move_register_level_register_next_level( int reqd_type, int level )
{
    code_stream() << mov()
                  << SPACE
                  << reg() << get_reg_type(reqd_type) << C_PAR_OP << level << C_PAR_CL
                  << C_COMMA
                  << reg() << get_reg_type(reqd_type) << C_PAR_OP << level + 1 << C_PAR_CL
                  << NEWLINE;
}

void operation_register_level_register_next_level( const expression_tree* node, int reqd_type, int level )
{
    code_stream() << get_opcode(node->op_type)
                  << SPACE
                  << reg() << get_reg_type(reqd_type) << C_PAR_OP << level << C_PAR_CL
                  << C_COMMA
                  << reg() << get_reg_type(reqd_type) << C_PAR_OP << level + 1 << C_PAR_CL
                  << NEWLINE;
}

void mov_var_into_reg(expression_tree* var_node, int reqd_type, int level,
                      const method* the_method, call_context* cc, int forced_mov)
{
    /* initialize the command output to move something into the register */
    code_stream() << mov()
                  << SPACE
                  << reg() << get_reg_type(reqd_type) << C_PAR_OP << level << C_PAR_CL;

    /* put in the next register the variable*/
    compile(var_node, the_method, cc, level, reqd_type, forced_mov);

    code_stream() << NEWLINE;
}

void operation_on_variable(call_context* cc, int opr, variable* var )
{
    code_stream() << get_opcode(opr) << SPACE << (std::string(cc->name) + STR_DOT  + var->name).c_str() << NEWLINE;
}

void operation_on_indexed(call_context* cc, int opr, const variable* var, int idxc )
{
    code_stream() << get_opcode(opr) << SPACE << ccidx() << C_PAR_OP << (std::string(cc->name) +STR_DOT + var->name).c_str()
    << C_COMMA << idxc << C_PAR_CL << NEWLINE;
}

void operation_target_var_source_reg(call_context* cc, int opr, variable* var, int level )
{
    code_stream() << NEWLINE << get_opcode(opr) << SPACE << (std::string(cc->name) + STR_DOT  + var->name).c_str() 
    << C_COMMA << reg() << get_reg_type(var->i_type) << C_PAR_OP << level << C_PAR_CL << NEWLINE;
}

void operation_target_indexed_source_reg(call_context* cc, int opr, variable* var, int index, int level )
{
    code_stream() << NEWLINE << get_opcode(opr) << SPACE << ccidx() << C_PAR_OP<< (std::string(cc->name) + STR_DOT  + var->name).c_str()
    << C_COMMA << index << C_PAR_CL << C_COMMA << reg() << get_reg_type(var->i_type) << C_PAR_OP << level << C_PAR_CL << NEWLINE;
}

void operation_start_register_atomic( const expression_tree* node, int reqd_type, int level )
{
    code_stream() << get_opcode(node->op_type) << SPACE << reg() << get_reg_type(reqd_type) << C_PAR_OP << level << C_PAR_CL << C_COMMA;
}

void cmp_register_with_zero( int reqd_type, int level )
{
    code_stream() << NEWLINE << cmp() << SPACE << reg() << get_reg_type(reqd_type) << C_PAR_OP<< level << C_PAR_CL << C_COMMA << '0' << NEWLINE;
}

void mov_var_into_reg(call_context* cc, variable* var, int level)
{
    code_stream() << mov() << SPACE << reg() << get_reg_type(var->i_type) << C_PAR_OP << level << C_PAR_CL << C_COMMA << (std::string(cc->name) + STR_DOT  + var->name).c_str() << NEWLINE;
}

void mov_indexed_into_reg(call_context* cc, variable* var, int level, int idxc )
{
    code_stream() << mov() << SPACE << reg() << get_reg_type(var->i_type) << C_PAR_OP << level << C_PAR_CL << C_COMMA << ccidx() << C_PAR_OP << (std::string(cc->name) + STR_DOT + var->name).c_str() << C_COMMA << idxc << C_PAR_CL << NEWLINE;
}

void mov_reg(int reqd_type, int level)
{
    code_stream() <<  mov() << SPACE << reg() << get_reg_type(reqd_type) << C_PAR_OP << level << C_PAR_CL << C_COMMA;
}

void operation_target_reg_source_reg( int req_type_1, int level_1, int req_type_2, int level_2 )
{
    code_stream() << mov() << SPACE << reg() << get_reg_type(req_type_1) << C_PAR_OP << level_1 << C_PAR_CL << reg() << get_reg_type(req_type_2) << C_PAR_OP << level_2 << C_PAR_CL << NEWLINE;
}

void resolve_variable_add_dimension_number(call_context* cc, variable* var, long dimension)
{
    code_stream() << call() << SPACE << grow() << C_PAR_OP << fully_qualified_varname(cc, var) << C_COMMA << dimension << C_PAR_CL << NEWLINE ;
}

void resolve_variable_add_dimension_regis(call_context* cc, variable* var, int level)
{
    code_stream() << call() << SPACE << grow() << C_PAR_OP << fully_qualified_varname(cc, var)<< C_COMMA  << reg() << 'i' << C_PAR_OP << level << C_PAR_CL << C_PAR_CL << NEWLINE;
}

void push_cc_start_marker()
{
    code_stream() << "marks" << NEWLINE;
}

void push_cc_end_marker()
{
    code_stream() << "clrs" << NEWLINE ;
}

void move_atomic_into_index_register( int& idxc, expression_tree_list* indxs, const method* the_method, call_context* cc, int level, int forced_mov )
{
    code_stream() << mov() << SPACE << "reg_idx" << C_PAR_OP << idxc << C_PAR_CL << C_COMMA;
    compile(indxs->root, the_method, cc, level, BASIC_TYPE_INT, forced_mov);
    code_stream() << NEWLINE;
}

void move_int_register_into_index_register( int& idxc, int level )
{
    code_stream() << mov() << SPACE << "reg_idx" << 'i' << C_PAR_OP << idxc << C_PAR_CL << C_COMMA << reg() << get_reg_type(BASIC_TYPE_INT) << C_PAR_OP<< level + 1 << C_PAR_CL << NEWLINE;
}

void move_start_register_atomic( variable* dest, int level )
{
    move_start_register_atomic_with_type(dest->i_type, level);
}

void move_start_register_atomic_with_type( int reqd_type, int level )
{
    code_stream() <<mov() << SPACE << reg() << get_reg_type(reqd_type) << C_PAR_OP << level << C_PAR_CL << C_COMMA;
}

void move_reg_into_var(call_context* cc,  variable* dest, int level )
{
    code_stream() << mov() << SPACE << fully_qualified_varname(cc, dest) << C_COMMA << reg() << get_reg_type(dest->i_type) << C_PAR_OP << level << C_PAR_CL << NEWLINE;
}


void output_mov_instruction()
{
    code_stream() << mov() << SPACE;
}

void second_operand_register_level( variable* dest, int level)
{
    code_stream() <<C_COMMA <<reg() <<get_reg_type(dest->i_type) << C_PAR_OP << level << C_PAR_CL << NEWLINE;
}


void move_register_level_into_indexe_variable( variable* dest, int idxc, int level )
{
    code_stream() << mov() << SPACE << ccidx() << C_PAR_OP << dest->name << C_COMMA << idxc << C_PAR_CL << '<' << reg() << get_reg_type(dest->i_type) <<C_PAR_OP << level << C_PAR_CL << NEWLINE ;
}

void push_variable(call_context* cc, variable* var)
{
    code_stream() << push() << SPACE << var->c_type << SPACE << fully_qualified_varname(cc, var) << NEWLINE;
}

void push_usertype_variable(call_context* cc, variable* var)
{
    code_stream() << call() << SPACE << "@crea" << C_PAR_OP << var->c_type << C_COMMA << fully_qualified_varname(cc, var) << C_PAR_CL<< NEWLINE;
    code_stream() << push() << SPACE << "ref" << SPACE << fully_qualified_varname(cc, var) << NEWLINE;
}

void exit_app()
{
    code_stream() <<"exit" << NEWLINE;
}

void peek(call_context* cc, const char *type, int idx, const char *dest)
{
    code_stream() <<"peek" << type << C_PAR_OP << idx << C_PAR_CL << C_COMMA << SPACE << fully_qualified_varname(cc, dest) << NEWLINE;
}

void jmp(const std::string& label)
{
    code_stream() << "jmp" << SPACE <<label << NEWLINE;
}

void ujmp(const std::string& label)
{
    code_stream() <<"ujmp" << SPACE << label << NEWLINE;
}

void jlbf(const std::string& label)
{
    code_stream() << "jlbf" << SPACE << label << NEWLINE;
}

void jnlbf(const char *label)
{
    code_stream() << "jnlbf" << SPACE << label << NEWLINE;
}

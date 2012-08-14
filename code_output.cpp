#include "code_output.h"
#include "consts.h"
#include "bsd_extr.h"
#include "type.h"
#include "evaluate.h"
#include "tree.h"
#include "variable.h"
#include "code_stream.h"
#include <stdio.h>

void print_newline()
{
    puts("");
}

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


const char* get_opcode(int opt, int mode)
{
    if(mode == MODE_ASM_OUTPUT)
    {
        switch(opt)
        {
        case OPERATOR_ADD:
        case OPERATOR_PLUS_EQUAL:
            return "add";
        case OPERATOR_ASSIGN:
            return "mov";
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
    }

    return "not implemented";
}


void op_reg_reg(int op_type, char reg1_ty, int reg1_level, char reg2_ty, int reg2_level, int output_mode)
{
    if(output_mode == MODE_ASM_OUTPUT)
    {
        code_stream() << get_opcode(op_type, output_mode) << SPACE << "reg" <<
                         get_reg_type(reg1_ty) << '(' << reg1_level << ')' << ',' << "reg" <<
                         get_reg_type(reg2_ty) << '(' << reg2_level << ')' << NEWLINE;
    }
    else
    {
        // TODO: Implement
    }
}

void move_register_level_register_next_level( int reqd_type, int level, int mode )
{
    if(mode == MODE_ASM_OUTPUT)
    {
        code_stream() << "mov" << SPACE << "reg" << get_reg_type(reqd_type) << '(' << level << ')' << ',' << "reg" <<
                         get_reg_type(reqd_type) << '(' << level + 1 << ')' << NEWLINE;
    }
    else
    {
        // TODO: implement
    }
}


void operation_register_level_register_next_level( const expression_tree* node, int reqd_type, int level, int mode )
{
    if(mode == MODE_ASM_OUTPUT)
    {
        code_stream() << get_opcode(node->op_type, mode)
                      << SPACE << "reg" << get_reg_type(reqd_type)
                      << '(' << level << ')' << ',' << "reg" << get_reg_type(reqd_type) << '(' << level + 1 << ')' << NEWLINE;
    }
    else
    {
        // TODO: implement
    }
}

/**
 * Puts instructions into the output flow to move a variable into a register.
 * @param var_node - is a tree node containing a variable
 * @param reqd_type - the type required
 * @param the_method - the method in which all this is happening
 * @param cc - the call context
 */
void mov_var_into_reg(expression_tree* var_node, int reqd_type, int level, const method* the_method, call_context* cc, int mode, int forced_mov)
{
    if(MODE_ASM_OUTPUT == mode)
    {
        code_stream() << "mov" << SPACE << "reg" << get_reg_type(reqd_type) << '(' << level << ')';		/* initialize */
        compile(var_node, the_method, cc, level, reqd_type, forced_mov, mode);	/* put in the next register the variable*/
        code_stream() << NEWLINE;
    }
    else
    if(MODE_BYTECODE_OUTPUT == mode)
    {
        // TODO: Implement
    }
}

/**
 * Inserts the bytecodes when making an operation on a variable (inc var, dec var, ...)
 * @param opr - is the operation that's being applied to the variable
 * @param var - is the variable which's being updated
 * @param mode - the output mode
 */
void operation_on_variable(int opr, variable* var, int mode )
{
    if(MODE_ASM_OUTPUT == mode)
    {
        code_stream() << get_opcode(opr, mode) << SPACE << var->name << NEWLINE;
    }
    else
    if(MODE_BYTECODE_OUTPUT == mode)
    {
        // TODO: Implement
    }
}


/**
 * Inserts the bytecodes when making an operation on an indexed variable (inc x[1], ...)
 * @param opr - the operation that will be made
 * @param var - the indexed variable
 * @param idxc - the number of idnexes that will be used
 * @param mode - the output mode
 */
void operation_on_indexed( int opr, const variable* var, int idxc, int mode )
{
    if(MODE_ASM_OUTPUT == mode)
    {
        code_stream() << get_opcode(opr, mode) << SPACE << "@ccidx_dest" << '(' << var->name << ',' << idxc << ')' << NEWLINE;
    }
    else
    if(MODE_BYTECODE_OUTPUT == mode)
    {
        // TODO: Implement
    }
}

/**
 * Inserts the bytecode/asm output for the operation between the variable and the register on the
 * given level, where the target is the variable.
 * @param opr - the operation to be executed
 * @param var - the variable we're working on
 * @param level - the level of the register
 * @param mode - the output mode
 */
void operation_target_var_source_reg( int opr, variable* var, int level, int mode )
{
    if(MODE_ASM_OUTPUT == mode)
    {
        code_stream() << NEWLINE << get_opcode(opr, mode) << SPACE << var->name << ',' << "reg" << get_reg_type(var->i_type) << '(' << level << ')' << NEWLINE;
    }
    else
    if(MODE_BYTECODE_OUTPUT == mode)
    {
        // TODO: Implement
    }
}

/**
 * Inserts the bytecodes (assembly commands) into the main output flow
 * @param opr - the operation
 * @param var - the variable
 * @param index - the number of idnexes to use
 * @param level - the level of the register
 * @param mode - the mode of the register
 */
void operation_target_indexed_source_reg( int opr, variable* var, int index, int level, int mode )
{
    if(MODE_ASM_OUTPUT == mode)
    {
        code_stream() << NEWLINE << get_opcode(opr, mode) << SPACE << "@ccidx" << '('<< var->name << ',' << index << ')' << ',' << "reg" << get_reg_type(var->i_type) << '(' << level << ')' << NEWLINE;
    }
    else
    if(MODE_BYTECODE_OUTPUT == mode)
    {
        // TODO: Implement
    }
}

void operation_start_register_atomic( const expression_tree* node, int reqd_type, int level, int mode )
{
    if(mode == MODE_ASM_OUTPUT)
    {
        code_stream() << get_opcode(node->op_type, mode) << SPACE << "reg" << get_reg_type(reqd_type) << '(' << level << ')';
    }
    else
    {
        // TODO: Implement
    }
}

void cmp_register_with_zero( int reqd_type, int level, int mode )
{
    if(mode == MODE_ASM_OUTPUT)
    {
        code_stream() << NEWLINE << "cmp" << SPACE << "reg" << get_reg_type(reqd_type) << '('<< level << ')' << ',' << '0' << NEWLINE;
    }
    else
    {
        // TODO: Implement
    }
}

void mov_var_into_reg(variable* var, int level, int mode)
{
    if(MODE_ASM_OUTPUT == mode)
    {
        code_stream() << "mov" << SPACE << "reg" << get_reg_type(var->i_type) << '(' << level << ')' << ',' << var->name << NEWLINE;
    }
    else
    if(MODE_BYTECODE_OUTPUT == mode)
    {
        // TODO: Implement
    }
}

/**
 * Moves an indexed into a register
 * @param var - the indexed variable
 * @param level - the level of the register
 * @param idxc - the total number of indexes to use
 * @param mode - the output mode
 */
void mov_indexed_into_reg( variable* var, int level, int idxc, int mode )
{
    if(MODE_ASM_OUTPUT == mode)
    {
        code_stream() << "mov" << SPACE << "reg" << get_reg_type(var->i_type) << '(' << level << ')' << ',' << "@ccidx" << '(' << var->name << ',' << idxc << ')' << NEWLINE;
    }
    else
    if(MODE_BYTECODE_OUTPUT == mode)
    {
        // TODO: Implement
    }
}


/**
 * Returns a specific "mov" operation for the given dest type.
 * This works like: for ex. "mov regi(0), 1" and "mov regi(0),x"
 * will generate different byte codes for the "mov" , since the bytecode interpreter
 * will not know other ways if the second operand is a normal number or a
 * number representing the location of a variable.
 */
static const char* get_mov_for_dest(int dest_type)
{
    switch(dest_type)
    {
    case BASIC_TYPE_STRING:
    case BASIC_TYPE_VARIABLE:
    case BASIC_TYPE_INT:
    case BASIC_TYPE_REAL:
    case BASIC_TYPE_CHAR:
    case BASIC_TYPE_BOOL:
    case ENVIRONMENT_VARIABLE:
    case TEMPLATED_VARIABLE:
    default:
        return "mov";
    }
}


/**
 * This based on the type of the dest node puts in the bytecode for initializing the
 * reg(level) with the dest_node. A different "mov" bytecode should be generated for different dest_nodes
 * @param updateable_node - this is the node which will be updated
 * @param dest_node - this is the node with which the updateable node will be updated
 * @param reqd_type - the regsiters of this type will be used
 * @param level - the register on this level will be updated
 * @param mode - whether we're outputting assembly or bytecode
 */
void init_reg_with_atomic(expression_tree* updateable_node, expression_tree* dest_node, int reqd_type, int level, int mode)
{
    if (MODE_ASM_OUTPUT == mode)
    {
        code_stream() <<  get_mov_for_dest(dest_node->op_type) << SPACE << "reg" << get_reg_type(reqd_type) << '(' << level << ')' << ',';
    }
    else
    if(MODE_BYTECODE_OUTPUT == mode)
    {
        // TODO: Implement
    }
}

/**
* Inserts the opcodes for an operation between two registers into the output
* @param req_type_1 - the type of the first register (int, float, etc)
* @param level_1 - the level of the first register
* @param req_type_2 - the type of the second register (int, float, etc)
* @param level_2 - the level of the second register
* @param mode - the output mode
*/
void operation_target_reg_source_reg( int req_type_1, int level_1, int req_type_2, int level_2, int mode )
{
    if(MODE_ASM_OUTPUT == mode)
    {
        code_stream() << "mov" << SPACE << "reg" << get_reg_type(req_type_1) << '(' << level_1 << ')' << "reg" << get_reg_type(req_type_2) << '(' << level_2 << ')' << NEWLINE;
    }
    else
    if(MODE_BYTECODE_OUTPUT == mode)
    {
        // TODO: Implement
    }
}


/**
* Clears the index registers, so that the next call of @ccidx will need fresh index registers
*/
void clear_indexes(call_context* cc, int mode)
{
    if(MODE_ASM_OUTPUT == mode)
    {
        code_stream() << "clidx" << NEWLINE;
    }
    else
    if(MODE_BYTECODE_OUTPUT == mode)
    {
        // TODO: Implement
    }
}


/**
 * Generates code to add a dimension to the variable, this dimension is a number
 * @param var - the variable taht will "grow"
 * @param dimension - the new dimension
 * @param mode - the output mode
 */
void resolve_variable_add_dimension_number(variable* var, long dimension, int mode)
{
    if(MODE_ASM_OUTPUT == mode)
    {
        code_stream() << "call" << SPACE << "@grow" << '(' << var->name << ',' << dimension << ')' << NEWLINE ;
    }
    else
    {
        // TODO: Implement
    }
}


/**
 * Generates code to add a dimension to the variable, this dimension is to be found in the register at the given level
 * @param var - the variable
 * @param level - the current level
 * @param mode - the output mode
 */
void resolve_variable_add_dimension_regis(variable* var, int level, int mode)
{
    if(MODE_ASM_OUTPUT == mode)
    {
        code_stream() <<"call" << SPACE << "@grow" << '(' << var->name << ','  << "reg" << 'i' << '(' << level << ')' << ')' << NEWLINE;
    }
    else
    {
        // TODO: Implement
    }
}

/**
 * Pushes a marker on the stack. In case that the CC closes everything on the stack till this point will be purged
 * and their destructors called ... when implemented
 * @param output_type - the output mode
 */
void push_cc_start_marker(int output_type)
{
    if(MODE_ASM_OUTPUT == output_type)
    {
        code_stream() << "marks" << NEWLINE;
    }
    else
    {
        // TODO: Implement
    }
}

/**
 * Clears the stack till the last marker found
 */
void push_cc_end_marker(int output_type)
{
    if(MODE_ASM_OUTPUT == output_type)
    {
        code_stream() << "clrs" << NEWLINE ;
    }
    else
    {
        // TODO: Implement
    }
}

void move_atomic_into_index_register( int& idxc, expression_tree_list* indxs, const method* the_method, call_context* cc, int level, int forced_mov, int mode )
{
    if(mode == MODE_ASM_OUTPUT)
    {
        code_stream() << "mov" << SPACE << "reg_idx" << '(' << idxc << ')' << ',';
        compile(indxs->root, the_method, cc, level, BASIC_TYPE_INT, forced_mov, mode);
        code_stream() << NEWLINE;
    }
    else
    {
        // TODO: Implement
    }
}

void move_int_register_into_index_register( int& idxc, int level, int mode )
{
    if(mode == MODE_ASM_OUTPUT)
    {
        code_stream() << "mov" << SPACE << "reg_idx" << 'i' << '(' << idxc << ')' << ',' << "reg" << get_reg_type(BASIC_TYPE_INT) << '('<< level + 1 << ')' << NEWLINE;
    }
    else
    {
        // TODO: Implement
    }
}

void move_start_register_atomic( variable* dest, int level, int mode )
{
    move_start_register_atomic_with_type(dest->i_type, level, mode);
}

void move_start_register_atomic_with_type( int reqd_type, int level, int mode )
{
    if(mode == MODE_ASM_OUTPUT)
    {
        code_stream() <<"mov" << SPACE << "reg" << get_reg_type(reqd_type) << '(' << level << ')' << ',';
    }
    else
    {
        // TODO: implement
    }
}

void move_reg_into_var( variable* dest, int level, int mode )
{
    if(mode == MODE_ASM_OUTPUT)
    {
        code_stream() << "mov" << SPACE << dest->name << ',' << "reg" << get_reg_type(dest->i_type) << '(' << level << ')' << NEWLINE;
    }
    else
    {
        // TODO: Implement
    }
}


void output_mov_instruction(int mode)
{
    if(mode == MODE_ASM_OUTPUT)
    {
        code_stream() << "mov" << SPACE;
    }
    else
    {
        // TODO: implement
    }

}

void second_operand_register_level( variable* dest, int level, int mode)
{
    if(mode == MODE_ASM_OUTPUT)
    {
        code_stream() <<',' <<"reg" <<get_reg_type(dest->i_type) << '(' << level << ')' << NEWLINE;
    }
    else
    {
        // TODO: Implement
    }
}


void move_register_level_into_indexe_variable( variable* dest, int idxc, int level )
{
    code_stream() << "mov" << SPACE << '@' << "ccidx" << '(' << dest->name << ',' << idxc << ')' << '<' << "reg" << get_reg_type(dest->i_type) <<'(' << level << ')' << NEWLINE ;
}

void push_variable(struct variable* var, int mode)
{
    if(mode == MODE_ASM_OUTPUT)
    {
        code_stream() << "push" << var->c_type << SPACE << var->name << NEWLINE;
    }
    else
    {
        // TODO: Implement
    }
}

void exit_app(int mode)
{
    if(mode == MODE_ASM_OUTPUT)
    {
        code_stream() <<"exit" << NEWLINE;
    }
    else
    {
        // TODO: Implement
    }
}

void peek(const char *type, int idx, const char *dest, int mode)
{
    if(mode == MODE_ASM_OUTPUT)
    {
        code_stream() <<"peek" << type << "(" << idx << ")" << SPACE << dest<< NEWLINE;
    }
    else
    {
        // TODO: Implement
    }
}

void jmp(const char *label, int mode)
{
    if(mode == MODE_ASM_OUTPUT)
    {
        code_stream() << "jmp" << SPACE <<label << NEWLINE;
    }
    else
    {
        // TODO: Implement
    }
}

void ujmp(const char *label, int mode)
{
    if(mode == MODE_ASM_OUTPUT)
    {
        code_stream() <<"ujmp" << SPACE << label << NEWLINE;
    }
    else
    {
        // TODO: Implement
    }
}

void jlbf(const char *label, int mode)
{
    if(mode == MODE_ASM_OUTPUT)
    {
        code_stream() << "jlbf" << SPACE << label << NEWLINE;
    }
    else
    {
        // TODO: Implement
    }
}

void jnlbf(const char *label, int mode)
{
    if(mode == MODE_ASM_OUTPUT)
    {
        code_stream() << "jnlbf" << SPACE << label << NEWLINE;
    }
    else
    {
        // TODO: Implement
    }
}

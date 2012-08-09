#ifndef _CODE_OUTPUT_H_
#define _CODE_OUTPUT_H_

struct expression_tree;
struct method;
struct variable;
struct call_context;
struct expression_tree_list;

const char* get_opcode(int opt, int mode);
void print_newline();
void op_reg_reg(int op_type, char reg1_ty, int reg1_level, char reg2_ty, int reg2_level, int output_mode);
void operation_start_register_atomic( const expression_tree* node, int reqd_type, int level, int mode );
void operation_register_level_register_next_level( const expression_tree* node, int reqd_type, int level, int mode );
void move_register_level_register_next_level( int reqd_type, int level, int mode );
void purge_till_marker(int output_type);
void move_atomic_into_index_register( int& idxc, expression_tree_list* indxs, const method* the_method, call_context* cc, int level, int forced_mov, int mode );
void move_int_register_into_index_register( int& idxc, int level, int mode );
void move_start_register_atomic( variable* dest, int level, int mode );
void move_reg_into_var( variable* dest, int level, int mode );
void output_mov_instruction(int mode);
void second_operand_register_level( variable* dest, int level, int mode);
void move_register_level_into_indexe_variable( variable* dest, int idxc, int level );
void move_start_register_atomic_with_type( int reqd_type, int level, int mode );
void move_register_level_register_next_level( int reqd_type, int level, int mode );
void cmp_register_with_zero( int reqd_type, int level, int mode );

/**
 * Generates code for indicating the executor to push a variable on the stack (meaning: create the variable)
 */
void push_variable(struct variable* var, int mode);

/**
 * Puts instructions into the output flow to move a variable into a register.
 * @param var_node - is a tree node containing a variable
 * @param reqd_type - the type required 
 * @param the_method - the method in which all this is happening
 * @param cc - the call context
 */
void mov_var_into_reg(expression_tree* var_node, int reqd_type, int level, const method* the_method, call_context* cc, int mode, int forced_mov);

/**
 * Inserts the bytecodes when making an operation on a variable (inc var, dec var, ...)
 * @param opr - is the operation that's being applied to the variable
 * @param var - is the variable which's being updated
 * @param mode - the output mode
 */
void operation_on_variable(int opr, variable* var, int mode );

/**
 * Inserts the bytecodes when making an operation on an indexed variable (inc x[1], ...)
 * @param opr - the operation that will be made
 * @param var - the indexed variable
 * @param idxc - the number of idnexes that will be used
 * @param mode - the output mode 
 */
void operation_on_indexed( int opr, const variable* var, int idxc, int mode );

/**
 * Inserts the bytecode/asm output for the operation between the variable and the register on the 
 * given level, where the target is the variable.
 * @param opr - the operation to be executed
 * @param var - the variable we're working on
 * @param level - the level of the register
 * @param mode - the output mode
 */
void operation_target_var_source_reg( int opr, variable* var, int level, int mode );

/**
 * Inserts the bytecodes (assembly commands) into the main output flow
 * @param opr - the operation
 * @param var - the variable
 * @param index - the number of idnexes to use
 * @param level - the level of the register
 * @param mode - the mode of the register
 */
void operation_target_indexed_source_reg( int opr, variable* var, int index, int level, int mode );

/**
 * Puts instructions to move a variable into a register into the main output flow
 * @param var - the variable that will be moved into the register
 * @param level - the level of the register
 * @param mode - whether the output is a simple printf or a bytecode stream
 */
void mov_var_into_reg(variable* var, int level, int mode);

/**
 * Moves an indexed into a register
 * @param var - the indexed variable 
 * @param level - the level of the register
 * @param idxc - the total number of indexes to use
 * @param mode - the output mode
 */
void mov_indexed_into_reg( variable* var, int level, int idxc, int mode );

/**
 * This based on the type of the dest node puts in the bytecode for initializing the 
 * reg(level) with the dest_node. A different "mov" bytecode should be generated for different dest_nodes
 * @param updateable_node - this is the node which will be updated
 * @param dest_node - this is the node with which the updateable node will be updated
 * @param reqd_type - the regsiters of this type will be used
 * @param level - the register on this level will be updated
 * @param mode - whether we're outputting assembly or bytecode
 */
void init_reg_with_atomic(expression_tree* updateable_node, expression_tree* dest_node, int reqd_type, int level, int mode);

/**
* Inserts the opcodes for an operation between two registers into the output
* @param req_type_1 - the type of the first register (int, float, etc)
* @param level_1 - the level of the first register
* @param req_type_2 - the type of the second register (int, float, etc)
* @param level_2 - the level of the second register
* @param mode - the output mode
*/
void operation_target_reg_source_reg( int req_type_1, int level_1, int req_type_2, int level_2, int mode );

/**
* Clears the index registers, so that the next call of @ccidx will need fresh index registers
*/
void clear_indexes(call_context* cc, int mode);

/**
* Generates code to add a dimension to the variable, this dimension is a number
* @param var - the variable taht will "grow"
* @param dimension - the new dimension
* @param mode - the output mode
*/
void resolve_variable_add_dimension_number(variable* var, long dimension, int mode);

/**
 * Generates code to add a dimension to the variable, this dimension is to be found in the register at the given level
 * @param var - the variable
 * @param level - the current level
 * @param mode - the output mode
 */
void resolve_variable_add_dimension_regis(variable* var, int level, int mode);

/**
 * Pushes a marker on the stack. In case that the CC closes everything on the stack till this point will be purged
 * and their destructors called ... when implemented
 * @param output_type - the output mode
 */
void push_cc_start_marker(int output_type);

/**
 * Clears the stack till the last marker found
 */
void push_cc_end_marker(int output_type);

void exit_app(int mode);

void peek(const char* type, int idx, const char* dest, int mode);




#endif

#ifndef _CODE_OUTPUT_H_
#define _CODE_OUTPUT_H_

struct expression_tree;
struct method;
struct variable;
struct call_context;
struct expression_tree_list;

const char* get_opcode(int opt);
void print_newline();
void op_reg_reg(int op_type, char reg1_ty, int reg1_level, char reg2_ty, int reg2_level);
void operation_start_register_atomic( const expression_tree* node, int reqd_type, int level );
void operation_register_level_register_next_level( const expression_tree* node, int reqd_type, int level );
void move_register_level_register_next_level( int reqd_type, int level );

void move_atomic_into_index_register( int& idxc, expression_tree_list* indxs, const method* the_method, call_context* cc, int level, int forced_mov );
void move_int_register_into_index_register( int& idxc, int level );
void move_start_register_atomic( variable* dest, int level );
void move_reg_into_var( call_context* cc, variable* dest, int level );
void output_mov_instruction();
void second_operand_register_level( variable* dest, int level);
void move_register_level_into_indexe_variable( variable* dest, int idxc, int level );
void move_start_register_atomic_with_type( int reqd_type, int level );
void move_register_level_register_next_level( int reqd_type, int level );
void cmp_register_with_zero( int reqd_type, int level );
void push_variable(call_context* cc, struct variable* var);
void push_usertype_variable(call_context* cc, struct variable* var);
void mov_var_into_reg(expression_tree* var_node, int reqd_type, int level, const method* the_method, call_context* cc, int forced_mov);
void operation_on_variable(call_context* cc, int opr, variable* var );
void operation_on_indexed( call_context* cc, int opr, const variable* var, int idxc );
void operation_target_var_source_reg( call_context* cc, int opr, variable* var, int level );
void operation_target_indexed_source_reg( call_context* cc, int opr, variable* var, int index, int level );
void mov_var_into_reg(call_context* cc, variable* var, int level);
void mov_indexed_into_reg( call_context* cc, variable* var, int level, int idxc );
void init_reg_with_atomic(expression_tree* updateable_node, expression_tree* dest_node, int reqd_type, int level);
void operation_target_reg_source_reg( int req_type_1, int level_1, int req_type_2, int level_2 );
void clear_indexes(call_context* cc);
void resolve_variable_add_dimension_number(call_context* cc, variable* var, long int dimension);
void resolve_variable_add_dimension_regis(call_context* cc, variable* var, int level);
void push_cc_start_marker();
void push_cc_end_marker();
void exit_app();
void peek(call_context* cc, const char* type, int idx, const char* dest);
void jmp(const char* label);
void ujmp(const char* label);
void jlbf(const char* label);
void jnlbf(const char* label);
void mov_number_into_reg();

#endif

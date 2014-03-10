#ifndef _CODE_OUTPUT_H_
#define _CODE_OUTPUT_H_

#include <string>

struct expression_tree;
struct method;
struct variable;
struct call_context;
struct expression_tree_list;
class nap_compiler;

// TODO: These easiyl can go into a compiler helper class or similar ...


const char* get_opcode(int opt);
void op_reg_reg(nap_compiler *_compiler, int op_type, char reg1_ty, int reg1_level, char reg2_ty, int reg2_level);
void operation_start_register_atomic(nap_compiler *_compiler, const expression_tree* node, int reqd_type, int level );
void operation_register_level_register_next_level(nap_compiler *_compiler, const expression_tree* node, int reqd_type, int level );
void move_register_level_register_next_level( int reqd_type, int level );
void move_atomic_into_index_register(nap_compiler *_compiler, int& idxc, expression_tree *indxs, const method* the_method, call_context* cc, int level, int forced_mov , bool &psuccess);
void move_int_register_into_index_register(nap_compiler *_compiler, int& idxc, int level );
void move_start_register_atomic(nap_compiler* _compiler,variable* dest, int level );
void move_reg_into_var(nap_compiler *_compiler, call_context* cc, variable* dest, int level );
void output_mov_instruction(nap_compiler *_compiler);
void second_operand_register_level(nap_compiler *_compiler, variable* dest, int level);
void move_register_level_into_indexe_variable(nap_compiler *_compiler, variable* dest, int idxc, int level );
void move_start_register_atomic_with_type(nap_compiler *_compiler, int reqd_type, int level );
void move_register_level_register_next_level(nap_compiler* _compiler, int reqd_type, int level );
void cmp_register_with_zero(nap_compiler *_compiler, int reqd_type, int level );
void push_variable(nap_compiler *_compiler, call_context* cc, variable* var);
void push_usertype_variable(nap_compiler *_compiler, call_context* cc, variable* var);
void mov_var_into_reg(nap_compiler *_compiler, expression_tree* var_node, int reqd_type, int level, const method* the_method, call_context* cc, int forced_mov, bool &psuccess);
void operation_on_variable(nap_compiler* _compiler,call_context* cc, int opr, variable* var );
void operation_on_indexed(nap_compiler *_compiler, call_context* cc, int opr, const variable* var, int idxc );
void operation_target_var_source_reg(nap_compiler *_compiler, call_context* cc, int opr, variable* var, int level );
void operation_target_indexed_source_reg(nap_compiler *_compiler, call_context* cc, int opr, variable* var, int index, int level );
void mov_var_into_reg(nap_compiler *_compiler, call_context* cc, variable* var, int level);
void mov_indexed_into_reg(nap_compiler *_compiler, call_context* cc, variable* var, int level, int idxc );
void mov_reg(nap_compiler *_compiler, int reqd_type, int level);
void operation_target_reg_source_reg(nap_compiler *_compiler, int req_type_1, int level_1, int req_type_2, int level_2 );
void clidx(nap_compiler *_compiler);
void resolve_variable_add_dimension_number(nap_compiler *_compiler, call_context* cc, variable* var, long int dimension);
void resolve_variable_add_dynamic_dimension(nap_compiler *_compiler, call_context* cc, variable* var);
void resolve_variable_add_dimension_regis(nap_compiler *_compiler, call_context* cc, variable* var, int level);
void push_cc_start_marker(nap_compiler *_compiler, const char *marker_name);
void push_cc_end_marker(nap_compiler *_compiler, const char* marker_name);
void exit_app(nap_compiler *_compiler);
void peek(nap_compiler *_compiler, call_context* cc, const std::string &type, int idx, const char* dest);
void jmp(nap_compiler *_compiler, const std::string &label);
void jlbf(nap_compiler *_compiler, const std::string &label);
void jnlbf(nap_compiler *_compiler, const char* label);
void mov_number_into_reg();
const char* push();
const char* ref();
const char* mov();
const char* call();
const char* pop();
const char* reg();
const char* idx();
const char* ccidx();

#endif

#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include "call_ctx.h"
#include "method.h"
#include "parser.h"

class interpreter
{
public:

    interpreter(nap_compiler* _compiler);

    /**
     * Builds an expression from the given string. Stores it in the tree found at node
     */
    void* build_expr_tree(const char *expr, expression_tree* node,
                          method* the_method, const char* orig_expr,
                          call_context* cc, int* result,
                          const expression_with_location* location, bool &psuccess);

private:

    std::vector<envelope*>* listv_prepare_list(const char* src,
                                               method* the_method,
                                               const char* orig_expr,
                                               call_context* cc,
                                               int* result,
                                               const expression_with_location* expwloc, bool &psuccess);

     std::vector<variable_definition*>* define_variables(char* var_def_type,
                                                         const char *expr_trim,
                                                         expression_tree* node,
                                                         method* the_method,
                                                         call_context* cc,
                                                         const char* orig_expr,
                                                         int* result,
                                                         const expression_with_location* expwloc, bool &psuccess);

     call_frame_entry* handle_function_call(char *expr_trim,
                                            int expr_len,
                                            expression_tree* node,
                                            method* func_call,
                                            method* the_method,
                                            const char* orig_expr,
                                            call_context* cc,
                                            int* result,
                                            const expression_with_location* expwloc,
                                            int type_of_call, bool &psuccess);

     void* deal_with_conditional_keywords(char* keyword_if,
                                          char* keyword_while,
                                          expression_tree* node,
                                          const expression_with_location* expwloc,
                                          const char *expr_trim,
                                          int expr_len,
                                          method* the_method,
                                          const char* orig_expr,
                                          call_context* cc,
                                          int* &result , bool &psuccess);

     method* define_method(const char* expr, int expr_len, expression_tree* node,
                           call_context* cc, const expression_with_location* expwloc, bool &psuccess);

     int get_operator(const char* expr, const char **foundOperator, int* ntype, bool &psuccess);

     int looks_like_function_def(const char* expr, int expr_len, const expression_tree* node, call_context* cc, bool &psuccess);

     bool is_list_value(const char* what);

     char* looks_like_var_def(const call_context* cc, const char *expr, int expr_len);

     int var_declaration_followed_by_initialization(const std::string &pexpr);

     int accepted_variable_name(const std::string &name);

     char* is_indexed(const char* expr_trim, int expr_len, char** index);

     char* is_some_statement(const char* expr_trim, const char* keyword);

     void* deal_with_one_word_keyword(call_context* cc, expression_tree* node, int* &result, const char* keyw, int statement, bool &psuccess);

     method* is_function_call(const char *s,  call_context* cc, int *special);
private:

    nap_compiler* mcompiler;
};

#endif

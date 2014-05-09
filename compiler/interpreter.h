#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include "call_ctx.h"
#include "method.h"
#include "parser.h"

class interpreter
{
public:

    interpreter(nap_compiler* _compiler);

    ~interpreter();

    void add_expression(expression_tree* node)
    {
        mexpressions.push_back(node);
    }

    /**
     * Builds an expression from the given string. Stores it in the tree found at node
     */
    void* build_expr_tree(const std::string &expr, expression_tree* node,
                          method* the_method, const char* orig_expr,
                          call_context* cc, int* result,
                          expression_with_location *location, bool &psuccess);

private:

    std::vector<envelope*>* listv_prepare_list(const std::string &src,
                                               method* the_method,
                                               const char* orig_expr,
                                               call_context* cc,
                                               int* result,
                                               expression_with_location *expwloc, bool &psuccess);

     std::vector<variable_definition*>* define_variables(const std::string &var_def_type,
                                                         const std::string &expr_trim,
                                                         expression_tree* node,
                                                         method* the_method,
                                                         call_context* cc,
                                                         const char* orig_expr,
                                                         int* result,
                                                         expression_with_location *expwloc, bool &psuccess);

     call_frame_entry* handle_function_call(const std::string &expr_trim,
                                            expression_tree* node,
                                            method* func_call,
                                            method* the_method,
                                            const char* orig_expr,
                                            call_context* cc,
                                            int* result,
                                            expression_with_location *expwloc,
                                            int type_of_call, bool &psuccess);

     void* deal_with_conditional_keywords(const std::string &keyword_if,
                                          const std::string &keyword_while,
                                          expression_tree* node,
                                          expression_with_location *expwloc,
                                          const std::string &expr_trim,
                                          method* the_method,
                                          const char* orig_expr,
                                          call_context* cc,
                                          int* &result , bool &psuccess);

     method* define_method(const std::string &expr, int expr_len, expression_tree* node,
                           call_context* cc, expression_with_location *expwloc, bool &psuccess);

     int get_operator(const std::string &expr, const char **foundOperator, int* ntype, bool &psuccess);

     int looks_like_function_def(const std::string &expr, int expr_len, const expression_tree* node, call_context* cc, bool &psuccess);

     bool is_list_value(const std::string &what);

     std::string looks_like_var_def(const call_context* cc, const std::string &expr, int expr_len);

     int var_declaration_followed_by_initialization(const std::string &pexpr);

     int accepted_variable_name(const std::string &name);

     std::string is_indexed(const std::string &expr_trim, int expr_len, std::string &index);

     std::string is_some_statement(const std::string &expr_trim, const std::string &keyword);

     void* deal_with_one_word_keyword(call_context* cc, expression_tree* node, int* &result, const char* keyw, int statement, bool &psuccess);

     method* is_function_call(const std::string &s,  call_context* cc, int *special);
private:

    nap_compiler* mcompiler;

    // has a list of all the expressions this interpreter has created.
    // Will be deleted upon deletion of this object
    std::vector<expression_tree*> mexpressions;
};

#endif

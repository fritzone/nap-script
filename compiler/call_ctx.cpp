#include "call_ctx.h"
#include "throw_error.h"
#include "code_output.h"
#include "sys_brkp.h"
#include "type.h"
#include "consts.h"
#include "interpreter.h"
#include "evaluate.h"
#include "variable.h"
#include "utils.h"
#include "code_stream.h"
#include "expression_tree.h"
#include <stdio.h>
#include <string.h>
#include <string>

using namespace std;

/**
 * Creates a new call context object
 */
call_context::call_context(int ptype, const char* pname, method* the_method, call_context* pfather)
{
    type = ptype;
    name = duplicate_string(pname);
    ccs_method = the_method;
    father = pfather;
}

class_declaration::class_declaration(const char* pname, call_context* pfather) : call_context(CLASS_DECLARATION, pname, 0,  pfather)
{
    ccs_method = 0;
    parent_class = 0;
    pfather->classes.push_back(this);
 }

/**
 * Adds a method to the given call context
 */
void call_context_add_method(call_context* cc,  method* the_method)
{
    cc->methods.push_back(the_method);
}

/**
 * Returns the variable from the call context if any, NULL if nothing found
 */
variable* call_context_get_variable(call_context* cc, const char* v_name)
{
    std::vector<variable*>::const_iterator vl = variable_list_has_variable(v_name, cc->variables);
    if(vl != cc->variables.end())
    {
        return (*vl);
    }
    else
    {
        return NULL;
    }
}

/**
 * Adds a variable to the call context
 */
variable* call_context_add_variable(call_context* cc, const char* name, const char* type, int dimension, const expression_with_location* expwloc)
{
    if(variable_list_has_variable(name, cc->variables) != cc->variables.end())
    {
        throw_error(E0034_SYMBOLDEFD, NULL);
    }
    variable* v = variable_list_add_variable(name, type, dimension, cc->variables, cc->ccs_method, cc, expwloc);
    v->cc = cc;
    return v;
}

/**
 * Retrieves the method object for the given name from the cc
 */
method* call_context_get_method(call_context* cc, const char* name)
{
    std::vector<method*>::const_iterator q = cc->methods.begin();
    while(q != cc->methods.end())
    {
        if((*q)->method_name && !strcmp((*q)->method_name, name))
        {
            return *q;
        }
        q ++;
    }
    if(cc->father)
    {
        return call_context_get_method(cc->father, name);
    }
    return NULL;
}

long call_context_add_label(call_context* cc, long position, const std::string& name)
{
    bytecode_label* bl = alloc_mem(bytecode_label, 1);
    bl->bytecode_location = position;
    bl->name = duplicate_string(name.c_str());
    bl->type = bytecode_label::LABEL_PLAIN;
    cc->labels.push_back(bl);
    return cc->labels.size();
}

struct bytecode_label* call_context_add_break_label(call_context* cc, long position, const std::string& name)
{
    bytecode_label* bl = alloc_mem(bytecode_label, 1);
    bl->bytecode_location = position;
    bl->name = duplicate_string(name.c_str());
    bl->type = bytecode_label::LABEL_BREAK;
    cc->break_label = bl;
    cc->labels.push_back(bl);
    return bl;
}

/**
 * Adds a new compiled expression to the given call context
 */
void call_context_add_compiled_expression(call_context* the_cc, const struct expression_tree* co_expr, const char* expr)
{
    expression_tree_list* tmp = expression_tree_list_add_new_expression(co_expr, &the_cc->expressions, expr);
    if(NULL == the_cc->expressions)
    {
        the_cc->expressions = tmp;
    }
}

/**
 * Adds a new expression to the method's list
 */
expression_tree* call_context_add_new_expression(call_context* the_cc, const char* expr, const expression_with_location* expwloc)
{
    expression_tree* new_expression = new_expression_tree(expwloc);
    char *t1 = duplicate_string(expr);
    int res;
    char*t = rtrim(t1);
    build_expr_tree(t, new_expression, the_cc->ccs_method, t, the_cc, &res, expwloc);
    expression_tree_list_add_new_expression(new_expression, &the_cc->expressions, t);

    return new_expression;
}

/**
 * This runs the global context. Loads the environment variables (from OS), and starts executingthe commands
 * found outside the methods.
 * Run method main
 */
void call_context_compile(call_context* cc)
{
    {
    expression_tree_list* q = cc->expressions;
    while(q)
    {
        int unknown_type = -1;
        compile(q->root, NULL, cc, 0, unknown_type, 0);
        q=q->next;
    }
    exit_app();
    }

    {
    // and now the functions
    std::vector<method*>::iterator ccs_methods = cc->methods.begin();
    while(ccs_methods != cc->methods.end())
    {
        code_stream() << NEWLINE << fully_qualified_label(std::string(std::string((*ccs_methods)->main_cc->father->name) +
                                                                      '.' +
                                                                      (*ccs_methods)->method_name).c_str()) << NEWLINE;
        code_stream() << "pushall" << NEWLINE;
        // now pop off the variables from the stack
        std::vector<variable*>::const_iterator vlist = (*ccs_methods)->variables.begin();
        int pctr = 0;
        while(vlist != (*ccs_methods)->variables.end())
        {
            peek((*ccs_methods)->main_cc, (*vlist)->c_type, pctr++, (*vlist)->name);
            vlist ++;
        }

        std::string fun_hash = generate_unique_hash();
        push_cc_start_marker(fun_hash.c_str());
        expression_tree_list* q1 = (*ccs_methods)->main_cc->expressions;
        while(q1)
        {
            int unknown_type = -1;
            compile(q1->root, (*ccs_methods), (*ccs_methods)->main_cc, 0, unknown_type, 0);
            q1=q1->next;
        }
        ccs_methods ++;
        push_cc_end_marker(fun_hash.c_str());
        code_stream() << "popall" << NEWLINE;
        code_stream() << "leave" << NEWLINE;
    }
    }

    {
    // and now all the functions from the classes
    for(unsigned int i=0; i<cc->classes.size(); i++)
    {
        class_declaration* cd = cc->classes.at(i);
        std::vector<method*>::iterator ccs_methods = cd->methods.begin();
        code_stream() << '@' << cd->name << NEWLINE;
        while(ccs_methods != cd->methods.end())
        {
            code_stream() <<  fully_qualified_label( (std::string(cd->name) + STR_DOT + (*ccs_methods)->method_name).c_str() ) << NEWLINE;
            // now pop off the variables from the stack
            code_stream() << "pushall" << NEWLINE;

            std::vector<variable*>::const_iterator vlist = (*ccs_methods)->variables.begin();
            int pctr = 0;
            while(vlist != (*ccs_methods)->variables.end())
            {
                peek((*ccs_methods)->main_cc, (*vlist)->c_type, pctr++, (*vlist)->name);
                vlist ++;
            }
            std::string class_fun_hash = generate_unique_hash();

            push_cc_start_marker(class_fun_hash.c_str());
            expression_tree_list* q1 = (*ccs_methods)->main_cc->expressions;
            while(q1)
            {
                int unknown_type = -1;
                compile(q1->root, (*ccs_methods), (*ccs_methods)->main_cc, 0, unknown_type, 0);
                q1=q1->next;
            }

            ccs_methods ++;
            push_cc_end_marker(class_fun_hash.c_str());
            code_stream() << "popall" << NEWLINE;
            code_stream() << "leave" << NEWLINE;
        }
    }
    }

}

/**
* Runs the given call context. If it encounters a return statement it will return the envelope
* holding the result of the function
*/
void call_context_run_inner(call_context* cc, int level, int reqd_type, int forced_mov)
{
    expression_tree_list* q = cc->expressions;
    while(q)
    {
        compile(q->root, cc->ccs_method, cc, level, reqd_type, forced_mov);
        q=q->next;
    }
}

struct bytecode_label* call_context_provide_label(call_context* cc)
{
    int maxlen = strlen(cc->name) + 32;
    char* label_name = alloc_mem(char, maxlen);
    sprintf(label_name, "%s_%d", cc->name, (int)cc->labels.size());    /* generating a name for the end of the while */
    long idx = call_context_add_label(cc, -1, label_name);
    return cc->labels.at(idx - 1);
}

class_declaration* call_context_get_class_declaration(const call_context* cc, const char* required_name)
{
    if(cc == NULL) return NULL;
    for(unsigned int i=0; i<cc->classes.size(); i++)
    {
        if(!strcmp(cc->classes.at(i)->name, required_name))
        {
            return cc->classes.at(i);
        }
    }
    return call_context_get_class_declaration(cc->father, required_name);
}

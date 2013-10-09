#include "call_ctx.h"
#include "throw_error.h"
#include "code_output.h"
#include "sys_brkp.h"
#include "type.h"
#include "preverify.h"
#include "consts.h"
#include "interpreter.h"
#include "evaluate.h"
#include "variable.h"
#include "utils.h"
#include "code_stream.h"
#include <stdio.h>
#include <string.h>
#include <string>

using namespace std;

struct call_context* global_cc = NULL;

/**
 * Creates a new call context object
 */
struct call_context* call_context_create(int type, const char* name, struct method* the_method, struct call_context* father)
{
    struct call_context* cc = alloc_mem(call_context,1);
    cc->type = type;
    cc->name = duplicate_string(name);
    cc->ccs_method = the_method;
    cc->father = father;
    cc->labels = new vector<bytecode_label*>();
    cc->classes = new vector<struct class_declaration*>();
    cc->interfaces = new vector<struct class_declaration*>();
    return cc;
}

struct class_declaration* class_declaration_create(const char* name, struct call_context* father)
{
    struct class_declaration* cc = alloc_mem(class_declaration, 1);
    cc->type = CLASS_DECLARATION;
    cc->name = duplicate_string(name);
    cc->ccs_method = 0;
    cc->father = father;
    cc->labels = new vector<bytecode_label*>();
    cc->parent_class = 0;
    cc->classes = new vector<struct class_declaration*>();
    cc->interfaces = new vector<struct class_declaration*>();
    father->classes->push_back(cc);
    return cc;
};

/**
 * Adds a method to the given call context
 */
method_list* call_context_add_method(call_context* cc,  method* the_method)
{
    method_list* q = method_list_insert(&cc->methods, the_method);
    return q;
}

/**
 * Returns the variable from the call context if any, NULL if nothing found
 */
variable* call_context_get_variable(call_context* cc, const char* v_name)
{
variable_list* vl = variable_list_has_variable(v_name, cc->variables);
    if(vl)
    {
        return vl->var;
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
    if(variable_list_has_variable(name, cc->variables))
    {
        throw_error(E0034_SYMBOLDEFD, NULL);
    }
    variable* v = variable_list_add_variable(name, type, dimension, &cc->variables, cc->ccs_method, cc, expwloc);
    v->cc = cc;
    return v;
}

/**
 * Retrieves the method object for the given name from the cc
 */
method* call_context_get_method(call_context* cc, const char* name)
{
    method_list* q = cc->methods;
    while(q)
    {
        if(!strcmp(q->the_method->name, name))
        {
            return q->the_method;
        }
        q=q->next;
    }
    if(cc->father)
    {
        return call_context_get_method(cc->father, name);
    }
    return NULL;
}

long call_context_add_label(struct call_context* cc, long position, const std::string& name)
{
    bytecode_label* bl = alloc_mem(bytecode_label, 1);
    bl->bytecode_location = position;
    bl->name = duplicate_string(name.c_str());
    bl->type = bytecode_label::LABEL_PLAIN;
    cc->labels->push_back(bl);
    return cc->labels->size();
}

struct bytecode_label* call_context_add_break_label(struct call_context* cc, long position, const std::string& name)
{
    bytecode_label* bl = alloc_mem(bytecode_label, 1);
    bl->bytecode_location = position;
    bl->name = duplicate_string(name.c_str());
    bl->type = bytecode_label::LABEL_BREAK;
    cc->break_label = bl;
    cc->labels->push_back(bl);
    return bl;
}

/**
 * Adds a new compiled expression to the given call context
 */
void call_context_add_compiled_expression(struct call_context* the_cc, const struct expression_tree* co_expr, const char* expr)
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
char *t;
int res;
    t = prepare_expression(expr);
    build_expr_tree(t, new_expression, the_cc->ccs_method, t, the_cc, &res, expwloc);
    validate(new_expression);
    expression_tree_list_add_new_expression(new_expression, &the_cc->expressions, t);
    free(t);
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
        compile(q->root, NULL, cc, 0, -1, 0);
        q=q->next;
    }
    exit_app();
    }

    {
    // and now the functions
    method_list* ccs_methods = cc->methods;
    while(ccs_methods)
    {
        code_stream() << NEWLINE << fully_qualified_label(ccs_methods->the_method->name) << NEWLINE;
        // now pop off the variables from the stack
        variable_list* vlist = ccs_methods->the_method->variables;
        int pctr = 0;
        while(vlist)
        {
            peek(ccs_methods->the_method->main_cc, vlist->var->c_type, pctr++, vlist->var->name);
            vlist = vlist->next;
        }

        expression_tree_list* q1 = ccs_methods->the_method->main_cc->expressions;
        while(q1)
        {
            compile(q1->root, ccs_methods->the_method,
                    ccs_methods->the_method->main_cc,
                    0, -1, 0);
            q1=q1->next;
        }
        ccs_methods = ccs_methods->next;
        code_stream() << "return" << NEWLINE;
    }
    }

    {
    // and now all the functions from the classes
    for(unsigned int i=0; i<cc->classes->size(); i++)
    {
        class_declaration* cd = cc->classes->at(i);
        method_list* ccs_methods = cd->methods;
        code_stream() << '@' << cd->name << NEWLINE;
        while(ccs_methods)
        {
            code_stream() <<  fully_qualified_label( (std::string(cd->name) + STR_DOT + ccs_methods->the_method->name).c_str() ) << NEWLINE;
            // now pop off the variables from the stack
            variable_list* vlist = ccs_methods->the_method->variables;
            int pctr = 0;
            while(vlist)
            {
                peek(ccs_methods->the_method->main_cc, vlist->var->c_type, pctr++, vlist->var->name);
                vlist = vlist->next;
            }
            push_cc_start_marker();
            expression_tree_list* q1 = ccs_methods->the_method->main_cc->expressions;
                while(q1)
                {
                    compile(q1->root, ccs_methods->the_method,
                            ccs_methods->the_method->main_cc,
                            0, -1, 0);
                    q1=q1->next;
                }


            ccs_methods = ccs_methods->next;
            code_stream() << "return" << NEWLINE;
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


/**
 * Adds a child call context to this
 */
void call_context_add_child_call_context(call_context* father,  call_context* child)
{
call_context_list* tmp = alloc_mem(call_context_list,1), *q;
    tmp->ctx = child;
    if(NULL == father->child_call_contexts)
    {
        father->child_call_contexts = tmp;
        return;
    }
    q=father->child_call_contexts;
    while(q->next) q=q->next;
    q->next = tmp;
}

struct bytecode_label* call_context_provide_label(struct call_context* cc)
{
    int maxlen = strlen(cc->name) + 32;
    char* label_name = alloc_mem(char, maxlen);
    sprintf(label_name, "%s_%d", cc->name, (int)cc->labels->size());	/* generating a name for the end of the while */
    long idx = call_context_add_label(cc, -1, label_name);
    return cc->labels->at(idx - 1);
}

class_declaration* call_context_get_class_declaration(const call_context* cc, const char* required_name)
{
    if(cc == NULL) return NULL;
    for(unsigned int i=0; i<cc->classes->size(); i++)
    {
        if(!strcmp(cc->classes->at(i)->name, required_name))
        {
            return cc->classes->at(i);
        }
    }
    return call_context_get_class_declaration(cc->father, required_name);
}

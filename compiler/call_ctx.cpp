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
call_context::call_context(int ptype, const char* pname,
                           method* the_method, call_context* pfather)
    : name(pname)
{
    type = ptype;
    ccs_method = the_method;
    father = pfather;
}

call_context::~call_context()
{
    for(size_t i=0; i<methods.size(); i++)
    {
        delete methods[i];
    }
}

class_declaration::class_declaration(const char* pname, call_context* pfather) : call_context(CLASS_DECLARATION, pname, 0,  pfather)
{
    parent_class = 0;
    pfather->add_class_declaration(this);
 }

/**
 * Adds a method to the given call context
 */
void call_context::add_method(method* the_method)
{
    methods.push_back(the_method);
}

/**
 * Adds a variable to the call context
 */
variable* call_context::add_variable(const char* name, const char* type, int dimension, const expression_with_location* expwloc)
{
    if(variable_list_has_variable(name, variables) != variables.end())
    {
        throw_error(E0034_SYMBOLDEFD, NULL);
    }
    variable* v = variable_list_add_variable(name, type, dimension, variables, ccs_method, this, expwloc);
    v->cc = this;
    return v;
}

/**
 * Retrieves the method object for the given name from the cc
 */
method* call_context::get_method(const char* name)
{
    std::vector<method*>::const_iterator q = methods.begin();
    while(q != methods.end())
    {
        if((*q)->method_name && !strcmp((*q)->method_name, name))
        {
            return *q;
        }
        q ++;
    }
    if(father)
    {
        return father->get_method(name);
    }
    return NULL;
}

long call_context::add_label(long position, const std::string& name)
{
    bytecode_label* bl = alloc_mem(bytecode_label, 1);
    bl->bytecode_location = position;
    bl->name = duplicate_string(name.c_str());
    bl->type = bytecode_label::LABEL_PLAIN;
    labels.push_back(bl);
    return get_label_count();
}

struct bytecode_label* call_context::add_break_label(long position, const std::string& name)
{
    bytecode_label* bl = alloc_mem(bytecode_label, 1);
    bl->bytecode_location = position;
    bl->name = duplicate_string(name.c_str());
    bl->type = bytecode_label::LABEL_BREAK;
    break_label = bl;
    labels.push_back(bl);
    return bl;
}

/**
 * Adds a new compiled expression to the given call context
 */
void call_context::add_compiled_expression(expression_tree* co_expr)
{
    expressions.push_back(co_expr);
}

/**
 * Adds a new expression to the method's list
 */
expression_tree* call_context::add_new_expression(const char* expr, const expression_with_location* expwloc)
{
    expression_tree* new_expression = new expression_tree(expwloc);
    garbage_bin<expression_tree*>::instance().place(new_expression);

    char *t1 = duplicate_string(expr);
    int res;
    char*t = rtrim(t1);
    build_expr_tree(t, new_expression, ccs_method, t, this, &res, expwloc);
    expressions.push_back(new_expression);
    return new_expression;
}



/**
* Runs the given call context.
*/
void call_context::compile_standalone(nap_compiler* _compiler, int level, int reqd_type, int forced_mov)
{
    std::vector<expression_tree*>::iterator q = expressions.begin();
    while(q != expressions.end())
    {
        ::compile(_compiler, *q, ccs_method, this, level, reqd_type, forced_mov);
        q ++;
    }
}


/**
 * This runs the global context. Loads the environment variables (from OS), and starts executingthe commands
 * found outside the methods.
 * Run method main
 */
void call_context::compile(nap_compiler* _compiler)
{
    {
        std::vector<expression_tree*>::iterator q = expressions.begin();
        while(q != expressions.end())
        {
            int unknown_type = -1;
            ::compile(_compiler, *q, NULL, this, 0, unknown_type, 0);
            q ++;
        }
        exit_app(_compiler);
    }

    {
    // and now the functions
    std::vector<method*>::iterator ccs_methods = methods.begin();
    while(ccs_methods != methods.end())
    {
        code_stream(_compiler) << NEWLINE << fully_qualified_label(std::string(std::string((*ccs_methods)->main_cc->father->name) +
                                                                      '.' +
                                                                      (*ccs_methods)->method_name).c_str()) << NEWLINE;
        code_stream(_compiler) << "pushall" << NEWLINE;
        // now pop off the variables from the stack
        std::vector<variable*>::const_iterator vlist = (*ccs_methods)->get_variables().begin();
        int pctr = 0;
        while(vlist != (*ccs_methods)->get_variables().end())
        {
            peek(_compiler, (*ccs_methods)->main_cc, (*vlist)->c_type, pctr++, (*vlist)->name);
            vlist ++;
        }

        std::string fun_hash = generate_unique_hash();
        push_cc_start_marker(_compiler, fun_hash.c_str());
        std::vector<expression_tree*>::const_iterator q1 = (*ccs_methods)->main_cc->get_expressions().begin();
        while(q1 != (*ccs_methods)->main_cc->get_expressions().end())
        {
            int unknown_type = -1;
            ::compile(_compiler, *q1, (*ccs_methods), (*ccs_methods)->main_cc, 0, unknown_type, 0);
            q1 ++;
        }
        ccs_methods ++;
        push_cc_end_marker(_compiler, fun_hash.c_str());
        code_stream(_compiler) << "popall" << NEWLINE;
        code_stream(_compiler) << "leave" << NEWLINE;
    }
    }

    {
    // and now all the functions from the classes
    for(unsigned int i=0; i<classes.size(); i++)
    {
        class_declaration* cd = classes.at(i);
        std::vector<method*>::iterator ccs_methods = cd->methods.begin();
        code_stream(_compiler) << '@' << cd->name << NEWLINE;
        while(ccs_methods != cd->methods.end())
        {
            code_stream(_compiler) <<  fully_qualified_label( (std::string(cd->name) + STR_DOT + (*ccs_methods)->method_name).c_str() ) << NEWLINE;
            // now pop off the variables from the stack
            code_stream(_compiler) << "pushall" << NEWLINE;

            std::vector<variable*>::const_iterator vlist = (*ccs_methods)->get_variables().begin();
            int pctr = 0;
            while(vlist != (*ccs_methods)->get_variables().end())
            {
                peek(_compiler, (*ccs_methods)->main_cc, (*vlist)->c_type, pctr++, (*vlist)->name);
                vlist ++;
            }
            std::string class_fun_hash = generate_unique_hash();

            push_cc_start_marker(_compiler, class_fun_hash.c_str());
            std::vector<expression_tree*>::const_iterator q1 = (*ccs_methods)->main_cc->get_expressions().begin();
            while(q1 != (*ccs_methods)->main_cc->get_expressions().end())
            {
                int unknown_type = -1;
                ::compile(_compiler, *q1, (*ccs_methods), (*ccs_methods)->main_cc, 0, unknown_type, 0);
                q1 ++;
            }

            ccs_methods ++;
            push_cc_end_marker(_compiler, class_fun_hash.c_str());
            code_stream(_compiler) << "popall" << NEWLINE;
            code_stream(_compiler) << "leave" << NEWLINE;
        }
    }
    }

}

bytecode_label* call_context::provide_label()
{
    int maxlen = name.length() + 32;
    char* label_name = alloc_mem(char, maxlen);
    sprintf(label_name, "%s_%d", name.c_str(), (int)get_label_count());    /* generating a name for the end of the while */
    long idx = add_label(-1, label_name);
    return labels.at(idx - 1);
}

class_declaration* call_context::get_class_declaration(const char* required_name) const
{
    for(unsigned int i=0; i<classes.size(); i++)
    {
        if(classes.at(i)->name == required_name)
        {
            return classes.at(i);
        }
    }
    if(father)
    {
        return father->get_class_declaration(required_name);
    }
    else
    {
        return 0;
    }
}

void call_context::add_class_declaration(class_declaration *cd)
{
    classes.push_back(cd);
}

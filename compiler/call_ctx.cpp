#include "call_ctx.h"
#include "code_output.h"
#include "garbage_bin.h"
#include "sys_brkp.h"
#include "type.h"
#include "consts.h"
#include "interpreter.h"
#include "evaluate.h"
#include "variable.h"
#include "utils.h"
#include "code_stream.h"
#include "expression_tree.h"
#include "compiler.h"

#include "nbci.h"
#include "funtable.h"

#include <stdio.h>
#include <string.h>
#include <string>
#include <sstream>

using namespace std;

/**
 * Creates a new call context object
 */
call_context::call_context(nap_compiler *_compiler, CC_TYPE ptype, const string &pname,
                           method* the_method, call_context* pfather)
    : compiler(_compiler), children()
{
    name = pname == "global"?pname : pname + "_" + generate_unique_hash();
    type = ptype;
    ccs_method = the_method;
    father = pfather;

    if(father)
    {
        father->add_child_cc(this);
    }
}

call_context::~call_context()
{
    for(size_t i=0; i<methods.size(); i++)
    {
        delete methods[i];
    }

    for(size_t i=0; i<children.size(); i++)
    {
        delete children[i];
    }
}

class_declaration::class_declaration(nap_compiler *_compiler, const string &name, call_context* pfather) :
    call_context(_compiler, CC_CLASS, name, 0,  pfather)
{
    parent_class = 0;
    pfather->classes.push_back(this);
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
variable* call_context::add_variable(const std::string& name, const std::string& type,
                                     int dimension,
                                     bool& psuccess)
{
    psuccess = true;
    if(has_variable(name) != variables.end())
    {
        psuccess = false;
        compiler->throw_error(E0034_SYMBOLDEFD, NULL);
        return 0;
    }

    variable* v = variable_holder::add_variable(name, type, dimension,
                                                ccs_method, this, psuccess);
    if(!psuccess)
    {
        return 0;
    }

    v->cc = this;
    return v;
}

/**
 * Retrieves the method object for the given name from the cc
 */
method* call_context::get_method(const string &pname)
{
    // first check, see if this is a method from the current VM
    std::vector<method*>::const_iterator q = methods.begin();
    while(q != methods.end())
    {
        if(pname == (*q)->method_name)
        {
            return *q;
        }
        q ++;
    }
    if(father)
    {
        return father->get_method(pname);
    }

    // then try to get it from the vm chain of the compiler ... if any
    struct nap_vm* chain = compiler->mvm_chain;
    while(chain)
    {
        struct funtable_entry* fe = nap_vm_get_method(chain, pname.c_str());
        if(fe)
        {
            call_context* chain_cc = new call_context(compiler, CC_CHAINED, "-", 0, 0);
            method* m = new method(compiler, pname.c_str(), (char*)get_reg_type(fe->return_type), chain_cc);
            this->compiler->add_external_method(m);
            // methods which are externals own their own fake cc, they should delete them!
            m->owns_cc = true;
            for(int i=0; i<fe->parameter_count; i++)
            {
                bool success = true;
                m->add_parameter(std::string("par_") +  get_reg_type(fe->parameter_types[i]), get_reg_type(fe->parameter_types[i]), 1, 0, success);
                if(!success) return 0;
            }


            return m;
        }
        chain = chain->parent;
    }
    return NULL;
}

bytecode_label call_context::add_break_label(long position, const std::string& name)
{
    bytecode_label bl;

    bl.bytecode_location = position;
    bl.name = name;
    bl.type = bytecode_label::LABEL_BREAK;
    break_label = bl;

    labels.push_back(bl);
    return bl;
}

/**
 * Adds a new expression to the method's list
 */
expression_tree* call_context::add_new_expression(const std::string& expr, expression_with_location* expwloc, bool& psuccess)
{
    psuccess = true;
    expression_tree* new_expression = expwloc->new_expression();

    int res;
    compiler->get_interpreter().build_expr_tree(expr, new_expression, ccs_method, expwloc->expression.c_str(), this, &res, expwloc, psuccess);
    if(!psuccess)
    {
        return 0;
    }
    expressions.push_back(new_expression);
    return new_expression;
}

/**
 * Compiles the given call context.
 */
void call_context::compile_standalone(nap_compiler* _compiler, int level, int reqd_type, int forced_mov, bool& psuccess)
{
    std::vector<expression_tree*>::iterator q = expressions.begin();
    while(q != expressions.end())
    {
        ::compile(_compiler, *q, ccs_method, this, level, reqd_type, forced_mov, psuccess);
        if(!psuccess)
        {
           return;
        }

        q ++;
    }
}

// warning! the numbers are not the same as from nap_Ext_gen
static const char* get_type_code(uint8_t el)
{
    switch(el)
    {
    case 1: return "i";
    case 2: return "r";
    case 3: return "s";
    case 4: return "b";
    default:return "v";
    }
}

static int signature_to_number(const std::string& sig)
{
    int res = 0;
    std::string base4 = "";
    for(size_t i=0; i<sig.length(); i++)
    {
        if(sig[i] == 'i') base4 += "1";
        if(sig[i] == 'r') base4 += "2";
        if(sig[i] == 's') base4 += "3";
        if(sig[i] == 'v') base4 += "0";
        if(sig[i] == 'b') base4 += "4";
    }
    char *t = 0;
    res = strtol(base4.c_str(), &t, 5);
    return res;
}

/**
 * This runs the global context. Loads the environment variables (from OS), and starts executingthe commands
 * found outside the methods.
 * Run method main
 */
void call_context::compile(nap_compiler* _compiler, bool&psuccess)
{
    {
        std::vector<expression_tree*>::iterator q = expressions.begin();
        while(q != expressions.end())
        {
            int unknown_type = -1;
            ::compile(_compiler, *q, NULL, this, 0, unknown_type, 0, psuccess);
            if(!psuccess)
            {
                return;
            }

            q ++;
        }
		_compiler->set_location(0);
        exit_app(_compiler);
    }

    {
    // and now the functions
    std::vector<method*>::iterator ccs_methods = methods.begin();
    while(ccs_methods != methods.end())
    {
        method* m = *ccs_methods;

        code_stream(_compiler) << NEWLINE << fully_qualified_label(std::string(std::string((*ccs_methods)->main_cc->father->name) +
                                                                      '.' +
                                                                      (*ccs_methods)->method_name).c_str()) << NEWLINE;
        code_stream(_compiler) << pushall() << NEWLINE;

        if(m->def_loc == DEF_INTERN)
        {
            // now pop off the variables from the stack
            std::vector<variable*>::const_reverse_iterator vlist = (*ccs_methods)->variables.rbegin();
            int pctr = 0;
            while(vlist != (*ccs_methods)->variables.rend())
            {
                peek(_compiler, (*ccs_methods)->main_cc, (*vlist)->c_type, pctr++, (*vlist)->name.c_str());
                vlist ++;
            }

            // marker
            std::string fun_hash = generate_unique_hash();
            push_cc_start_marker(_compiler, fun_hash.c_str());

            // and compile the isntructions
            std::vector<expression_tree*>::const_iterator q1 = (*ccs_methods)->main_cc->expressions.begin();
            while(q1 != (*ccs_methods)->main_cc->expressions.end())
            {
                int unknown_type = -1;
                ::compile(_compiler, *q1, (*ccs_methods), (*ccs_methods)->main_cc, 0, unknown_type, 0, psuccess);
                if(!psuccess)
                {
                    return;
                }

                q1 ++;
            }

            // end marker
            push_cc_end_marker(_compiler, fun_hash.c_str());
        }
        else
        {
            // determine the fun ptr number (the index from the nap_int_init_ext_func_array
            std::string type_encoding = get_type_code(m->ret_type);

            // now the types of the parameters
            std::vector<variable*>::const_iterator vlist = m->variables.begin();
            while(vlist != m->variables.end())
            {
                variable* v = *vlist;
                type_encoding += get_type_code((uint8_t)v->i_type);
                vlist ++;
            }

            // now create the deciaml number from the string below (that being bsae 4)
            code_stream(_compiler) << mov() <<
                                      reg() << "string" << '(' << 0 << ')' <<
                                      ',' << "\"" + type_encoding + "\"" << NEWLINE;

            code_stream(_compiler) << mov() <<
                                      reg() << "string" << '(' << 1 << ')' <<
                                      ',' << "\"" + m->method_name + "\"" << NEWLINE;

            code_stream(_compiler) << mov() <<
                                      reg() << "string" << '(' << 2 << ')' <<
                                      ',' << "\"" + m->library_name + "\"" << NEWLINE;

            code_stream(_compiler) << mov () << reg() << "int" << '(' << 0 << ')' <<
                                      ',' << signature_to_number(type_encoding) << NEWLINE;

            code_stream(_compiler) << "intr" << 4 << NEWLINE;
        }
        code_stream(_compiler) << popall() << NEWLINE;
        code_stream(_compiler) << leave() << NEWLINE;

        // nest method
        ccs_methods ++;
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

            std::vector<variable*>::const_reverse_iterator vlist = (*ccs_methods)->variables.rbegin();
            int pctr = 0;
            while(vlist != (*ccs_methods)->variables.rend())
            {
                peek(_compiler, (*ccs_methods)->main_cc, (*vlist)->c_type, pctr++, (*vlist)->name.c_str());
                vlist ++;
            }
            std::string class_fun_hash = generate_unique_hash();

            push_cc_start_marker(_compiler, class_fun_hash.c_str());
            std::vector<expression_tree*>::const_iterator q1 = (*ccs_methods)->main_cc->expressions.begin();
            while(q1 != (*ccs_methods)->main_cc->expressions.end())
            {
                int unknown_type = -1;
                ::compile(_compiler, *q1, (*ccs_methods), (*ccs_methods)->main_cc, 0, unknown_type, 0, psuccess);
                if(!psuccess)
                {
                    return;
                }

                q1 ++;
            }

            ccs_methods ++;
            push_cc_end_marker(_compiler, class_fun_hash.c_str());
            code_stream(_compiler) << popall() << NEWLINE;
            code_stream(_compiler) << leave() << NEWLINE;
        }
    }
    }

}

bytecode_label call_context::provide_label()
{
    std::stringstream ss;
    ss << name << C_UNDERLINE << (int)labels.size();
    long idx = add_label(-1, ss.str());
    return labels.at(idx - 1);
}

long call_context::add_label(long position, const std::string& name)
{
    bytecode_label bl;

    bl.bytecode_location = position;
    bl.name = name;
    bl.type = bytecode_label::LABEL_PLAIN;

    labels.push_back(bl);
    return labels.size();
}

class_declaration* call_context::get_class_declaration(const std::string& required_name) const
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

void call_context::add_child_cc(call_context *child_cc)
{
    children.push_back(child_cc);
}

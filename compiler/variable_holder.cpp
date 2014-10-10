#include "variable_holder.h"
#include "variable.h"
#include "compiler.h"
#include "utils.h"
#include "consts.h"

/**
 * this function checks if s appears in the hashtable
 */
variable_holder::variable_holder() : variables()
{
}

variable_holder::~variable_holder()
{
    for(size_t i=0; i<variables.size(); i++)
    {
        delete variables[i];
    }
}

std::vector<variable*>::const_iterator variable_holder::has_variable(const std::string& s)
{
    std::vector<variable*>::const_iterator q = variables.begin();
    while(q != variables.end())
    {
        if((*q)->name == s)
        {
            return q;
        }
        q ++;
    }
    return variables.end();
}


/**
 * This function adds a new  variable to the hashlist in first. Always adds the new  variable to the head of the list
 */
variable* variable_holder::add_variable(const std::string& var_name,
                                     const std::string& var_type,
                                     int var_size,
                                     method* the_method,
                                     call_context* cc,
                                     bool& psuccess)
{
    //

    if(!valid_variable_name(var_name) && the_method && the_method->def_loc == DEF_INTERN)
    {
        //compiler->throw_error("Invalid variable name", var_name, var_type);
        psuccess = false;
        std::string var_name_err = "Invalid variable name:" + var_name;
        cc->compiler->throw_error("Syntax Error", var_name_err, var_name);
        return 0;
    }

    int itype = get_typeid(var_type);

    if(itype == BASIC_TYPE_DONTCARE)
    {
        if(cc->get_class_declaration(var_type))
        {
            itype = BASIC_TYPE_USERDEF;
        }
    }
    if(itype == BASIC_TYPE_DONTCARE)
    {
        return 0;
    }

    if(var_size < 1)
    {
        cc->compiler->throw_error(STR_INVALID_DIMENSION, NULL);
        psuccess = false;
        return 0;
    }

    variable* var = new variable(var_size, itype, var_name, var_type, cc);
    variables.push_back(var);
    return var;
}

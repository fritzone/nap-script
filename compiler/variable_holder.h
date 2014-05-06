#ifndef VARIABLE_HOLDER_H
#define VARIABLE_HOLDER_H

#include <vector>
#include <string>

struct variable;
struct method;
struct call_context;
struct expression_with_location;

struct variable_holder
{

    virtual ~variable_holder() {}

    /**
     * Checks if the variable named 's' is in the hash list 'first'
     * @param s - the name of the variable
     * @param first - the list we're searching
     */
    std::vector<variable*>::const_iterator has_variable(const std::string &s);

    /**
     * Adds a new hash  variable to the variable list. Creates a variable object and adds that to the list
     * @param var_name - the name of the variable
     * @param var_type - the type of the variable
     * @param var_size - the size of the variable (as defined by the script, the 'dimension' of it)
     * @param first - this is the variable list we're adding the new variable
     * @param the_method - this is happening in this method
     * @param cc - and/or in this call context
     * @param expwloc - at this location in the script file
     */
    variable* add_variable(const std::string &var_name,
                                         const std::string &var_type,
                                         int var_size,
                                         method* the_method,
                                         call_context* cc, bool &psuccess);

    /* the list of variable that have been defined in this container */
    std::vector<variable*> variables;
};

#endif // VARIABLE_HOLDER_H

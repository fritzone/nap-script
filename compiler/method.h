#ifndef _METHOD_H_
#define _METHOD_H_

#include "parser.h"

/* forward declaration, we need this in the method */

struct call_frame_entry;
struct call_context_list;
struct call_context;
struct call_frame_entry;
struct variable_definition;
struct variable;
struct class_declaration;

class nap_compiler;

/**
 * Contains the definition of a method
 */
struct method
{

    /**
     * @brief method Constructor
     * @param name
     * @param return_type
     * @param cc
     */
    method(nap_compiler *_compiler, char* name, char* preturn_type, call_context* cc);

    /**
     * Adds a new parameter to the method
     * @param name - the name of the parameter
     * @param type - the type of the parameter (as string)
     * @param modifiable - if the parameter is a reference or not (C++ rulez :) )
     */
    parameter* add_parameter(char* pname,  char* ptype, int pdimension, const expression_with_location* pexpwloc, call_context *cc, bool &psuccess);

    /**
     * @brief get_parameter return the parameter at the given location
     * @param i
     * @return
     **/
    parameter* get_parameter(size_t i);

    /**
     * @brief add_new_variable Adds a new variable to the method ...
     * @param name
     * @param type
     * @param dimension
     * @param expwloc
     * @return
     */
    variable* add_new_variable(char* pname,  char* type, int dimension, const expression_with_location* expwloc, bool &psuccess);
    /**
     * @brief has_variable
     * @param cc
     * @param varname
     * @param templed
     * @param env_var
     * @return
     */
    variable* has_variable(call_context* cc, char* varname, int* templed, int* env_var, bool &psuccess);

    /**
     * @brief feed_parameter_list Feeds in the given parameter list to the method for it to create its parameter structures
     * @param par_list
     * @param expwloc
     */
    void feed_parameter_list(char* par_list, const expression_with_location* expwloc, bool &psuccess);


    const std::vector<variable*>& get_variables() const
    {
        return variables;
    }

    const nap_compiler* get_compiler() const
    {
        return mcompiler;
    }


    /* the name of the method */
    std::string method_name;

    /* the return type of the function */
    std::string return_type;

    /* the parameters of the method */
    std::vector<parameter*> parameters;

    /* this is the call context of the method, created when the method is created */
    call_context *main_cc;

    /* the current call frame. stores all the variables of the method which are not static. At each calling of a method
     * this variable gets populated with the call frame that has been constructed for the current stage
     */
    call_frame_entry *cur_cf;

    /* the definition location 0 - normal, 1 - extern*/
    int def_loc;

    int ret_type;

private:
    /* the list of struct variables that are defined in the method. This list is used in two situations:
     * 1. in the interpret phase, when the code is built up, this is used as references to find the variables.
     * 2. in the running phase, but only for static variables.
     */
    std::vector<variable*> variables;

    nap_compiler* mcompiler;

};

/**
 * A constructor call structure for the "new" keyword
 **/
struct constructor_call : public method
{
    constructor_call(char* name, call_context* cc);

    class_declaration *the_class;
};

/**
 * The call frame structure represents the "stack" way the functions are being called.
 * Each time a function is called it puts it "signature" (the address of the_method)
 * and the parameters onto a global call_frame variable. There is always required to be
 * at least one element on the global call_frame stack (the main application).
 * Also this structure holds the variable of the method being called at the current stage.
 * If the main application exits then the call_frame will be empty.
 * (The best of all: This can be used at a (very) later stage when developing threading. Each
 * thread like object will have its own call frame, and they'll be free to run independently)
 */
struct call_frame_entry
{
    /* this is the method that was called */
    method *the_method;

    /* this are the parameters passed to the method */
    std::vector<parameter*> parameters;

    /* holds the previous call frame of the method... just in case*/
    call_frame_entry *previous_cf;
};


#endif

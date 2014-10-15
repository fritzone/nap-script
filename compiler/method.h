#ifndef _METHOD_H_
#define _METHOD_H_

#include "parser.h"
#include "variable_holder.h"

struct call_frame_entry;
struct call_context_list;
struct call_context;
struct call_frame_entry;
struct variable_definition;
struct variable;
struct class_declaration;

class nap_compiler;
class interpreter;

/**
 * Contains the definition of a method
 */
struct method : public variable_holder
{

    /**
     * @brief builtin_method will provide a builtin method for the given ID
     * @param method
     * @return
     */
    static method* builtin_method(nap_compiler *compiler, int);

    /**
     * @brief method Constructor
     * @param name
     * @param return_type
     * @param cc
     */
    method(nap_compiler *_compiler, const std::string& name, const std::string& preturn_type, call_context* cc);

    virtual ~method();

    /**
     * Adds a new parameter to the method
     * @param name - the name of the parameter
     * @param type - the type of the parameter (as string)
     * @param modifiable - if the parameter is a reference or not (C++ rulez :) )
     */
    parameter* add_parameter(bool reference, std::string pname,  const std::string &ptype, int pdimension, interpreter *interp, const char *orig_expr, expression_with_location *pexpwloc, bool &psuccess);

    void add_parameter(parameter* p)
    {
        parameters.push_back(p);
    }

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
    variable* add_new_variable(const std::string &pname,  const std::string &ptype, int dimension, bool &psuccess);
    /**
     * @brief has_variable
     * @param cc
     * @param varname
     * @param templed
     * @param env_var
     * @return
     */
    variable* has_variable(call_context* cc, const std::string &varname, int* env_var);

    /**
     * @brief feed_parameter_list Feeds in the given parameter list to the method for it to create its parameter structures
     * @param par_list
     * @param expwloc
     */
    void feed_parameter_list(const char *par_list, interpreter* interp, expression_with_location *expwloc, bool &psuccess);

    /* the name of the method */
    std::string method_name;

    /* the name of the library where the method is. If empty, the current
     * executable is queried*/
    std::string library_name;

    /* the return type of the function */
    std::string return_type;

    /* the parameters of the method */
    std::vector<parameter*> parameters;

    /* this is the call context of the method, created when the method is created */
    call_context *main_cc;

    /* the definition location 0 - normal, 1 - extern*/
    int def_loc;

    uint8_t ret_type;

    nap_compiler* mcompiler;

    std::string ret_type_array_dim;
    bool dynamic_ret_type_array;

    /* Tells us if this method owns his CC, ie: the cc was created specially for
       this method and not inherited from above. This is the case of external
       methods that are inherited from virtual machines above us*/
    bool owns_cc;

private:

    method(nap_compiler* compiler);
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
    call_frame_entry() : the_method(0), target(0), parameters() {}

    ~call_frame_entry();

    /* this is the method that was called */
    method *the_method;

    // the target variable in which the result will go. In the return stage there will be a simply move in this
    // if it represents a real variable. Otherwise the rv registers are used for simple data types, and
    // well, I have no idea what for complex data types. Maybe we create an entry somewhere...
    variable* target;

    /* these are the parameters passed to the method */
    std::vector<parameter*> parameters;

};


#endif

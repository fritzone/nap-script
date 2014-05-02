#ifndef _CALL_CTX_H_
#define _CALL_CTX_H_

#include "method.h"
#include "type.h"

#include <vector>

class nap_compiler;

/*
 * The call context is something like a namespace and/or code-block ... See the code
 * below, to understand:
 *
 * // by default a 'global' namespace starts here, with name 'global'.
 *
 * int myGlobalWarming;         // global variable, will be visible everywhere in the program
 * {                            // new call context starts here
 * int x;
 * int func()
 * {                            // another call context, which can see the x from above
 * int a;
 *     {                        // just a local nameless call context
 *     int b;                   // b is a local variable, a still visible above
 *     }                        // b is not visible anymore, it was destroyed at the end of the nameless call context
 * }
 * }                            // x is destroyed here by default
 */


/**
 * The call context is something like a namespace, or a class... basically its role is to group the methods
 * that belong to the same call context, and makes visible to each other. There is always a 'global' call context.
 * Variables by default belong to a call cotnext and they are destroyed when the call context ends
 */
struct call_context
{

    /**
     * Create a new call context
     * @param type - the type of the call context
     * @param name - the name of the call context
     * @param the_method - the method in which this call context is
     * @param father - the father of this call cotnext
     **/
    call_context (nap_compiler* _compiler,
                  int ptype,
                  const std::string& pname,
                  method *the_method,
                  call_context *pfather);

    ~call_context();

    /**
     * @brief call_context_add_method adds a method to the call context
     * @param the_method - the method which is to be added
     * @return the location in the list of the inserted method
     */
    void add_method(method *the_method);


    /**
     * @brief call_context_get_method returns a method from the call context
     * @param name - the name of the method
     * @return the method of found, null if not found
     */
    method *get_method(const std::string&pname);


    /**
     * @brief adds a variable to the call context
     * @param name - the name of the variable
     * @param type - the tye of the variable
     * @param dimension - the dimension of the variable
     * @param expwloc - the location of the piece of code for this
     * @return - the newly created variable
     */
    variable *add_variable(const char *name,
                           const char *type,
                           int dimension,
                           const expression_with_location *expwloc, bool &psuccess);


    /**
     * @brief adds a label to the call context
     * @param position - the poisition where to add it
     * @param name - the name of the label
     * @return how many labels are there in this call context
     */
    long add_label(long position, const std::string &name);

    /**
     * @brief add a "break" label to this call context
     * @param position - where to add the label
     * @param name - the name of the label
     * @return - the bytecode label object of the new label
     */
    bytecode_label *add_break_label(long position, const std::string &name);


    /**
     * Creates a new label structure and inserts it into the vector of the bytecode labels of this call_context
     */
    bytecode_label *provide_label();

    /**
     * @brief adds a compiled expression to the call context
     * @param co_expr - the compiled expression
     */
    void add_compiled_expression(expression_tree *co_expr);

    /**
     * @brief call_context_add_new_expression add a new expression to this call context
     * @param expr - the expression tah will be addedd
     * @param expwloc - the location of the expression in the file
     * @return the new expression_tree object
     */
    expression_tree *add_new_expression(const char *expr, const expression_with_location *expwloc, bool &psuccess);


    /**
     * Compile the given call context, and output the bytecode to the
     * system bytecode stream
     */
    void compile(nap_compiler *_compiler, bool &psuccess);

    /**
     * Runs this call context, the call context is supposed to be an "inner" call cotnext (mostly nameless call cotnext)
     */
    void compile_standalone(nap_compiler* _compiler, int level, int reqd_type, int forced_mov, bool &psuccess);

    /**
     * @brief call_context_get_class_declaration return a clas declaration from the
     * given call context if any
     * @param required_name - the name of the call context
     * @return
     */
    class_declaration *get_class_declaration(const std::string &required_name) const;

    /**
     * @brief add_class_declaration
     */
    void add_class_declaration(class_declaration*);

    bytecode_label* get_break_label() const
    {
        return break_label;
    }

    size_t get_label_count() const
    {
        return labels.size();
    }

    call_context* get_father() const
    {
        return father;
    }

    const std::vector<expression_tree*>& get_expressions() const
    {
        return expressions;
    }

    const std::vector<variable*>& get_variables() const
    {
        return variables;
    }

    const std::string& get_name() const
    {
        return name;
    }

    int get_type() const
    {
        return type;
    }

    nap_compiler* compiler() const
    {
        return mcompiler;
    }

    // TODO: These two do not belong specially here. The method shares them too
    /**
     * Checks if the variable named 's' is in the hash list 'first'
     * @param s - the name of the variable
     * @param first - the list we're searching
     */
    std::vector<variable*>::const_iterator variable_list_has_variable(const char *s,
                                              const std::vector<variable *> &first);

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
    variable* variable_list_add_variable(const char *var_name,
                                         const char* var_type,
                                         int var_size,
                                         std::vector<variable *> &first,
                                         method* the_method,
                                         call_context* cc,
                                         const expression_with_location* expwloc, bool &psuccess);


    /* the methods of this call context */
    std::vector<method*> methods;

private:

    /* the type of the call context: 0 - global, 1 - named, 2 - chained*/
    int type;

    /* the name of the call context */
    std::string name;


    /* the list of variable that have been defined in this call context */
    std::vector<variable*> variables;

    /* the (compiled) expressions in the call context */
    std::vector<expression_tree*> expressions;

    /* the father call context of this */
    call_context *father;

    /* if the call context is from a method this is that method */
    method *ccs_method;

    /* this is a vector of label locations for this*/
    std::vector<bytecode_label *> labels;

    bytecode_label *break_label;

    /* the classes that are defined in this call context */
    std::vector<class_declaration *> classes;

    nap_compiler* mcompiler;
};

/**
 * Class representing a class declaration with relationships to a parent class
 * and a list of implemented interfaces
 **/
struct class_declaration : public call_context
{
    /**
     * @brief class_declaration_create creates a new class declaration
     * @param name - the name of the class
     * @param father - the call context in which this is
     * @return the newly created class declaration
     */
    class_declaration (nap_compiler* _compiler, const char *pname, call_context *pfather);

    class_declaration *parent_class;
    std::vector<class_declaration *> implemented_interfaces;
};

#endif

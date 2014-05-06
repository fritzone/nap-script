#ifndef _CALL_CTX_H_
#define _CALL_CTX_H_

#include "method.h"
#include "type.h"
#include "variable_holder.h"

#include <vector>

class nap_compiler;


/**
 * Class, describing a label, a jump location.
 */
struct bytecode_label
{
    bytecode_label() : name(), bytecode_location(0), type(LABEL_PLAIN) {}

    enum label_type
    {
        LABEL_PLAIN = 0,
        LABEL_BREAK = 1,
        LABEL_CONTINUE = 2
    };

    /* the name of the label */
    std::string name;

    /* the location of the label in the bytecode stream */
    long bytecode_location;

    /* the type of the location, can be 0 if this is just a plain label,
     *1 if this is a "break" location label,
     *2 if this is a "continue" location label */
    label_type type;
};

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
struct call_context : public variable_holder
{

    //0 - global, 1 - named, 2 - chained
    enum CC_TYPE
    {
        CC_GLOBAL = 0,
        CC_NAMED = 1,
        CC_CHAINED = 2,
        CC_UNNAMED = 3,
        CC_IF = 4,
        CC_ELSE = 5,
        CC_WHILE = 5,
        CC_FOR = 7,
        CC_CLASS = 320
    };

public: /* Public methods */

    /**
     * Create a new call context
     * @param type - the type of the call context
     * @param name - the name of the call context
     * @param the_method - the method in which this call context is
     * @param father - the father of this call cotnext
     **/
    call_context (nap_compiler* _compiler,
                  CC_TYPE ptype,
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
    variable *add_variable(const std::string &name,
                           const std::string &type,
                           int dimension, bool &psuccess);


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
    bytecode_label add_break_label(long position, const std::string &name);


    /**
     * Creates a new label structure and inserts it into the vector of the bytecode labels of this call_context
     */
    bytecode_label provide_label();

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

public: /* Public members */

    /* the methods of this call context */
    std::vector<method*> methods;

    /* the type of the call context: 0 - global, 1 - named, 2 - chained*/
    CC_TYPE type;

    /* the name of the call context */
    std::string name;

    /* the (compiled) expressions in the call context */
    std::vector<expression_tree*> expressions;

    /* the father call context of this */
    call_context *father;

    /* if the call context is from a method this is that method */
    method *ccs_method;

    /* this is a vector of label locations for this*/
    std::vector<bytecode_label> labels;

    bytecode_label break_label;

    /* the classes that are defined in this call context */
    std::vector<class_declaration *> classes;

    nap_compiler* compiler;
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
    class_declaration (nap_compiler* _compiler, const std::string& name, call_context *pfather);

    class_declaration *parent_class;
    std::vector<class_declaration *> implemented_interfaces;
};

#endif

#ifndef _variable_H_
#define _variable_H_

#include "parser.h"
#include "common_structs.h"

/**
 * This structure defines a variable. The following things are characterizing a struct variable:
 * . their name
 * . their type as a string
 * . their dimension, meaning, they can be a vector, or a single struct variable
 * . their type as an int, from the famous BASIC_TYPE... list
 *
 * A variable can be bound together with a C/C++ variable of type long, double and with any dimensions
 * suitable. When binding with multi dimensions, each variable's number* to_interpret's location will
 * point to the specific index in the array passed in.
 */
struct variable
{

    /**
     * @brief variable represents a variable in the source code
     * @param pdimension - the number of dimensions this variable has
     * @param type - the type of the variable
     * @param pname - the name of the variable
     * @param pctype - the string type of the variable
     * @param pcc - the call context of the variable
     */
    variable(int pdimension, int type, const std::string& pname, const std::string& pctype, call_context* pcc)
        : name(pname), dimension(pdimension), c_type(pctype), cc(pcc)
    {
        i_type = type;
        mult_dim_def = NULL;
        func_par = NULL;
    }

    /* the name of the variable */
    std::string name;

    /* the size of the variable (ie. dimension) */
    int dimension;

    /* the type of the variable */
    std::string c_type;

    /* the type of the variable */
    int i_type;

    /* the dimension definition of this variable */
    multi_dimension_def *mult_dim_def;

    /* populated with NULL if this is not a function parameter, or the actual function parameter if it is */
    parameter *func_par;

    /* the call context in which this variable is to be found */
    call_context *cc;
};

#endif

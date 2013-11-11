#ifndef _HASH_H_
#define _HASH_H_

struct expr_tree_list;
struct variable;
struct method;
struct call_context;
struct expression_with_location;

#include <vector>
#include <string>
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
                                     const expression_with_location* expwloc);

/**
 * Checks if the variable named 's' is in the hash list 'first'
 * @param s - the name of the variable
 * @param first - the list we're searching
 */
std::vector<variable*>::const_iterator variable_list_has_variable(const char *s,
                                          const std::vector<variable *> &first);
 
/**
 * Creates a new string list from the instr which is separated by the given separator
 * @param instr - the input string
 * @param sep - the expected separator
 */
std::vector<std::string> string_list_create_bsep(const char* instr, char sep);

#endif

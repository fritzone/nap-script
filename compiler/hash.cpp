#include <string.h>
#include <stdlib.h>

#include "hash.h"
#include "type.h"
#include "consts.h"
#include "throw_error.h"
#include "utils.h"
#include "variable.h"
#include "call_ctx.h"

/**
 * This function adds a new  variable to the hashlist in first. Always adds the new  variable to the head of the list
 */
variable* variable_list_add_variable(const char *var_name, 
                                     const char* var_type, 
                                     int var_size, 
                                     std::vector<variable*>& first,
                                     method* the_method,  
                                     call_context* cc, 
                                     const expression_with_location* expwloc)
{
    if(!valid_variable_name(var_name))
    {
        throw_error("Invalid variable name", var_name, var_type);
    }
    int itype = get_typeid(var_type);

    if(itype == BASIC_TYPE_DONTCARE)
    {
        if(call_context_get_class_declaration(cc, var_type))
        {
            itype = BASIC_TYPE_USERDEF;
        }
    }
    if(itype == BASIC_TYPE_DONTCARE)
    {
        return 0;
    }
    
    variable* var = new variable(var_size, itype);

    var->name = duplicate_string(var_name);
    var->c_type = duplicate_string(var_type);
    var->cc = cc;

    var->dimension = var_size;

    /* now fix the stuff to include template parameters if any */

    variable_resolve_templates(var, the_method, cc, expwloc);

    first.push_back(var);
    return var;
}


/**
 * this function checks if s appears in the hashtable
 */
std::vector<variable*>::const_iterator variable_list_has_variable(const char *s, const std::vector<variable*>& first)
{
    std::vector<variable*>::const_iterator q = first.begin();
    while(q != first.end())
    {
        if(!strcmp(s, (*q)->name))
        {
            return q;
        }
        q ++;
    }
    return first.end();
}

/**
 * Creates a new string list from the instr which is separated by the given separator
 */
string_list* string_list_create_bsep(const char* instr, char sep)
{
    string_list* head = alloc_mem(string_list,1);
    string_list* q = head;
    string_list* tmp = NULL;
    const char* p = instr, *frst = instr;
    char* cur_elem = NULL;
    while(*p)
    {
        bool already_increased = false; /* when we load the expressions in parentheses we do an extra increment. This signals that this extra increment was done */
        if(C_SQPAR_OP == *p)    /* for now skip the things that are between index indicators*/
        {
        int can_stop = 0;
        int level = 0;
            p++;
            while(*p && !can_stop)
            {
                if(*p == C_SQPAR_CL && --level == -1) can_stop = 1;
                if(*p == C_SQPAR_OP) level ++;
                if(!can_stop) p++;
            }
            if(!*p)
            {
                throw_error(E0009_PARAMISM, instr, NULL);
            }
            p++;    /* to skip the last closing brace*/
            already_increased = true;
        }

        if(*p == C_PAR_OP)    /* for now skip the things that are between parantheses too ...*/
        {
            int can_stop = 0;
            int level = 0;
            p++;
            while(*p && !can_stop)
            {
                if(*p == C_PAR_CL && --level == -1) can_stop = 1;
                if(*p == C_PAR_OP) level ++;
                if(!can_stop) p++;
            }
            if(!*p)
            {
                throw_error(E0009_PARAMISM, instr, NULL);
            }
            p++;    /* to skip the last closing brace*/
            already_increased = true;
        }

        if(*p == '{' )    /* for now skip the things that are between curly braces too ...*/
        {
            int can_stop = 0;
            int level = 0;
            p++;
            while(*p && !can_stop)
            {
                if(*p == '}' && --level == -1) can_stop = 1;
                if(*p == '{' ) level ++;
                if(!can_stop) p++;
            }
            if(!*p)
            {
                throw_error(E0009_PARAMISM, instr, NULL);
            }
            p++;    /* to skip the last closing brace*/
            already_increased = true;
        }


        if(*p == C_QUOTE)    /* skip stuff between quotes*/
        {
        int can_stop = 0;
            p++;
            while(*p && !can_stop)
            {
                if(*p == C_QUOTE && *(p-1) != C_BACKSLASH) can_stop = 1;
                p++;
            }
        }

        if(*p == sep)
        {
            cur_elem = new_string(p - frst + 1);
            strncpy(cur_elem, frst, p - frst);
            q->str = duplicate_string(trim(cur_elem));
            q->len = strlen(q->str);
            tmp = alloc_mem(string_list,1);
            q->next = tmp;
            q = tmp;
            frst = p + 1;
        }
        if(! already_increased)
        {
            p++;
        }
    }
    /* and now the last element */
    cur_elem = new_string(p - frst + 1);
    strncpy(cur_elem, frst, p - frst);
    q->str = duplicate_string(trim(cur_elem));
    q->len = strlen(q->str);
    return head;
}

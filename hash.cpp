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
variable* variable_list_add_variable(const char *var_name, const char* var_type, int var_size, variable_list** first,  method* the_method,  call_context* cc, const expression_with_location* expwloc)
{
    if(!valid_variable_name(var_name))
    {
        throw_error("Invalid variable name", var_name, var_type, (void*)expwloc);
    }
int itype = get_typeid(var_type);
variable_list* tmp = alloc_mem(variable_list,1);
    if(!tmp)
    {
        throw_error("Memory eror");
    }
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
    
variable* var = new_variable(var_size, itype);

    var->name = duplicate_string(var_name);
    var->c_type = duplicate_string(var_type);

    var->dimension = var_size;
    variable_reset(var);

    /* now fix the stuff to include template parameters if any */

    variable_resolve_templates(var, the_method, cc, expwloc);

    tmp->next=*first;
    tmp->var = var;
    *first=tmp;
    return var;
}


/**
 * this function checks if s appears in the hashtable
 */
variable_list *variable_list_has_variable(const char *s, variable_list* first)
{
 variable_list *q=first;
    while(q && q->var)
    {
        if(!strcmp(s, q->var->name))
        {
            return q;
        }
        q=q->next;
    }
    return NULL;
}

/**
 * This function returns the idxth. parameter if any. idx must start from 1
 */
variable_list *variable_list_get_at(variable_list* first, int idx)
{
    if(idx <= 0)
    {
        return NULL;
    }
variable_list *q=first;
int i, tcc = 0;	/* total count of variables in this list */
    while(q)
    {
        tcc ++;
        q=q->next;
    }
    q = first;
    for(i=tcc; i!=idx && q->next && i > 1; i--)
    {
        if(i!=idx) q=q->next;
    }
    if(i < idx) return NULL;
    return q;
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
        if(C_SQPAR_OP == *p)	/* for now skip the things that are between index indicators*/
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
            p++;	/* to skip the last closing brace*/
        }

        if(*p == C_PAR_OP)	/* for now skip the things that are between parantheses too ...*/
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
            p++;	/* to skip the last closing brace*/
        }

        if(*p == '{' )	/* for now skip the things that are between curly braces too ...*/
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
            p++;	/* to skip the last closing brace*/
        }


        if(*p == C_QUOTE)	/* skip stuff between quotes*/
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
        p++;
    }
    /* and now the last element */
    cur_elem = new_string(p - frst + 1);
    strncpy(cur_elem, frst, p - frst);
    q->str = duplicate_string(trim(cur_elem));
    q->len = strlen(q->str);
    return head;
}

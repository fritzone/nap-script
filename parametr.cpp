#include "parametr.h"
#include "type.h"
#include "evaluate.h"
#include "utils.h"
#include "tree.h"
#include "variable.h"

#include <stdlib.h>
#include <string.h>

/**
 * Creates a new function parameter
 */
parameter* new_parameter()
{
parameter* tmp = alloc_mem(parameter,1);
	tmp->simple_value = 1;
	return tmp;
}

/**
 * Adds a new parameter to the given list
 */
void parameter_list_add_parameter( parameter_list** fpl,  parameter* par)
{
parameter_list* tmp = alloc_mem(parameter_list,1);
	tmp->param = par;
	if(NULL == *fpl)
	{
		*fpl = tmp;
		return;
	}
 parameter_list* q = *fpl;
	while(q->next) q=q->next;
	if(q!= *fpl)
	{
		q->param = par;
		q->next = alloc_mem(parameter_list,1);
	}
	else
	{
		(*fpl)->param = par;
		(*fpl)->next = alloc_mem(parameter_list,1);
	}
}

/**
 * Returns the value of the parameters with the given name
 */
envelope* parameter_list_get_named_parameters_value(const parameter_list* pl, const char* name)
{
	while(pl && pl->param && pl->param->name && strcmp(pl->param->name, name)) pl=pl->next;
	if(pl && pl->param)	return pl->param->value;
	return NULL;
}

envelope* parameter_list_get_named_parameters_initial_value(const parameter_list* pl, const char* name, const method* the_method, call_context* cc)
{
	while(pl && pl->param && pl->param->name && strcmp(pl->param->name, name)) pl=pl->next;
	//if(pl && pl->param && pl->param->initial_value)	return evaluate(pl->param->initial_value, the_method, cc);
	return NULL;
}

/**
 * Returns the basic type of the parameters with the given name
 */
int parameter_list_get_named_parameters_type(const parameter_list* pl, const char* name)
{
	while(pl && pl->param && pl->param->name && strcmp(pl->param->name, name)) pl=pl->next;
	if(pl && pl->param && pl->param->value)
	{
		switch(pl->param->value->type)
		{
		case BASIC_TYPE_INT:
		case BASIC_TYPE_STRING:
		case BASIC_TYPE_REAL:
			return pl->param->value->type;
		case BASIC_TYPE_VARIABLE:
			return variable_get_basic_type((variable*)pl->param->value->to_interpret);
			break;
		}
	}
	return 0;
}

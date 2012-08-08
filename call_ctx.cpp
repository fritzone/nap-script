#include "call_ctx.h"
#include "utils.h"
#include "throw_error.h"
#include "code_output.h"
#include "sys_brkp.h"
#include "type.h"
#include "preverify.h"
#include "consts.h"
#include "interpreter.h"
#include "evaluate.h"
#include "bt_string.h"
#include "method.h"
#include "variable.h"

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>

struct call_context* global_cc = NULL;

/**
 * Creates a new call cotnext object
 */
struct call_context* call_context_create(int type, const char* name, struct method* the_method, struct call_context* father)
{
struct call_context* cc = alloc_mem(call_context,1);
	cc->type = type;
	cc->name = duplicate_string(name);
	cc->ccs_method = the_method;
	cc->father = father;
	cc->labels = new vector<bytecode_label*>();
	return cc;
}

/**
 * Adds a method to the given call context
 */
method_list* call_context_add_method(call_context* cc,  method* the_method)
{
method_list* q = method_list_insert(&cc->methods, the_method);
	return q;
}

/**
 * Returns the variable from the call context if any, NULL if nothing found
 */
variable* call_context_get_variable(call_context* cc, const char* v_name)
{
variable_list* vl = variable_list_has_variable(v_name, cc->variables);
	if(vl)
	{
		return vl->var;
	}
	else
	{
		return NULL;
	}
}

//bool is_valid_variable_name

/**
 * Adds a variable to the call context
 */
variable* call_context_add_variable(call_context* cc, const char* name, const char* type, int dimension, const expression_with_location* expwloc)
{
	if(variable_list_has_variable(name, cc->variables))
	{
		throw_error(E0034_SYMBOLDEFD, NULL);
	}
	return variable_list_add_variable(name, type, dimension, &cc->variables, cc->ccs_method, cc, expwloc);
}

/**
 * Retrieves the method object for the given name from the cc
 */
method* call_context_get_method(call_context* cc, const char* name)
{
method_list* q = cc->methods;
	while(q)
	{
		if(!strcmp(q->the_method->name, name))
		{
			return q->the_method;
		}
		q=q->next;
	}
	if(cc->father)
	{
		return call_context_get_method(cc->father, name);
	}
	return NULL;
}

long call_context_add_label(struct call_context* cc, long position, const char* name)
{
bytecode_label* bl = alloc_mem(bytecode_label, 1);
	bl->bytecode_location = position;
	bl->name = duplicate_string(name);
	bl->type = 0;

	cc->labels->push_back(bl);
	return cc->labels->size();
}

struct bytecode_label* call_context_add_break_label(struct call_context* cc, long position, const char* name)
{
bytecode_label* bl = alloc_mem(bytecode_label, 1);
	bl->bytecode_location = position;
	bl->name = duplicate_string(name);
	bl->type = 1;
	cc->break_label = bl;
	cc->labels->push_back(bl);
	return bl;
}


/**
 * Adds a new compiled expression to the given call context
 */
void call_context_add_compiled_expression(struct call_context* the_cc, const struct expression_tree* co_expr, const char* expr)
{
expression_tree_list* tmp = expression_tree_list_add_new_expression(co_expr, &the_cc->expressions, expr);
	if(NULL == the_cc->expressions)
	{
		the_cc->expressions = tmp;
	}
}

/**
 * Adds a new expression to the method's list
 */
expression_tree* call_context_add_new_expression(call_context* the_cc, const char* expr, const expression_with_location* expwloc)
{
expression_tree* new_expression = new_expression_tree(expwloc);
char *t;
int res;
	t = prepare_expression(expr);
	build_expr_tree(t, new_expression, the_cc->ccs_method, t, the_cc, &res, expwloc);
	validate(new_expression);
	expression_tree_list_add_new_expression(new_expression, &the_cc->expressions, t);
	free(t);
	return new_expression;
}

/**
 * This runs the global context. Loads the environment variables (from OS), and starts executingthe commands
 * found outside the methods.
 * Run method main
 */
void call_context_compile(call_context* cc, char* envp[])
{
    {
    expression_tree_list* q = cc->expressions;
    while(q)
	{
		compile(q->root, NULL, cc, 0, -1, 0, MODE_ASM_OUTPUT);
		q=q->next;
	}
    }
    // and now the functions
    method_list* ccs_methods = cc->methods;
    while(ccs_methods)
    {
        printf(".%s:\n", ccs_methods->the_method->name);
        // now pop off the variables from the stack
        variable_list* vlist = ccs_methods->the_method->variables;
        int pctr = 0;
        while(vlist)
        {
            printf("peek%s(%i) %s\n", vlist->var->c_type, pctr++, vlist->var->name);
            vlist = vlist->next;
        }
        push_cc_start_marker(MODE_ASM_OUTPUT);
        expression_tree_list* q1 = ccs_methods->the_method->main_cc->expressions;
            while(q1)
            {
                compile(q1->root, ccs_methods->the_method,
                        ccs_methods->the_method->main_cc,
                        0, -1, 0, MODE_ASM_OUTPUT);
                q1=q1->next;
            }


        ccs_methods = ccs_methods->next;
        printf("ret\n");
    }

}

/**
* Runs the given call context. If it encounters a return statement it will return the envelope
* holding the result of the function
*/
void call_context_run_inner(call_context* cc, int level, int reqd_type, int forced_mov, int mode)
{
	expression_tree_list* q = cc->expressions;
	while(q)
	{
		compile(q->root, cc->ccs_method, cc, level, reqd_type, forced_mov, mode);
		q=q->next;
	}
}

/**
 * Runs the given call context. If it encounters a return statement it will return the envelope
 * holding the result of the function
 */
/*
envelope* call_context_run_complete(call_context* cc)
{
expression_tree_list* q = cc->expressions;
	while(q)
	{
		rst(q->root);
	 envelope* result_env = compile(q->root, cc->ccs_method, cc );
		if(result_env && RETURN_STATEMENT == result_env->type)
		{
			return (envelope*)result_env->to_interpret;
		}
		q=q->next;
	}
	return NULL;
}
*/

/**
 * Adds a child call context to this
 */
void call_context_add_child_call_context(call_context* father,  call_context* child)
{
call_context_list* tmp = alloc_mem(call_context_list,1), *q;
	tmp->ctx = child;
	if(NULL == father->child_call_contexts)
	{
		father->child_call_contexts = tmp;
		return;
	}
	q=father->child_call_contexts;
	while(q->next) q=q->next;
	q->next = tmp;
}

struct bytecode_label* call_context_provide_label(struct call_context* cc, int break_label)
{
	int maxlen = strlen(cc->name) + 32;
	char* label_name = alloc_mem(char, maxlen);
	snprintf(label_name, maxlen, "%s_%d", cc->name, (int)cc->labels->size());	/* generating a name for the end of the while */
	long idx = call_context_add_label(cc, -1, label_name);
	return cc->labels->at(idx - 1);
}

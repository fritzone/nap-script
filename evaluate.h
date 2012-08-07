#ifndef _EVALUATE_H_
#define _EVALUATE_H_

#include "bsd_enve.h"
//#include "method.h"
#include "call_ctx.h"

/**
 * This calculates the value of the expression, for defined values, and also numbers
 */ 
//envelope* evaluate(const expression_tree* node, const method* the_method, call_context* cc);

void compile(const expression_tree* node,
             const method* the_method,
             call_context* cc,
             int level,
             int reqd_type,
             int forced_mov,
             int mode);

void deliver_ccidx_dest(const expression_tree* node, int level, const method* the_method, call_context* cc, int reqd_type, int& idxc, const variable* var, int forced_mov, int mode);

char get_reg_type(int req_type);

#endif

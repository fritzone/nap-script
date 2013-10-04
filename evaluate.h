#ifndef _EVALUATE_H_
#define _EVALUATE_H_

#include "common_structs.h"
#include "call_ctx.h"

void compile(const expression_tree* node,
             const method* the_method,
             call_context* cc,
             int level,
             int reqd_type,
             int forced_mov);

void deliver_ccidx_dest(const expression_tree* node, int level, 
                        const method* the_method, call_context* cc, 
                        int reqd_type, int& idxc, const variable* var, 
                        int forced_mov);

char get_reg_type(int req_type);

#endif

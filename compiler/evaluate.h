#ifndef _EVALUATE_H_
#define _EVALUATE_H_

#include "common_structs.h"
#include "call_ctx.h"

void compile(nap_compiler* _compiler, const expression_tree* node,
             const method* the_method,
             call_context* cc,
             int level,
             int &reqd_type,
             int forced_mov);

const char *get_reg_type(int req_type);

#endif

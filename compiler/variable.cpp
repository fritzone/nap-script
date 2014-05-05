#include "variable.h"
#include "number.h"
#include "bt_string.h"
#include "type.h"
#include "method.h"
#include "interpreter.h"
#include "utils.h"
#include "sys_brkp.h"
#include "consts.h"
#include "expression_tree.h"
#include "parameter.h"
#include "compiler.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

/**
 * Creates a new multi-dimension index object
 * @return a new object, inited to 0
 */
multi_dimension_index* new_multi_dimension_index(const char* indx_id, const nap_compiler *_compiler)
{
multi_dimension_index* tmp = alloc_mem(multi_dimension_index,1, _compiler);
    tmp->id = _compiler->duplicate_string(indx_id);
    return tmp;
}

variable::variable(int pdimension, int type, const std::string &pname,
                   const std::string &pctype, call_context *pcc)
    : name(pname), dimension(pdimension), c_type(pctype), cc(pcc)
{
    multi_dim_count = 1;
    i_type = type;
    mult_dim_def = NULL;
    func_par = NULL;
    dynamic_dimension = 0;
}


#include "operations.h"
#include "number.h"
#include "variable.h"
#include "bt_string.h"
#include "indexed.h"
#include "consts.h"
#include "is.h"
#include "throw_error.h"
#include "bt_string.h"
#include "type.h"
#include "type.h"
#include "utils.h"
#include "listv.h"
#include "evaluate.h"
#include "code_stream.h"
#include "notimpl.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void do_list_assignment( envelope* rvalue, variable* var, int level, const method* the_method, call_context* cc, int reqd_type )
{
    struct listv* lst = (struct listv*)rvalue->to_interpret;
    int indxctr = 0;
    while(lst->next)
    {

        // now put the value of the list element to the next pointer

        if(((expression_tree*)lst->val->to_interpret)->op_type <= var->i_type)
        {
            code_stream() << "mov" << SPACE << "reg" << get_reg_type(var->i_type) << '(' << level << ')' << ',';
        }
        compile((expression_tree*)lst->val->to_interpret, the_method, cc, level + 1, reqd_type, 0);
        if(((expression_tree*)lst->val->to_interpret)->op_type <= var->i_type)
        {
            code_stream() << NEWLINE;
        }
        code_stream() << "mov" << SPACE << "reg" << "idx" << '(' << '0' << ')' << ',' << indxctr << NEWLINE;
        code_stream() << "mov" << SPACE << '@' << "ccidx" << '(' << var->name << ',' << '1' << ')' << ',' << "reg"
                      << get_reg_type(var->i_type) << '(' << level + 1 << ')' << NEWLINE ;
        code_stream() << "clidx" << NEWLINE;
        lst = lst->next;
        indxctr ++;
    }
}

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
            printf("mov reg%c(%d),", get_reg_type(var->i_type), level+1);
        }
        compile((expression_tree*)lst->val->to_interpret, the_method, cc, level + 1, reqd_type, 0, MODE_ASM_OUTPUT);
        if(((expression_tree*)lst->val->to_interpret)->op_type <= var->i_type)
        {
            printf("\n");
        }
        printf("mov regidx(0),%d\n",indxctr);
        printf("mov @ccidx(%s,1),reg%c(%i)\n", var->name, get_reg_type(var->i_type), level + 1);
        printf("clidx\n");
        lst = lst->next;
        indxctr ++;
    }
}

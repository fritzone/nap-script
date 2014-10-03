#include "opcodes.h"
#include "strtable.h"
#include "metatbl.h"
#include "jmptable.h"
#include "nbci.h"
#include "stack.h"

#include "nbci_impl.h"
#include "nap_consts.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

/******************************************************************************/
/*                             Debugging section                              */
/******************************************************************************/

void nap_vm_dump(struct nap_vm* vm, FILE *fp)
{
    uint64_t i;
    puts("");
    for(i=0; i<vm->meta_size; i++)
    {
        if(vm->metatable[i]->instantiation)
        {
            if(vm->metatable[i]->instantiation->value)
            {
                if(vm->metatable[i]->instantiation->type == STACK_ENTRY_INT)
                {
                    fprintf(fp, "E:[%s=%" PRINT_d "](%" PRINT_u "/%" PRINT_st ")\n",
                           vm->metatable[i]->name,
                           *(nap_int_t*)(vm->metatable[i]->instantiation->value)
                           ,i, vm->meta_size);
                }
                else
                if(vm->metatable[i]->instantiation->type == STACK_ENTRY_BYTE)
                {
                    fprintf(fp, "E:[%s=%d](%" PRINT_u "/%" PRINT_st ")\n",
                           vm->metatable[i]->name,
                           *(nap_byte_t*)(vm->metatable[i]->instantiation->value)
                           ,i, vm->meta_size);
                }
                else
                if(vm->metatable[i]->instantiation->type == STACK_ENTRY_STRING)
                {
                    fprintf(fp, "E:[%s=%s](%" PRINT_u "/%" PRINT_st ")\n",
                           vm->metatable[i]->name,
                           (char*)(vm->metatable[i]->instantiation->value)
                           ,i, vm->meta_size);
                }
                else
                if(vm->metatable[i]->instantiation->type == STACK_ENTRY_REAL)
                {
                    fprintf(fp, "E:[%s=%.5Lf](%" PRINT_u "/%" PRINT_st ")\n",
                           vm->metatable[i]->name,
                           *(nap_real_t*)(vm->metatable[i]->instantiation->value)
                           ,i, vm->meta_size);
                }
                else
                {
                    fprintf(fp, "X:[%s=%"PRINT_d"](%" PRINT_u "/%" PRINT_st ")\n",
                           vm->metatable[i]->name,
                           *(nap_int_t*)(vm->metatable[i]->instantiation->value)
                           ,i, vm->meta_size);

                }
            }
            else
            {
                fprintf(fp, "N:[%s=??](%" PRINT_u "/%" PRINT_st ")\n", vm->metatable[i]->name,
                       i, vm->meta_size);

            }
        }
        else
        {
            fprintf(fp, "?:[%s=??](%" PRINT_u "/%" PRINT_st ")\n", vm->metatable[i]->name,
                   i, vm->meta_size);
        }
    }
}

/* dumps the stack */
void dump_stack(struct nap_vm* vm, FILE *fp)
{
    int64_t tempst;
    uint8_t dim_ctr;
    nap_int_t value_ctr;
    fprintf(fp, "*** STACK DUMP ***\n");
    for(tempst = STACK_INIT; tempst > -1; tempst --)
    {
        if(vm->cec->stack[tempst])
        {
            if(vm->cec->stack_pointer == tempst)
            {
                fprintf(fp, ">");
            }
            else
            {
                fprintf(fp, " ");
            }


            fprintf(fp, "[%.5"PRINT_d"]", tempst);
            if(vm->cec->stack[tempst]->stored)
            {
                fprintf(fp, "*");
            }
            else
            {
                fprintf(fp, " ");
            }
            fprintf(fp, "%8s", nap_get_type_description(vm->cec->stack[tempst]->type));
            if(vm->cec->stack[tempst]->var_def)
            {
                fprintf(fp, "(%s", vm->cec->stack[tempst]->var_def->name);
                if(vm->cec->stack[tempst]->var_def->dimension_count >= 1)
                {
                    fprintf(fp, "[");
                    for(dim_ctr = 0; dim_ctr<vm->cec->stack[tempst]->var_def->dimension_count; dim_ctr ++)
                    {
                        if(dim_ctr == 0) /* will print oly the first few value */
                        {
                            fprintf(fp, "%"PRINT_d":{", vm->cec->stack[tempst]->var_def->dimensions[dim_ctr]);
                            for(value_ctr = 0; value_ctr < MIN(10, vm->cec->stack[tempst]->var_def->dimensions[dim_ctr]); value_ctr ++)
                            {
                                fprintf(fp, "%"PRINT_d, ((nap_int_t*)vm->cec->stack[tempst]->value)[value_ctr]);
                                if(value_ctr < vm->cec->stack[tempst]->var_def->dimensions[dim_ctr] - 1)
                                {
                                    fprintf(fp, ",");
                                }
                            }
                            fprintf(fp, "}");
                        }
                        if(dim_ctr < vm->cec->stack[tempst]->var_def->dimension_count - 1)
                        {
                            fprintf(fp, ",");
                        }
                    }
                    fprintf(fp, "]");
                }
                else
                {
                    fprintf(fp, "=%"PRINT_d, ((nap_int_t*)vm->cec->stack[tempst]->value)[0]);
                }
                fprintf(fp, ")");
            }

            switch(vm->cec->stack[tempst]->type)
            {
            case STACK_ENTRY_MARKER_NAME:
                fprintf(fp, ":[%.10d]",*(nap_mark_t*)(vm->cec->stack[tempst]->value));
                break;
            default:
                break;
            }
            fprintf(fp, "\n");

        }
    }
}


/******************************************************************************/
/*                             Access section                                 */
/******************************************************************************/

char *nap_vm_get_string(struct nap_vm* vm, char* name, int* found)
{
    uint64_t i;
    char* finame = name;
	char error[256] = {0};
    if(name == NULL)
    {
        *found = 0;
        return NULL;
    }

    for(i=0; i<vm->meta_size; i++)
    {
        if(vm->metatable[i]->instantiation)
        {
            if(vm->metatable[i]->instantiation->value)
            {
                if(vm->metatable[i]->instantiation->type == STACK_ENTRY_STRING)
                {
                    if(vm->metatable[i]->name && !strcmp(vm->metatable[i]->name, finame))
                    {
						size_t dest_len = 0, real_len = 0;
                        char* result = NULL;
                        if(finame != name)
                        {
                            NAP_MEM_FREE(finame);
                        }
                        *found = 1;
                        dest_len = vm->metatable[i]->instantiation->len;
                        result = convert_string_from_bytecode_file(
                                    vm, (char*)(vm->metatable[i]->instantiation->value),
                                    vm->metatable[i]->instantiation->len * CC_MUL,
                                    dest_len,
                                    &real_len);
                        if(result == NULL)
                        {
                            *found = 0; /* The vm already has the error */
                            return NULL;
                        }
                        else
                        {
                            return result;
                        }
                    }
                }
            }
        }
    }

    SNPRINTF(error, MAX_BUF_SIZE(255), "Not found the variable: [%s]", name);
    nap_vm_set_error_description(vm, error);
    *found = 0;
    return NULL;
}

nap_int_t nap_vm_get_int(struct nap_vm* vm, char* name, int* found)
{
    size_t i;
    char* finame = name;

    if(name == NULL)
    {
        *found = 0;
        return NAP_NO_VALUE;
    }

    for(i=0; i<vm->meta_size; i++)
    {
        if(vm->metatable[i]->instantiation)
        {
            if(vm->metatable[i]->instantiation->value)
            {
                if(vm->metatable[i]->instantiation->type == STACK_ENTRY_INT)
                {
                    if(vm->metatable[i]->name && !strcmp(vm->metatable[i]->name, finame))
                    {
                        if(finame != name)
                        {
                            NAP_MEM_FREE(finame);
                        }
                        *found = 1;
                        return *(nap_int_t*)(vm->metatable[i]->instantiation->value);
                    }
                }
            }
        }
    }
    *found = 0;
    return NAP_NO_VALUE;
}

nap_real_t nap_vm_get_real(struct nap_vm* vm, char* name, int* found)
{
    size_t i;
    char* finame = name;

    if(name == NULL)
    {
        *found = 0;
        return NAP_NO_VALUE;
    }

    for(i=0; i<vm->meta_size; i++)
    {
        if(vm->metatable[i]->instantiation)
        {
            if(vm->metatable[i]->instantiation->value)
            {
                if(vm->metatable[i]->instantiation->type == STACK_ENTRY_REAL)
                {
                    if(vm->metatable[i]->name && !strcmp(vm->metatable[i]->name, finame))
                    {
                        if(finame != name)
                        {
                            NAP_MEM_FREE(finame);
                        }
                        *found = 1;
                        return *(nap_real_t*)(vm->metatable[i]->instantiation->value);
                    }
                }
            }
        }
    }
    *found = 0;
    return NAP_NO_VALUE;
}

nap_byte_t nap_vm_get_byte(struct nap_vm* vm, char* name, int *found)
{
    size_t i;
    char* finame = name;

    if(name == NULL)
    {
        *found = 0;
        return NAP_NO_VALUE;
    }

    for(i=0; i<vm->meta_size; i++)
    {
        if(vm->metatable[i]->instantiation)
        {
            if(vm->metatable[i]->instantiation->value)
            {
                if(vm->metatable[i]->instantiation->type == STACK_ENTRY_BYTE)
                {
                    if(vm->metatable[i]->name && !strcmp(vm->metatable[i]->name, finame))
                    {
                        if(finame != name)
                        {
                            NAP_MEM_FREE(finame);
                        }
                        *found = 1;
                        return *(nap_byte_t*)(vm->metatable[i]->instantiation->value);
                    }
                }
            }
        }
    }
    *found = 0;
    return NAP_NO_VALUE;
}

/******************************************************************************/
/*                             Execute section                                */
/******************************************************************************/

void nap_vm_run(struct nap_vm* vm)
{
    while(nap_ip(vm) < vm->meta_location)
    {
        vm->cec->current_opcode = vm->content[nap_ip(vm)];
        vm->cec->lia = nap_ip(vm);
        nap_step_ip(vm);

        if(vm->opcode_handlers[vm->cec->current_opcode] != 0)
        {
            TRY_CALL(vm->opcode_handlers[vm->cec->current_opcode],
                    vm->opcode_error_codes[vm->cec->current_opcode]);
        }
        else
        if(vm->cec->current_opcode == OPCODE_EXIT) /* quit the application ... */
        {
            return;
        }
        else
        if(vm->cec->current_opcode == OPCODE_CLBF) /* clear last boolean flag */
        {
            vm->cec->lbf = UNDECIDED;
        }
        else
        {
            fprintf(stderr, "invalid opcode [%x] at %"PRINT_u" (%"PRINT_x")\n",
                    vm->cec->current_opcode, nap_ip(vm) - 1, nap_ip(vm) - 1);
            nap_vm_cleanup(vm);
            exit(5);
        }
    }
}

/* global since we want to have it on the stack in case of memory allocation issues */
static char error[256] = {0};
int nap_vm_set_error_description(struct nap_vm* vm, const char *error_desc)
{

    strncpy(error, error_desc, 256);
    vm->error_description = error;

    return NAP_FAILURE;
}

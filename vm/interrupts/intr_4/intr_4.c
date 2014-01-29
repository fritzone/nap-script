#include "intr_4.h"
#include "nbci.h"
#include "nap_consts.h"
#include "nbci_impl.h"

#include "nap_ext_def.h"

#include <string.h>
#include <stdlib.h>

#include <dlfcn.h>

void extfun(nap_int_t a, nap_int_t b)
{
    printf("YOO %"PRINT_d", %"PRINT_d"\n", a, b);
}

void populate_par_desc(struct nap_ext_par_desc* pd, int index, void* new_data)
{
    if(index == 0)
    {
        pd->p0 = new_data;
    }
    if(index == 1)
    {
        pd->p1 = new_data;
    }
}

/*
 * regs 0 -> teh signature, first character is the return type
 * regs 1 -> the name of the function
 * regs 2 -> the name of the library (- if none)
 * regi 0 -> the index in the pregenerated table
 */
uint8_t intr_4(struct nap_vm* vm)
{
    size_t sig_dest_len = vm->regslens[0] * CC_MUL, sig_real_len = 0;
    char* signature = convert_string_from_bytecode_file(vm->regs[0],
            vm->regslens[0] * CC_MUL, sig_dest_len, &sig_real_len);
    size_t fun_dest_len = vm->regslens[1] * CC_MUL, fun_real_len = 0;
    char* function_name = convert_string_from_bytecode_file(vm->regs[1],
            vm->regslens[1] * CC_MUL, fun_dest_len, &fun_real_len);
    size_t lib_dest_len = vm->regslens[2] * CC_MUL, lib_real_len = 0;
    char* library_name = convert_string_from_bytecode_file(vm->regs[2],
            vm->regslens[2] * CC_MUL, lib_dest_len, &lib_real_len);
    size_t i;

    static char ext_init = 0;
    if(ext_init == 0)
    {
        nap_int_init_ext_func_array();
        ext_init = 1;
    }

    /* parameters peeked from the first one */
    int cur_stack_peeker = 0;
    struct nap_ext_par_desc* pd = (struct nap_ext_par_desc*)(calloc(1, sizeof(struct nap_ext_par_desc)));
    for(i=1; i<strlen(signature); i++)
    {
        if(signature[i] == 'i') // popping in integer variable
        {
            int64_t* temp = (int64_t*)calloc(1, sizeof(int64_t));
            *temp = *(int64_t*)vm->stack[vm->stack_pointer - cur_stack_peeker]->value; /* STACK VALUE FROM peek_index */
            populate_par_desc(pd, strlen(signature) - cur_stack_peeker - 1 - 1, temp);
            /* first -1: because we remvoe the return type,
               second -1: because it's zero based */
        }
        else
        {
            _NOT_IMPLEMENTED
        }

        cur_stack_peeker ++;
    }

    /* going to the generated file */
    nap_ext_caller nec = ext_callers[vm->regi[0] ];

    void* lib_handle = dlopen(library_name[0] == '-'? NULL : library_name,
                              RTLD_LAZY);
    if(!lib_handle)
    {
        return CANNOT_LOAD_LIBRARY;
    }
    dlerror();

    void* function_to_call = dlsym(lib_handle, function_name);
    char* error;
    if ((error = dlerror()) != NULL)
    {
         fprintf(stderr, "%s\n", error);
         dlclose(lib_handle);
         return CANNOT_LOAD_FUNCTION;
     }

    nec(function_to_call, pd, 0);

    /* load the library (if any) fetch the address of the function */
    return 0;
}

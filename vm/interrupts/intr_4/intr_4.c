#include "intr_4.h"
#include "nbci.h"
#include "nap_consts.h"
#include "nbci_impl.h"

#include "nap_ext_def.h"

#include <string.h>
#include <stdlib.h>

#ifndef _WINDOWS
#include <dlfcn.h>
#else
#include <windows.h>
#include <strsafe.h>
#pragma warning(disable: 4054)
#endif
/*
 * regs 0 -> the signature, first character is the return type
 * regs 1 -> the name of the function
 * regs 2 -> the name of the library (- if none)
 * regi 0 -> the index in the pregenerated table
 */
uint16_t intr_4(struct nap_vm* vm)
{

    size_t sig_dest_len = nap_regs(vm, 0)->l * CC_MUL, sig_real_len = 0;
    size_t fun_dest_len = nap_regs(vm, 1)->l * CC_MUL, fun_real_len = 0;
    size_t lib_dest_len = nap_regs(vm, 2)->l * CC_MUL, lib_real_len = 0;
    size_t i = 0;
    nap_ext_caller nec = NULL;
    int cur_stack_peeker = 0; /* parameters peeked from the first one */
    static char ext_init = 0; /* flag for checking the init state of the external functions table*/
    struct nap_ext_par_desc* pd = NULL; /* Parameter descriptor */

    char* signature = NULL;
    char* function_name = NULL;
    char* library_name = NULL;

    signature = convert_string_from_bytecode_file(vm, nap_regs(vm, 0)->s,
            sig_dest_len, sig_dest_len, &sig_real_len);
    function_name = convert_string_from_bytecode_file(vm, nap_regs(vm, 1)->s,
            fun_dest_len, fun_dest_len, &fun_real_len);
    library_name = convert_string_from_bytecode_file(vm, nap_regs(vm, 2)->s,
            lib_dest_len, lib_dest_len, &lib_real_len);

    pd = NAP_MEM_ALLOC(1, struct nap_ext_par_desc);
    NAP_NN_ASSERT(vm, pd);

    /* one time job, initialize the external functions array */
	if(ext_init == 0)
    {
        nap_int_init_ext_func_array();
        ext_init = 1;
    }
    nec = ext_callers[ nap_regi(vm, 0) ];

    /* first character (index 0) is the return type */
    for(i=1; i<strlen(signature); i++)
    {
        if(signature[i] == TYPE_INT) /* popping in integer variable */
        {
            nap_int_t* temp = NAP_MEM_ALLOC(1, nap_int_t);
			NAP_NN_ASSERT(vm, temp);

            *temp = *(nap_int_t*)vm->cec->stack[nap_sp(vm) - cur_stack_peeker]->value; /* STACK VALUE FROM peek_index */
            nap_populate_par_desc(pd, strlen(signature) - cur_stack_peeker - 1 - 1, temp);
            /* first -1: because we remove the return type,
               second -1: because it's zero based */
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }

        cur_stack_peeker ++;
    }

    /* And now load the library (if any) fetch the address of the function */

#ifndef _WINDOWS
    /* if the name starts with '-' we use the current module otherwise an
     * external library */
    void* lib_handle = dlopen(library_name[0] == '-'? NULL : library_name,
                              RTLD_LAZY);
    if(!lib_handle)
    {
         free(library_name);
         free(signature);
         free(function_name);
         nap_free_parameter_descriptor(&pd);
		 
         return INTR_4_CANNOT_LOAD_LIBRARY;
    }

    void* function_to_call = dlsym(lib_handle, function_name);
    char* error;
    if ((error = dlerror()) != NULL)
    {
         fprintf(stderr, "%s\n", error);
         dlclose(lib_handle);

         free(library_name);
         free(signature);
         free(function_name);
         nap_free_parameter_descriptor(&pd);

         return INTR_4_CANNOT_LOAD_FUNCTION;
     }

    /* going to the generated file */
    nec(function_to_call, pd, 0);

    int t = dlclose(lib_handle);
    fprintf(stderr, "DC:%d\n", t);
#else
	{
		FARPROC function_to_call = NULL;
		HMODULE hLib = library_name[0] == '-'?GetModuleHandle(NULL) : LoadLibrary(library_name);
		if(hLib == NULL)
		{
			free(library_name);
			free(signature);
			free(function_name);
			nap_free_parameter_descriptor(&pd);
            return INTR_4_CANNOT_LOAD_LIBRARY;
		}

		function_to_call = GetProcAddress(hLib, function_name);

		if(function_to_call == NULL)
		{
			free(library_name);
			free(signature);
			free(function_name);
			nap_free_parameter_descriptor(&pd);
            return INTR_4_CANNOT_LOAD_FUNCTION;
		}

		nec((LPVOID)function_to_call, pd, 0);
	}
#endif

    free(library_name);
    free(signature);
    free(function_name);

    nap_free_parameter_descriptor(&pd);
    return NAP_SUCCESS;
}

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
void ErrorExit(LPTSTR lpszFunction) 
{ 
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw); 
}

#endif

/*
 * regs 0 -> the signature, first character is the return type
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
	int cur_stack_peeker = 0; /* parameters peeked from the first one */
    nap_ext_caller nec;
    static char ext_init = 0;

    struct nap_ext_par_desc* pd = NAP_MEM_ALLOC(1, struct nap_ext_par_desc);
    NAP_NN_ASSERT(vm, pd);

    /* one time job, initialize the external functions array */
	if(ext_init == 0)
    {
        nap_int_init_ext_func_array();
        ext_init = 1;
    }
    nec = ext_callers[ vm->regi[0] ];

    /* first character (index 0) is the return type */
    for(i=1; i<strlen(signature); i++)
    {
        if(signature[i] == TYPE_INT) /* popping in integer variable */
        {
            nap_int_t* temp = NAP_MEM_ALLOC(1, nap_int_t);
            *temp = *(nap_int_t*)vm->stack[vm->stack_pointer - cur_stack_peeker]->value; /* STACK VALUE FROM peek_index */
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
		 
		 return CANNOT_LOAD_LIBRARY;
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

         return CANNOT_LOAD_FUNCTION;
     }

    /* going to the generated file */
    nec(function_to_call, pd, 0);

    dlclose(lib_handle);
#else
	{
		void* function_to_call = NULL;
		HANDLE hLib = library_name[0] == '-'?GetModuleHandle(NULL) : LoadLibrary(library_name);
		if(hLib == NULL)
		{
			free(library_name);
			free(signature);
			free(function_name);
			nap_free_parameter_descriptor(&pd);
			fprintf(stderr, "\nCannot load library\n");
			return CANNOT_LOAD_LIBRARY;
		}

		function_to_call = (void*)GetProcAddress(hLib, function_name);

		if(function_to_call == NULL)
		{
			free(library_name);
			free(signature);
			free(function_name);
			nap_free_parameter_descriptor(&pd);
			fprintf(stderr, "\nCannot load function\n");
			ErrorExit("intr_4");
			return CANNOT_LOAD_FUNCTION;
		}

		nec(function_to_call, pd, 0);
	}
#endif

    free(library_name);
    free(signature);
    free(function_name);

    nap_free_parameter_descriptor(&pd);
    return 0;
}

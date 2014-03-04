#include "nap_runtime.h"

extern "C"
{
#include "nbci.h"
#include "nap_structs.h"
#include "funtable.h"
}

#include "garbage_bin.h"
#include "compiler.h"
#include "opcodes.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <string>
#include <sstream>

/* The nap runtime environment. There can be multiple runtimes */
struct nap_runtime
{
    nap_runtime() : compiler(nap_compiler::create_compiler()), vm(0), chunks(),
        last_error("[NAP_OK] no error"), name("")
    {}

    // the compiler of the runtime
    std::auto_ptr<nap_compiler> compiler;

    // the virtual machine
    nap_vm* vm;

    // the list of chunks that were compiled
    std::vector<nap_bytecode_chunk*> chunks;

    // the last error
    std::string last_error;

    std::string name;
};

NAP_LIB_API nap_runtime* nap_runtime_create(const char* name)
{
    static size_t runtime_counter = 0;

    struct nap_runtime* result = new nap_runtime;
    if(name)
    {
        result->name = name;
    }
    else
    {
        std::stringstream ss;
        ss << ++runtime_counter;
        result->name = std::string("naprt_") + ss.str();
    }

    return result;
}

NAP_LIB_API nap_bytecode_chunk* nap_runtime_compile(struct nap_runtime* runtime,
                                                    const char* commands,
                                                    const char *name)
{
    if(runtime != NULL)
    {
        bool success = true;
        bool source_set = runtime->compiler->set_source(commands, success);

        if(source_set)
        {
            if(runtime->compiler->compile())
            {
                nap_bytecode_chunk* chunk = (struct nap_bytecode_chunk*)
                                       calloc(sizeof(struct nap_bytecode_chunk), 1);
                runtime->compiler->deliver_bytecode(chunk->code, chunk->length);
                runtime->chunks.push_back(chunk);
                if(name != NULL)
                {
                    size_t len = strlen(name);
                    chunk->name = (char*)(calloc(len, sizeof(char)));
                    strncpy(chunk->name, name, len);
                }
                return chunk;
            }
            else
            {
                runtime->last_error = runtime->compiler->get_error();
                return NULL;
            }
        }
        else
        {
            runtime->last_error = runtime->compiler->get_error();
            return NULL;
        }
    }
    return NULL;
}

NAP_LIB_API int nap_runtime_execute(struct nap_runtime* runtime,
                        struct nap_bytecode_chunk* bytecode)
{
    if(runtime != NULL)
    {
        runtime->vm = nap_vm_inject(bytecode->code, bytecode->length, EMBEDDED);
        nap_vm_run(runtime->vm);

        return NAP_EXECUTE_SUCCESS;
    }
    else
    {
        return NAP_EXECUTE_FAILURE;
    }
}

NAP_LIB_API nap_byte_t nap_runtime_get_byte(struct nap_runtime* runtime,
                                 const char* variable_name, int *found)
{
    if(runtime != NULL)
    {
        char* t = (char*)calloc(strlen(variable_name) + 1, 1);
        strcpy(t, variable_name);
        nap_byte_t temp = nap_vm_get_byte(runtime->vm, t, found);
        free(t);
        return temp;
    }
    else
    {
        *found = NAP_VARIABLE_NOT_FOUND;
        return NAP_NO_VALUE;
    }
}

NAP_LIB_API nap_int_t nap_runtime_get_int(struct nap_runtime* runtime,
                                 const char* variable_name, int *found)
{
    if(runtime != NULL)
    {
        char* t = (char*)calloc(strlen(variable_name) + 1, 1);
        strcpy(t, variable_name);
        nap_int_t temp = nap_vm_get_int(runtime->vm, t, found);
        free(t);
        return temp;
    }
    else
    {
        *found = NAP_VARIABLE_NOT_FOUND;
        return NAP_NO_VALUE;
    }
}



NAP_LIB_API nap_real_t nap_runtime_get_real(nap_runtime * /*runtime*/,
                                const char * /*variable_name*/)
{
    return 0;
}


NAP_LIB_API nap_string_t nap_runtime_get_string(nap_runtime* runtime,
                                    const char* variable_name, int *found)
{
    if(runtime != NULL)
    {
        char* t = (char*)calloc(strlen(variable_name) + 1, 1);
        strcpy(t, variable_name);
        char* temp = nap_vm_get_string(runtime->vm, t, found);
        free(t);
        return temp;
    }
    else
    {
        *found = NAP_VARIABLE_NOT_FOUND;
        return NULL;
    }
}


NAP_LIB_API void nap_runtime_shutdown(nap_runtime **runtime)
{
    if(!runtime)
    {
        return;
    }
    if(!*runtime)
    {
        return;
    }

    nap_compiler::release_compiler((*runtime)->compiler);
    if((*runtime)->vm)
    {
        nap_vm_cleanup((*runtime)->vm);
    }
    for(size_t i=0; i<(*runtime)->chunks.size(); i++)
    {
        free( (*runtime)->chunks.at(i)->code);
        if((*runtime)->chunks.at(i)->name)
        {
            free((*runtime)->chunks.at(i)->name);
        }

        free( (*runtime)->chunks.at(i));
    }
    delete (*runtime);
    *runtime = 0;
    garbage_bin_bin::shutdown();
}


int nap_execute_method(nap_runtime *runtime, void *return_value, const char *method_name, ...)
{
    if(runtime != NULL)
    {
        funtable_entry* fe = nap_vm_get_method(runtime->vm, method_name);
        if(!fe)
        {
            return NAP_EXECUTE_FAILURE;
        }

        va_list argptr;
        std::string method_cmd = method_name;
        method_cmd += "(";
        va_start(argptr, method_name);
        for(int i=0; i<fe->parameter_count; i++)
        {
            if(fe->parameter_types[i] == OPCODE_INT)
            {
                nap_int_t v = va_arg(argptr, nap_int_t);
                std::stringstream ss;
                ss << v;
                method_cmd += ss.str();
            }

            if(fe->parameter_types[i] == OPCODE_BYTE)
            {
                nap_byte_t v = (nap_byte_t)va_arg(argptr, int);
                std::stringstream ss;
                ss << v;
                method_cmd += ss.str();
            }

            if(fe->parameter_types[i] == OPCODE_FLOAT)
            {
                nap_real_t v = va_arg(argptr, nap_real_t);
                std::stringstream ss;
                ss << v;
                method_cmd += ss.str();
            }

            if(fe->parameter_types[i] == OPCODE_STRING)
            {
                nap_string_t v = va_arg(argptr, nap_string_t);
                std::stringstream ss;
                ss << v;
                method_cmd += ss.str();
            }

            if(i < fe->parameter_count - 1)
            {
                method_cmd += ",";
            }
        }
        // build up a string, which is just a method call
        va_end(argptr);
        method_cmd += ")";
        const char* t = method_cmd.c_str();
        int ex_c = nap_execute_code(runtime, t);
        if(ex_c == NAP_EXECUTE_FAILURE)
        {
            return NAP_EXECUTE_FAILURE;
        }

        if(return_value != 0 && fe->return_type != 0)
        {
            if(fe->return_type == OPCODE_INT)
            {
                (*(nap_int_t*)return_value) = runtime->vm->rvi;
            }
            else
            if(fe->return_type == OPCODE_BYTE)
            {
                (*(nap_byte_t*)return_value) = runtime->vm->rvb;
            }

        }
    }
    else
    {
        return NAP_EXECUTE_FAILURE;
    }

}


int nap_execute_code(nap_runtime *runtime, const char *script)
{
    if(runtime != NULL)
    {
        int temp = 1;

        // create a compiler
        std::auto_ptr<nap_compiler> compiler = nap_compiler::create_compiler();
        compiler->set_vmchain(runtime->vm);
        bool success = true;

        bool source_set = compiler->set_source(script, success);

        if(!source_set)
        {
            nap_compiler::release_compiler(compiler);
            return NAP_EXECUTE_FAILURE;
        }

        // compile the call command
        if(compiler->compile())
        {
            nap_bytecode_chunk* chunk = (struct nap_bytecode_chunk*)
                                   calloc(sizeof(struct nap_bytecode_chunk), 1);
            compiler->deliver_bytecode(chunk->code, chunk->length);

            if(runtime->vm->chunk_counter + 1 > runtime->vm->allocated_chunks)
            {
                runtime->vm->allocated_chunks *= 2;
                runtime->vm->btyecode_chunks = (struct nap_bytecode_chunk**)realloc(
                            runtime->vm->btyecode_chunks,
                            runtime->vm->allocated_chunks * sizeof(struct nap_bytecode_chunk*));

                /* TODO: is this NULL? */
            }

            runtime->vm->btyecode_chunks[runtime->vm->chunk_counter] = chunk;
            runtime->vm->chunk_counter ++;
        }
        else
        {
            nap_compiler::release_compiler(compiler);
            return NAP_EXECUTE_FAILURE;
        }

        garbage_bin_bin::instance().empty(compiler.get());
        nap_compiler::release_compiler(compiler);
        runtime->vm->regi[0] = runtime->vm->chunk_counter - 1 ;

        // create a new VM to call the method
        struct nap_vm* child_vm = NULL;
        child_vm = nap_vm_inject(runtime->vm->btyecode_chunks[runtime->vm->regi[0]]->code,
                runtime->vm->btyecode_chunks[runtime->vm->regi[0]]->length, INTERRUPT);


        if(child_vm == NULL)
        {
            nap_compiler::release_compiler(compiler);
            return NAP_EXECUTE_FAILURE;
        }

        /* no errors, run the new VM  */
        child_vm->parent = runtime->vm;
        nap_vm_run(child_vm);
        if(child_vm->error_code != 0) /* any errors? */
        {
            /* take them over here */
            runtime->vm->error_code = child_vm->error_code;
            runtime->vm->error_message = child_vm->error_message;

            /* cleanup */
            nap_vm_cleanup(child_vm);
            nap_compiler::release_compiler(compiler);

            return NAP_EXECUTE_FAILURE;
        }

        /* just fetch the return values of child_vm into runtime->vm, someone
         * might use them */
        runtime->vm->rvb = child_vm->rvb;
        runtime->vm->rvi = child_vm->rvi;
        runtime->vm->rvl = child_vm->rvl;
        if(runtime->vm->rvl)
        {
            runtime->vm->rvs = (char*)calloc(runtime->vm->rvl + 1, sizeof(char));
            memcpy(runtime->vm->rvs, child_vm->rvs, runtime->vm->rvl);
        }

        /* performs the cleanup */
        nap_vm_cleanup(child_vm);
        nap_compiler::release_compiler(compiler);

        return temp;
    }
    else
    {
        return NAP_EXECUTE_FAILURE;
    }
}


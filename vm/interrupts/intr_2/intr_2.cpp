#include "intr_2.h"
#include "nbci.h"
#include "garbage_bin.h"
#include "compiler.h"

class gb_releaser
{
public:
    ~gb_releaser()
    {
        garbage_bin_bin::shutdown();
    }
};

static gb_releaser gbr;

uint8_t intr_2(struct nap_vm* vm)
{
    std::cerr << "INTR2_STARTS" << std::endl;

    std::auto_ptr<nap_compiler> compiler = nap_compiler::create_compiler();
    compiler->set_vmchain(vm);
    bool success = true;
    bool source_set = compiler->set_source(vm->regs[0], success);
    if(!source_set)
    {
        nap_compiler::release_compiler(compiler);
        garbage_bin_bin::release();

        return CANNOT_SET_SOURCE;
    }

    if(compiler->compile())
    {
        nap_bytecode_chunk* chunk = (struct nap_bytecode_chunk*)
                               calloc(sizeof(struct nap_bytecode_chunk), 1);
        compiler->deliver_bytecode(chunk->code, chunk->length);

        if(vm->chunk_counter + 1 > vm->allocated_chunks)
        {
            vm->allocated_chunks *= 2;
            vm->btyecode_chunks = (struct nap_bytecode_chunk**)realloc(
                        vm->btyecode_chunks,
                        vm->allocated_chunks * sizeof(struct nap_bytecode_chunk*));

            /* TODO: is this NULL? */
        }

        vm->btyecode_chunks[vm->chunk_counter] = chunk;
        vm->chunk_counter ++;
    }
    else
    {
        nap_compiler::release_compiler(compiler);
        garbage_bin_bin::release();

        return CANNOT_COMPILE_SOURCE;
    }

    std::cerr << "INTR2_RELEASE" << std::endl;

    garbage_bin_bin::instance().empty(compiler.get());
    nap_compiler::release_compiler(compiler);
    vm->regi[0] = vm->chunk_counter - 1 ;

    std::cerr << "INTR2_ENDS" << std::endl;

    return 0;
}

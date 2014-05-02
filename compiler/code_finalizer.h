#ifndef CODE_FINALIZER_H
#define CODE_FINALIZER_H

#include "file_abstraction.h"

class nap_compiler;

class code_finalizer
{
public:
    code_finalizer(nap_compiler* _compiler) : mcompiler(_compiler), f(mcompiler)
    {}

    void finalize();

private:

    void finalize_metatable();
    void finalize_strtable();
    void finalize_jumptable();
    void finalize_funtable();
    void finalize_locations();

    nap_compiler* mcompiler;

    uint32_t meta_location ;
    uint32_t strtable_location ;
    uint32_t jumptable_location ;
    uint32_t funtable_location ;
    file_abstraction f;

    uint32_t jumptable_count;
};


#endif // CODE_FINALIZER_H

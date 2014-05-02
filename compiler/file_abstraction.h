#ifndef FILE_ABSTRACTION_H
#define FILE_ABSTRACTION_H

#include <stdint.h>
#include <stdlib.h>

class nap_compiler;

class file_abstraction
{
public:

    file_abstraction(nap_compiler* _compiler);

    void write_stuff_to_file_64(uint64_t stuff, int pos = -1);

    void write_stuff_to_file_32(uint32_t stuff, int pos = -1);

    void write_stuff_to_file_16(uint16_t stuff, int pos = -1);

    void write_stuff_to_file_8(uint8_t stuff, int pos = -1);

    size_t write_string_to_file(const char* s, int cnt, int needs_conv, int pos = -1);

    uint32_t ftell();

private:

    nap_compiler* mcompiler;
};


#endif // FILE_ABSTRACTION_H

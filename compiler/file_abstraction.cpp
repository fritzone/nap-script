#include "file_abstraction.h"
#include "compiler.h"
#include "charconverter.h"

extern "C"
{
#include "byte_order.h"
}

file_abstraction::file_abstraction(nap_compiler* _compiler) : mcompiler(_compiler)
{
}

void file_abstraction::write_stuff_to_file_64(uint64_t stuff, int pos)
{
    uint64_t network_order_stuff = htovm_64(stuff);
    mcompiler->place_bytes(pos, &network_order_stuff, 8);
}

void file_abstraction::write_stuff_to_file_32(uint32_t stuff, int pos)
{
    uint32_t network_order = htovm_32(stuff);
    mcompiler->place_bytes(pos, &network_order, 4);
}

void file_abstraction::write_stuff_to_file_16(uint16_t stuff, int pos)
{
    uint16_t network_order = htovm_16(stuff);
    mcompiler->place_bytes(pos, &network_order, 2);
}

void file_abstraction::write_stuff_to_file_8(uint8_t stuff, int pos)
{
    uint8_t netw = stuff;
    mcompiler->place_bytes(pos, &netw, 1);
}

size_t file_abstraction::write_string_to_file(const char* s, int cnt, int needs_conv, int pos)
{
    // now convert the bytes to wchar_t
    if(needs_conv)
    {
        size_t used_len = cnt;
        char* t = to_nap_format(s, cnt, &used_len);
        if(t == 0)
        {
            mcompiler->place_bytes(pos, s, cnt);
        }
        else
        {
            mcompiler->place_bytes(pos, t, used_len);
            free(t);
            return used_len;
        }
    }
    else
    {
        mcompiler->place_bytes(pos, s, cnt);
    }

    return cnt;
}

uint32_t file_abstraction::ftell()
{
    return mcompiler->last_bc_pos();
}


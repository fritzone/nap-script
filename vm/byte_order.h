#ifndef BYTE_ORDER_H
#define BYTE_ORDER_H

#include <stdint.h>

typedef union{
    unsigned char c[4];
    unsigned short s[2];
    uint32_t l;
}U4;



typedef union{
    unsigned char c[8];
    unsigned char c2[2][4];
    unsigned short s[4];
    unsigned long l[2];
    unsigned long long ll;
}U8;


typedef union{
    unsigned char c[2];
    unsigned short s;
}U2;

uint16_t htovm_16(uint16_t s);

uint32_t htovm_32(uint32_t l);

uint64_t htovm_64(uint64_t ll);

#endif

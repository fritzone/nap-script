#ifndef BYTE_ORDER_H
#define BYTE_ORDER_H

#include <stdint.h>

//the same for 4 bytes
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

//you can use the standard htons or this
uint16_t htovm_16(uint16_t s);

//you can use the standard htonl or this
uint32_t htovm_32(uint32_t l);

//for int64 you can use the int64 and do the same, or you can to do it with 2*4 like i did
//you can give a void pointer as well..
uint64_t htovm_64(uint64_t ll);

#endif // BYTE_ORDER_H


#include "byte_order.h"

//you can use the standard htonl or this
uint32_t htovm_32(uint32_t l)
{
    U4 mask,res;
    unsigned char* p = (unsigned char*)&l;
    mask.l = 0x00010203;
    res.c[mask.c[0]] = p[0];
    res.c[mask.c[1]] = p[1];
    res.c[mask.c[2]] = p[2];
    res.c[mask.c[3]] = p[3];
    return res.l;
}

//for int64 you can use the int64 and do the same, or you can to do it with 2*4 like i did
//you can give a void pointer as well..
uint64_t htovm_64(uint64_t ll)//void htonll(void* arg, void* result)
{
    U2 mask1;
    U4 mask2;
    U8 res;

    unsigned char* p = (unsigned char*)&ll; //or (unsigned char*)arg
    mask1.s = 0x0001;
    mask2.l = 0x00010203;
    //I didn't use the int64 for convertion
    res.c2[mask1.c[0]][mask2.c[0]] = p[0];
    res.c2[mask1.c[0]][mask2.c[1]] = p[1];
    res.c2[mask1.c[0]][mask2.c[2]] = p[2];
    res.c2[mask1.c[0]][mask2.c[3]] = p[3];
    res.c2[mask1.c[1]][mask2.c[0]] = p[4];
    res.c2[mask1.c[1]][mask2.c[1]] = p[5];
    res.c2[mask1.c[1]][mask2.c[2]] = p[6];
    res.c2[mask1.c[1]][mask2.c[3]] = p[7];

    //memcpy(result,res.c,8);
    return res.ll;
}

//you can use the standard htons or this
uint16_t htovm_16(uint16_t s)
{
    U2 mask,res;
    unsigned char* p = (unsigned char*)&s;
    mask.s = 0x0001;
    res.c[mask.c[0]] = p[0];
    res.c[mask.c[1]] = p[1];
    return res.s;
}

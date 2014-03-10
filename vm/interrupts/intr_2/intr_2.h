#ifndef _INTR_2_H_
#define _INTR_2_H_

#ifdef __cplusplus
extern "C" {
#endif

#include  <stdint.h>

static const int CANNOT_SET_SOURCE = 1;
static const int CANNOT_COMPILE_SOURCE = 2;

struct nap_vm;

uint8_t intr_2(struct nap_vm*);

#ifdef __cplusplus
}
#endif

#endif

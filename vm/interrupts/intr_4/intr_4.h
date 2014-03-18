#ifndef _INTR_4_H_
#define _INTR_4_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct nap_vm;

static const int CANNOT_LOAD_LIBRARY = 1;
static const int CANNOT_LOAD_FUNCTION = 2;

static const char TYPE_INT = 'i';

uint8_t intr_4(struct nap_vm*);

#ifdef __cplusplus
}
#endif

#endif


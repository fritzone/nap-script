#ifndef _INTR_3_H_
#define _INTR_3_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct nap_vm;

static const int INTR_3_CANNOT_CREATE_VM = 3;
static const int INTR_3_COULD_NOT_RUN_CODE = 2;

uint8_t intr_3(struct nap_vm*);

#ifdef __cplusplus
}
#endif

#endif


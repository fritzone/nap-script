#ifndef CHARCONVERTER_H
#define CHARCONVERTER_H

#include <stddef.h>

#include "nap_consts.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * will convert the input text (system encoding, usually UTF-8) in the
 * nap format of UTF-32BE. Returns a string in UTF-32BE format, the user
 * must free it. The returned length = 4 * in_len
 *
 * @param in - the system string
 * @param in_len - the length of the string
 * @param used_len - the length of the outgoing string
 *
 * @return the UTF32-BE representation of the same string
 **/
char* to_nap_format(const char* in, size_t in_len, size_t *used_len);

#ifdef __cplusplus
}
#endif

#endif // CHARCONVERTER_H

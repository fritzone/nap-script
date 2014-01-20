#ifndef CHARCONVERTER_H
#define CHARCONVERTER_H

#include <stddef.h>

#include "nap_consts.h"

// will convert the input text (system encoding, usually UTF-8) in the
// nap format of UTF-32BE. Returns a string in UTF-32BE format, the user
// must free it. The returned length = 4 * in_len
char* to_nap_format(const char* in, size_t in_len, size_t &used_len);

#endif // CHARCONVERTER_H

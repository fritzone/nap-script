#ifndef _BT_STRING_H_
#define _BT_STRING_H_

#include "consts.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>

/**
 * The String Basic Type
 */
struct bt_string
{
    bt_string(const char* src);
    ~bt_string();

    char *m_the_string;
};

#endif

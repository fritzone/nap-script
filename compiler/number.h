#ifndef _NUMBER_H_
#define _NUMBER_H_

#include "type.h"
#include "utils.h"
#include "garbage_bin.h"

#include <stdlib.h>
#include <string.h>

/**
 * Represents a  number.
 */
class number
{
public:

    number(const char* src)
    {
        int type = number_get_type(src);
        if (BASIC_TYPE_INT == type)
        {
            number_from_long(atol(src));
        }
        else
        if (BASIC_TYPE_REAL == type)
        {
            number_from_double(atof(src));
        }
    }

    int type() const
    {
        return m_type;
    }

    void* location() const
    {
        return m_location;
    }

private:

    void number_from_long(long src)
    {
        long *new_long = new long;
        *new_long = src;
        m_type = BASIC_TYPE_INT;
        m_location = new_long;

        garbage_bin<long*>::instance().place(new_long);

    }

    void number_from_double(double src)
    {
        double *new_double = new double;
        *new_double = src;
        m_type = BASIC_TYPE_REAL;
        m_location = new_double;
        garbage_bin<double*>::instance().place(new_double);
    }

    /**
     * Returns the type of the given string as a  number... or at least tries to guess
     */
    int number_get_type(const char *src)
    {
        int i = 0;
        int len = strlen(src);
        if (!isnumber(src))
        {
            return 0;
        }
        while (i < len)
        {
            if (src[i] == '.')
            {
                return BASIC_TYPE_REAL;
            }
            i++;
        }

        return BASIC_TYPE_INT;
    }

    /* type of the struct number */
    int m_type;

    /* the location of the number */
    void *m_location;
};

#endif

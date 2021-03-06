#ifndef _NUMBER_H_
#define _NUMBER_H_

#include "type.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

/**
 * Represents a  number.
 */
class number
{
public:

    number(const std::string& src)
    {
        int type = number_get_type(src);
        if (BASIC_TYPE_INT == type)
        {
            number_from_long(atol(src.c_str()));
        }
        else
        if (BASIC_TYPE_REAL == type)
        {
            number_from_double(atof(src.c_str()));
        }
    }

    ~number()
    {
        free(m_location);
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
        long *new_long = (long*)calloc(1, sizeof(long));
		if(new_long == NULL)
		{
			return;
		}
        *new_long = src;
        m_type = BASIC_TYPE_INT;
        m_location = new_long;
    }

    void number_from_double(double src)
    {
        double *new_double = (double*)calloc(1, sizeof(double));
		if(new_double == NULL)
		{
			return;
		}
        *new_double = src;
        m_type = BASIC_TYPE_REAL;
        m_location = new_double;
    }

    /* type of the struct number */
    int m_type;

    /* the location of the number */
    void *m_location;
};

#endif

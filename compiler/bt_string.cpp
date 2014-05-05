#include "bt_string.h"

bt_string::bt_string(const char *src)
{
    int len = strlen(src);
    char *tmp = (char*)calloc(len + 1, sizeof(char));

    // skips the backslashed characters.
    for(int i=0, j=0; i<len; i++)
    {
        if(src[i] != '\\')
        {
            tmp[j++] = src[i];
        }
        else
        {
            if(i<len-1)
            {
                tmp[j++] = src[++i];
            }
        }
    }

    int len2 = strlen(tmp);
    m_the_string = new char[len2 + 1];
    memset(m_the_string, 0, len2 + 1);
    strncpy(m_the_string, tmp, len2);

    len = len2;
    free (tmp);
}

bt_string::~bt_string()
{
    delete [] m_the_string;
}

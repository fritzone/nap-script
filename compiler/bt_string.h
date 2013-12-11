#ifndef _BT_STRING_H_
#define _BT_STRING_H_

#include "consts.h"

#include <string.h>
#include <stdlib.h>

/**
 * The String Basic Type
 */
struct bt_string
{
public:
    bt_string(const char* src)
    {
        int len = strlen(src);
        char *tmp = (char*)calloc(len + 1, sizeof(char));

        // skips the backslahsed characters.
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

    const char* the_string() const
    {
        return m_the_string;
    }

    /**
     * @brief is_string returns true if the string is enclosed in quotes
     * @param expr_trim
     * @param expr_len
     * @return
     */
    static int is_string(const char* expr_trim, int expr_len)
    {
        return is_enclosed_string(expr_trim, expr_len, C_QUOTE, C_QUOTE);
    }

    /**
     * @brief is_statement_string returns true if the strign is enclosed in bacquotes
     * @param expr_trim
     * @param expr_len
     * @return
     */
    static int is_statement_string(const char* expr_trim, int expr_len)
    {
        return is_enclosed_string(expr_trim, expr_len, C_BACKQUOTE, C_BACKQUOTE);
    }

private:

    static int is_enclosed_string(const char* expr_trim, int expr_len, char encls, char encle)
    {
        if(expr_trim[0] == encls)    /* we can check them ... */
        {
            int i=1;
            while(i<expr_len)
            {
                if(expr_trim[i] == encle && expr_trim[i-1] != '\\' && i < expr_len - 1)
                {
                    return 0; /* meaning: enclosing character found before end of expression */
                }
                i++;
            }

            return 1;
        }
        else
        {
            return 0;
        }
    }

    /* this is the actual string */
    char *m_the_string;

    /* the length of the string */
    int len;
};

#endif

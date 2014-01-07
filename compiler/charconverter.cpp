#include "charconverter.h"
#include <iconv.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <errno.h>
#include <string.h>

#include <locale>

char* to_nap_format(const char* in, size_t in_len, size_t& used_len)
{
    std::locale s("");
    char* locn = (char*)calloc(s.name().length() + 1, sizeof(char));
    strcpy(locn, s.name().c_str());
    const char* enc = strchr(locn, '.') + 1;
    iconv_t foo = iconv_open("UTF-32BE", enc);

    if(*(reinterpret_cast<int*>(foo)) == -1)
    {
        if (errno == EINVAL)
        {
            fprintf(stderr, "Conversion from %s is not supported\n", enc);
        }
        else
        {
            fprintf(stderr, "Initialization failure:\n");
        }
        free(locn);
        return 0;
    }

    size_t out_len = 4 * in_len;
    size_t saved_in_len = in_len;
    iconv(foo, NULL, NULL, NULL, NULL);
    char* converted = (char*)calloc(out_len, sizeof(char));
    char *converted_start = converted;
    char* t = const_cast<char*>(in);
    int ret = iconv(foo,
                    &t,
                    &in_len,
                    &converted,
                    &out_len);
    iconv_close(foo);
    used_len = 4 * saved_in_len - out_len;

    if(ret == -1)
    {
        switch(errno)
        {
        case EILSEQ:
            fprintf(stderr,  "EILSEQ\n");
            break;
        case EINVAL:
            fprintf(stderr,  "EINVAL\n");
            break;
        }

        perror("iconv");
        free(locn);
        return 0;
    }
    else
    {
        free(locn);
        return converted_start;
    }
}

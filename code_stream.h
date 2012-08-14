#ifndef CODE_STREAM_H
#define CODE_STREAM_H

static const char SPACE = ' ';
static const char NEWLINE = '\n';

class code_stream
{
public:

    code_stream& operator << (const char* s)
    {
        printf("%s", s);
        return *this;
    }

    code_stream& operator << (char c)
    {
        printf("%c", c);
        return *this;
    }

    code_stream& operator << (int i)
    {
        printf("%i", i);
        return *this;
    }

    code_stream& operator << (double d)
    {
        printf("%f", d);
        return *this;
    }

    code_stream& operator << (long l)
    {
        printf("%li", l);
        return *this;
    }
};



#endif // CODE_STREAM_H

#ifndef CODE_STREAM_H
#define CODE_STREAM_H

#include <stdio.h>

static const char SPACE = ' ';
static const char* NEWLINE = "NEWLINE";

class code_stream
{
public:

    code_stream& operator << (const char* s)
    {
        output_bytecode(s);;
        return *this;
    }

    code_stream& operator << (char c)
    {
        char s[10];
        sprintf(s, "%c", c);
        output_bytecode(s);;
        return *this;
    }

    code_stream& operator << (int i)
    {
        char s[32];
        sprintf(s, "%i", i);
        output_bytecode(s);
        return *this;
    }

    code_stream& operator << (double d)
    {
        char s[32];
        sprintf(s, "%f", d);
        output_bytecode(s);
        return *this;
    }

    code_stream& operator << (long l)
    {
        char s[32];
        sprintf(s, "%li", l);
        output_bytecode(s);
        return *this;
    }
    
private:
    
    void output_bytecode(const char* s);
    
    static char last_opcode;
        
};

#endif // CODE_STREAM_H

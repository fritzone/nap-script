#ifndef CODE_STREAM_H
#define CODE_STREAM_H

#include <stdio.h>
#include <string>
#include <stdint.h>

class nap_compiler;

static const char SPACE = ' ';
extern const char* NEWLINE;

class code_stream
{
public:

    code_stream(nap_compiler* _compiler) : mcompiler(_compiler) {}

    code_stream& operator << (const char* s)
    {
        output_bytecode(s);;
        return *this;
    }
    
    code_stream& operator << (const std::string s)
    {
        output_bytecode(s.c_str());
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

    code_stream& operator << (uint32_t i)
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

    static int max_mark_index;

private:
    
    void output_bytecode(const char* s);

    nap_compiler* mcompiler;

};

#endif // CODE_STREAM_H

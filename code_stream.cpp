#include "code_stream.h"

#include <string>
#include <fstream>
#include <ios>

char code_stream::last_opcode = 0;

    
void code_stream::output_bytecode(const char* s)
{
    printf("%s", s);
    std::string expr = s;
    
    std::ofstream f("test.ncb", std::ios::out | std::ios::binary | std::fstream::app);
    
    char opcode = 0;
    // and now a long list of btyecodes
    
    if(expr == "push") opcode = 0xBC;
    if(expr == "ref")  opcode = 0xEF;
    if(expr == "int")  opcode = 0x01;
    if(expr == "float")  opcode = 0x02;
    if(expr == "string")  opcode = 0x03;
    
    if(opcode)
    {
        f.write(&opcode, sizeof(char));
        last_opcode = opcode;
    }
    else
    {
    }
}
#include "opcodes.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define STACK_INIT   4096                     /* initially 4096 entries  */

/**
 * Structure representing a stack entry. 
 **/
struct stack_entry
{
    /* the type of the entry 0 - int, 1 - real, 2 - string, 3 - reference*/
    uint8_t type;
    
    /* the value of it. At this address usually there is an uint64_t, a souble
       or a char*, or an object descriptor */
    void* value;
};

/**
 * Structure containing the definition of a variable
 **/
struct variable_entry
{
    /* the index of the variable */
    uint32_t index;
    
    /* the name of the variable */
    unsigned char* name;
};

static struct stack_entry** stack = NULL;    /* in this stack */
static uint64_t stack_size = STACK_INIT;         /* initial stack size */
static uint64_t stack_pointer = 0;                /* the stack pointer */
static uint8_t * content = 0;                    /* the content of the file */         
static uint64_t cc = 0;                          /* the instruction pointer */
static struct variable_entry** variables = NULL; /* the variables */
static uint32_t var_size = 0;

/**
 * Read the variables from the given file
 **/
static void read_variables(FILE* fp, uint32_t meta_location)
{
    fseek(fp, meta_location + 5, SEEK_SET); /* skip the ".meta"*/
    int can_go = 1;
    while(can_go)
    {
        uint32_t index = 0;
        fread(&index, sizeof(uint32_t), 1, fp);
        if(index == 1920234286) /* ".str" */
        {
            can_go = 0;
            break;
        }
        else
        {
            uint16_t len = 0;
            fread(&len, sizeof(uint16_t), 1, fp);
            unsigned char* name = (unsigned char*)calloc(sizeof(unsigned char), 
                                                        len + 1);
            fread(name, sizeof(unsigned char), len, fp);
            if(var_size < index + 1)
            {
                variables = (struct variable_entry**) realloc(variables, 
                                   sizeof(struct variable_entry**)*(index + 1));
            }
            struct variable_entry* new_var = (struct variable_entry*)
                                    calloc(sizeof(struct variable_entry), 1);
            new_var->index = index;
            new_var->name = name;
            variables[index] = new_var;
        }
    }
}

int main()
{
    FILE* fp = fopen("test.ncb", "rb");
    if(!fp) exit(1);
    fseek(fp, 0, SEEK_END);
    
    /* read in all the data in memory. Should be faster */
    uint64_t fsize = ftell(fp);
    content = (uint8_t *) calloc(sizeof(uint8_t ), fsize);
    fseek(fp, 0, SEEK_SET);
    fread(content, sizeof(uint8_t ), fsize, fp);
    
    fseek(fp, 0, SEEK_SET);
    
    uint32_t meta_location = 0;
    uint32_t stringtable_location = 0;
    uint32_t jumptable_location = 0;
    
    /* create the stack */
    stack = (struct stack_entry**) calloc( 
                                        sizeof(struct stack_entry*), stack_size
                                      );
    
    /* read in the important addresses from the bytecode file*/
    fread(&meta_location, sizeof(uint32_t), 1, fp);
    fread(&stringtable_location, sizeof(uint32_t), 1, fp);
    fread(&jumptable_location, sizeof(uint32_t), 1, fp);
    
    read_variables(fp, meta_location);
    
    /* done with the file */
    fclose(fp);
    
    /* cc is the instruction pointer */
    cc = 3 * sizeof(uint32_t);
    
    /* and start interpreting the code */
    while(1 && cc < fsize)
    {
        uint8_t current_opcde = content[cc];
        cc ++;

        /* is this a push operation? */
        if(current_opcde == OPCODE_PUSH)
        {
            struct stack_entry* se = (struct stack_entry*)(
                                        calloc(sizeof(struct stack_entry), 1));
            uint8_t push_what = content[cc ++];
            se->type = content[cc ++];

            if(push_what == OPCODE_VAR)
            {
                uint32_t* p_var_index = (uint32_t*)(content + cc);
                cc += sizeof(uint32_t);
                se->value = variables[*p_var_index];                
            }
                
            stack[stack_pointer ++ ] = se;
            
            
        }
    }
    
    return 0;
}

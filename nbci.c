#include "opcodes.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

/******************************************************************************/
/*                             Registers section                              */
/******************************************************************************/

int64_t regi[256] = {0}; /* the integer registers */
uint8_t lbf = 0;         /* the last boolean flag of the machine */

/******************************************************************************/
/*                             Stack section                                  */
/******************************************************************************/

#define STACK_INIT   4096                     /* initially 4096 entries  */

typedef enum TStackEntryType
{
    STACK_ENTRY_INT = 1,
    STACK_ENTRY_REAL = 2,
    STACK_ENTRY_STRING = 3,
    STACK_ENTRY_REF = 4,
    STACK_ENTRY_MARKER = 5,
    STACK_ENTRY_IMMEDIATE_INT = 6,
    STACK_ENTRY_MARKER_NAME = 7
} StackEntryType;

/**
 * Structure representing a stack entry. 
 **/
struct stack_entry
{
    /* the type of the entry 0 - int, 1 - real, 2 - string, 3 - reference*/
    StackEntryType type;
    
    /* the value of it. At this address usually there is:
     * 1. for imemdiate values:
     *     - an uint64_t ,
     *     - a double
     *     - a char*
     *     - or an object descriptor
     * 2. for variables:
     *     - a variable entry object
     */
    void* value;
};

/* variables for the stack */
static struct stack_entry** stack = NULL;           /* in this stack */
static uint64_t stack_size = STACK_INIT;            /* initial stack size */
static uint64_t stack_pointer = 0;                  /* the stack pointer */

/* generic variables regarding teh file */
static uint8_t* content = 0;                       /* the content of the file */
static uint8_t file_bitsize = 0;                   /* the bit size: 0x32, 0x64*/

/* variables regarding the execution flow */
static uint64_t cc = 0;                            /* the instruction pointer */
static uint64_t call_frames[STACK_INIT] = {0};     /* the jump back points */
static uint32_t cfsize = 0;

/******************************************************************************/
/*                             Metatable section                              */
/******************************************************************************/

/**
 * Structure containing the definition of a variable
 **/
struct variable_entry
{
    /* the index of the variable */
    uint32_t index;

    /* the name of the variable */
    uint8_t* name;

    /* the actual value of the variable */
    struct stack_entry* instantiation;
};

/* variables for the meta table*/
static struct variable_entry** metatable = NULL;    /* the variables */
static uint64_t meta_size = 0;           /* the size of  the variables vector */

/*
 * Read the metatable of the bytecode file. Exits on error.
 */
static void read_metatable(FILE* fp, uint64_t meta_location)
{
    fseek(fp, meta_location + 5, SEEK_SET); /* skip the ".meta" */
    uint32_t count = 0;
    fread(&count, sizeof(uint32_t), 1, fp);
    if(count == 0)
    {
        return;
    }
    meta_size = count;
    metatable = (struct variable_entry**) calloc(meta_size + 1,
                                               sizeof(struct variable_entry**));
    while(1)
    {
        uint32_t index = 0;
        fread(&index, file_bitsize, 1, fp);
        int end_of_file = feof(fp);
        if(index == 1920234286 || end_of_file) /* ".str" */
        {
            if(end_of_file)
            {
                fprintf(stderr, "wrong content: stringtable not found\n");
                exit(2);
            }
            break;
        }
        else
        {
            uint16_t len = 0;
            fread(&len, sizeof(uint16_t), 1, fp);
            uint8_t* name = (uint8_t*)calloc(sizeof(uint8_t), len + 1);
            fread(name, sizeof(uint8_t), len, fp);
            if(meta_size < index + 1)
            { /* seems there is something wrong with the metatable size*/
                metatable = (struct variable_entry**) realloc(metatable,
                                   sizeof(struct variable_entry**)*(index + 1));
            }
            struct variable_entry* new_var = (struct variable_entry*)
                                    calloc(1, sizeof(struct variable_entry));
            new_var->index = index;
            new_var->name = name;
            new_var->instantiation = 0;
            metatable[index] = new_var;
        }
    }
}

/******************************************************************************/
/*                             Stringtable section                            */
/******************************************************************************/

/**
 * Structure for holding a stringtable entry.
 */
struct strtable_entry
{
    /* the index of the string as referred in the btyecode */
    uint64_t index;

    /* the string itself */
    uint8_t* string;
};
/* variables for the stringtable */
static struct strtable_entry** stringtable = NULL;  /* the stringtable itself */
static uint64_t strt_size = 0;                 /* the size of the stringtable */

/*
 * Read the stringtable of the bytecode file. Exits on failure.
 */
static void read_stringtable(FILE* fp, uint64_t stringtable_location)
{
    fseek(fp, stringtable_location + 4, SEEK_SET); /* skip the ".str" */
    uint32_t count = 0;
    fread(&count, sizeof(uint32_t), 1, fp);
    if(count == 0)
    {
        return;
    }
    strt_size = count;
    stringtable = (struct strtable_entry**) calloc(strt_size + 1,
                                               sizeof(struct strtable_entry**));
    while(1)
    {
        uint32_t index = 0;
        fread(&index, file_bitsize, 1, fp);
        int end_of_file = feof(fp);

        if(index == 1886218798 || end_of_file) /* .jmp */
        {
            if(end_of_file)
            {
                fprintf(stderr, "wrong content: jumptable not found\n");
                exit(1);
            }
            break;
        }
        else
        {
            uint32_t len = 0;
            fread(&len, sizeof(uint32_t), 1, fp);
            uint8_t* str = (uint8_t*)calloc(sizeof(uint8_t), len + 1);
            fread(str, sizeof(uint8_t), len, fp);
            if(strt_size < index + 1)
            {
                stringtable = (struct strtable_entry**) realloc(stringtable,
                                 sizeof(struct strtable_entry**) * (index + 1));
            }
            struct strtable_entry* new_strentry = (struct strtable_entry*)
                                       calloc(1, sizeof(struct strtable_entry));
            new_strentry->index = index;
            new_strentry->string = str;
            stringtable[index] = new_strentry;
        }
    }
}

/******************************************************************************/
/*                             Jumptable section                              */
/******************************************************************************/

/*
 * Structure representing an entry in the jumptable
 */
struct jumptable_entry
{
    /* the location in the bytecode stream of the jump destination */
    uint32_t location;
};

/* variables for the jumptable */
static struct jumptable_entry** jumptable = NULL;   /* the jumptable itself */
static uint64_t jumptable_size = 0;              /* the size of the jumptable */
static uint jmpc = 0;               /* counts the jumptable entries on loading*/
/*
 * Read the stringtable of the bytecode file. Exits on failure.
 */
static void read_jumptable(FILE* fp, uint64_t stringtable_location)
{
    fseek(fp, stringtable_location + 4, SEEK_SET); /* skip the ".str" */
    uint32_t count = 0;
    fread(&count, sizeof(uint32_t), 1, fp);
    if(count == 0)
    {
        return;
    }
    jumptable_size = count;
    jumptable = (struct jumptable_entry**) calloc(jumptable_size + 1,
                                               sizeof(struct jumptable_entry**));
    while(1)
    {
        uint32_t index = 0;
        fread(&index, sizeof(uint32_t), 1, fp);
        int end_of_file = feof(fp);
        if(end_of_file)
        {
            return;
        }


        struct jumptable_entry* new_jmpentry = (struct jumptable_entry*)
                calloc(1, sizeof(struct jumptable_entry));
        new_jmpentry->location = index;
        jumptable[ jmpc++ ] = new_jmpentry;
    }
}

/******************************************************************************/
/*                             Other functions section                        */
/******************************************************************************/

/**
 * Sets the last boolean flag according to the operation found int current_opcode
 * @param reg - the registers value to check
 * @param immediate - against this value
 * @param current_opcode - the operation which is supposed to be executed
 */
void set_lbf_to_op_result(int64_t reg, int64_t immediate, uint8_t opcode)
{
    if(opcode == OPCODE_EQ)  lbf = (reg == immediate);
    if(opcode == OPCODE_NEQ) lbf = (reg != immediate);
    if(opcode == OPCODE_LT)  lbf = (reg <  immediate);
    if(opcode == OPCODE_GT)  lbf = (reg >  immediate);
    if(opcode == OPCODE_LTE) lbf = (reg <= immediate);
    if(opcode == OPCODE_GTE) lbf = (reg >= immediate);
}

/*
 * Main entry point
 */
int main()
{
    FILE* fp = fopen("test.ncb", "rb");
    if(!fp) exit(1);
    fseek(fp, 0, SEEK_END);
    
    /* read in all the data in memory. Should be faster */
    uint64_t fsize = ftell(fp);
    content = (uint8_t *) calloc(sizeof(uint8_t), fsize);
    fseek(fp, 0, SEEK_SET);
    fread(content, sizeof(uint8_t ), fsize, fp);
    
    fseek(fp, 0, SEEK_SET);
    
    uint32_t meta_location = 0;
    uint32_t stringtable_location = 0;
    uint32_t jumptable_location = 0;

    uint8_t type = 0;
    /* create the stack */
    stack = (struct stack_entry**) calloc( 
                                        sizeof(struct stack_entry*), stack_size
                                      );
    
    /* the format of the addresses in the file 32 or 64 bit addresses */
    fread(&type, sizeof(uint8_t), 1, fp);
    if(type == 0x32)
    {
        file_bitsize = sizeof(uint32_t);
    }
    else
    {
        file_bitsize = sizeof(uint64_t);
    }

    /* read in the important addresses from the bytecode file*/
    fread(&meta_location, file_bitsize, 1, fp);
    fread(&stringtable_location, file_bitsize, 1, fp);
    fread(&jumptable_location, file_bitsize, 1, fp);
    
    /* prepare the meta table of the application */
    read_metatable(fp, meta_location);

    /* read the stringtable */
    read_stringtable(fp, stringtable_location);

    /* read the jumptable */
    read_jumptable(fp, jumptable_location);
    
    /* done with the file */
    fclose(fp);
    
    /* cc is the instruction pointer: skip the 3 addresses and the startbyte */
    cc = 3 * file_bitsize + 1;
    
    /* and start interpreting the code */
    while(cc < meta_location)
    {
        uint8_t current_opcode = content[cc];
        cc ++;

        /* is this a PUSH operation? */
        if(current_opcode == OPCODE_PUSH)
        {
            struct stack_entry* se = (struct stack_entry*)(
                                        calloc(sizeof(struct stack_entry), 1));
            se->type = content[cc ++];

            if(se->type == OPCODE_INT)
            {
                uint8_t push_what = content[cc ++];

                if(push_what == OPCODE_VAR)
                {
                    uint32_t* p_var_index = (uint32_t*)(content + cc);
                    cc += sizeof(uint32_t);
                    struct variable_entry* ve = metatable[*p_var_index];
                    ve->instantiation = (struct stack_entry*)(
                                calloc(sizeof(struct stack_entry), 1));
                    ve->instantiation->type = se->type;
                    if(se->type == OPCODE_INT)
                    {
                        int64_t* temp = (int64_t*)calloc(1, sizeof(int64_t));
                        *temp = 0;
                        ve->instantiation->value = temp;
                    }
                    else
                    {
                        /* TODO */
                    }

                    /* setting the value of the stack entry */
                    se->value = ve;
                }
                else
                {
                    fprintf(stderr, "unimplemented push target 0x%x", push_what);
                    exit(66);
                }
            }
            else
            if(se->type == OPCODE_REG) /* pushing a register */
            {
                uint8_t reg_type = content[cc ++];
                uint8_t reg_idx = content[cc ++];

                if(reg_type == OPCODE_INT) /* pushing an int register */
                {
                    int64_t* temp = (int64_t*)calloc(1, sizeof(int64_t));
                    *temp = regi[reg_idx];

                    /* setting the value of the stack entry */
                    se->value = temp;
                }
                else
                {
                    /* TODO */
                }
            }
            else
            {
                fprintf(stderr, "invalid push destination: [0x%x]\n", se->type);
                exit(10);
            }
                
            stack[stack_pointer ++ ] = se;
        }
        else
        /* is this checking for something? */
        if(current_opcode == OPCODE_EQ)
        {
            uint8_t mov_target = content[cc ++];   /* what to check (reg only)*/

            if(mov_target == OPCODE_REG) /* do we move in a register? */
            {
                uint8_t register_type = content[cc ++]; /* int/string/float...*/

                /* we are dealing with an INT type register */
                if(register_type == OPCODE_INT)
                {
                    uint8_t register_index = content[cc ++]; /* 0, 1, 2 ...*/
                    uint8_t move_source = content[cc ++]; /* what are we checking against*/

                    if(move_source == OPCODE_IMMEDIATE) /* immediate value (1,..) */
                    {
                        uint8_t imm_size = content[cc ++];
                        if(imm_size == OPCODE_BYTE)
                        {
                            int8_t* immediate = (int8_t*)(content + cc);
                            set_lbf_to_op_result(regi[register_index],
                                                    *immediate,
                                                    current_opcode);
                            cc ++;
                        }
                        if(imm_size == OPCODE_SHORT)
                        {
                            int16_t* immediate = (int16_t*)(content + cc);
                            set_lbf_to_op_result(regi[register_index],
                                                    *immediate,
                                                    current_opcode);
                            cc += 2;
                        }
                        if(imm_size == OPCODE_LONG)
                        {
                            int32_t* immediate = (int32_t*)(content + cc);
                            set_lbf_to_op_result(regi[register_index],
                                                    *immediate,
                                                    current_opcode);
                            cc += 4;
                        }
                        if(imm_size == OPCODE_HUGE)
                        {
                            int64_t* immediate = (int64_t*)(content + cc);
                            set_lbf_to_op_result(regi[register_index],
                                                    *immediate,
                                                    current_opcode);
                            cc += 8;
                        }
                    }

                }
                else
                {
                    /* TODO */
                }
            }
            else
            {
                fprintf(stderr, "eq works only on registers\n");
                exit(8);
            }
        }
        else
        /* is this a MOV operation? */
        if(current_opcode == OPCODE_MOV)
        {
            uint8_t mov_target = content[cc ++];   /* where we move (reg, var)*/

            if(mov_target == OPCODE_REG) /* do we move in a register? */
            {
                uint8_t register_type = content[cc ++]; /* int/string/float...*/

                /* we are dealing with an INT type register */
                if(register_type == OPCODE_INT)
                {
                    uint8_t register_index = content[cc ++]; /* 0, 1, 2 ...*/
                    uint8_t move_source = content[cc ++]; /* what are we moving in*/

                    if(move_source == OPCODE_IMMEDIATE) /* immediate value (1,..) */
                    {
                        uint8_t imm_size = content[cc ++];
                        // and now write the number according to the size
                        if(imm_size == OPCODE_BYTE)
                        {
                            int8_t* immediate = (int8_t*)(content + cc);
                            regi[register_index] = *immediate;
                            cc ++;
                        }
                        if(imm_size == OPCODE_SHORT)
                        {
                            int16_t* immediate = (int16_t*)(content + cc);
                            regi[register_index] = *immediate;
                            cc += 2;
                        }
                        if(imm_size == OPCODE_LONG)
                        {
                            int32_t* immediate = (int32_t*)(content + cc);
                            regi[register_index] = *immediate;
                            cc += 4;
                        }
                        if(imm_size == OPCODE_HUGE)
                        {
                            int64_t* immediate = (int64_t*)(content + cc);
                            regi[register_index] = *immediate;
                            cc += 8;
                        }
                    }

                    if(move_source == OPCODE_VAR) /* movin a variable into reg*/
                    {
                        uint32_t* p_var_index = (uint32_t*)(content + cc);

                        /* fetch the variable from the given index */
                        struct variable_entry* var = metatable[*p_var_index];
                        if(var->instantiation == 0)
                        {
                            fprintf(stderr,
                                    "variable %s not initialised correctly\n",
                                    var->name);
                            exit(3);
                        }

                        if(var->instantiation->type != STACK_ENTRY_INT)
                        {
                            fprintf(stderr,
                                    "variable %s has wrong type\n",
                                    var->name);
                            exit(4);
                        }

                        /* and moving the value in the regsiter itself */
                        regi[register_index] = *(int64_t*)var->instantiation->value;

                        /* forwarding the instructions pointer to next bytecode*/
                        cc += sizeof(uint32_t);
                    }
                }
                else
                {
                    /* TODO */
                }
            }
            else
            if(mov_target == OPCODE_VAR) /* we move into a variable */
            {
                uint32_t* p_var_index = (uint32_t*)(content + cc);
                cc += sizeof(uint32_t);

                struct variable_entry* var = metatable[*p_var_index];

                /* first time usage of this variable? */
                if(var->instantiation == 0)
                {
                    fprintf(stderr,
                            "using variable [%s] without being on stack\n",
                            var->name);
                    exit(6);
                }

                /* and now let's see what we move in the variable */
                uint8_t move_source = content[cc ++];

                /* moving a register in a variable? */
                if(move_source == OPCODE_REG)
                {
                    uint8_t register_type = content[cc ++]; /* int/string/float...*/

                    /* we are dealing with an INT type register */
                    if(register_type == OPCODE_INT)
                    {
                        uint8_t register_index = content[cc ++]; /* 0, 1, 2 ...*/

                        /* perform the operation only if the values are not the same already*/
                        if(var->instantiation->value)
                        {
                            if(*(int64_t*)var->instantiation->value != regi[register_index])
                            {
                                int64_t* temp = (int64_t*)calloc(1, sizeof(int64_t));
                                *temp = regi[register_index];
                                var->instantiation->value = temp;
                            }
                        }
                    }
                    else
                    {
                        /* TODO */
                    }
                }
                else
                {
                    fprintf(stderr, "only register can be moved to var [%s]\n",
                            var->name);
                    exit(5);
                }
            }
            else
            {
                fprintf(stderr, "cannot move to a target\n");
                exit(9);
            }
        }
        else
        /* jumping somewhere ? */
        if(current_opcode == OPCODE_JLBF || current_opcode == OPCODE_JMP)
        {
            uint32_t* p_jmpt_index = (uint32_t*)(content + cc);

            /* and simply set cc to be where we need to go */
            cc = jumptable[*p_jmpt_index]->location;
        }
        else
        /* Marking our territory */
        if(current_opcode == OPCODE_MARKS)
        {
            struct stack_entry* marker = (struct stack_entry*)(
                        calloc(sizeof(struct stack_entry), 1));
            marker->type = STACK_ENTRY_MARKER;
            marker->value = 0;
            stack[stack_pointer ++] = marker;
        }
        else
        if(current_opcode == OPCODE_MARKS_NAME)
        {
            struct stack_entry* marker = (struct stack_entry*)(
                        calloc(sizeof(struct stack_entry), 1));
            marker->type = STACK_ENTRY_MARKER_NAME;


            uint32_t* p_marker_code = (uint32_t*)(content + cc);
            cc += sizeof(uint32_t);

            int32_t* temp = (int32_t*)calloc(1, sizeof(int32_t));
            *temp = *p_marker_code;

            marker->value = temp;

            stack[stack_pointer ++] = marker;
        }
        else
        /* giving up our territory? */
        if(current_opcode == OPCODE_CLRS)
        {
            int temp_sp = stack_pointer - 1;
            while(temp_sp >= 0 && stack[temp_sp]->type != STACK_ENTRY_MARKER)
            {
                if(stack[temp_sp]->value)
                {
                    /* TODO: this is a variable entry, free it properly !!! */
                    free(stack[temp_sp]->value);
                }
                free(stack[temp_sp]);
                temp_sp --;
            }

            /* remove the marker */
            if(temp_sp >= 0)
            {
                free(stack[temp_sp]);
                temp_sp --;
                stack_pointer = temp_sp;
            }
            else
            {
                stack_pointer = 0;
            }

        }
        else
        /* calling someone */
        if(current_opcode == OPCODE_CALL)
        {
            uint32_t* p_jmpt_index = (uint32_t*)(content + cc);
            cc += sizeof(uint32_t);
            call_frames[cfsize ++] = cc;

            /* and simply set cc to be where we need to go */
            cc = jumptable[*p_jmpt_index]->location;
        }
        else
        if(current_opcode == OPCODE_PEEK)
        {
            uint8_t peek_type = content[cc ++]; /* int/string/float...*/

            /* we are dealing with an INT type peek */
            if(peek_type == OPCODE_INT)
            {
                uint8_t peek_index_type = content[cc ++]; /* what are we moving in*/
                uint32_t peek_index = 0;

                if(peek_index_type == OPCODE_IMMEDIATE) /* immediate value (1,..) */
                {
                    uint8_t imm_size = content[cc ++];
                    // and now write the number according to the size
                    if(imm_size == OPCODE_BYTE)
                    {
                        int8_t* immediate = (int8_t*)(content + cc);
                        peek_index = *immediate;
                        cc ++;
                    }
                    if(imm_size == OPCODE_SHORT)
                    {
                        int16_t* immediate = (int16_t*)(content + cc);
                        peek_index = *immediate;
                        cc += 2;
                    }
                    if(imm_size == OPCODE_LONG)
                    {
                        int32_t* immediate = (int32_t*)(content + cc);
                        peek_index = *immediate;
                        cc += 4;
                    }
                    if(imm_size == OPCODE_HUGE)
                    {
                        int64_t* immediate = (int64_t*)(content + cc);
                        peek_index = *immediate;
                        cc += 8;
                    }
                }

                /* now we know the peek index, see into what are we peeking */
                uint8_t peek_target = content[cc ++];
                if(peek_target == OPCODE_VAR)
                {
                    uint32_t* p_var_index = (uint32_t*)(content + cc);
                    cc += sizeof(uint32_t);

                    struct variable_entry* ve = metatable[*p_var_index];

                    /* there is no instantioation at this point for the var */
                    ve->instantiation = (struct stack_entry*)(
                                calloc(sizeof(struct stack_entry), 1));
                    ve->instantiation->type = peek_type;
                    if(peek_type == OPCODE_INT)
                    {
                        int64_t* temp = (int64_t*)calloc(1, sizeof(int64_t));
                        *temp = *(int64_t*)stack[stack_pointer - peek_index - 1]->value; // STACK VALUE FROM peek_index
                        ve->instantiation->value = temp;
                    }
                    else
                    {
                        /* TODO */
                    }


                }
            }
            else
            {
                /* TODO */
            }
        }
        else
        if(current_opcode == OPCODE_POP)
        {
        }
        else
        if(current_opcode == OPCODE_RETURN)
        {
            cc = call_frames[--cfsize];
        }
        else
        if(current_opcode == OPCODE_EXIT)
        {
            printf("bye\n");
            exit(0);
        }
        else
        {
            fprintf(stderr, "invalid opcode [%x] at %"PRIu64" (%"PRIx64")\n",
                    current_opcode, cc - 1, cc - 1);
            exit(5);

        }
    }

    printf("exit\n");

    return 0;
}
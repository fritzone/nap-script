/* 24.12.2008 last modification: 26.06.2013
   Copyright (c) 2008-2013 by Siegfried Koepf

   This file is distributed under the terms of the GNU General Public License
   version 3 as published by the Free Software Foundation.
   For information on usage and redistribution and for a disclaimer of all
   warranties, see the file COPYING in this distribution.

   Modified by fritzone to suit the purpose of the nap-project.
*/

#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <string>
#include <map>
#include <algorithm>

static const unsigned char m                   = 4;    //How many types
static unsigned char NUM_PARMS                 = 8;    //How many parameters
static std::vector<std::string> useds;
static std::map<unsigned long, std::string> mappings;
static unsigned long max_array = 0;

//return values of generation functions
#define GEN_NEXT  0 //ok, print and continue
#define GEN_TERM  1 //ok, terminate
#define GEN_EMPTY 2 //ok, print EMPTY SET and continue
#define GEN_ERROR 3 //an error occured, print an error message and terminate

static int gen_vari_rep_lex_init(unsigned char *vector, const unsigned char m, const unsigned char n)
{
    int j; //index

    //test for special cases
    if(m == 0 || n == 0)
        return(GEN_EMPTY);

    //initialize: vector[0, ..., n - 1] are zero
    for(j = 0; j < n; j++)
        vector[j] = 0;

    return(GEN_NEXT);
}

static int gen_vari_rep_lex_next(unsigned char *vector, const unsigned char m, const unsigned char n)
{
    int j; //index

    //easy case, increase rightmost element
    if(vector[n - 1] < m - 1)
    {
        vector[n - 1]++;
        return(GEN_NEXT);
    }

    //find rightmost element to increase and reset right-hand elements
    for(j = n - 2; j >= 0; j--)
    {
        vector[j + 1] = 0;

        if(vector[j] < m - 1)
            break;
    }

    //terminate if all elements are m - 1
    if(j < 0)
        return(GEN_TERM);

    //increase
    vector[j]++;

    return(GEN_NEXT);
}

static const char* get_type_code(unsigned char el, int print_void)
{
    switch(el)
    {
    case 1: return "i";
    case 2: return "r";
    case 3: return "s";
    default:
        if(print_void) return "v";
        else return "";

    }
}

static const char* get_type(unsigned char el, int print_void)
{
    switch(el)
    {
    case 1: return "nap_int_t";
    case 2: return "nap_real_t";
    case 3: return "nap_string_t";
    default:
        if(print_void)
        {
            return "void";
        }
        else
        {
            return "";
        }
    }
}

// prints a function prototype based on the input vector.
// The first element is the return type
static void print_function(unsigned char* vec, int also_body, FILE* fp)
{
    std::string curr = get_type_code(vec[0], 1);
    std::string typedef_name = "nap_ext_";
    std::string to_print = "typedef";
    std::string func_name = "nap_ext_caller_";
    std::string signature = "";
    int valid_params = 0;

    to_print += " ";
    to_print += get_type(vec[0], 1);
    to_print += " (*";

    signature = get_type_code(vec[0], 1); // for the return type we need also the void
    signature += "__";
    
    for(int i=1; i<NUM_PARMS; i++)
    {
        signature += get_type_code(vec[i], 0); // for the typename we don't need the void
        curr += get_type_code(vec[i], 0);
        if(i < NUM_PARMS-1 && vec[i] != 0)
        {
            valid_params ++;
            signature += "_";
        }
    }
    
    if(std::find(useds.begin(), useds.end(), curr) != useds.end() )
    {
        return;
    }

    typedef_name += signature;
    to_print += typedef_name;
    func_name += signature;

    to_print += ")(";
    for(int i=1; i<NUM_PARMS; i++)
    {
        to_print += get_type(vec[i], 0);
        if(i < NUM_PARMS-1 && vec[i] != 0)
        {
            to_print += ", ";
        }
    }
    to_print += ");\n";

    // put the marker only if we are past generating the body too.
    if(also_body)
    {
        useds.push_back(curr);
    }

    char *nr = (char*)calloc(NUM_PARMS, sizeof(char));
    for(int i=0; i<NUM_PARMS; i++)
    {
        nr[i] = '0' + vec[i];
    }

    char *end = 0;
    unsigned long l = strtol(nr, &end, 4);

    // create a mapping
    mappings[l] = func_name;
    if(l > max_array)
    {
        max_array = l;
    }

    // function prototype
    fprintf(fp, "void %s(void*%s, struct nap_ext_par_desc*%s, void*%s)", func_name.c_str(), also_body?" fun":"", also_body?" pars":"", also_body?" retv":"");
    if(also_body == 0)
    {
        fprintf(fp, ";\n");
        return;
    }

    // typedef
    fprintf(fp, "\n{\n    %s\n",to_print.c_str());
    // function pointer
    fprintf(fp, "    %s local_fun = (%s)(fun);\n", typedef_name.c_str(), typedef_name.c_str());
	// unused parameters? No warnings please.
	if(valid_params == 0)
	{
		fprintf(fp, "\n    UNUSED(pars);\n");
	}
    // calling the local function
    if(vec[0] != 0) // populating the return value
    {
        fprintf(fp, "    *((%s*)(retv)) = ", get_type(vec[0], 1));
    }
    else
    {
        fprintf(fp, "    UNUSED(retv);\n    ");
    }

    fprintf(fp, "local_fun(\n");
    // and the parameters
    bool more_than_zero_par = false;
    int par_counter = 1;
    for(int i=1; i<NUM_PARMS; i++)
    {
        if(vec[i] != 0)
        {
            more_than_zero_par = true;
            fprintf(fp, "        *((%s*)(pars->p%d))", get_type(vec[i], 0), par_counter - 1);
            if(i < NUM_PARMS-1)
            {
                fprintf(fp, ", // par %d as %s \n", par_counter,
                        get_type(vec[par_counter], 0) );
                par_counter ++;
            }
            else
            {
                fprintf(fp, " // par %d as %s \n", par_counter,
                        get_type(vec[par_counter], 0) );
            }
        }
    }
    fprintf(fp, "%s    );\n}\n\n",more_than_zero_par?"":"");
}

int main(int argc, char* argv[])
{
    std::string target_dir = "./";
    std::string max_par = "8";

    for(int i=0; i<argc; i++)
    {
        std::string carg = argv[i];
        if(carg.length() == 2 && carg[0] == '-')
        {
            if(carg[1] == 'n') // -n : how many parameters to generate
            {
                if(i < argc - 1)
                {
                    max_par = argv[i+1];
                }
            }
            if(carg[1] == 'd') // -d : directory
            {
                if(i < argc - 1)
                {
                    target_dir = argv[i+1];
                }
            }
        }
    }

    std::string cpp_file = target_dir + "/nap_ext_def.c";
    std::string header_file = target_dir + "/nap_ext_def.h";
    int t = atoi(max_par.c_str());
    if(t > 32)
    {
        t = 8;
    }
    NUM_PARMS = t;
    printf("support for %d parameters\n", NUM_PARMS);

    unsigned char *vector     = NULL; //where the current figure is stored
    int           gen_result;         //return value of generation functions
    unsigned int  set_counter;        //counting generated sequences

    //alloc memory for vector
    vector = (unsigned char *)malloc(sizeof(unsigned char) * NUM_PARMS);
    if(vector == NULL)
    {
        fprintf(stderr, "error: insufficient memory\n");
        exit(EXIT_FAILURE);
    }

    set_counter = 0;
    FILE* fp_header = fopen(header_file.c_str(), "wt+");
    fputs("/*This file is automatically generated by nap-ext-gen. Do not touch. */\n", fp_header);
    fputs("#ifndef NAP_EXT_DEF_H\n#define NAP_EXT_DEF_H\n\n", fp_header);
    fputs("#include \"nap_types.h\"\n\n", fp_header);
    fputs("#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n", fp_header);
    fputs("#define UNUSED(x) (void)(x)\n\n", fp_header);

    FILE* fp_body = fopen(cpp_file.c_str(), "wt+");
    fputs("#include \"nap_ext_def.h\"\n\n", fp_body);
    fputs("#include <stdlib.h>\n\n", fp_body);

    // parameters
    fputs("struct nap_ext_par_desc\n{", fp_header);
    for(int i=0; i< NUM_PARMS; i++)
    {
        fprintf(fp_header, "\n    void* p%d;", i);
    }
    fputs("\n};\n\n", fp_header);

    //initialize
    gen_result = gen_vari_rep_lex_init(vector, m, NUM_PARMS);

    if(gen_result == GEN_EMPTY)
    {
        set_counter++;
    }

    //generate all successors
    while(gen_result == GEN_NEXT)
    {
        set_counter++;

        print_function(vector, 0, fp_header);
        print_function(vector, 1, fp_body);

        gen_result = gen_vari_rep_lex_next(vector, m, NUM_PARMS);
    }

    // define the typedef for the function pointers and their array
    fputs("\ntypedef void (*nap_ext_caller)(void*, struct nap_ext_par_desc*, void*);\n", fp_header);
    fputs("\nextern nap_ext_caller ext_callers[", fp_header);
    fprintf(fp_header, "%lu];\n", max_array);

    fputs("void nap_int_init_ext_func_array();\n", fp_header);
    fputs("void nap_populate_par_desc(struct nap_ext_par_desc* pd, int index, void* new_data);\n", fp_header);
    fputs("void nap_free_parameter_descriptor(struct nap_ext_par_desc** pd);\n", fp_header);

    // close the headers
    fputs("\n#ifdef __cplusplus\n}\n#endif", fp_header);
    fputs("\n\n#endif\n", fp_header);

    // now in the body create the number to function pointer array
    // and write the array of function pointers
    fputs("\nnap_ext_caller ext_callers[", fp_body);
    fprintf(fp_body, "%lu] = {0};\n", max_array);

    // and the initialization function
    fputs("\nvoid nap_int_init_ext_func_array()\n{\n", fp_body);
    for(unsigned long i=0; i<=max_array; i++)
    {
        if(mappings.count(i) == 1)
        {
            fprintf(fp_body, "    ext_callers[%lu]=%s;\n", i, mappings[i].c_str());
        }
    }
    fputs("}\n\n", fp_body);

    // the nap_populate_parameter_descriptor function
    fputs("void nap_populate_par_desc(struct nap_ext_par_desc* pd, int index, void* new_data)\n{\n", fp_body);
    for(int i=0; i<NUM_PARMS; i++)
    {
        fprintf(fp_body, "    if(index == %d)\n    {\n    pd->p%d=new_data;\n}\n", i, i);
    }
    fprintf(fp_body, "}\n\n");

    // the nap_free_parameter_descriptor function
    fputs("void nap_free_parameter_descriptor(struct nap_ext_par_desc** pd)\n{\n", fp_body);
    for(int i=0; i<NUM_PARMS; i++)
    {
        fprintf(fp_body, "    if( (*pd)->p%d )\n    {\n        free( (*pd)->p%d );\n    }", i, i);
    }
    fputs("    free(*pd);\n}", fp_body);

    fclose(fp_header);
    fclose(fp_body);
    free(vector);
    return(EXIT_SUCCESS);
}

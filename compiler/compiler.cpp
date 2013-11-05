#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include <vector>
#include <string>
#include <algorithm>

#include "hash.h"
#include "tree.h"
#include "consts.h"
#include "variable.h"
#include "utils.h"
#include "interpreter.h"
#include "method.h"
#include "bt_string.h"
#include "type.h"
#include "number.h"
#include "call_ctx.h"
#include "sys_brkp.h"
#include "throw_error.h"
#include "parser.h"
#include "res_wrds.h"
#include "envelope.h"

static std::vector<std::string> loaded_files;


void load_next_block(parsed_file* pf, method* the_method, call_context* par_cc, int current_level, int orig_level);
void load_next_single_phrase(expression_with_location* expwloc, method* cur_method, call_context* cur_cc, char* delim, int level, parsed_file* pf);

/**
 * This method loads the while commands block
 */
static void deal_with_while_loading(call_context* cc, expression_tree* new_node, method* the_method, char delim, int current_level, parsed_file* pf, expression_with_location* expwloc)
{
    /* firstly create a call context for it */
    char* while_cc_name = new_string(strlen(cc->name) + 10);
    sprintf(while_cc_name, "%s%s%s", cc->name, STR_CALL_CONTEXT_SEPARATOR, STR_WHILE);
    call_context* while_cc = call_context_create(CALL_CONTEXT_TYPE_WHILE, while_cc_name, the_method, cc);
    if(C_OPEN_BLOCK == delim && new_node->op_type == STATEMENT_WHILE)    /* normal WHILE with { }*/
    {
        load_next_block(pf, the_method, while_cc, current_level + 1, current_level);    /* loading the function*/
    }
    else if(C_OPEN_BLOCK == delim && new_node->op_type == STATEMENT_WHILE_1L) /* while (a) if (b) if (c) { blabla; }*/
    {
        /* load the next statements into the while's call context */
        char d = delim;
        expression_with_location* next_exp = alloc_mem(expression_with_location,1);
        next_exp->location = expwloc->location;
        next_exp->expression = duplicate_string(new_node->info);
        //pf->position += strlen(new_node->info);
        load_next_single_phrase(next_exp, the_method, while_cc, &d, current_level + 1, pf);
        new_node->op_type = STATEMENT_WHILE;
        new_node->info = duplicate_string(STR_WHILE);
    }
    else if(C_SEMC == delim && new_node->op_type == STATEMENT_WHILE_1L)            /* one lined while, load the next expression, which is being 'hacked' into new_node.info by the interpreter  */
    {
        call_context_add_new_expression(while_cc, new_node->info, expwloc);
        new_node->op_type = STATEMENT_WHILE;
        new_node->info = duplicate_string(STR_WHILE);
    }
    else if(C_SEMC == delim && new_node->op_type == STATEMENT_WHILE)            /* one lined while, with empty command, skip pf to the first position after the ; */
    {
        while(is_whitespace(pf->content[pf->position])) pf->position++;    /* skip the whitespace */
        if(pf->content[pf->position] == C_SEMC) pf->position ++;        /* skip the ';'*/
    }

    resw_while* while_st = new resw_while;
    while_st->logical_expr = (expression_tree*)new_node->reference->to_interpret;
    while_st->operations = while_cc;
    new_node->reference = new_envelope(while_st, STATEMENT_WHILE);
}

static void deal_with_class_declaration(call_context* /*cc*/,
                                        expression_tree* new_node,
                                        method* /*the_method*/,
                                        char /*delim*/,
                                        int /*current_level*/,
                                        parsed_file* pf,
                                        expression_with_location* /*expwloc*/)
{
    char* new_block = alloc_mem(char, pf->content_size - pf->position); // should be enough ...
    int lev = 1;
    int pos = pf->position;
    int nbpos = 0;
    bool can_go = true;

    while(can_go && pos < pf->content_size)
    {
        new_block[nbpos] = pf->content[pos];
        if(pf->content[pos] == '{') lev ++;
        if(pf->content[pos] == '}') lev --;
        if(lev == 0) can_go = false;
        pos ++;
        nbpos ++;
    }
    pf->position = pos + 1;     // skips the closing brace
    new_block[nbpos - 1] = 0;   // remove the closing brace
    parsed_file* npf = new_parsed_file(new_block);
    npf->name = duplicate_string(pf->name);
    expression_with_location* nexpwloc = NULL;
    char ndelim = 0;
    method* nmethod = 0;
    nexpwloc = parser_next_phrase(npf, &ndelim);
    call_context* class_cc = (call_context*)((envelope*)new_node->reference)->to_interpret;
    int nlevel  = -1;
    while(nexpwloc)
    {
        load_next_single_phrase(nexpwloc, nmethod, class_cc, &ndelim, nlevel, npf);
        nexpwloc = parser_next_phrase(npf, &ndelim);
    }

    free(new_block);
}

/**
* This method loads the for commands block
*/
static void deal_with_for_loading(call_context* cc, expression_tree* new_node, method* the_method, char delim, int current_level, parsed_file* pf, expression_with_location* expwloc)
{
    /* firstly create a call context for it */
    char* for_cc_name = new_string(strlen(cc->name) + 10);
    sprintf(for_cc_name, "%s%s%s", cc->name, STR_CALL_CONTEXT_SEPARATOR, STR_FOR);
    call_context* for_cc = call_context_create(CALL_CONTEXT_TYPE_FOR, for_cc_name, the_method, cc);
    if(C_OPEN_BLOCK == delim && new_node->op_type == STATEMENT_FOR)    /* normal FOR with { }*/
    {
        load_next_block(pf, the_method, for_cc, current_level + 1, current_level);    /* loading the function*/
    }
    else if(C_OPEN_BLOCK == delim && new_node->op_type == STATEMENT_FOR_1L) /* for (a) if (b) if (c) { blabla; }*/
    {
        /* load the next statements into the for's call context */
        char d = delim;
        expression_with_location* next_exp = alloc_mem(expression_with_location,1);
        next_exp->location = expwloc->location;
        next_exp->expression = duplicate_string(new_node->info);
        //pf->position += strlen(new_node->info);
        load_next_single_phrase(next_exp, the_method, for_cc, &d, current_level + 1, pf);
        new_node->op_type = STATEMENT_FOR;
        new_node->info = duplicate_string(STR_FOR);
    }
    else if(C_SEMC == delim && new_node->op_type == STATEMENT_FOR_1L)            /* one lined for, load the next expression, which is being 'hacked' into new_node.info by the interpreter  */
    {
        call_context_add_new_expression(for_cc, new_node->info, expwloc);
        new_node->op_type = STATEMENT_FOR;
        new_node->info = duplicate_string(STR_FOR);
    }
    else if(C_SEMC == delim && new_node->op_type == STATEMENT_FOR)            /* one lined for, with empty command, skip pf to the first position after the ; */
    {
        while(is_whitespace(pf->content[pf->position])) pf->position++;    /* skip the whitespace */
        if(pf->content[pf->position] == C_SEMC) pf->position ++;        /* skip the ';'*/
    }
    resw_for* rsfor = (resw_for*)new_node->reference->to_interpret;
    rsfor->operations = for_cc;
}


/**
 * Loads the next block of the 'if' command
 */
static void deal_with_ifs_loading(call_context* cc, expression_tree* new_node, method* the_method, char delim, int current_level, parsed_file* pf, expression_with_location* expwloc)
{
    /* firstly create a call context for it */
    char* if_cc_name = new_string(strlen(cc->name) + 6);
    sprintf(if_cc_name, "%s%s%s", cc->name, STR_CALL_CONTEXT_SEPARATOR, STR_IF);
    call_context* if_cc = call_context_create(CALL_CONTEXT_TYPE_IF, if_cc_name, the_method, cc);
    if(C_OPEN_BLOCK == delim && new_node->op_type == STATEMENT_IF)    /* normal IF with { }*/
    {
        load_next_block(pf, the_method, if_cc, current_level + 1, current_level);    /* loading the function*/
    }
    else if(C_OPEN_BLOCK == delim && new_node->op_type == STATEMENT_IF_1L) /* if (a) if (b) if (c) { blabla; }*/
    {
        /* load the next statements into the if's call context */
        char d = delim;
        expression_with_location* next_exp = alloc_mem(expression_with_location,1);
        next_exp->location = expwloc->location;
        next_exp->expression = duplicate_string(new_node->info);
        //pf->position += strlen(new_node->info);
        load_next_single_phrase(next_exp, the_method, if_cc, &d, current_level + 1, pf);
        new_node->op_type = STATEMENT_IF;
        new_node->info = duplicate_string(STR_IF);
    }
    else if(C_SEMC == delim && new_node->op_type == STATEMENT_IF_1L)            /* one lined if, load the next expression, which is being 'hacked' into new_node.info by the interpreter  */
    {
        call_context_add_new_expression(if_cc, new_node->info, expwloc);
        new_node->op_type = STATEMENT_IF;
        new_node->info = duplicate_string(STR_IF);
    }
    else if(C_SEMC == delim && new_node->op_type == STATEMENT_IF)            /* one lined if, with empty command, skip pf to the first position after the ; */
    {
        while(is_whitespace(pf->content[pf->position])) pf->position++;    /* skip the whitespace */
        if(pf->content[pf->position] == C_SEMC) pf->position ++;        /* skip the ';'*/
    }

    /* here read the next word check if it's a plain 'else' */
    char delim2;
    char* nextw = parser_preview_next_word(pf, &delim2);
    call_context* else_cc = NULL;
    if(nextw && !strcmp(nextw, STR_ELSE))
    {
        char* else_cc_name = new_string(strlen(cc->name) + 16);
        sprintf(else_cc_name, "%s%s%s:%s", cc->name, STR_CALL_CONTEXT_SEPARATOR, STR_IF, STR_ELSE);
        else_cc = call_context_create(CALL_CONTEXT_TYPE_ELSE, else_cc_name, the_method, cc);
        if(delim2 == C_SEMC)
        {
            expression_with_location* expwloc2 = parser_next_phrase(pf, &delim);
            char* else_expression = trim(expwloc2->expression + strlen(STR_ELSE));
            if(strlen(else_expression) > 1)
            {
                call_context_add_new_expression(else_cc, else_expression, expwloc2);
            }
        }
        else if(C_OPEN_BLOCK == delim2)
        {
            /* here we should skip: the whitespace before the else, the else, the whitespace and the opening '{' */
            parser_skip_next_word(pf);
            if(pf->content[pf->position] != C_OPEN_BLOCK)
            {
                if(is_identifier_char(pf->content[pf->position]))
                {   /* else followed by another statement. Load it with the single phrase loader int the else_cc */
                    char delim3 = 0;
                    expression_with_location* expwloc3 = parser_next_phrase(pf, &delim3);
                    load_next_single_phrase(expwloc3, the_method, else_cc, &delim3, current_level, pf);
                }
                else
                {
                    throw_error("Internal 1", NULL);    /* actually we shouldn't get here at all*/
                }
            }
            else
            {
                pf->position ++;    /* skip the { */
                load_next_block(pf, the_method, else_cc, current_level, current_level + 1);    /* loading the function*/
            }
        }
    }

    resw_if* if_st = new resw_if;
    if_st->logical_expr = (expression_tree*)new_node->reference->to_interpret;
    if_st->if_branch = if_cc;
    if_st->else_branch = else_cc;

    new_node->reference = new_envelope(if_st, STATEMENT_IF);
}

void load_next_assembly_block(parsed_file* pf, call_context* par_cc)
{
    int can_stop = 0;
    std::vector<std::string> assemblys;
    while(!can_stop) /*read till we get '}' */
    {
        char delim = 0;
        expression_with_location* expwloc = parser_next_phrase(pf, &delim);

        expression_tree* tmp = alloc_mem(expression_tree,1);
        tmp->v_type = BASIC_TYPE_DONTCARE;
        tmp->op_type = ASM_BLOCK;
        tmp->expwloc = expwloc;

        expression_tree_list_add_new_expression(tmp, &par_cc->expressions, expwloc->expression);

        if(delim == '}') can_stop = 1;
    }
}

/**
 * Loads the body of a function from the given parsed file
 */
void load_next_block(parsed_file* pf, method* the_method, call_context* par_cc, int current_level, int orig_level)
{
    /* at this stage the pf points to the first statement in the file ...*/
    call_context* cc = par_cc;
    int can_stop = 0;
    while(!can_stop)
    {
        char delim = 0;
        int use_delim = 1;    /* the scope ofthis var is to check whether we need to open new unnamed code blocks. keywords which need blocks (if, while, etc) will make this zero, they are managing their own code blocks */
        expression_with_location* expwloc = parser_next_phrase(pf, &delim);
        if(expwloc || (!expwloc && delim == C_OPEN_BLOCK) )
        {
            if(expwloc)
            {
                if( strlen(expwloc->expression) > 0 )
                {
                    expression_tree* new_node = call_context_add_new_expression(cc, expwloc->expression, expwloc);
                    /* now check whether this node was an If, a for, a while, a do or anything else that might need loading more rows and creating more contexts */
                    if(new_node->op_type == STATEMENT_IF || new_node->op_type == STATEMENT_IF_1L)
                    {
                        use_delim = 0;
                        deal_with_ifs_loading(cc, new_node, the_method, delim, current_level, pf, expwloc);
                    }
                    else if(new_node->op_type == STATEMENT_WHILE || new_node->op_type == STATEMENT_WHILE_1L)
                    {
                        use_delim = 0;
                        deal_with_while_loading(cc, new_node, the_method, delim, current_level, pf, expwloc);
                    }
                    else if(new_node->op_type == STATEMENT_FOR || new_node->op_type == STATEMENT_FOR_1L)
                    {
                        use_delim = 0;
                        deal_with_for_loading(cc, new_node, the_method, delim, current_level, pf, expwloc);
                    }
                }
            }

            if(use_delim)
            {
                switch(delim)
                {
                case C_OPEN_BLOCK:
                {
                    char* new_cc_name = new_string(strlen(cc->name) + 10);
                    sprintf(new_cc_name, "%s%s%s", cc->name, STR_CALL_CONTEXT_SEPARATOR, STR_UNNAMED_CC);
                    call_context* child_cc = call_context_create(CALL_CONTEXT_TYPE_UNNAMED, new_cc_name, the_method, cc);
                    expression_tree* new_node = call_context_add_new_expression(cc, STR_OPEN_BLOCK, expwloc);
                    //todo: add a new expression in the current call CONTEXT to start executing the next call context.
                    new_node->reference = new_envelope(child_cc, ENV_TYPE_CC);
                    cc = child_cc;
                    current_level ++;
                    pf->position ++;    /* hack again, to skip to the next character, to not to loop on the { 4ever */
                }
                break;
                case C_CLOSE_BLOCK:
                {
                    if(expwloc->expression[0] != '}') // put a new close block only if this is not a method def.
                    {
                        call_context_add_new_expression(cc, STR_CLOSE_BLOCK, expwloc);
                    }
                    cc = cc->father;    /* stepping back */
                    current_level --;
                    if(current_level == orig_level)
                    {
                        can_stop = 1;
                    }
                }
                break;

                case C_SEMC:
                    break;
                default:
                    can_stop = 1;    /* means: we're at the end*/
                    break;
                }
            }
        }
        else
        {
            if(delim == C_CLOSE_BLOCK)
            {
                call_context_add_new_expression(cc, "}", expwloc);
                cc = cc->father;    /* stepping back */
                current_level --;
            }
            can_stop = 1;
        }
    }
}

void load_next_single_phrase(expression_with_location* expwloc, method* cur_method, call_context* cur_cc, char* delim, int level, parsed_file* pf)
{
    char* first = expwloc->expression;
    if(first && strlen(first) > 0)
    {
        int op_res = -1;
        expression_tree* cnode = new_expression_tree(expwloc);
        void* gen_res = build_expr_tree(first, cnode, cur_method, first, cur_cc, &op_res, expwloc);
        switch(op_res)
        {
        case ASM_BLOCK: /*load the next block and directly feed in the statements to the cnode's envelope*/
        {
            load_next_assembly_block(pf, cur_cc);    /* loading the function*/
            break;
        }
        case NT_VARIABLE_DEF_LST:
            call_context_add_compiled_expression(cur_cc, cnode, first);
            break;
        case FUNCTION_DEFINITION:
            cur_method = (method*)gen_res;
            if(cur_method->def_loc != DEF_EXTERN)
            {
                if(*delim == C_SEMC)    /* forward declaration of method */
                {

                }
                else                /* normal declaration of method */
                {
                    int saved_level = level;
                    level ++;    /* stepping into a method, increase the current level */
                    call_context* saved_cc = cur_cc; /* saving the current cc*/
                    cur_cc = cur_method->main_cc;
                    load_next_block(pf, cur_method, cur_cc, level, saved_level);    /* loading the function*/
                    cur_cc = saved_cc; /* restoring the current cc */
                }
            }
            cur_method = NULL;
            break;

        case STATEMENT_IF:
        case STATEMENT_IF_1L:
            call_context_add_compiled_expression(cur_cc, cnode, expwloc->expression);
            deal_with_ifs_loading(cur_cc, cnode, cur_method, *delim, level, pf, expwloc);
            break;

        case STATEMENT_WHILE:
        case STATEMENT_WHILE_1L:
            call_context_add_compiled_expression(cur_cc, cnode, expwloc->expression);
            deal_with_while_loading(cur_cc, cnode, cur_method, *delim, level, pf, expwloc);
            break;

        case STATEMENT_FOR:
        case STATEMENT_FOR_1L:
            call_context_add_compiled_expression(cur_cc, cnode, expwloc->expression);
            deal_with_for_loading(cur_cc, cnode, cur_method, *delim, level, pf, expwloc);
            break;

        case CLASS_DECLARATION:
            deal_with_class_declaration(cur_cc, cnode, 0, *delim, level, pf, expwloc);
            break;

        case FUNCTION_CALL:
        case FUNCTION_CALL_CONSTRUCTOR:
        case FUNCTION_CALL_OF_OBJECT:
        case FUNCTION_CALL_STATIC:
        default:
            call_context_add_compiled_expression(cur_cc, cnode, first);
            break;
        }
    }
}

static void load_file(call_context* cc, const char* file_name, method* cur_method)
{
    int level = -1; /* currently we are in a global context */
    expression_with_location* expwloc = NULL;
    char delim;
    parsed_file* pf = open_file(file_name);
    if(!pf)
    {
        return;
    }
    expwloc = parser_next_phrase(pf, &delim);
    while(expwloc)
    {
        char* exp_trim = trim(duplicate_string(expwloc->expression));
        if(strstr(exp_trim, "import") == exp_trim)
        {
            exp_trim += 6;
            char* file_to_load = trim(exp_trim);
            if(std::find(loaded_files.begin(), loaded_files.end(), file_to_load) == loaded_files.end())
            {
                loaded_files.push_back(file_to_load);
                load_file(cc, file_to_load, cur_method);
                expwloc = parser_next_phrase(pf, &delim);
            }
        }
        else
        {
            load_next_single_phrase(expwloc, cur_method, cc, &delim, level, pf);
            expwloc = parser_next_phrase(pf, &delim);
        }
    }
}

int main(int argc, char* argv[])
{
    call_context* cur_cc = call_context_create(0, "global", NULL, NULL) ;
    global_cc = cur_cc;

    method* cur_method = NULL;
    const char* file_name = argc > 1?argv[1]:"test.nap";

    load_file(cur_cc, file_name, cur_method);
    call_context_compile(global_cc);
    return 1;
}

#include "parser.h"
#include "utils.h"
#include "consts.h"
#include "call_ctx.h"
#include "res_wrds.h"
#include "envelope.h"
#include "throw_error.h"
#include "common_structs.h"
#include "expression_tree.h"
#include "interpreter.h"
#include "compiler.h"

#include <stdlib.h>
#include <string.h>

/**
 * This file removes the comments from the input file. This is the first step and should be done right after the
 */
void parsed_file::remove_comments()
{
    int cloc = 0;
    while (cloc < content_size)
    {
        /* grab a whole bunch of comments at once ... */
        /* first comment style: Unix style # and C++'s //. Both of these go till end of line*/
        if (C_HASH == content[cloc] || (C_SLASH == content[cloc] && cloc + 1 < content_size && content[cloc + 1] == C_SLASH))
        {
            while (cloc < content_size && (content[cloc] != '\n' && content[cloc] != '\r'))    /* End-line ... */
            {
                content[cloc] = C_SPACE;
                cloc ++;
            }
        }
        /* second comment style: C's classic comments. like this. */
        if (C_SLASH == content[cloc] && cloc + 1 < content_size && C_STAR == content[cloc + 1])
        {
            content[cloc++] = C_SPACE;                 /* skip the / and * */
            content[cloc++] = C_SPACE;

            while (cloc < content_size - 1 && (C_STAR != content[cloc] && C_SLASH != content[cloc + 1]))    /* End-comment ... will stop at the closing '*' */
            {
                content[cloc++] = C_SPACE;
            }
            content[cloc++] = C_SPACE;                /* and the closing ones too */
            content[cloc++] = C_SPACE;
        }
        cloc ++;
    }
}

parsed_file::parsed_file(const nap_compiler *_compiler) : mcompiler(_compiler)
{
    position = 0;
    content = 0;
    content_size = 0;
    current_line = 0;
}

parsed_file::parsed_file(const char *pcontent, const nap_compiler* _compiler) : mcompiler(_compiler)
{
    position = 0;
    content = alloc_mem(char, strlen(pcontent), mcompiler);
    strcpy(content, pcontent);
    content_size = strlen(content);
    current_line = 0;
}

/**
 * Opens the given file, creates a new prsed file structure
 */
parsed_file *parsed_file::open_file(const char *name,  const nap_compiler* _compiler)
{
    parsed_file *f = new parsed_file(_compiler);
    long size = -1;
    f->name = name;
    /* the file pointer*/
    FILE *fp;

    fp = fopen(f->name, "rb");
    if (!fp)
    {
        delete f;
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    if (size == -1)
    {
        delete f;
        return NULL;
    }
    fseek(fp, 0, SEEK_SET);
    f->content = alloc_mem(char, size, _compiler);
    f->content_size = size;
    fread(f->content, size, sizeof(char), fp);
    fclose(fp);
    f->position = 0;
    f->current_line = 0;
    f->remove_comments();
    return f;
}

parsed_file *parsed_file::set_source(const char *src,  const nap_compiler* _compiler)
{
    parsed_file *f = new parsed_file(_compiler);
    long size = strlen(src);
    f->name = NULL;
    /* the file pointer*/

    f->content = alloc_mem(char, size, _compiler);
    f->content_size = size;
    memcpy(f->content, src, size);

    f->position = 0;
    f->current_line = 0;
    f->remove_comments();
    return f;
}

/**
 * Reads the next phrase, returns it
 */
expression_with_location* parsed_file::parser_next_phrase(char *delim)
{
    if(position == content_size)
    {
        return NULL;
    }
    previous_position = position;
    expression_with_location *expwloc = alloc_mem(expression_with_location, 1, mcompiler);
    expwloc->location = alloc_mem(file_location, 1, mcompiler);

    expwloc->location->location = position;
    expwloc->location->start_line_number = current_line + 1;
    expwloc->location->end_line_number = current_line + 1;
    expwloc->location->file_name = name;

    long cur_save = position;
    long size = -1;
    char *phrase = NULL;
    int i = 0;
    int phrase_read = 0;
    int skipper = 1; // the scope of this variable is to skip one character forward if we reached a ";" separator, but nothing if we reached "{" or "}"
    while (cur_save < content_size && !phrase_read)
    {
        i++;
        /* trying to bypass a string and ignore the special characters inside it */
        if (is_string_delimiter(content[cur_save]))
        {
            cur_save ++;
            i++;
            int strlimend = 0; /* string limit ended*/
            while (!strlimend)
            {
                if (is_string_delimiter(content[cur_save]) && content[cur_save - 1] != C_BACKSLASH)
                {
                    strlimend = 1;
                }
                if (!strlimend)
                {
                    cur_save ++;
                }
            }
        }

        /* now check for the special case of 'for' statement. This must read in two ';' separators and what's after the second */
        if (i >= 3 && content[cur_save] == 'r' && content[cur_save - 1] == 'o' && content[cur_save - 2] == 'f')
        {
            int sepctr = 0;
            cur_save ++;
            while (sepctr != 3) /* 0 - start, 1 - end of init, 2 - end of condition, 3 - end of expression and maybe the statement if it's a 1 line for*/
            {
                if (is_string_delimiter(content[cur_save]))    /* skip the strings inside */
                {
                    cur_save ++;
                    int strlimend = 0; /* string limit ended*/
                    while (!strlimend)
                    {
                        if (is_string_delimiter(content[cur_save]) && content[cur_save - 1] != C_BACKSLASH)
                        {
                            strlimend = 1;
                        }
                        if (!strlimend)
                        {
                            cur_save ++;
                        }
                    }
                }
                cur_save ++;
                if (is_phrase_delimiter(content[cur_save]))
                {
                    sepctr ++;
                }
            }
        }


        /* now check if we made to a phrase delimiter */
        if (is_phrase_delimiter(content[cur_save]))
        {
            int regular_delim = 1;
            if (content[cur_save] == '{')
            {
                /* now check if this curly brace is after the followings: = , in (
                . : ? If yes then this will be considered
                an array operation and the content of the curly brace will be
                dealt as an array expression*/
                int save_cur_save = cur_save - 1;
                while (is_whitespace(content[save_cur_save]))
                {
                    save_cur_save --;
                }
                switch (content[save_cur_save])
                {
                case '=':
                case ',':
                case '(':
                case ':':
                case '?':
                    regular_delim = 0;
                    break;
                case 'n':
                    if (save_cur_save > position)
                    {
                        if (content[save_cur_save] == 'i')
                        {
                            regular_delim = 0;
                        }
                    }
                    break;
                }
            }
            if (regular_delim)
            {
                *delim  = content[cur_save];
                phrase_read = 1;
                if (C_SEMC == content[cur_save])
                {
                    cur_save ++;
                    skipper = 0;
                    while (cur_save < content_size &&    is_whitespace(content[cur_save]))
                    {
                        cur_save ++;
                    }
                    while (cur_save < content_size && (C_CARRET == content[cur_save] || C_NEWLINE == content[cur_save]))
                    {
                        cur_save ++;
                    }
                }
                else if (C_CLOSE_BLOCK == content[cur_save])
                {
                    skipper = 0;
                }
            }
            else
            {
                int level = 1;
                int can_stop = 0;
                cur_save ++; /* skip the opening brace */
                while (cur_save < content_size && !can_stop)
                {
                    if (C_QUOTE == content[cur_save])       /* skip the strings */
                    {
                        cur_save ++;
                        int can_stop2 = 0;
                        while (cur_save < content_size && !can_stop2)
                        {
                            if (C_QUOTE == content[cur_save] && C_BACKSLASH != content[cur_save])
                            {
                                can_stop2 = 1;
                            }
                            cur_save ++;
                        }
                    }
                    else
                    if (C_OPEN_BLOCK == content[cur_save])
                    {
                        level ++;
                    }
                    else
                    if (C_CLOSE_BLOCK == content[cur_save])
                    {
                        level --;
                    }

                    if (level == 0)
                    {
                        can_stop = 1;
                    }
                    if (!can_stop)
                    {
                        cur_save ++;
                    }
                }
            }
        }
        if (!phrase_read)
        {
            cur_save ++;
        }
    }

    if (cur_save == position)
    {
        if (content[cur_save] == '}')
        {
            cur_save ++;
        }
        else if (content[cur_save] == '{')
        {
            cur_save ++;
        }
        else
        {
            return NULL;
        }
    }

    size = cur_save - position;
    phrase = mcompiler->new_string(size + 1);
    strncpy(phrase, content + position, size);
    position = cur_save + skipper; /* to skip the delimiter */

    /* now patch the phrase, so that \n or \r or \t will be replaced with a space */
    int lineIncd = 0;
    for (i = 0; i < size; i++)
    {
        if (phrase[i] == C_CARRET || phrase[i] == C_NEWLINE)
        {
            expwloc->location->end_line_number ++;
            lineIncd ++;
            phrase[i] = C_SPACE;
            if (i + 1 < size && C_NEWLINE == phrase[i + 1])
            {
                phrase[i + 1] = C_SPACE;
            }
        }
        if (phrase[i] == C_TAB)
        {
            phrase[i] = C_SPACE;
        }
    }
    phrase = trim(phrase, mcompiler);
    if (phrase[strlen(phrase) - 1] == C_SEMC)
    {
        phrase[strlen(phrase) - 1] = 0;
    }
    expwloc->expression = phrase;
    current_line = expwloc->location->end_line_number - 1;
    return expwloc;
}

/**
 * Previews the next word, does not touch the parser's position
 */
char *parsed_file::parser_preview_next_word(char *delim)
{
    int save_prev_p = position;
    int save_prev_prev_p = previous_position;
    expression_with_location *expwloc = parser_next_phrase(delim);
    position = save_prev_p;                        /* nothing happened */
    previous_position = save_prev_prev_p;
    if (!expwloc)
    {
        return NULL;
    }
    char *fw = expwloc->expression;
    while (is_identifier_char(*fw))
    {
        fw ++;
    }
    char *result = mcompiler->new_string(fw - expwloc->expression + 1);
    strncpy(result, expwloc->expression, fw - expwloc->expression);
    result = trim(result, mcompiler);
    return result;
}

/**
 * Skips the whitespace, updates the position
 */
void parsed_file::parser_skip_whitespace()
{
    while (is_whitespace(content[position]))
    {
        position++;
    }
}

/**
 * Skips the next word and the leading/trailing whitespace before/after it
 */
void parsed_file::parser_skip_next_word()
{
    char delim;
    char *next_word = parser_preview_next_word(&delim);
    if (!next_word)
    {
        return;
    }
    parser_skip_whitespace();
    position += strlen(next_word);
    parser_skip_whitespace();
}

/**
 * This method loads the while commands block
 */
void parsed_file::deal_with_while_loading(call_context* cc, expression_tree* new_node,
                             method* the_method, char delim,
                             int current_level,
                                          expression_with_location* expwloc)
{
    /* firstly create a call context for it */
    char* while_cc_name = mcompiler->new_string(cc->get_name().length() + 10);
    sprintf(while_cc_name, "%s%s%s", cc->get_name().c_str(), STR_CALL_CONTEXT_SEPARATOR, STR_WHILE);
    call_context* while_cc = new call_context(cc->compiler(), CALL_CONTEXT_TYPE_WHILE, while_cc_name, the_method, cc);
    if(C_OPEN_BLOCK == delim && new_node->op_type == STATEMENT_WHILE)    /* normal WHILE with { }*/
    {
        load_next_block(the_method, while_cc, current_level + 1, current_level);    /* loading the function*/
    }
    else if(C_OPEN_BLOCK == delim && new_node->op_type == STATEMENT_WHILE_1L) /* while (a) if (b) if (c) { blabla; }*/
    {
        /* load the next statements into the while's call context */
        char d = delim;
        expression_with_location* next_exp = alloc_mem(expression_with_location,1, mcompiler);
        next_exp->location = expwloc->location;
        next_exp->expression = (mcompiler->duplicate_string(new_node->info.c_str()));
        //pf->position += strlen(new_node->info);
        load_next_single_phrase(next_exp, the_method, while_cc, &d, current_level + 1);
        new_node->op_type = STATEMENT_WHILE;
        new_node->info = mcompiler->duplicate_string(STR_WHILE);
    }
    else if(C_SEMC == delim && new_node->op_type == STATEMENT_WHILE_1L)            /* one lined while, load the next expression, which is being 'hacked' into new_node.info by the interpreter  */
    {
        while_cc->add_new_expression(new_node->info.c_str(), expwloc);
        new_node->op_type = STATEMENT_WHILE;
        new_node->info = mcompiler->duplicate_string(STR_WHILE);
    }
    else if(C_SEMC == delim && new_node->op_type == STATEMENT_WHILE)            /* one lined while, with empty command, skip pf to the first position after the ; */
    {
        parser_skip_whitespace();
        if(content[position] == C_SEMC)
        {
            position ++;        /* skip the ';'*/
        }
    }

    resw_while* while_st = new resw_while;
    while_st->logical_expr = (expression_tree*)new_node->reference->to_interpret;
    while_st->operations = while_cc;
    new_node->reference = new_envelope(while_st, STATEMENT_WHILE, mcompiler);
}

void parsed_file::deal_with_class_declaration(call_context* /*cc*/,
                                        expression_tree* new_node,
                                        method* /*the_method*/,
                                        char /*delim*/,
                                        int /*current_level*/,
                                        expression_with_location* /*expwloc*/)
{
    char* new_block = alloc_mem(char, content_size - position, mcompiler); // should be enough ...
    int lev = 1;
    int pos = position;
    int nbpos = 0;
    bool can_go = true;

    while(can_go && pos < content_size)
    {
        new_block[nbpos] = content[pos];
        if(content[pos] == '{') lev ++;
        if(content[pos] == '}') lev --;
        if(lev == 0) can_go = false;
        pos ++;
        nbpos ++;
    }
    position = pos + 1;     // skips the closing brace
    new_block[nbpos - 1] = 0;   // remove the closing brace
    parsed_file* npf = new parsed_file(new_block, mcompiler);
    npf->name = mcompiler->duplicate_string(name);
    expression_with_location* nexpwloc = NULL;
    char ndelim = 0;
    method* nmethod = 0;
    nexpwloc = npf->parser_next_phrase(&ndelim);
    call_context* class_cc = (call_context*)((envelope*)new_node->reference)->to_interpret;
    int nlevel  = -1;
    while(nexpwloc)
    {
        npf->load_next_single_phrase(nexpwloc, nmethod, class_cc, &ndelim, nlevel);
        nexpwloc = npf->parser_next_phrase(&ndelim);
    }

    free(new_block);
}

/**
* This method loads the for commands block
*/
void parsed_file::deal_with_for_loading(call_context* cc, expression_tree* new_node, method* the_method, char delim, int current_level, expression_with_location* expwloc)
{
    /* firstly create a call context for it */
    char* for_cc_name = mcompiler->new_string(cc->get_name().length() + 10);
    sprintf(for_cc_name, "%s%s%s", cc->get_name().c_str(), STR_CALL_CONTEXT_SEPARATOR, STR_FOR);
    call_context* for_cc = new call_context(cc->compiler(), CALL_CONTEXT_TYPE_FOR, for_cc_name, the_method, cc);
    if(C_OPEN_BLOCK == delim && new_node->op_type == STATEMENT_FOR)    /* normal FOR with { }*/
    {
        load_next_block(the_method, for_cc, current_level + 1, current_level);    /* loading the function*/
    }
    else if(C_OPEN_BLOCK == delim && new_node->op_type == STATEMENT_FOR_1L) /* for (a) if (b) if (c) { blabla; }*/
    {
        /* load the next statements into the for's call context */
        char d = delim;
        expression_with_location* next_exp = alloc_mem(expression_with_location,1, mcompiler);
        next_exp->location = expwloc->location;
        next_exp->expression = mcompiler->duplicate_string(new_node->info.c_str());
        //pf->position += strlen(new_node->info);
        load_next_single_phrase(next_exp, the_method, for_cc, &d, current_level + 1);
        new_node->op_type = STATEMENT_FOR;
        new_node->info = mcompiler->duplicate_string(STR_FOR);
    }
    else if(C_SEMC == delim && new_node->op_type == STATEMENT_FOR_1L)            /* one lined for, load the next expression, which is being 'hacked' into new_node.info by the interpreter  */
    {
        for_cc->add_new_expression(new_node->info.c_str(), expwloc);
        new_node->op_type = STATEMENT_FOR;
        new_node->info = mcompiler->duplicate_string(STR_FOR);
    }
    else if(C_SEMC == delim && new_node->op_type == STATEMENT_FOR)            /* one lined for, with empty command, skip pf to the first position after the ; */
    {
        parser_skip_whitespace();
        if(content[position] == C_SEMC)
        {
            position ++;        /* skip the ';'*/
        }
    }
    resw_for* rsfor = (resw_for*)new_node->reference->to_interpret;
    rsfor->operations = for_cc;
}

/**
 * Loads the next block of the 'if' command
 */
void parsed_file::deal_with_ifs_loading(call_context* cc, expression_tree* new_node, method* the_method, char delim, int current_level, expression_with_location* expwloc)
{
    /* firstly create a call context for it */
    char* if_cc_name = mcompiler->new_string(cc->get_name().length() + 6);
    sprintf(if_cc_name, "%s%s%s", cc->get_name().c_str(), STR_CALL_CONTEXT_SEPARATOR, STR_IF);
    call_context* if_cc = new call_context(cc->compiler(), CALL_CONTEXT_TYPE_IF, if_cc_name, the_method, cc);
    garbage_bin<call_context*>::instance().place(if_cc, mcompiler);

    if(C_OPEN_BLOCK == delim && new_node->op_type == STATEMENT_IF)    /* normal IF with { }*/
    {
        load_next_block(the_method, if_cc, current_level + 1, current_level);    /* loading the function*/
    }
    else if(C_OPEN_BLOCK == delim && new_node->op_type == STATEMENT_IF_1L) /* if (a) if (b) if (c) { blabla; }*/
    {
        /* load the next statements into the if's call context */
        char d = delim;
        expression_with_location* next_exp = alloc_mem(expression_with_location,1, mcompiler);
        next_exp->location = expwloc->location;
        next_exp->expression = mcompiler->duplicate_string(new_node->info.c_str());
        //pf->position += strlen(new_node->info);
        load_next_single_phrase(next_exp, the_method, if_cc, &d, current_level + 1);
        new_node->op_type = STATEMENT_IF;
        new_node->info = mcompiler->duplicate_string(STR_IF);
    }
    else if(C_SEMC == delim && new_node->op_type == STATEMENT_IF_1L)            /* one lined if, load the next expression, which is being 'hacked' into new_node.info by the interpreter  */
    {
        if_cc->add_new_expression(new_node->info.c_str(), expwloc);
        new_node->op_type = STATEMENT_IF;
        new_node->info = mcompiler->duplicate_string(STR_IF);
    }
    else if(C_SEMC == delim && new_node->op_type == STATEMENT_IF)            /* one lined if, with empty command, skip pf to the first position after the ; */
    {
        parser_skip_whitespace();
        if(content[position] == C_SEMC)
        {
            position ++;        /* skip the ';'*/
        }
    }

    /* here read the next word check if it's a plain 'else' */
    char delim2;
    char* nextw = parser_preview_next_word(&delim2);
    call_context* else_cc = NULL;
    if(nextw && !strcmp(nextw, STR_ELSE))
    {
        char* else_cc_name = mcompiler->new_string(cc->get_name().length() + 16);
        sprintf(else_cc_name, "%s%s%s:%s", cc->get_name().c_str(), STR_CALL_CONTEXT_SEPARATOR, STR_IF, STR_ELSE);
        else_cc = new call_context(cc->compiler(), CALL_CONTEXT_TYPE_ELSE, else_cc_name, the_method, cc);
        if(delim2 == C_SEMC)
        {
            expression_with_location* expwloc2 = parser_next_phrase(&delim);
            char* else_expression = trim(expwloc2->expression + strlen(STR_ELSE), mcompiler);
            if(strlen(else_expression) > 1)
            {
                else_cc->add_new_expression(else_expression, expwloc2);
            }
        }
        else if(C_OPEN_BLOCK == delim2)
        {
            /* here we should skip: the whitespace before the else, the else, the whitespace and the opening '{' */
            parser_skip_next_word();
            if(content[position] != C_OPEN_BLOCK)
            {
                if(is_identifier_char(content[position]))
                {   /* else followed by another statement. Load it with the single phrase loader int the else_cc */
                    char delim3 = 0;
                    expression_with_location* expwloc3 = parser_next_phrase(&delim3);
                    load_next_single_phrase(expwloc3, the_method, else_cc, &delim3, current_level);
                }
                else
                {
                    throw_error("Internal 1", NULL);    /* actually we shouldn't get here at all*/
                }
            }
            else
            {
                position ++;    /* skip the { */
                load_next_block(the_method, else_cc, current_level, current_level + 1);    /* loading the function*/
            }
        }
    }

    resw_if* if_st = alloc_mem(resw_if, 1, mcompiler);
    if_st->logical_expr = (expression_tree*)new_node->reference->to_interpret;
    if_st->if_branch = if_cc;
    if_st->else_branch = else_cc;

    new_node->reference = new_envelope(if_st, STATEMENT_IF, mcompiler);
}

/**
 * Loads the body of a function from the given parsed file
 */
void parsed_file::load_next_block(method* the_method, call_context* par_cc, int current_level, int orig_level)
{
    /* at this stage the pf points to the first statement in the file ...*/
    call_context* cc = par_cc;
    int can_stop = 0;
    while(!can_stop)
    {
        char delim = 0;
        int use_delim = 1;    /* the scope ofthis var is to check whether we need to open new unnamed code blocks. keywords which need blocks (if, while, etc) will make this zero, they are managing their own code blocks */
        expression_with_location* expwloc = parser_next_phrase(&delim);
        if(expwloc || (!expwloc && delim == C_OPEN_BLOCK) )
        {
            if(expwloc)
            {
                if( strlen(expwloc->expression) > 0 )
                {
                    expression_tree* new_node = cc->add_new_expression(expwloc->expression, expwloc);
                    /* now check whether this node was an If, a for, a while, a do or anything else that might need loading more rows and creating more contexts */
                    if(new_node->op_type == STATEMENT_IF || new_node->op_type == STATEMENT_IF_1L)
                    {
                        use_delim = 0;
                        deal_with_ifs_loading(cc, new_node, the_method, delim, current_level, expwloc);
                    }
                    else if(new_node->op_type == STATEMENT_WHILE || new_node->op_type == STATEMENT_WHILE_1L)
                    {
                        use_delim = 0;
                        deal_with_while_loading(cc, new_node, the_method, delim, current_level, expwloc);
                    }
                    else if(new_node->op_type == STATEMENT_FOR || new_node->op_type == STATEMENT_FOR_1L)
                    {
                        use_delim = 0;
                        deal_with_for_loading(cc, new_node, the_method, delim, current_level, expwloc);
                    }
                }
            }

            if(use_delim)
            {
                switch(delim)
                {
                case C_OPEN_BLOCK:
                {
                    char* new_cc_name = mcompiler->new_string(cc->get_name().length() + 10);
                    sprintf(new_cc_name, "%s%s%s", cc->get_name().c_str(), STR_CALL_CONTEXT_SEPARATOR, STR_UNNAMED_CC);
                    call_context* child_cc = new call_context(cc->compiler(), CALL_CONTEXT_TYPE_UNNAMED, new_cc_name, the_method, cc);
                    expression_tree* new_node = cc->add_new_expression(STR_OPEN_BLOCK, expwloc);
                    //todo: add a new expression in the current call CONTEXT to start executing the next call context.
                    new_node->reference = new_envelope(child_cc, ENV_TYPE_CC, mcompiler);
                    cc = child_cc;
                    current_level ++;
                    position ++;    /* hack again, to skip to the next character, to not to loop on the { 4ever */
                }
                break;
                case C_CLOSE_BLOCK:
                {
                    if(expwloc->expression[0] != '}') // put a new close block only if this is not a method def.
                    {
                        cc->add_new_expression(STR_CLOSE_BLOCK, expwloc);
                    }
                    cc = cc->get_father();    /* stepping back */
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
                cc->add_new_expression("}", expwloc);
                cc = cc->get_father();    /* stepping back */
                current_level --;
            }
            can_stop = 1;
        }
    }
}

void parsed_file::load_next_single_phrase(expression_with_location* expwloc, method* cur_method, call_context* cur_cc, char* delim, int level)
{
    char* first = expwloc->expression;
    if(first && strlen(first) > 0)
    {
        int op_res = -1;
        expression_tree* cnode = new expression_tree(expwloc);
        garbage_bin<expression_tree*>::instance().place(cnode, mcompiler);

        void* gen_res = cur_cc->compiler()->get_interpreter().build_expr_tree(first, cnode, cur_method, first, cur_cc, &op_res, expwloc);
        switch(op_res)
        {
        case ASM_BLOCK: /*load the next block and directly feed in the statements to the cnode's envelope*/
        {
            load_next_assembly_block(cur_cc);    /* loading the function*/
            break;
        }
        case NT_VARIABLE_DEF_LST:
            cur_cc->add_compiled_expression(cnode);
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
                    load_next_block(cur_method, cur_cc, level, saved_level);    /* loading the function*/
                    cur_cc = saved_cc; /* restoring the current cc */
                }
            }
            cur_method = NULL;
            break;

        case STATEMENT_IF:
        case STATEMENT_IF_1L:
            cur_cc->add_compiled_expression(cnode);
            deal_with_ifs_loading(cur_cc, cnode, cur_method, *delim, level, expwloc);
            break;

        case STATEMENT_WHILE:
        case STATEMENT_WHILE_1L:
            cur_cc->add_compiled_expression(cnode);
            deal_with_while_loading(cur_cc, cnode, cur_method, *delim, level, expwloc);
            break;

        case STATEMENT_FOR:
        case STATEMENT_FOR_1L:
            cur_cc->add_compiled_expression(cnode);
            deal_with_for_loading(cur_cc, cnode, cur_method, *delim, level, expwloc);
            break;

        case CLASS_DECLARATION:
            deal_with_class_declaration(cur_cc, cnode, 0, *delim, level, expwloc);
            break;

        case FUNCTION_CALL:
        case FUNCTION_CALL_CONSTRUCTOR:
        case FUNCTION_CALL_OF_OBJECT:
        case FUNCTION_CALL_STATIC:
        default:
            cur_cc->add_compiled_expression(cnode);
            break;
        }
    }
}

void parsed_file::load_next_assembly_block(call_context* par_cc)
{
    int can_stop = 0;
    while(!can_stop) /*read till we get '}' */
    {
        char delim = 0;
        expression_with_location* expwloc = parser_next_phrase(&delim);

        expression_tree* tmp = new expression_tree(expwloc) ;
        tmp->op_type = ASM_BLOCK;

        par_cc->add_compiled_expression(tmp);

        if(delim == '}') can_stop = 1;
    }
}

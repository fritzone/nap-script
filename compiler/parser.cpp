#include "parser.h"
#include "utils.h"
#include "consts.h"
#include "call_ctx.h"
#include "res_wrds.h"
#include "envelope.h"
#include "common_structs.h"
#include "expression_tree.h"
#include "interpreter.h"
#include "compiler.h"

#include <stdlib.h>
#include <string.h>
#include <sstream>

/**
 * Reads the next phrase, returns it
 */
parsed_file::~parsed_file()
{
    for(size_t i=0; i<expressions.size(); i++)
    {
        delete expressions[i];
    }
}

/**
 * This file removes the comments from the input file. This is the first step and should be done right after the
 */
void parsed_file::remove_comments()
{
    int cloc = 0;
    std::string new_content = "";
    while (cloc < content_size)
    {

        bool already_inced = false;

        /* grab a whole bunch of comments at once ... */
        /* first comment style: C++'s // */
        if ((C_SLASH == content[cloc] && cloc + 1 < content_size && content[cloc + 1] == C_SLASH))
        {
            while (cloc < content_size && (content[cloc] != '\n' && content[cloc] != '\r'))    /* End-line ... */
            {
                content[cloc] = C_SPACE;
                cloc ++;
                already_inced = true;
            }
        }


        /* second comment style: C's classic comments. like this. */
        if (C_SLASH == content[cloc] && cloc + 1 < content_size && C_STAR == content[cloc + 1])
        {
            content[cloc++] = C_SPACE;                 /* skip the / and * */
            content[cloc++] = C_SPACE;

            char a = content[cloc];
            if(cloc + 1 >= content_size - 1)
            {
                mcompiler->throw_error("Comment is not closed till the end of the file");
                return;
            }

            char b = content[cloc + 1];

			for (;;)
            {
                a = content[cloc];
                b = content[cloc + 1];
                content[cloc++] = C_SPACE; // overwrites the *


                if(a == C_STAR && b == C_SLASH )
                {
                    break;
                }

                // Did we run out of the code? ie: Unfinished comment?
                if(cloc == content_size - 1)
                {
                    mcompiler->throw_error("Comment is not closed till the end of the file");
                    return;
                }

            }

            content[cloc++] = C_SPACE; // overwrites the /
            already_inced = true;
        }
        else
        {
            new_content += content[cloc];
        }
        if(!already_inced)
        {
            cloc ++;
        }
    }
    content = new_content;
    content_size = new_content.size();
}

void parsed_file::add_new_expression(expression_with_location *expw)
{
    expressions.push_back(expw);
}

/**
 * Opens the given file, creates a new prsed file structure
 */
parsed_file *parsed_file::open_file(const std::string& name,  const nap_compiler* _compiler)
{
    parsed_file *f = new parsed_file(_compiler);
    long size = -1;
    f->name = name;
    /* the file pointer*/
    FILE *fp;

    fp = fopen(f->name.c_str(), "rb");
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
    char* s = (char*)calloc(size + 1, 1);
	if(s == NULL)
	{
		delete f;
		return NULL;
	}
    f->content_size = size;

    size_t r = fread(s, size, sizeof(char), fp);
    if(r == 0)
    {
        free(s);
        fclose(fp);
        return f;
    }
    f->content = s;
    free(s);
    fclose(fp);
    f->position = 0;
    f->current_line = 1;
    f->remove_comments();
    return f;
}

parsed_file *parsed_file::set_source(const char *src,  const nap_compiler* _compiler)
{
    parsed_file *f = new parsed_file(_compiler);
    long size = strlen(src);

    f->content_size = size;
    f->content = src;

    f->position = 0;
    f->current_line = 1;
    f->remove_comments();
    return f;
}

expression_with_location* parsed_file::parser_next_phrase(char *delim)
{
    if(position >= content_size)
    {
        return NULL;
    }
    previous_position = position;
    // owned by the parsed file
    expression_with_location *expwloc = new expression_with_location;
    add_new_expression(expwloc);

    expwloc->location = file_location(position, current_line, current_line, mcompiler->file_index_for_name(name) );

    long cur_save = position;

    // very first step: skip the "leading" spaces and newlines
    while (cur_save < content_size && is_whitespace(content[cur_save]))
    {
        if(content[cur_save] == '\r' || content[cur_save] == '\n')
        {
            expwloc->location.start_line_number ++;
            expwloc->location.end_line_number ++;
            current_line ++;
        }
        cur_save ++;
        position ++;
    }

    long size = -1;
    int i = 0;
    int phrase_read = 0;
    int skipper = 1; // the scope of this variable is to skip one character forward if we reached a ";" separator, but nothing if we reached "{" or "}"
    while (cur_save < content_size && !phrase_read)
    {
        // and again skip the "leading" spaces and newlines
        while (cur_save < content_size && is_whitespace(content[cur_save]))
        {
            if(content[cur_save] == '\r' || content[cur_save] == '\n')
            {
                expwloc->location.start_line_number ++;
                expwloc->location.end_line_number ++;
                current_line ++;
            }
            cur_save ++;
        }

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

    if (cur_save == position && position < content_size)
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

    // did we go after the last character?
    if(cur_save > content_size)
    {
        return NULL;
    }

    size = cur_save - position;
    std::string phrase;
    phrase = content.substr(position, size);

    position = cur_save + skipper; /* to skip the delimiter */
    if(size == 1 && phrase == "{")
    {
        position --; // go back one if this just opened a block
    }

    /* now patch the phrase, so that \n or \r or \t will be replaced with a space */
    int lineIncd = 0;
    for (i = 0; i < size; i++)
    {
        if (phrase[i] == C_CARRET || phrase[i] == C_NEWLINE)
        {
            // inc the end lin, but only if there is something after it, such as
            // this is an endline in the middle ofthe phrase, not at the end
            bool can_inc_endline = false;
            for(int j=i + 1; j < size; j++)
            {
                if(!is_whitespace(phrase[j]))
                {
                    can_inc_endline = true;
                    break;
                }
            }
            if(can_inc_endline)
            {
                expwloc->location.end_line_number  ++;
            }

            current_line ++;
            lineIncd ++;
            phrase[i] = C_SPACE;
        }
        if (phrase[i] == C_TAB)
        {
            phrase[i] = C_SPACE;
        }
    }

    strim(phrase);
	if (!phrase.empty() && phrase[phrase.length() - 1] == C_SEMC)
    {
        phrase = phrase.substr(0, phrase.length() - 1);
    }
    expwloc->expression = phrase;
    return expwloc;
}

/**
 * Previews the next word, does not touch the parser's position
 */
std::string parsed_file::parser_preview_next_word(char *delim)
{
    int save_prev_p = position;
    int save_prev_prev_p = previous_position;
    int save_current_line = current_line;

    expression_with_location *expwloc = parser_next_phrase(delim);

    position = save_prev_p;                        /* nothing happened */
    previous_position = save_prev_prev_p;
    current_line  =save_current_line;

    if (!expwloc)
    {
        return NULL;
    }
    const char *fw = expwloc->expression.c_str();
    std::string s;
    while (is_identifier_char(*fw))
    {
        s += (*fw);
        fw ++;
    }
    strim(s);
    return s;
}

/**
 * Skips the whitespace, updates the position
 */
void parsed_file::parser_skip_whitespace()
{
    while (is_whitespace(content[position]))
    {
        if(content[position] == '\r' || content[position] == '\n')
        {
            current_line ++;
        }
        position++;
    }
}

/**
 * Skips the next word and the leading/trailing whitespace before/after it
 */
void parsed_file::parser_skip_next_word()
{
    char delim;
    std::string next_word = parser_preview_next_word(&delim);
    if (next_word.empty())
    {
        return;
    }
    parser_skip_whitespace();
    position += next_word.length();
    parser_skip_whitespace();
}

/**
 * This method loads the while commands block
 */
void parsed_file::deal_with_while_loading(call_context* cc, expression_tree* new_node,
                             method* the_method, char delim,
                             int current_level, expression_with_location* expwloc, bool &psuccess)
{
    /* firstly create a call context for it */
    std::stringstream ss;
    ss << cc->name << STR_CALL_CONTEXT_SEPARATOR << STR_WHILE;

    call_context* while_cc = new call_context(cc->compiler, call_context::CC_WHILE, ss.str(), the_method, cc);
    if(C_OPEN_BLOCK == delim && new_node->op_type == STATEMENT_WHILE)    /* normal WHILE with { }*/
    {
        load_next_block(the_method, while_cc, current_level + 1, current_level, psuccess);    /* loading the function*/
        SUCCES_OR_RETURN;
    }
    else if(C_OPEN_BLOCK == delim && new_node->op_type == STATEMENT_WHILE_1L) /* while (a) if (b) if (c) { blabla; }*/
    {
        /* load the next statements into the while's call context */
        char d = delim;
        expression_with_location* next_exp = new expression_with_location;
        add_new_expression(next_exp);
        next_exp->location = expwloc->location;
        next_exp->expression = new_node->info;

        load_next_single_phrase(next_exp, the_method, while_cc, &d, current_level + 1, psuccess);
        SUCCES_OR_RETURN;

        new_node->op_type = STATEMENT_WHILE;
        new_node->info = STR_WHILE;
    }
    else if(C_SEMC == delim && new_node->op_type == STATEMENT_WHILE_1L)            /* one lined while, load the next expression, which is being 'hacked' into new_node.info by the interpreter  */
    {
        while_cc->add_new_expression(new_node->info.c_str(), expwloc, psuccess);
        SUCCES_OR_RETURN;

        new_node->op_type = STATEMENT_WHILE;
        new_node->info = STR_WHILE;
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
                                        expression_with_location* /*expwloc*/, bool& psuccess)
{
    std::string new_block;
    int lev = 1;
    int pos = position;
    bool can_go = true;

    while(can_go && pos < content_size)
    {
        new_block += content[pos];
        if(content[pos] == '{') lev ++;
        if(content[pos] == '}') lev --;
        if(lev == 0) can_go = false;
        pos ++;
    }
    position = pos + 1;     // skips the closing brace
    new_block = new_block.substr(0, new_block.length() - 1); // remove closing brace
    parsed_file* npf = new parsed_file(new_block.c_str(), new_block.length(), mcompiler);
    npf->name = this->name;
    expression_with_location* nexpwloc = NULL;
    char ndelim = 0;
    method* nmethod = 0;
    nexpwloc = npf->parser_next_phrase(&ndelim);
    call_context* class_cc = (call_context*)((envelope*)new_node->reference)->to_interpret;
    int nlevel  = -1;
    while(nexpwloc)
    {
        npf->load_next_single_phrase(nexpwloc, nmethod, class_cc, &ndelim, nlevel, psuccess);
        SUCCES_OR_RETURN;

        nexpwloc = npf->parser_next_phrase(&ndelim);
    }
}

/**
* This method loads the for commands block
*/
void parsed_file::deal_with_for_loading(call_context* cc,
                                        expression_tree* new_node,
                                        method* the_method,
                                        char delim,
                                        int current_level,
                                        expression_with_location* expwloc, bool& psuccess)
{
    /* firstly create a call context for it */
    std::stringstream ss;
    ss << cc->name << STR_CALL_CONTEXT_SEPARATOR << STR_FOR;

    call_context* for_cc = new call_context(cc->compiler, call_context::CC_FOR, ss.str(), the_method, cc);

    if(C_OPEN_BLOCK == delim && new_node->op_type == STATEMENT_FOR)    /* normal FOR with { }*/
    {
        load_next_block(the_method, for_cc, current_level + 1, current_level, psuccess);    /* loading the function*/
        SUCCES_OR_RETURN;
    }
    else if(C_OPEN_BLOCK == delim && new_node->op_type == STATEMENT_FOR_1L) /* for (a) if (b) if (c) { blabla; }*/
    {
        /* load the next statements into the for's call context */
        char d = delim;
        expression_with_location* next_exp = new expression_with_location;
        next_exp->location = expwloc->location;
        next_exp->expression = new_node->info;

        load_next_single_phrase(next_exp, the_method, for_cc, &d, current_level + 1, psuccess);
        SUCCES_OR_RETURN;

        new_node->op_type = STATEMENT_FOR;
        new_node->info = STR_FOR;
    }
    else if(C_SEMC == delim && new_node->op_type == STATEMENT_FOR_1L)            /* one lined for, load the next expression, which is being 'hacked' into new_node.info by the interpreter  */
    {
        for_cc->add_new_expression(new_node->info, expwloc, psuccess);
        SUCCES_OR_RETURN;

        new_node->op_type = STATEMENT_FOR;
        new_node->info = STR_FOR;
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

void parsed_file::deal_with_codeblock_loading(call_context *cc, expression_tree *new_node,
                                              method *the_method, char /*delim*/, int current_level, expression_with_location */*expwloc*/,
                                              bool &psuccess)
{
    std::stringstream ss;
    ss << cc->name << STR_CALL_CONTEXT_SEPARATOR << generate_unique_hash();
    call_context* new_cc = new call_context(cc->compiler, call_context::CC_UNNAMED, ss.str(), the_method, cc);

    load_next_block(the_method, new_cc, current_level + 1, current_level, psuccess);    /* loading the block*/
    new_node->reference =  new_envelope(new_cc, STATEMENT_NEW_CC, mcompiler);
    new_node->op_type = STATEMENT_NEW_CC;
    SUCCES_OR_RETURN;
}

/**
 * Loads the next block of the 'if' command
 */
void parsed_file::deal_with_ifs_loading(call_context* cc,
                                        expression_tree* new_node,
                                        method* the_method,
                                        char delim,
                                        int current_level,
                                        expression_with_location* expwloc, bool& psuccess)
{
    /* firstly create a call context for it */
    std::stringstream ss;
    ss << cc->name << STR_CALL_CONTEXT_SEPARATOR << STR_IF;
    call_context* if_cc = new call_context(cc->compiler, call_context::CC_IF, ss.str(), the_method, cc);

    if(C_OPEN_BLOCK == delim && new_node->op_type == STATEMENT_IF)    /* normal IF with { }*/
    {
        load_next_block(the_method, if_cc, current_level + 1, current_level, psuccess);    /* loading the function*/
        SUCCES_OR_RETURN;
    }
    else if(C_OPEN_BLOCK == delim && new_node->op_type == STATEMENT_IF_1L) /* if (a) if (b) if (c) { blabla; }*/
    {
        /* load the next statements into the if's call context */
        char d = delim;
        expression_with_location* next_exp = new expression_with_location;
        next_exp->location = expwloc->location;
        next_exp->expression = new_node->info;

        load_next_single_phrase(next_exp, the_method, if_cc, &d, current_level + 1, psuccess);
        SUCCES_OR_RETURN;

        new_node->op_type = STATEMENT_IF;
        new_node->info = STR_IF;
    }
    else if(C_SEMC == delim && new_node->op_type == STATEMENT_IF_1L)            /* one lined if, load the next expression, which is being 'hacked' into new_node.info by the interpreter  */
    {
        if_cc->add_new_expression(new_node->info.c_str(), expwloc, psuccess);
        SUCCES_OR_RETURN;

        new_node->op_type = STATEMENT_IF;
        new_node->info = STR_IF;
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
    std::string nextw = parser_preview_next_word(&delim2);
    call_context* else_cc = NULL;
    if(nextw == STR_ELSE)
    {
        std::stringstream ss;
        ss << cc->name << STR_CALL_CONTEXT_SEPARATOR << STR_IF << C_UNDERLINE << STR_ELSE;
        else_cc = new call_context(cc->compiler, call_context::CC_ELSE, ss.str(), the_method, cc);
        if(delim2 == C_SEMC)
        {
            expression_with_location* expwloc2 = parser_next_phrase(&delim);
            std::string s = expwloc2->expression.substr(strlen(STR_ELSE));
            if(s.length() > 1)
            {
                /* if after a one line if there is an else */
                if( !expwloc->expression.empty() )
                {
                    expression_tree* new_node2 = else_cc->add_new_expression(s, expwloc, psuccess);
                    SUCCES_OR_RETURN;
                    /* now check whether this node was an If, a for, a while, a do or anything else that might need loading more rows and creating more contexts */
                    if(new_node2->op_type == STATEMENT_IF || new_node2->op_type == STATEMENT_IF_1L)
                    {
                        deal_with_ifs_loading(else_cc, new_node2, the_method, delim, current_level, expwloc, psuccess);
                        SUCCES_OR_RETURN;
                    }
                    else if(new_node2->op_type == STATEMENT_WHILE || new_node2->op_type == STATEMENT_WHILE_1L)
                    {
                        deal_with_while_loading(else_cc, new_node2, the_method, delim, current_level, expwloc, psuccess);
                        SUCCES_OR_RETURN;
                    }
                    else if(new_node2->op_type == STATEMENT_FOR || new_node2->op_type == STATEMENT_FOR_1L)
                    {
                        deal_with_for_loading(else_cc, new_node2, the_method, delim, current_level, expwloc, psuccess);
                        SUCCES_OR_RETURN;
                    }
                }
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
                    load_next_single_phrase(expwloc3, the_method, else_cc, &delim3, current_level, psuccess);
                    SUCCES_OR_RETURN;
                }
                else
                {
                    mcompiler->throw_error("Internal 1", NULL);    /* actually we shouldn't get here at all*/
                    psuccess = false;
                    return;
                }
            }
            else
            {
                position ++;    /* skip the { */
                load_next_block(the_method, else_cc, current_level, current_level + 1, psuccess);    /* loading the function*/
                SUCCES_OR_RETURN;
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
void parsed_file::load_next_block(method* the_method, call_context* par_cc, int current_level, int orig_level, bool& psuccess)
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
                if( !expwloc->expression.empty() )
                {
                    expression_tree* new_node = cc->add_new_expression(expwloc->expression, expwloc, psuccess);
                    SUCCES_OR_RETURN;

                    /* now check whether this node was an If, a for, a while, a do or anything else that might need loading more rows and creating more contexts */
                    if(new_node->op_type == STATEMENT_IF || new_node->op_type == STATEMENT_IF_1L)
                    {
                        use_delim = 0;
                        deal_with_ifs_loading(cc, new_node, the_method, delim, current_level, expwloc, psuccess);
                        SUCCES_OR_RETURN;
                    }
                    else if(new_node->op_type == STATEMENT_WHILE || new_node->op_type == STATEMENT_WHILE_1L)
                    {
                        use_delim = 0;
                        deal_with_while_loading(cc, new_node, the_method, delim, current_level, expwloc, psuccess);
                        SUCCES_OR_RETURN;
                    }
                    else if(new_node->op_type == STATEMENT_FOR || new_node->op_type == STATEMENT_FOR_1L)
                    {
                        use_delim = 0;
                        deal_with_for_loading(cc, new_node, the_method, delim, current_level, expwloc, psuccess);
                        SUCCES_OR_RETURN;
                    }
                }
            }

            if(use_delim)
            {
                switch(delim)
                {
                case C_OPEN_BLOCK:
                {
                    std::stringstream ss;
                    ss << cc->name << STR_CALL_CONTEXT_SEPARATOR << STR_UNNAMED_CC;
                    call_context* child_cc = new call_context(cc->compiler, call_context::CC_UNNAMED, ss.str(), the_method, cc);
                    expression_tree* new_node = cc->add_new_expression(STR_OPEN_BLOCK, expwloc, psuccess);
                    SUCCES_OR_RETURN;
                    //todo: add a new expression in the current call CONTEXT to start executing the next call context.
                    new_node->reference = new_envelope(child_cc, ENV_TYPE_CC, mcompiler);
                    cc = child_cc;
                    current_level ++;
                    position ++;    /* hack again, to skip to the next character, to not to loop on the { 4ever */
                }
                break;
                case C_CLOSE_BLOCK:
                {
                    if(expwloc->expression[0] != C_CLOSE_BLOCK) // put a new close block only if this is not a method def.
                    {
                        cc->add_new_expression(STR_CLOSE_BLOCK, expwloc, psuccess);
                        SUCCES_OR_RETURN;
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
                cc->add_new_expression("}", expwloc, psuccess);
                SUCCES_OR_RETURN;

                cc = cc->father;    /* stepping back */
                current_level --;
            }
            can_stop = 1;
        }
    }
}

void parsed_file::load_next_single_phrase(expression_with_location* expwloc, method* cur_method, call_context* cc, char* delim, int level, bool& psuccess)
{
    if(!expwloc->expression.empty())
    {
        int op_res = -1;
        expression_tree* cnode = expwloc->new_expression();


        void* gen_res = cc->compiler->get_interpreter().build_expr_tree(expwloc->expression,
                                                                        cnode, cur_method, expwloc->expression.c_str(), cc, &op_res, expwloc, psuccess, 0);
        SUCCES_OR_RETURN;

        switch(op_res)
        {
        case ASM_BLOCK: /*load the next block and directly feed in the statements to the cnode's envelope*/
        {
            load_next_assembly_block(cc);    /* loading the function*/
            break;
        }
        case NT_VARIABLE_DEF_LST:
            cc->expressions.push_back(cnode);
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
                    call_context* saved_cc = cc; /* saving the current cc*/
                    cc = cur_method->main_cc;

                    load_next_block(cur_method, cc, level, saved_level, psuccess);    /* loading the function*/
                    SUCCES_OR_RETURN;
                    cc = saved_cc; /* restoring the current cc */
                }
            }
            cur_method = NULL;
            break;

        case STATEMENT_IF:
        case STATEMENT_IF_1L:
        {
            cc->expressions.push_back(cnode);

            deal_with_ifs_loading(cc, cnode, cur_method, *delim, level, expwloc, psuccess);
            SUCCES_OR_RETURN;

            break;
        }
        case STATEMENT_WHILE:
        case STATEMENT_WHILE_1L:
        {
            cc->expressions.push_back(cnode);
            deal_with_while_loading(cc, cnode, cur_method, *delim, level, expwloc, psuccess);
            SUCCES_OR_RETURN;
            break;
        }
        case STATEMENT_FOR:
        case STATEMENT_FOR_1L:
        {
            cc->expressions.push_back(cnode);
            deal_with_for_loading(cc, cnode, cur_method, *delim, level, expwloc, psuccess);
            SUCCES_OR_RETURN;
            break;
        }
        case CLASS_DECLARATION:
        {
            deal_with_class_declaration(cc, cnode, 0, *delim, level, expwloc, psuccess);
            SUCCES_OR_RETURN;
            break;
        }

        case STATEMENT_NEW_CC:
            cc->expressions.push_back(cnode);
            deal_with_codeblock_loading(cc, cnode, 0, *delim, level, expwloc, psuccess);
            break;

        case FUNCTION_CALL:
        case FUNCTION_CALL_CONSTRUCTOR:
        case FUNCTION_CALL_OF_OBJECT:
        case FUNCTION_CALL_STATIC:

        default:
            cc->expressions.push_back(cnode);
            break;


        }
    }
}

void parsed_file::load_next_assembly_block(call_context* cc)
{
    int can_stop = 0;
    while(!can_stop) /*read till we get '}' */
    {
        char delim = 0;
        expression_with_location* expwloc = parser_next_phrase(&delim);
        if(!expwloc)
        {
            can_stop = 1;
        }
        else
        {
            if(expwloc->expression[0] != '}')
            {
                expression_tree* tmp = expwloc->new_expression();
                tmp->op_type = ASM_BLOCK;

                cc->expressions.push_back(tmp);
            }
        }
        if(delim == '}')
        {
            can_stop = 1;
        }
    }
}

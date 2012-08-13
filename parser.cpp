#include "parser.h"
#include "utils.h"
#include "consts.h"
#include "is.h"

#include <stdlib.h>
#include <string.h>

/**
 * This file removes the comments from the input file. This is the first step and should be done right after the
 */
static void remove_comments(parsed_file* f)
{
int cloc = 0;
    while(cloc < f->content_size)
    {
        /* grab a whole bunch of comments at once ... */
        /* first comment style: Unix style # and C++'s //. Both of these go till end of line*/
        if(C_HASH == f->content[cloc] || (C_SLASH == f->content[cloc] && cloc+1 < f->content_size && f->content[cloc + 1] == C_SLASH) )
        {
            while(cloc < f->content_size && (f->content[cloc] != '\n' && f->content[cloc] != '\r'))	/* End-line ... */
            {
                f->content[cloc] = C_SPACE;
                cloc ++;
            }
        }
        /* second comment style: C's classic comments. like this. */
        if(C_SLASH == f->content[cloc] && cloc + 1 < f->content_size && C_STAR == f->content[cloc + 1])
        {
            f->content[cloc++] = C_SPACE; 				/* skip the / and * */
            f->content[cloc++] = C_SPACE;

            while(cloc < f->content_size - 1 && (C_STAR != f->content[cloc] && C_SLASH != f->content[cloc + 1] ))	/* End-comment ... will stop at the closing '*' */
            {
                f->content[cloc++] = C_SPACE;
            }
            f->content[cloc++] = C_SPACE;				/* and the closing ones too */
            f->content[cloc++] = C_SPACE;
        }
        cloc ++;
    }
}

/**
 * Opens the given file, creates a new prsed file structure
 */
parsed_file* open_file(const char* name)
{
parsed_file* f = alloc_mem(parsed_file,1);
long size = -1;
    f->name = name;
    f->fp = fopen(f->name, "rb");
    if(!f->fp)
    {
        free(f);
        return NULL;
    }

    fseek(f->fp, 0, SEEK_END);
    size=ftell(f->fp);
    if(size == -1)
    {
        free(f);
        return NULL;
    }
    fseek(f->fp, 0, SEEK_SET);
    f->content = alloc_mem(char,size);
    f->content_size = size;
    fread(f->content, size, sizeof(char), f->fp);
    fclose(f->fp);
    f->position = 0;
    f->current_line = 0;
    remove_comments(f);
    return f;
}

/**
 * Reads the next phrase, returns it
 */
expression_with_location* parser_next_phrase(parsed_file* f, char* delim)
{
    f->previous_position = f->position;
expression_with_location* expwloc = alloc_mem(expression_with_location,1);
    expwloc->location = alloc_mem(file_location,1);
    expwloc->location->location = f->position;
    expwloc->location->start_line_number = f->current_line + 1;
    expwloc->location->end_line_number = f->current_line + 1;
    expwloc->location->file_name = f->name;
long cur_save = f->position;
long size = -1;
char* phrase = NULL;
int i = 0;
int phrase_read = 0;
int skipper = 1;
    while(cur_save < f->content_size && !phrase_read)
    {
        i++;
        /* trying to bypass a string and ignore the special characters inside it */
        if(is_string_delimiter(f->content[cur_save]))
        {
            cur_save ++;
            i++;
        int strlimend = 0; /* string limit ended*/
            while(!strlimend)
            {
                if(is_string_delimiter(f->content[cur_save]) && f->content[cur_save-1] != C_BACKSLASH)
                {
                    strlimend = 1;
                }
                if(!strlimend) cur_save ++;
            }
        }

        /* now check for the special case of 'for' statement. This must read in two ';' separators and what's after the second */
        if(i >=3 && f->content[cur_save] == 'r' && f->content[cur_save - 1] == 'o' && f->content[cur_save - 2] == 'f')
        {
        int sepctr = 0;
            cur_save ++;
            while(sepctr != 3) /* 0 - start, 1 - end of init, 2 - end of condition, 3 - end of expression and maybe the statement if it's a 1 line for*/
            {
                if(is_string_delimiter(f->content[cur_save]))	/* skip the strings inside */
                {
                    cur_save ++;
                int strlimend = 0; /* string limit ended*/
                    while(!strlimend)
                    {
                        if(is_string_delimiter(f->content[cur_save]) && f->content[cur_save-1] != C_BACKSLASH)
                        {
                            strlimend = 1;
                        }
                        if(!strlimend) cur_save ++;
                    }
                }
                cur_save ++;
                if(is_phrase_delimiter(f->content[cur_save])) sepctr ++;
            }
        }


        /* now check if we made to a phrase delimiter */
        if(is_phrase_delimiter(f->content[cur_save]))
        {
            int regular_delim = 1;
            if(f->content[cur_save] == '{')
            {
                /* now check if this curly brace is after the followings: = , in (
                . : ? If yes then this will be considered
                an array operation and the content of the curly brace will be
                dealt as an array expression*/
                int save_cur_save = cur_save - 1;
                while(is_whitespace(f->content[save_cur_save])) save_cur_save --;
                switch(f->content[save_cur_save])
                {
                case '=':
                case ',':
                case '(':
                case ':':
                case '?':
                    regular_delim = 0;
                    break;
                case 'n':
                    if(save_cur_save > f->position)
                    {
                        if(f->content[save_cur_save] == 'i')
                        {
                            regular_delim = 0;
                        }
                    }
                    break;
                }
            }
            if(regular_delim)
            {
                *delim  = f->content[cur_save];
                phrase_read = 1;
                if(C_SEMC == f->content[cur_save])
                {
                    cur_save ++;
                    skipper = 0;
                    while(cur_save < f->content_size &&	is_whitespace(f->content[cur_save])) cur_save ++;
                    while(cur_save < f->content_size && (C_CARRET == f->content[cur_save] || C_NEWLINE == f->content[cur_save]))
                    {
                        cur_save ++;
                    }
                }
                else
                if(C_CLOSE_BLOCK == f->content[cur_save])
                {
                    skipper = 1;
                }
            }
            else
            {
            int level = 1;
            int can_stop = 0;
                cur_save ++; /* skip the opening brace */
                while(cur_save < f->content_size && !can_stop)
                {
                    if(C_QUOTE == f->content[cur_save])        /* skip the strings */
                    {
                        cur_save ++;
                        int can_stop2 = 0;
                        while(cur_save < f->content_size && !can_stop2)
                        {
                            if(C_QUOTE == f->content[cur_save] && C_BACKSLASH != f->content[cur_save]) can_stop2 = 1;
                            cur_save ++;
                        }
                    }
                    if(f->content[cur_save] == C_OPEN_BLOCK) level ++;
                    if(f->content[cur_save] == C_CLOSE_BLOCK) level --;
                    if(level == 0) can_stop = 1;
                    if(!can_stop) cur_save ++;
                }
            }
        }
        if(!phrase_read) cur_save ++;
    }

    if(cur_save == f->position)
    {
        if(f->content[cur_save] == '}')
        {
            cur_save ++;
        }
        else
        if(f->content[cur_save] == '{')
        {
            cur_save ++;
        }
        else
        {
            return NULL;
        }
    }

    size = cur_save - f->position;
    phrase = new_string(size + 1);
    strncpy(phrase, f->content + f->position, size);
    f->position = cur_save + skipper; /* to skip the delimiter */

    /* now patch the phrase, so that \n or \r or \t will be replaced with a space */
int lineIncd = 0;
    for(i=0; i<size; i++)
    {
        if(phrase[i] == C_CARRET || phrase[i] == C_NEWLINE)
        {
            expwloc->location->end_line_number ++;
            lineIncd ++;
            phrase[i] = C_SPACE;
            if(i+1 < size && C_NEWLINE == phrase[i+1]) phrase[i + 1] = C_SPACE;
        }
        if(phrase[i] == C_TAB)
        {
            phrase[i] = C_SPACE;
        }
    }
    phrase = trim(phrase);
    if(phrase[strlen(phrase) - 1] == C_SEMC) phrase[strlen(phrase) - 1] = 0;
    expwloc->expression = phrase;
    f->current_line = expwloc->location->end_line_number - 1;
    return expwloc;
}

/**
 * Previews the next word, does not touch the parser's position
 */
char* parser_preview_next_word(struct parsed_file* f, char* delim)
{
expression_with_location* expwloc = parser_next_phrase(f, delim);
    f->position = f->previous_position;						/* nothing happened */
    if(!expwloc)
    {
        return NULL;
    }
char *fw = expwloc->expression;
    while(is_identifier_char(*fw)) fw ++;
char* result = new_string(fw - expwloc->expression + 1);
    strncpy(result, expwloc->expression, fw - expwloc->expression);
    result = trim(result);
    return result;
}

/**
 * Skips the whitespace, updates the position
 */
void parser_skip_whitespace(struct parsed_file* pf)
{
    while(is_whitespace(pf->content[pf->position])) pf->position++;
}

/**
 * Skips the next word and the leading/trailing whitespace before/after it
 */
void parser_skip_next_word(struct parsed_file* pf)
{
char delim;
char* next_word = parser_preview_next_word(pf, &delim);
    if(!next_word)
    {
        return;
    }
    parser_skip_whitespace(pf);
    pf->position += strlen(next_word);
    parser_skip_whitespace(pf);
}

#ifndef _PARSER_H_
#define _PARSER_H_

#include "common_structs.h"

/**
 * Opens the given file, and in case of success returns a parsed_file structure
 */


/**
 * The structure of a parsed file
 */
struct parsed_file
{
public:
    parsed_file(const char *content);

    parsed_file();

    /**
     * Retrieves the next phrase from the given parsed file. The phrase delimitators are the followings:
     * { } ;
     * The first two denote the beginning and end of blocks, the ; denotes the end of a command
     * If NULL the file is finished
     */
    expression_with_location *parser_next_phrase(char *delim);

    /**
     * Skips the whitespace in this parser
     */
    void parser_skip_whitespace();

    /**
     * Tells the parser to skip the next word. Also skips the whitespace before and after the word.
     * Positions the parser to the next word in the flow
     */
    void parser_skip_next_word();


    /**
     * Previews the next word from the parsed file, without modifying the buffer's position.
     * Returns the word, and populates the delim with the delimiter.
     */
    char* parser_preview_next_word(char* delim);

    void remove_comments();

    static parsed_file* open_file(const char* name);

    static parsed_file* set_source(const char* src);

    /* TODO: make this private, create another start parsing function ...*/
    void load_next_single_phrase(expression_with_location* expwloc,
                                 method* cur_method, call_context* cur_cc,
                                 char* delim, int level);
private:

    void deal_with_while_loading(call_context* cc, expression_tree* new_node,
                                 method* the_method, char delim,
                                 int current_level, expression_with_location* expwloc);

    void deal_with_class_declaration(call_context* /*cc*/,
                                            expression_tree* new_node,
                                            method* /*the_method*/,
                                            char /*delim*/,
                                            int /*current_level*/,
                                            expression_with_location* /*expwloc*/);

    void deal_with_for_loading(call_context* cc, expression_tree* new_node,
                               method* the_method, char delim,
                               int current_level, expression_with_location* expwloc);

    void deal_with_ifs_loading(call_context* cc, expression_tree* new_node,
                               method* the_method, char delim, int current_level,
                               expression_with_location* expwloc);

    void load_next_block(method* the_method, call_context* par_cc,
                         int current_level, int orig_level);


    void load_next_assembly_block(call_context* par_cc);

private:
    /* name of the file */
    const char *name;

    /* this is the content of the file*/
    char *content;

    /* the current position of the reader */
    long position;

    /* the size of the file ... */
    long content_size;

    /* the current line number */
    long current_line;

    /* the previous position where the parser was before reading the current expression */
    long previous_position;
};

#endif

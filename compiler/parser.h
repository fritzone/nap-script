#ifndef _PARSER_H_
#define _PARSER_H_

#include "common_structs.h"

class nap_compiler;

/**
 * The structure of a parsed file
 */
struct parsed_file
{
public:

    parsed_file(const nap_compiler* _compiler): name(), position(0),
        content_size(0), current_line(1), mcompiler(_compiler), expressions()
    {}

    parsed_file(const char *pcontent, size_t content_len, const nap_compiler* _compiler) :
        name(), content(pcontent), position(0), content_size(content_len),
        current_line(1), mcompiler(_compiler), expressions()
    {}

    virtual ~parsed_file();

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
    std::string parser_preview_next_word(char* delim);

    void remove_comments();

    void add_new_expression(expression_with_location* expw);

    static parsed_file* open_file(const std::string &name,  const nap_compiler* _compiler );

    static parsed_file* set_source(const char* src,  const nap_compiler* _compiler);

    /* TODO: make this private, create another start parsing function ...*/
    void load_next_single_phrase(expression_with_location* expwloc,
                                 method* cur_method, call_context* cc,
                                 char* delim, int level, bool &psuccess);
private:

    void deal_with_while_loading(call_context* cc, expression_tree* new_node,
                                 method* the_method, char delim,
                                 int current_level, expression_with_location* expwloc, bool &psuccess);

    void deal_with_class_declaration(call_context* /*cc*/,
                                            expression_tree* new_node,
                                            method* /*the_method*/,
                                            char /*delim*/,
                                            int /*current_level*/,
                                            expression_with_location* /*expwloc*/, bool &psuccess);

    void deal_with_for_loading(call_context* cc, expression_tree* new_node,
                               method* the_method, char delim,
                               int current_level, expression_with_location* expwloc, bool &psuccess);

    void deal_with_codeblock_loading(call_context* cc, expression_tree* new_node,
                               method* the_method, char delim,
                               int current_level, expression_with_location* expwloc, bool &psuccess);



    void deal_with_ifs_loading(call_context* cc, expression_tree* new_node,
                               method* the_method, char delim, int current_level,
                               expression_with_location* expwloc, bool &psuccess);

    void load_next_block(method* the_method, call_context* par_cc,
                         int current_level, int orig_level, bool&psuccess);


    void load_next_assembly_block(call_context* cc);

private:
    /* name of the file */
    std::string name;

    /* this is the content of the file*/
    std::string content;

    /* the current position of the reader */
    long position;

    /* the size of the file ... */
    long content_size;

    /* the current line number */
    long current_line;

    /* the previous position where the parser was before reading the current expression */
    long previous_position;

    const nap_compiler* mcompiler;

    std::vector<expression_with_location*> expressions;
};

#endif

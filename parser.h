#ifndef _PARSER_H_
#define _PARSER_H_

#include "common_structs.h"

/**
 * Returns an empty parsed file
 */
parsed_file* new_parsed_file(const char* content);

/**
 * Opens the given file, and in case of success returns a parsed_file structure
 */
parsed_file* open_file(const char* name);

/**
 * Retrieves the next phrase from the given parsed file. The phrase delimitators are the followings:
 * { } ;
 * The first two denote the beginning and end of blocks, the ; denotes the end of a command
 * If NULL the file is finished
 */
expression_with_location* parser_next_phrase(struct parsed_file* f, char* delim);

/**
 * Previews the next word from the parsed file, without modifying the buffer's position.
 * Returns the word, and populates the delim with the delimiter.
 */
char* parser_preview_next_word(struct parsed_file* f, char* delim);

/**
 * Skips the whitespace in this parser
 */
void parser_skip_whitespace(struct parsed_file* pf);

/**
 * Tells the parser to skip the next word. Also skips the whitespace before and after the word.
 * Positions the parser to the next word in the flow
 */
void parser_skip_next_word(struct parsed_file* pf);

#endif

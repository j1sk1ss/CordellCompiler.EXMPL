#ifndef TKNZ_H_
#define TKNZ_H_

#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#include "mm.h"
#include "str.h"

#define TOKEN_MAX_SIZE  128
#define BUFFER_SIZE     8192

typedef enum {
    // Unknowns
    UNKNOWN_CHAR_VALUE,
    UNKNOWN_STRING_TOKEN,
    UNKNOWN_NUMERIC_TOKEN,
    UNKNOWN_COMMAND_TOKEN,

    COMMENT_TOKEN,
    DELIMITER_TOKEN,
    COMMA_TOKEN,
    OPEN_INDEX_TOKEN,
    CLOSE_INDEX_TOKEN,
    OPEN_BRACKET_TOKEN,
    CLOSE_BRACKET_TOKEN,
    OPEN_BLOCK_TOKEN,
    CLOSE_BLOCK_TOKEN,

    // Types
    PTR_TYPE_TOKEN,   // ptr
    RO_TYPE_TOKEN,    // ro
    GLOB_TYPE_TOKEN,  // glob
    LONG_TYPE_TOKEN,
    INT_TYPE_TOKEN,
    SHORT_TYPE_TOKEN,
    CHAR_TYPE_TOKEN,
    STR_TYPE_TOKEN,
    ARRAY_TYPE_TOKEN,
    
    // Commands
    IMPORT_TOKEN,        // import
    IMPORT_SELECT_TOKEN, // from
    START_TOKEN,         // start
    RETURN_TOKEN,        // return
    EXIT_TOKEN,          // exit
    SYSCALL_TOKEN,       // syscall
    CALL_TOKEN,
    
    // Function
    FUNC_TOKEN,          // function
    
    // Condition scope
    SWITCH_TOKEN,        // switch
    CASE_TOKEN,          // case
    DEFAULT_TOKEN,       // default
    WHILE_TOKEN,         // while
    IF_TOKEN,            // if
    ELSE_TOKEN,          // else
    
    // Statements
    PLUS_TOKEN,          // +
    MINUS_TOKEN,         // -
    MULTIPLY_TOKEN,      // *
    DIVIDE_TOKEN,        // /
    MODULO_TOKEN,        // %
    ASIGN_TOKEN,         // =
    COMPARE_TOKEN,       // ==
    NCOMPARE_TOKEN,      // !=
    LOWER_TOKEN,         // <
    LARGER_TOKEN,        // >
    BITMOVE_LEFT_TOKEN,  // >>
    BITMOVE_RIGHT_TOKEN, // <<
    BITAND_TOKEN,        // &
    BITOR_TOKEN,         // |
    BITXOR_TOKEN,        // ^
    AND_TOKEN,           // &&
    OR_TOKEN,            // ||
    
    // Vars
    LONG_VARIABLE_TOKEN,  // long
    INT_VARIABLE_TOKEN,   // int
    SHORT_VARIABLE_TOKEN, // short
    CHAR_VARIABLE_TOKEN,  // char
    STR_VARIABLE_TOKEN,   // str
    ARR_VARIABLE_TOKEN,   // arr

    // Values
    STRING_VALUE_TOKEN,
    CHAR_VALUE_TOKEN
} token_type_t;

typedef enum {
    CHAR_ALPHA,
    CHAR_DIGIT,
    CHAR_QUOTE,
    CHAR_SING_QUOTE,
    CHAR_BRACKET,
    CHAR_OTHER,
    CHAR_SPACE,
    CHAR_DELIMITER,
    CHAR_COMMA,
    CHAR_COMMENT,
    CHAR_NEWLINE
} char_type_t;

typedef struct token {
    // Token compiler information
    int ro;   // ReadOnly flag
    int glob; // Global flag
    int ptr;  // Is pointer flag
    token_type_t t_type;
    unsigned char value[TOKEN_MAX_SIZE];

    // Arch information
    struct token* next;
    
    // Symantic information
    int line_number;
} token_t;


/*
Allocate and create token.
Params:
    - type - Token type.
    - value - Token content.
    - len - Value variable size.
    - line - Token line.

Return pointer to token, or NULL if malloc error.
*/
token_t* create_token(token_type_t type, const unsigned char* value, size_t len, int line);

/*
Tokenize input file by provided FD.
Params:
    - fd - File descriptor of target file.

Return NULL or pointer to linked list of tokens.
Note: Function don't close file.
*/
token_t* tokenize(int fd);

/*
Unload linked list of tokens.
Params:
    - head - Linked list head.

Return 1 if unload success.
*/
int unload_tokens(token_t* head);

#endif
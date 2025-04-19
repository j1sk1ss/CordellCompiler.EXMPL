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
    UNKNOWN_STRING_TOKEN,
    UNKNOWN_NUMERIC_TOKEN,
    UNKNOWN_SYMBOL_TOKEN,
    UNKNOWN_COMMAND_TOKEN,

    COMMENT_TOKEN,
    DELIMITER_TOKEN,
    OPEN_INDEX_TOKEN,
    CLOSE_INDEX_TOKEN,
    OPEN_BLOCK_TOKEN,
    CLOSE_BLOCK_TOKEN,

    // Types
    PTR_TYPE_TOKEN,
    INT_TYPE_TOKEN,
    SHORT_TYPE_TOKEN,
    CHAR_TYPE_TOKEN,
    STRING_TYPE_TOKEN,
    ARRAY_TYPE_TOKEN,

    // Commands
    IMPORT_TOKEN,
    IMPORT_SELECT_TOKEN,
    START_TOKEN,
    RETURN_TOKEN,
    EXIT_TOKEN,
    SYSCALL_TOKEN,
    CALL_TOKEN,
    LABEL_TOKEN,

    // Function
    FUNC_TOKEN,
    
    // Condition scope
    WHILE_TOKEN,
    IF_TOKEN,
    ELSE_TOKEN,

    // Statements
    PLUS_TOKEN,
    MINUS_TOKEN,
    MULTIPLY_TOKEN,
    DIVIDE_TOKEN,
    ASIGN_TOKEN,
    COMPARE_TOKEN,
    NCOMPARE_TOKEN,
    LOWER_TOKEN,
    LARGER_TOKEN,
    BITMOVE_LEFT_TOKEN,
    BITMOVE_RIGHT_TOKEN,
    BITAND_TOKEN,
    BITOR_TOKEN,

    // Vars
    PTR_VARIABLE_TOKEN,
    INT_VARIABLE_TOKEN,
    SHORT_VARIABLE_TOKEN,
    CHAR_VARIABLE_TOKEN,
    STR_VARIABLE_TOKEN,
    ARR_VARIABLE_TOKEN,

    // Values
    INT_VALUE_TOKEN,
    STRING_VALUE_TOKEN
} token_type_t;

typedef enum {
    CHAR_ALPHA,
    CHAR_DIGIT,
    CHAR_QUOTE,
    CHAR_OPEN_INDEX,
    CHAR_CLOSE_INDEX,
    CHAR_OTHER,
    CHAR_SPACE,
    CHAR_DELIMITER,
    CHAR_COMMENT,
    CHAR_NEWLINE
} char_type_t;

typedef struct token {
    token_type_t t_type;
    int line_number;
    unsigned char value[TOKEN_MAX_SIZE];
    struct token* next;
} token_t;


token_t* create_token(token_type_t type, const unsigned char* value, size_t len, int line);

/*
*/
token_t* tokenize(int fd);

/*
*/
int unload_tokens(token_t* head);

#endif
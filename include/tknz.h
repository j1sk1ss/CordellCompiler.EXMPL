#ifndef TKNZ_H_
#define TKNZ_H_

#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#include "mm.h"
#include "str.h"

#define TOKEN_MAX_SIZE  36
#define BUFFER_SIZE     4096


typedef enum {
    // Unknowns
    UNKNOWN_STRING_TOKEN,
    UNKNOWN_NUMERIC_TOKEN,
    UNKNOWN_SYMBOL_TOKEN,
    UNKNOWN_COMMAND_TOKEN,

    // Commands
    DELIMITER_TOKEN,
    START_TOKEN,
    INT_TYPE_TOKEN,
    STRING_TYPE_TOKEN,
    ARRAY_TYPE_TOKEN,
    EXIT_TOKEN,
    WHILE_TOKEN,
    WHILE_START_TOKEN,
    WHILE_END_TOKEN,

    // Statements
    PLUS_TOKEN,
    MINUS_TOKEN,
    ASIGN_TOKEN,
    COMPARE_TOKEN,
    LOWER_TOKEN,
    LARGER_TOKEN,

    // Values
    INT_VALUE_TOKEN,
    STRING_VALUE_TOKEN
} token_type_t;

typedef enum {
    CHAR_ALPHA,
    CHAR_DIGIT,
    CHAR_OTHER,
    CHAR_SPACE,
    CHAR_DELIMITER,
    CHAR_NEWLINE
} char_type_t;

typedef struct token {
    token_type_t t_type;
    unsigned char value[TOKEN_MAX_SIZE];
    struct token* next;
} token_t;


token_t* tknz_tokenize(int fd);
int unload_tokens(token_t* head);

#endif
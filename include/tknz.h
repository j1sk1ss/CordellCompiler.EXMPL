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
    UNKNOWN_STRING,
    UNKNOWN_NUMERIC
} token_type_t;

typedef struct token {
    token_type_t t_type;
    unsigned char value[TOKEN_MAX_SIZE];
    struct token* next;
} token_t;


token_t* tknz_tokenize(int fd);
int unload_tokens(token_t* head);

#endif
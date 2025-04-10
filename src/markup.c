
#include "../include/syntax.h"


typedef struct {
    char* value;
    token_type_t type;
} markup_token_t;

static markup_token_t _markups[] = {
    { .value = START_COMMAND,       .type = START_TOKEN },
    { .value = INT_VARIABLE,        .type = INT_TYPE_TOKEN },
    { .value = STR_VARIABLE,        .type = STRING_TYPE_TOKEN },
    { .value = ARR_VARIABLE,        .type = ARRAY_TYPE_TOKEN },
    { .value = WHILE_COMAND,        .type = WHILE_TOKEN },
    { .value = WHILE_START_COMMAND, .type = WHILE_START_TOKEN },
    { .value = WHILE_END_COMMAND,   .type = WHILE_END_TOKEN },
    { .value = IF_COMMAND,          .type = IF_TOKEN },
    { .value = IF_START_COMMAND,    .type = IF_START_TOKEN },
    { .value = IF_END_COMMAND,      .type = IF_END_TOKEN },
    { .value = SYSCALL_COMMAND,     .type = SYSCALL_TOKEN },
    { .value = EXIT_COMMAND,        .type = EXIT_TOKEN },
    { .value = ASIGN_STATEMENT,     .type = ASIGN_TOKEN },
    { .value = COMPARE_STATEMENT,   .type = COMPARE_TOKEN },
    { .value = PLUS_STATEMENT,      .type = PLUS_TOKEN },
    { .value = MINUS_STATEMENT,     .type = MINUS_TOKEN },
    { .value = LARGER_STATEMENT,    .type = LARGER_TOKEN },
    { .value = LOWER_STATEMENT,     .type = LOWER_TOKEN },
    { .value = MULTIPLY_STATEMENT,  .type = MULTIPLY_TOKEN },
    { .value = DIVIDE_STATEMENT,    .type = DIVIDE_TOKEN }
};


int command_markup(token_t* head) {
    token_t* curr = head;
    while (curr) {
        for (int i = 0; i < (int)(sizeof(_markups) / sizeof(_markups[0])); i++) {
            if (((char*)curr->value)[0] != _markups[i].value[0]) continue;
            else if (!str_strcmp((char*)curr->value, _markups[i].value)) {
                curr->t_type = _markups[i].type;
            }
        }

        curr = curr->next;
    }

    return 1;
}

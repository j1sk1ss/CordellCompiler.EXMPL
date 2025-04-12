
#include "../include/syntax.h"


typedef struct {
    char* value;
    token_type_t type;
} markup_token_t;

typedef struct Variable {
    unsigned char name[TOKEN_MAX_SIZE];
    token_type_t type;
} variable_t;

static markup_token_t _markups[] = {
    // Special
    { .value = START_COMMAND,          .type = START_TOKEN       },
    { .value = SYSCALL_COMMAND,        .type = SYSCALL_TOKEN     },
    { .value = EXIT_COMMAND,           .type = EXIT_TOKEN        },

    // Function
    { .value = CALLFUNC_COMMAND,       .type = CALL_TOKEN        },
    { .value = FUNCTION_COMMAND,       .type = FUNC_TOKEN        },
    { .value = FUNCTION_START_COMMAND, .type = FUNC_START_TOKEN  },
    { .value = FUNCTION_END_COMMAND,   .type = FUNC_END_TOKEN    },

    // Variable
    { .value = INT_VARIABLE,           .type = INT_TYPE_TOKEN    },
    { .value = STR_VARIABLE,           .type = STRING_TYPE_TOKEN },
    { .value = ARR_VARIABLE,           .type = ARRAY_TYPE_TOKEN  },

    // While
    { .value = WHILE_COMAND,           .type = WHILE_TOKEN       },
    { .value = WHILE_START_COMMAND,    .type = WHILE_START_TOKEN },
    { .value = WHILE_END_COMMAND,      .type = WHILE_END_TOKEN   },

    // If
    { .value = IF_COMMAND,             .type = IF_TOKEN          },
    { .value = IF_START_COMMAND,       .type = IF_START_TOKEN    },
    { .value = IF_END_COMMAND,         .type = IF_END_TOKEN      },

    // Operators
    { .value = ASIGN_STATEMENT,        .type = ASIGN_TOKEN       },
    { .value = COMPARE_STATEMENT,      .type = COMPARE_TOKEN     },
    { .value = NCOMPARE_STATEMENT,     .type = NCOMPARE_TOKEN    },
    { .value = PLUS_STATEMENT,         .type = PLUS_TOKEN        },
    { .value = MINUS_STATEMENT,        .type = MINUS_TOKEN       },
    { .value = LARGER_STATEMENT,       .type = LARGER_TOKEN      },
    { .value = LOWER_STATEMENT,        .type = LOWER_TOKEN       },
    { .value = MULTIPLY_STATEMENT,     .type = MULTIPLY_TOKEN    },
    { .value = DIVIDE_STATEMENT,       .type = DIVIDE_TOKEN      }
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

int variable_markup(token_t* head) {
    token_t* curr = head;
    variable_t* variables = NULL;
    size_t var_count = 0;

    while (curr) {
        if (curr->t_type == INT_TYPE_TOKEN || curr->t_type == STRING_TYPE_TOKEN || curr->t_type == ARRAY_TYPE_TOKEN) {
            token_t* next = curr->next;
            if (next && next->t_type == UNKNOWN_STRING_TOKEN) {
                variables = mm_realloc(variables, (var_count + 1) * sizeof(variable_t));
                str_strncpy((char*)variables[var_count].name, (char*)next->value, TOKEN_MAX_SIZE);
                if (curr->t_type == INT_TYPE_TOKEN) variables[var_count].type = INT_VARIABLE_TOKEN;
                else if (curr->t_type == STRING_TYPE_TOKEN) variables[var_count].type = STR_VARIABLE_TOKEN;
                else if (curr->t_type == ARRAY_TYPE_TOKEN) variables[var_count].type = ARR_VARIABLE_TOKEN;
                var_count++;
            }
        }

        curr = curr->next;
    }

    curr = head;
    while (curr) {
        if (curr->t_type == UNKNOWN_STRING_TOKEN) {
            for (size_t i = 0; i < var_count; i++) {
                if (str_strncmp((char*)curr->value, (char*)variables[i].name, TOKEN_MAX_SIZE) == 0) {
                    curr->t_type = variables[i].type;
                    break;
                }
            }
        }

        curr = curr->next;
    }

    mm_free(variables);
    return 1;
}

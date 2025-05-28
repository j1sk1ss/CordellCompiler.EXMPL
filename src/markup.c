
#include "../include/syntax.h"


typedef struct {
    char* value;
    token_type_t type;
} markup_token_t;

typedef struct {
    int ro;
    int ptr;
    int glob;
    token_type_t type;
    unsigned char name[TOKEN_MAX_SIZE];
} variable_t;

static markup_token_t _markups[] = {
    /*
    Special single place tokens.
    */
    { .value = IMPORT_SELECT_COMMAND, .type = IMPORT_SELECT_TOKEN },
    { .value = IMPORT_COMMAND,        .type = IMPORT_TOKEN        },
    { .value = START_COMMAND,         .type = START_TOKEN         },
    { .value = EXIT_COMMAND,          .type = EXIT_TOKEN          },

    /*
    Bracket tokens.
    */
    { .value = OPEN_BLOCK,    .type = OPEN_BLOCK_TOKEN    },
    { .value = CLOSE_BLOCK,   .type = CLOSE_BLOCK_TOKEN   },
    { .value = OPEN_INDEX,    .type = OPEN_INDEX_TOKEN    },
    { .value = CLOSE_INDEX,   .type = CLOSE_INDEX_TOKEN   },
    { .value = OPEN_BRACKET,  .type = OPEN_BRACKET_TOKEN  },
    { .value = CLOSE_BRACKET, .type = CLOSE_BRACKET_TOKEN },

    /*
    Function and jmp tokens.
    */
    { .value = FUNCTION_COMMAND, .type = FUNC_TOKEN    },
    { .value = RETURN_COMMAND,   .type = RETURN_TOKEN  },
    { .value = SYSCALL_COMMAND,  .type = SYSCALL_TOKEN },

    /*
    Variable tokens.
    */
    { .value = PTR_COMMAND,    .type = PTR_TYPE_TOKEN   },
    { .value = RO_COMMAND,     .type = RO_TYPE_TOKEN    },
    { .value = GLOB_COMMAND,   .type = GLOB_TYPE_TOKEN  },
    { .value = LONG_VARIABLE,  .type = LONG_TYPE_TOKEN  },
    { .value = INT_VARIABLE,   .type = INT_TYPE_TOKEN   },
    { .value = SHORT_VARIABLE, .type = SHORT_TYPE_TOKEN },
    { .value = CHAR_VARIABLE,  .type = CHAR_TYPE_TOKEN  },
    { .value = STR_VARIABLE,   .type = STR_TYPE_TOKEN   },
    { .value = ARR_VARIABLE,   .type = ARRAY_TYPE_TOKEN },

    /*
    Little jump tokens.
    */
    { .value = SWITCH_COMMAND,  .type = SWITCH_TOKEN  },
    { .value = CASE_COMMAND,    .type = CASE_TOKEN    },
    { .value = DEFAULT_COMMAND, .type = DEFAULT_TOKEN },
    { .value = WHILE_COMAND,    .type = WHILE_TOKEN   },
    { .value = IF_COMMAND,      .type = IF_TOKEN      },
    { .value = ELSE_COMMAND,    .type = ELSE_TOKEN    },

    /*
    Binary operands.
    */
    { .value = ASIGN_STATEMENT,        .type = ASIGN_TOKEN         },
    { .value = COMPARE_STATEMENT,      .type = COMPARE_TOKEN       },
    { .value = NCOMPARE_STATEMENT,     .type = NCOMPARE_TOKEN      },
    { .value = PLUS_STATEMENT,         .type = PLUS_TOKEN          },
    { .value = MINUS_STATEMENT,        .type = MINUS_TOKEN         },
    { .value = LARGER_STATEMENT,       .type = LARGER_TOKEN        },
    { .value = LOWER_STATEMENT,        .type = LOWER_TOKEN         },
    { .value = MULTIPLY_STATEMENT,     .type = MULTIPLY_TOKEN      },
    { .value = DIVIDE_STATEMENT,       .type = DIVIDE_TOKEN        },
    { .value = MODULO_STATEMENT,       .type = MODULO_TOKEN        },
    { .value = BITMOVE_LEFT_STATEMENT, .type = BITMOVE_LEFT_TOKEN  },
    { .value = BITMOVE_RIGHT_STATMENT, .type = BITMOVE_RIGHT_TOKEN },
    { .value = BITAND_STATEMENT,       .type = BITAND_TOKEN        },
    { .value = BITOR_STATEMENT,        .type = BITOR_TOKEN         },
    { .value = BITXOR_STATEMENT,       .type = BITXOR_TOKEN        },
    { .value = AND_STATEMENT,          .type = AND_TOKEN           },
    { .value = OR_STATEMENT,           .type = OR_TOKEN            }
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

    int is_ro = 0;
    int is_glob = 0;
    int is_ptr = 0;
    while (curr) {
        switch (curr->t_type) {
            case IMPORT_TOKEN:
                curr = curr->next;
                while (curr->t_type != DELIMITER_TOKEN) {
                    variables = mm_realloc(variables, (var_count + 1) * sizeof(variable_t));
                    str_strncpy((char*)variables[var_count].name, (char*)curr->value, TOKEN_MAX_SIZE);
                    variables[var_count].type = CALL_TOKEN;
                    variables[var_count].ptr  = 0;
                    variables[var_count].glob = 0;
                    variables[var_count].ro   = 0;
                    var_count++;

                    curr = curr->next;
                }
            break;

            case GLOB_TYPE_TOKEN: is_glob = 1; break;
            case RO_TYPE_TOKEN:   is_ro = 1;   break;
            case PTR_TYPE_TOKEN:  is_ptr = 1;  break;

            case FUNC_TOKEN:
            case INT_TYPE_TOKEN:
            case STR_TYPE_TOKEN:
            case LONG_TYPE_TOKEN:
            case CHAR_TYPE_TOKEN:
            case SHORT_TYPE_TOKEN:
            case ARRAY_TYPE_TOKEN: {
                token_t* next = curr->next;
                if (next && (next->t_type == UNKNOWN_STRING_TOKEN || next->t_type == UNKNOWN_CHAR_VALUE)) {
                    variables = mm_realloc(variables, (var_count + 1) * sizeof(variable_t));
                    str_strncpy((char*)variables[var_count].name, (char*)next->value, TOKEN_MAX_SIZE);

                    switch (curr->t_type) {
                        case FUNC_TOKEN:        variables[var_count].type = CALL_TOKEN; break;
                        case INT_TYPE_TOKEN:    variables[var_count].type = INT_VARIABLE_TOKEN; break;
                        case STR_TYPE_TOKEN:    variables[var_count].type = STR_VARIABLE_TOKEN; break;
                        case LONG_TYPE_TOKEN:   variables[var_count].type = LONG_VARIABLE_TOKEN; break;
                        case CHAR_TYPE_TOKEN:   variables[var_count].type = CHAR_VARIABLE_TOKEN; break;
                        case SHORT_TYPE_TOKEN:  variables[var_count].type = SHORT_VARIABLE_TOKEN; break;
                        case ARRAY_TYPE_TOKEN:  variables[var_count].type = ARR_VARIABLE_TOKEN; break;
                        default: break;
                    }

                    curr->ro   = is_ro;
                    curr->glob = is_glob;
                    curr->ptr  = is_ptr;
                    variables[var_count].ro   = is_ro;
                    variables[var_count].glob = is_glob;
                    variables[var_count].ptr  = is_ptr;
                    var_count++;
                }

                is_ro = 0;
                is_ptr = 0;
                is_glob = 0;
            }
            break;
            default: break;
        }

        curr = curr->next;
    }

    curr = head;
    while (curr) {
        if (curr->t_type == UNKNOWN_STRING_TOKEN || curr->t_type == UNKNOWN_CHAR_VALUE) {
            for (size_t i = 0; i < var_count; i++) {
                if (str_strncmp((char*)curr->value, (char*)variables[i].name, TOKEN_MAX_SIZE) == 0) {
                    curr->t_type = variables[i].type;
                    curr->glob   = variables[i].glob;
                    curr->ro     = variables[i].ro;
                    curr->ptr    = variables[i].ptr;
                    break;
                }
            }
        }

        curr = curr->next;
    }

    mm_free(variables);
    return 1;
}

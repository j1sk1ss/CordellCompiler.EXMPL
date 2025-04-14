#include "../include/vars.h"


int get_variable_size(token_type_t token) {
    switch (token) {
        case UNKNOWN_NUMERIC_TOKEN:
        case STRING_VALUE_TOKEN:
        case PTR_VARIABLE_TOKEN:
        case INT_VARIABLE_TOKEN:
        case ARR_VARIABLE_TOKEN:
        case STR_VARIABLE_TOKEN: return 32;
        case SHORT_VARIABLE_TOKEN: return 16;
        case CHAR_VARIABLE_TOKEN: return 8;
        default: return 1;
    }
}

int is_variable(token_type_t token) {
    switch (token) {
        case PTR_TYPE_TOKEN:
        case INT_TYPE_TOKEN:
        case SHORT_TYPE_TOKEN:
        case CHAR_TYPE_TOKEN:
        case STRING_TYPE_TOKEN: return 1;
        default: return 0;
    }
}

int is_operand(token_type_t token) {
    switch (token) {
        case PLUS_TOKEN:
        case MINUS_TOKEN:
        case MULTIPLY_TOKEN:
        case DIVIDE_TOKEN:
        case BITAND_TOKEN:
        case BITOR_TOKEN:
        case BITMOVE_LEFT_TOKEN:
        case BITMOVE_RIGHT_TOKEN: return 1;
        default: return 0;
    }
}

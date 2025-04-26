#include "../include/vars.h"


/*
Return 1 if variable is an array-like data object.
Return 32 if 32bit variable
Return 16 if 16bit variable
Return 8 if 8bit variable
*/
int get_variable_type(token_type_t token) {
    switch (token) {
        case UNKNOWN_NUMERIC_TOKEN:
        case STR_VARIABLE_TOKEN:
        case ARR_VARIABLE_TOKEN:
        case STRING_VALUE_TOKEN: return 1;
        case PTR_VARIABLE_TOKEN:
        case INT_VARIABLE_TOKEN: return 32;
        case SHORT_VARIABLE_TOKEN: return 16;
        case CHAR_VARIABLE_TOKEN: return 8;
        default: return 0;
    }
} 

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
        case ARRAY_TYPE_TOKEN:
        case PTR_TYPE_TOKEN:
        case INT_TYPE_TOKEN:
        case SHORT_TYPE_TOKEN:
        case CHAR_TYPE_TOKEN:
        case STR_TYPE_TOKEN: return 1;
        default: return 0;
    }
}

int is_operand(token_type_t token) {
    switch (token) {
        case NCOMPARE_TOKEN:
        case COMPARE_TOKEN:
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

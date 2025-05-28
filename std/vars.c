#include "../include/vars.h"


/*
Return 1 if variable is an array-like data object.
Return 64 if 64bit variable.
Return 32 if 32bit variable.
Return 16 if 16bit variable.
Return 8 if 8bit variable.
*/
int get_variable_type(token_t* token) {
    if (token->ptr) return 1;
    switch (token->t_type) {
        case UNKNOWN_NUMERIC_TOKEN:
        case STR_VARIABLE_TOKEN:
        case ARR_VARIABLE_TOKEN:
        case STRING_VALUE_TOKEN: return 1;
        case LONG_VARIABLE_TOKEN: return 32;
        case INT_VARIABLE_TOKEN: return 32;
        case SHORT_VARIABLE_TOKEN: return 16;
        case CHAR_VALUE_TOKEN:
        case CHAR_VARIABLE_TOKEN: return 8;
        default: return 0;
    }
} 

/*
With token handling
*/
int get_variable_size(token_t* token) {
    if (token->ptr) return 32;
    switch (token->t_type) {
        case UNKNOWN_NUMERIC_TOKEN:
        case STRING_VALUE_TOKEN:
        case ARR_VARIABLE_TOKEN:
        case STR_VARIABLE_TOKEN: 
        case LONG_VARIABLE_TOKEN: return 32;
        case INT_VARIABLE_TOKEN: return 32;
        case SHORT_VARIABLE_TOKEN: return 16;
        case CHAR_VALUE_TOKEN:
        case CHAR_VARIABLE_TOKEN: return 8;
        default: return 1;
    }
}

/*
Without token handling
*/
int get_variable_size_wt(token_t* token) {
    switch (token->t_type) {
        case UNKNOWN_NUMERIC_TOKEN:
        case STRING_VALUE_TOKEN:
        case ARR_VARIABLE_TOKEN:
        case STR_VARIABLE_TOKEN: 
        case LONG_VARIABLE_TOKEN: return 32;
        case INT_VARIABLE_TOKEN: return 32;
        case SHORT_VARIABLE_TOKEN: return 16;
        case CHAR_VALUE_TOKEN:
        case CHAR_VARIABLE_TOKEN: return 8;
        default: return 1;
    }
}

int is_variable_decl(token_type_t token) {
    switch (token) {
        case ARRAY_TYPE_TOKEN:
        case LONG_TYPE_TOKEN:
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

int get_token_priority(token_type_t type) {
    switch (type) {
        case OR_TOKEN:             return 1;
        case AND_TOKEN:            return 2;
        case BITOR_TOKEN:          return 3;
        case BITXOR_TOKEN:         return 4;
        case BITAND_TOKEN:         return 5;
        case COMPARE_TOKEN:
        case NCOMPARE_TOKEN:
        case LOWER_TOKEN:
        case LARGER_TOKEN:         return 6;
        case BITMOVE_LEFT_TOKEN:
        case BITMOVE_RIGHT_TOKEN:  return 7;
        case PLUS_TOKEN:
        case MINUS_TOKEN:          return 8;
        case MULTIPLY_TOKEN:
        case DIVIDE_TOKEN:
        case MODULO_TOKEN:         return 9;
        case ASIGN_TOKEN:          return 0;
        default:                   return -1;
    }
}

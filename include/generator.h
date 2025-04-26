#ifndef TRANSLATOR_H_
#define TRANSLATOR_H_

#include "vars.h"
#include "arrmem.h"
#include "syntax.h"


#define ARRAYS_MAX_TOKEN 100
#define iprintf(out, fmt, ...) fprintf(out, "%*s" fmt, _current_depth * 4, "", ##__VA_ARGS__)
#define GET_ASMVAR(node) \
    (!((node)->token->ro || (node)->token->glob) ? \
        format_from_stack((node)->variable_offset) : \
        format_from_data((node)->token->value, (node)->token->t_type))      

static inline char* format_from_stack(int offset) {
    static char stack_buff[64] = { 0 };
    snprintf(stack_buff, sizeof(stack_buff), "[ebp - %d]", ALIGN_TO(offset, 4));
    return stack_buff;
}

static inline char* format_from_data(unsigned char* name, token_type_t type) {
    if (type != UNKNOWN_NUMERIC_TOKEN) {
        static char data_buff[64] = { 0 };
        snprintf(data_buff, sizeof(data_buff), "__%s__", name);
        return data_buff;
    }
    else {
        return (char*)name;
    }
}


/*
*/
int generate_asm(tree_t* root, FILE* output);

#endif
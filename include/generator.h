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
        format_from_data((node)->token->value))      

static inline char* format_from_stack(int offset) {
    static char buf[64] = { 0 };
    snprintf(buf, sizeof(buf), "[ebp - %d]", offset);
    return buf;
}

static inline char* format_from_data(unsigned char* name) {
    return (char*)name;
}


/*
*/
int generate_asm(tree_t* root, FILE* output);

#endif
#ifndef TRANSLATOR_H_
#define TRANSLATOR_H_

#include "vars.h"
#include "syntax.h"

#define ARRAYS_MAX_TOKEN    100
#define iprintf(out, fmt, ...) fprintf(out, "%*s" fmt, _current_depth * 4, "", ##__VA_ARGS__)

/*
*/
int generate_asm(tree_t* root, FILE* output);

#endif
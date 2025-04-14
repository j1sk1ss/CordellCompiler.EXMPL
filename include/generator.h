#ifndef TRANSLATOR_H_
#define TRANSLATOR_H_

#include "vars.h"
#include "syntax.h"

#define ARRAYS_MAX_TOKEN    100

/*
*/
int generate_asm(tree_t* root, FILE* output);

#endif
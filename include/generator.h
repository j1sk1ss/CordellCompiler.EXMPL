#ifndef TRANSLATOR_H_
#define TRANSLATOR_H_

#include "regs.h"
#include "vars.h"
#include "qsort.h"
#include "arrmem.h"
#include "syntax.h"

#define ARRAYS_MAX_TOKEN 100
#define iprintf(out, fmt, ...) fprintf(out, "%*s" fmt, _current_depth * 4, "", ##__VA_ARGS__)


/*
generate_asm function generates ASM code for target platform.
Params:
- root - AST tree root.
- output - Output file.

Return 1 if generation success.
Return 0 if something goes wrong.
*/
int generate_asm(const tree_t* root, FILE* output);

#endif
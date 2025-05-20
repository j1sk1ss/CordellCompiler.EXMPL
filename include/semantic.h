#ifndef SEMANTIC_H_
#define SEMANTIC_H_

#include "str.h"
#include "vars.h"
#include "logg.h"
#include "token.h"
#include "syntax.h"

/*
check_semantic function iterate throught AST and check semantic.
Params:
- root - AST head.

Return 1 if semantic is true.
Return 0 if semantic is wrong.
*/
int check_semantic(tree_t* root);

#endif
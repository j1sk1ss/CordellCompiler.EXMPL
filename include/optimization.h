#ifndef OPTIMIZATION_H_
#define OPTIMIZATION_H_

#include "mm.h"
#include "str.h"
#include "token.h"
#include "syntax.h"
#include "varmem.h"

#include <stdio.h>


int string_optimization(tree_t* root);
int assign_optimization(tree_t* root);
int varuse_optimization(tree_t* root);
int muldiv_optimization(tree_t* root);
int offset_optimization(tree_t* root);

#endif
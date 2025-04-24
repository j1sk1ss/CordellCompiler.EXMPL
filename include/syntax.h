#ifndef PARSER_H_
#define PARSER_H_

#include "arrmem.h"
#include "varmem.h"
#include "token.h"
#include "dict.h"
#include "vars.h"
#include "str.h"

#define ALIGN_TO(x, a) (((x) + (a) - 1) & ~((a) - 1))

typedef struct tree {
    token_t* token;
    struct tree* parent;
    struct tree* first_child;
    struct tree* next_sibling;
    int variable_offset;
    int variable_size;
} tree_t;


/*
Preparing tokens list for parsing tree.
We mark every token by command / symbol ar value type.
Params:
- head - Tokens list head.

Return -1 if somehing goes wrong.
Return 1 if markup success.
*/
int command_markup(token_t* head);

/*
*/
int variable_markup(token_t* head);

/*
*/
tree_t* create_syntax_tree(token_t* head);

/*
*/
int unload_syntax_tree(tree_t* node);

#endif
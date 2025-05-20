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
Iterate throught tokens and mark variables. 
Params:
- head - Tokens list head.

Return -1 if somehing goes wrong.
Return 1 if markup success.
*/
int variable_markup(token_t* head);

/*
Generate AST from tokens list.
Params: 
- head - Tokens list head.

Return NULL if somehing goes wrong.
Return pointer to tree head if markup success.
*/
tree_t* create_syntax_tree(token_t* head);

/*
Create new tree node with token.
Note: Avoid token free before tree free.
Params:
- token - Pointer to token.

Return pointer to tree.
*/
tree_t* create_tree_node(token_t* token);

/*
*/
int add_child_node(tree_t* parent, tree_t* child);

/*
*/
int remove_child_node(tree_t* parent, tree_t* child);

/*
*/
int unload_syntax_tree(tree_t* node);

#endif
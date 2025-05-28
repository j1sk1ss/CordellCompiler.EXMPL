#ifndef VARS_H_
#define VARS_H_

#include "token.h"

int get_variable_type(token_t* token);
int get_variable_size(token_t* token);
int get_variable_size_wt(token_t* token);
int is_variable_decl(token_type_t token);
int is_operand(token_type_t token);
int get_token_priority(token_type_t type);

#endif
#ifndef VARS_H_
#define VARS_H_

#include "token.h"

int get_variable_type(token_t* token);
int get_variable_size(token_t* token);
int is_variable(token_type_t token);
int is_operand(token_type_t token);

#endif
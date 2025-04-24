#ifndef VARMEM_H_
#define VARMEM_H_

#include "vars.h"
#include "token.h"

typedef struct variable_info {
    int size;
    int offset;
    char name[TOKEN_MAX_SIZE];
    char func[TOKEN_MAX_SIZE];
    struct variable_info* next;
} variable_info_t;


int get_vars_offset();
int set_vars_offset(int off);

int get_var_info(char* variable, char* func, variable_info_t* info);
int add_variable_info(char* name, int size, char* func);

int unload_varmap();

#endif
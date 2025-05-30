#ifndef VARMEM_H_
#define VARMEM_H_

#include "regs.h"
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
variable_info_t* get_varmap_head();
int set_varmap_head(variable_info_t* h);

int get_var_info(const char* variable, const char* func, variable_info_t* info);
int add_variable_info(const char* name, int size, const char* func);

int unload_varmap(variable_info_t* h);

#endif
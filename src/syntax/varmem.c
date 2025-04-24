#include "../../include/varmem.h"


static variable_info_t* _vars_h = NULL;
static int _current_offset_var = 0;

int get_vars_offset() {
    return _current_offset_var;
}

int set_vars_offset(int off) {
    _current_offset_var = off;
    return 1;
}

static variable_info_t* _find_variable(char* name, char* func) {
    variable_info_t* h = _vars_h;
    while (h) {
        if ((!func || !str_strcmp(func, h->func)) && !str_strcmp(name, h->name)) return h;
        h = h->next;
    }

    return NULL;
}

int get_var_info(char* variable, char* func, variable_info_t* info) {
    variable_info_t* data = _find_variable(variable, func);
    if (!data) return 0;

    str_memcpy(info, data, sizeof(variable_info_t));
    return 1;
}

static variable_info_t* _create_variable_info(char* name, int size, char* func) {
    variable_info_t* var = (variable_info_t*)mm_malloc(sizeof(variable_info_t));
    if (!var) return NULL;

    if (name) str_strncpy(var->name, name, TOKEN_MAX_SIZE);
    if (func) str_strncpy(var->func, func, TOKEN_MAX_SIZE);
    var->offset = _current_offset_var + size;
    var->size = size;
    var->next = NULL;

    _current_offset_var += size;
    return var;
}

int add_variable_info(char* name, int size, char* func) {
    variable_info_t* new_node = _create_variable_info(name, size, func);
    if (!new_node) return 0;
    
    variable_info_t* h = _vars_h;
    if (!h) {
        _vars_h = new_node;
        return new_node->offset;
    }

    while (h->next) {
        h = h->next;
    }

    if (h) {
        h->next = new_node;
        if (h->next) return new_node->offset;
        else return -1;
    }

    return 0;
}

int unload_varmap() {
    while (_vars_h) {
        variable_info_t* n = _vars_h->next;
        mm_free(_vars_h);
        _vars_h = n;
    }

    return 1;
}

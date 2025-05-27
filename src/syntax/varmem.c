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

variable_info_t* get_varmap_head() {
    return _vars_h;
}

int set_varmap_head(variable_info_t* h) {
    _vars_h = h;
    return 1;
}

int get_var_info(const char* variable, const char* func, variable_info_t* info) {
    variable_info_t* h = _vars_h;
    while (h) {
        if (((!func && h->func[0] == '\0') || !str_strcmp(func, h->func)) && !str_strcmp(variable, h->name)) {
            if (info) str_memcpy(info, h, sizeof(variable_info_t));
            return 1;
        }

        h = h->next;
    }
    
    return 0;
}

static variable_info_t* _create_variable_info(const char* name, int size, const char* func) {
    variable_info_t* var = (variable_info_t*)mm_malloc(sizeof(variable_info_t));
    if (!var) return NULL;

    if (func) str_strncpy(var->func, func, TOKEN_MAX_SIZE);
    else var->func[0] = '\0';

    str_strncpy(var->name, name, TOKEN_MAX_SIZE);
    _current_offset_var += size;
    _current_offset_var = (_current_offset_var + 7) & ~(7);
    var->offset = _current_offset_var;
    var->size = size;
    var->next = NULL;
    return var;
}

int add_variable_info(const char* name, int size, const char* func) {
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

    h->next = new_node;
    return new_node->offset;
}

int unload_varmap(variable_info_t* h) {
    while (h) {
        variable_info_t* n = h->next;
        mm_free(h);
        h = n;
    }

    _current_offset_var = 0;
    return 1;
}

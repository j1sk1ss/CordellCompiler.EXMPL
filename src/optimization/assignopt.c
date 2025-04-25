#include "../../include/optimization.h"


typedef struct unused_vars {
    variable_info_t* info;
    struct unused_vars* next;
} unused_vars_t;

static unused_vars_t* _unused_h = NULL;

static unused_vars_t* _create_unused_var_info(variable_info_t* var) {
    unused_vars_t* node = (unused_vars_t*)mm_malloc(sizeof(unused_vars_t));
    if (!node) return NULL;
    node->info = var;
    node->next = NULL;
    return node;
}

static int _add_unused_var(variable_info_t* var) {
    unused_vars_t* node = _create_unused_var_info(var);
    if (!node) return 0;

    unused_vars_t* h = _unused_h;
    if (!h) {
        _unused_h = node;
        return 1;
    }

    while (h->next) {
        h = h->next;
    }

    h->next = node;
    return 1;
}

static int _del_unused_var(variable_info_t* var) {
    unused_vars_t* prev = NULL;
    unused_vars_t* curr = _unused_h;

    while (curr) {
        if (curr->info == var) {
            if (prev) prev->next = curr->next;
            else _unused_h = curr->next;
            
            mm_free(curr);
            return 1;
        }

        prev = curr;
        curr = curr->next;
    }

    return 0;
}


static int _find_usage(tree_t* root) {
    if (!root) return 0;
    for (tree_t* t = root->first_child; t; t = t->next_sibling) {
        if (!t->token) {
            _find_usage(t);
            continue;
        }

        switch (t->token->t_type) {
            case IF_TOKEN:
            case WHILE_TOKEN: _find_usage(t->first_child->next_sibling); continue;
            case FUNC_TOKEN: _find_usage(t->first_child->next_sibling->next_sibling); continue;
            default: break;
        }

    }

    return 1;
}

static int _find_decl(tree_t* root, tree_t* entry, const char* func) {
    if (!root) return 0;
    for (tree_t* t = root->first_child; t; t = t->next_sibling) {
        if (!t->token) {
            _find_decl(t, entry, func);
            continue;
        }

        switch (t->token->t_type) {
            case IF_TOKEN:
            case WHILE_TOKEN: _find_decl(t->first_child->next_sibling, entry, func); continue;
            case FUNC_TOKEN: _find_decl(t->first_child->next_sibling->next_sibling, entry, (char*)t->first_child->token->value); continue;
            default: break;
        }

        if (get_variable_type(t->token->t_type) != 1) {
            tree_t* name_node = t->first_child;
            variable_info_t info;
            if (!get_var_info((char*)name_node->token->value, func, &info)) continue;
            
            variable_info_t* unused_info = (variable_info_t*)mm_malloc(sizeof(variable_info_t));
            if (!unused_info) return 0;
            else str_memcpy(unused_info, &info, sizeof(variable_info_t));
            _add_unused_var(unused_info);
        }
    }

    return 1;
}


int assign_optimization(tree_t* root) {
    return 1;
}

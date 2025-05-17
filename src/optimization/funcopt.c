#include "../../include/optimization.h"

static tree_t* _ast[100] = { NULL };
static int _ast_count = 0;

int funcopt_add_ast(tree_t* root) {
    _ast[_ast_count++] = root;
    return 1;
}

static int _find_func_usage_file(tree_t* root, const char* func, int* is_used) {
    if (!root) return 0;
    for (tree_t* t = root->first_child; t; t = t->next_sibling) {
        if (!t->token) {
            _find_func_usage_file(t, func, is_used);
            continue;
        }

        switch (t->token->t_type) {
            case FUNC_TOKEN: 
                if (str_strcmp((char*)t->first_child->token->value, func)) {
                    _find_func_usage_file(t->first_child->next_sibling->next_sibling, func, is_used);
                }
            break;
            case IMPORT_SELECT_TOKEN: continue;
            default: _find_func_usage_file(t, func, is_used); break;
        }

        if (t->token->t_type == CALL_TOKEN) {
            if (!str_strcmp((char*)t->token->value, func)) {
                *is_used = 1;
                return 1;
            }
        }
    }

    return 1;
}

static int _find_func_usage(const char* func, int* is_used) {
    for (int i = _ast_count - 1; i >= 0; i--) _find_func_usage_file(_ast[i], func, is_used);
    return 1;
}

static int _find_func(tree_t* root, int* delete) {
    if (!root) return 0;
    for (tree_t* t = root->first_child; t; t = t->next_sibling) {
        if (!t->token) {
            _find_func(t, delete);
            continue;
        }

        switch (t->token->t_type) {
            case FUNC_TOKEN: 
                int used = 0;    
                _find_func_usage((char*)t->first_child->token->value, &used);
                if (!used) {
                    remove_child_node(root, t);
                    unload_syntax_tree(t);
                    *delete = 1;
                }
                else _find_func(t->first_child->next_sibling->next_sibling, delete);
            break;
            default: _find_func(t, delete); break;
        }
    }

    return 1;
}

int func_optimization() {
    for (int i = _ast_count - 1; i >= 0; i--) {
        int is_delete = 0;
        do {
            is_delete = 0;
            _find_func(_ast[i], &is_delete);
        } while (is_delete);
    }

    return 1;
}

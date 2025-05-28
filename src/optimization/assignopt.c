#include "../../include/optimization.h"


static int _find_assign(tree_t* root, char* varname, int* status, int local) {
    if (!root) return 0;
    for (tree_t* t = root->first_child; t; t = t->next_sibling) {
        if (!t->token) {
            _find_assign(t, varname, status, local);
            continue;
        }

        switch (t->token->t_type) {
            case CASE_TOKEN:
            case SWITCH_TOKEN:
            case DEFAULT_TOKEN: _find_assign(t, varname, status, local); continue;
            case IF_TOKEN:
            case WHILE_TOKEN: _find_assign(t->first_child->next_sibling, varname, status, local); continue;
            case FUNC_TOKEN: if (!local) _find_assign(t->first_child->next_sibling->next_sibling, varname, status, local); continue;
            default: break;
        }

        if (t->token->t_type == ASIGN_TOKEN) {
            tree_t* left = t->first_child;
            if (!str_strncmp(varname, (char*)left->token->value, TOKEN_MAX_SIZE)) {
                *status = 1;
                return 1;
            }
        }
    }

    return 1;
}

static int _change_decl(tree_t* root, char* varname, int value, int local, int offset) {
    if (!root) return 0;

    int index = 0;
    for (tree_t* t = root->first_child; t; t = t->next_sibling) {
        if (index++ < offset) continue;
        if (!t->token) {
            _change_decl(t, varname, value, local, 0);
            continue;
        }
        
        switch (t->token->t_type) {
            case CASE_TOKEN: _change_decl(t, varname, value, local, 0); break;
            case LONG_TYPE_TOKEN:
            case INT_TYPE_TOKEN:
            case CHAR_TYPE_TOKEN: 
            case SHORT_TYPE_TOKEN: _change_decl(t, varname, value, local, 1); continue;
            case IF_TOKEN:
            case EXIT_TOKEN:
            case CALL_TOKEN:
            case PLUS_TOKEN:
            case ASIGN_TOKEN:
            case MINUS_TOKEN:
            case BITOR_TOKEN:
            case WHILE_TOKEN:
            case DIVIDE_TOKEN:
            case BITAND_TOKEN:
            case RETURN_TOKEN:
            case SWITCH_TOKEN:
            case SYSCALL_TOKEN:
            case DEFAULT_TOKEN:
            case COMPARE_TOKEN:
            case NCOMPARE_TOKEN:
            case MULTIPLY_TOKEN:
            case ARR_VARIABLE_TOKEN:
            case STR_VARIABLE_TOKEN:
            case BITMOVE_LEFT_TOKEN:
            case BITMOVE_RIGHT_TOKEN:
            case ARRAY_TYPE_TOKEN: _change_decl(t, varname, value, local, 0); continue;
            case FUNC_TOKEN: if (!local) _change_decl(t->first_child->next_sibling->next_sibling, varname, value, local, 0); continue;
            default: break;
        }

        if (!str_strncmp(varname, (char*)t->token->value, TOKEN_MAX_SIZE)) {
            snprintf((char*)t->token->value, TOKEN_MAX_SIZE, "%d", value);
            t->token->t_type = t->token->t_type != CASE_TOKEN ? UNKNOWN_NUMERIC_TOKEN : CASE_TOKEN;
            t->token->glob = t->token->t_type != CASE_TOKEN ? 1 : 0;
        }   
    }

    return 1;
}

static int _find_decl(tree_t* root, tree_t* entry, int* change) {
    if (!root) return 0;
    for (tree_t* t = root->first_child; t; t = t->next_sibling) {
        if (!t->token) {
            _find_decl(t, entry, change);
            continue;
        }

        switch (t->token->t_type) {
            case IF_TOKEN:
            case SWITCH_TOKEN:
            case WHILE_TOKEN: _find_decl(t->first_child->next_sibling, entry, change); continue;
            case FUNC_TOKEN: _find_decl(t->first_child->next_sibling->next_sibling, entry, change); continue;
            default: break;
        }

        if (is_variable_decl(t->token->t_type) && t->token->t_type != STR_TYPE_TOKEN && t->token->t_type != ARRAY_TYPE_TOKEN) {
            int is_changed = 0;
            tree_t* name_node = t->first_child;
            if (t->token->ro || t->token->glob) _find_assign(entry, (char*)name_node->token->value, &is_changed, 0);
            else _find_assign(root, (char*)name_node->token->value, &is_changed, 1);

            if (!is_changed) {
                tree_t* val_node = name_node->next_sibling;
                if (val_node->token->t_type != UNKNOWN_NUMERIC_TOKEN) continue;
                int value = str_atoi((char*)val_node->token->value);
                if (t->token->ro || t->token->glob) _change_decl(entry, (char*)name_node->token->value, value, 0, 0);
                else _change_decl(root, (char*)name_node->token->value, value, 1, 0);
                *change = 1;
            }
        }
    }

    return 1;
}


int assign_optimization(tree_t* root) {
    if (!root) return 0;
    int is_changed = 0;
    _find_decl(root, root, &is_changed);
    return is_changed;
}

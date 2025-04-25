#include "../../include/optimization.h"


static int _find_muldiv(tree_t* root) {
    if (!root) return 0;
    for (tree_t* t = root->first_child; t; t = t->next_sibling) {
        if (!t->token) {
            _find_muldiv(t);
            continue;
        }

        switch (t->token->t_type) {
            case PTR_TYPE_TOKEN:
            case INT_TYPE_TOKEN:
            case SHORT_TYPE_TOKEN:
            case CHAR_TYPE_TOKEN: 
            case CALL_TOKEN:
            case SYSCALL_TOKEN:
            case RETURN_TOKEN: _find_muldiv(t); continue;
            case IF_TOKEN:
            case WHILE_TOKEN: _find_muldiv(t->first_child->next_sibling); continue;
            case FUNC_TOKEN: _find_muldiv(t->first_child->next_sibling->next_sibling); continue;
            default: break;
        }

        if (t->token->t_type == MULTIPLY_TOKEN || t->token->t_type == DIVIDE_TOKEN) {
            tree_t* left = t->first_child;
            tree_t* right = left->next_sibling;
            if (right->token->t_type != UNKNOWN_NUMERIC_TOKEN) continue;

            int right_val = str_atoi((char*)right->token->value);
            // Is power of 2
            if ((right_val & (right_val - 1)) != 0) continue;
            
            int shift = 0;
            while (right_val >>= 1) shift++;
            t->token->t_type = (t->token->t_type == MULTIPLY_TOKEN) ? BITMOVE_LEFT_TOKEN : BITMOVE_RIGHT_TOKEN;
            snprintf((char*)right->token->value, TOKEN_MAX_SIZE, "%d", shift);
        }
    }

    return 1;
}

int muldiv_optimization(tree_t* root) {
    _find_muldiv(root);
    return 1;
}

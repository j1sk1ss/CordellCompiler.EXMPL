#include "../../include/optimization.h"


static int _find_muldiv(tree_t* root, int* fold) {
    if (!root) return 0;
    for (tree_t* t = root->first_child; t; t = t->next_sibling) {
        if (!t->token) {
            _find_muldiv(t, fold);
            continue;
        }

        switch (t->token->t_type) {
            case LONG_TYPE_TOKEN:
            case INT_TYPE_TOKEN:
            case SHORT_TYPE_TOKEN:
            case CHAR_TYPE_TOKEN: 
            case CALL_TOKEN:
            case SYSCALL_TOKEN:
            case RETURN_TOKEN: _find_muldiv(t, fold); continue;
            case IF_TOKEN:
            case WHILE_TOKEN: _find_muldiv(t->first_child->next_sibling, fold); continue;
            case FUNC_TOKEN: _find_muldiv(t->first_child->next_sibling->next_sibling, fold); continue;
            default: break;
        }

        /*
        Constant folding
        */
        if (is_operand(t->token->t_type)) {
            _find_muldiv(t, fold);
            tree_t* left = t->first_child;
            tree_t* right = left->next_sibling;
            if (left->token->t_type != UNKNOWN_NUMERIC_TOKEN || right->token->t_type != UNKNOWN_NUMERIC_TOKEN) break;

            int l_val = str_atoi((char*)left->token->value);
            int r_val = str_atoi((char*)right->token->value);
            int result = 0;
            switch (t->token->t_type) {
                case PLUS_TOKEN: result = l_val + r_val; break;
                case MINUS_TOKEN: result = l_val - r_val; break;
                case MULTIPLY_TOKEN: result = l_val * r_val; break;
                case DIVIDE_TOKEN: 
                    if (r_val == 0) break;
                    result = l_val / r_val; 
                    break;
                case BITAND_TOKEN: result = l_val & r_val; break;
                case BITOR_TOKEN: result = l_val | r_val; break;
                case BITMOVE_LEFT_TOKEN: result = l_val << r_val; break;
                case BITMOVE_RIGHT_TOKEN: result = l_val >> r_val; break;
                default: break;
            }

            *fold = 1;

            snprintf((char*)t->token->value, TOKEN_MAX_SIZE, "%d", result);
            t->token->t_type = UNKNOWN_NUMERIC_TOKEN;
            t->token->glob = 1;
            unload_syntax_tree(t->first_child->next_sibling);
            unload_syntax_tree(t->first_child);
            t->first_child = NULL;
        }
        
        /*
        Mult and div optimisation after folding
        */
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
    int is_fold = 0;
    _find_muldiv(root, &is_fold);
    return is_fold;
}

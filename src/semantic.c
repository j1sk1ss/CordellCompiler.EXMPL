#include "../include/semantic.h"


static int __check_operant_type(token_type_t token) {
    switch (token) {
        case UNKNOWN_NUMERIC_TOKEN:
        case STRING_VALUE_TOKEN:
        case PTR_VARIABLE_TOKEN:
        case INT_VARIABLE_TOKEN:
        case ARR_VARIABLE_TOKEN:
        case STR_VARIABLE_TOKEN: return 32;
        case SHORT_VARIABLE_TOKEN: return 16;
        case CHAR_VARIABLE_TOKEN: return 8;
        default: return 1;
    }
}


int check_semantic(tree_t* node) {
    if (!node) return 1;

    int result = 1;
    if (node->token) {
        switch (node->token->t_type) {
            case MINUS_TOKEN:
            case DIVIDE_TOKEN:
            case MULTIPLY_TOKEN:
            case BITMOVE_LEFT_TOKEN:
            case BITMOVE_RIGHT_TOKEN:
            case BITOR_TOKEN:
            case BITAND_TOKEN:
            case ASIGN_TOKEN:
            case PLUS_TOKEN: {
                tree_t* left = node->first_child;
                if (!left) {
                    print_error("Line %i, incorrect operand struct! Expected a <op> b", node->token->line_number);
                    result = 0;
                    break;
                }

                tree_t* right = node->first_child->next_sibling;
                if (!right) {
                    print_error("Line %i, incorrect operand struct! Expected a <op> b, but got a <op>", node->token->line_number);
                    result = 0;
                    break;
                }

                int left_size = __check_operant_type(left->token->t_type);
                int right_size = __check_operant_type(right->token->t_type);
                if (left_size != right_size) {
                    print_warn(
                        "Danger shadow type cast at line %i. Different size [%i] and [%i]. Did you expect this?", 
                        node->token->line_number, left_size, right_size
                    );
                }
                
                break;
            }

            default: break;
        }
    }
    
    if (node->first_child) check_semantic(node->first_child);
    if (node->next_sibling) check_semantic(node->next_sibling);
    return result;
}

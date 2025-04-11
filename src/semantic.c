#include "../include/semantic.h"


static int _check_assignment(tree_t* node);
static int _check_expression(tree_t* node);
static int _check_while(tree_t* node);
static int _check_if(tree_t* node);
static int _check_syscall(tree_t* node);


static int _check_binary_operator(tree_t* node, token_type_t op) {
    if (!node->first_child || !node->first_child->next_sibling) {
        printf("[ERROR] Invalid binary operation '%i' structure (line %d)\n", op, node->token->line_number);
        return 0;
    }

    return _check_expression(node->first_child) && _check_expression(node->first_child->next_sibling);
}

static int _check_unary_operator(tree_t* node, const char* op_name) {
    if (!node->first_child) {
        printf("[ERROR] Invalid unary operation '%s' structure (line %d)\n",
              op_name, node->token->line_number);
        return 0;
    }
    return _check_expression(node->first_child);
}

static int _check_assignment(tree_t* node) {
    if (!node || !node->first_child || !node->first_child->next_sibling) {
        printf("[ERROR] Invalid assignment structure\n");
        return 0;
    }

    tree_t* target = node->first_child;
    tree_t* value = target->next_sibling;

    if (!target->token || !value->token) {
        printf("[ERROR] Missing token in assignment\n");
        return 0;
    }

    const int target_type = target->token->t_type;
    const int value_type = value->token->t_type;
    print_debug("%s <--> %s", target->token->value, value->token->value);

    int is_valid = 0;
    const char* type_name = "";

    if (target_type == INT_VARIABLE_TOKEN) {
        is_valid = (value_type == UNKNOWN_NUMERIC_TOKEN || value_type == INT_VALUE_TOKEN || value_type == INT_VARIABLE_TOKEN);
        type_name = "integer";
    } 
    else if (target_type == STR_VARIABLE_TOKEN) {
        is_valid = (value_type == UNKNOWN_STRING_TOKEN || value_type == STRING_VALUE_TOKEN || value_type == STR_VARIABLE_TOKEN);
        type_name = "string";
    }

    if (!is_valid) {
        printf("[ERROR] %s [%i != %i] variable assignment to incompatible type (line %d)\n", type_name, target_type, value_type, target->token->line_number);
        return 0;
    }

    return _check_expression(value);
}

static int _check_expression(tree_t* node) {
    if (!node || !node->token) {
        printf("[ERROR] Invalid expression node\n");
        return 0;
    }

    switch(node->token->t_type) {
        case IF_TOKEN: return _check_if(node);
        case WHILE_TOKEN: return _check_while(node);
        case SYSCALL_TOKEN: return _check_syscall(node);
        case ASIGN_TOKEN: return _check_assignment(node);
        
        case INT_TYPE_TOKEN:
        case STRING_TYPE_TOKEN: {
            if (node->child_count != 2 || !node->first_child->next_sibling) {
                printf("[ERROR] Invalid variable declaration (line %d)\n", node->token->line_number);
                return 0;
            }

            tree_t* value_node = node->first_child->next_sibling;
            return (value_node->token->t_type == (node->token->t_type == INT_TYPE_TOKEN ? INT_VALUE_TOKEN : STRING_VALUE_TOKEN)) || _check_expression(value_node);
        }
        
        case STR_VARIABLE_TOKEN:
        case INT_VARIABLE_TOKEN:
        case ARR_VARIABLE_TOKEN:
            return node->first_child ? _check_expression(node->first_child) : 1;

        case PLUS_TOKEN:
        case MINUS_TOKEN:
        case MULTIPLY_TOKEN:
        case DIVIDE_TOKEN:
            return _check_binary_operator(node, node->token->t_type);

        case LARGER_TOKEN:
        case COMPARE_TOKEN:
            return _check_binary_operator(node, node->token->t_type);

        case EXIT_TOKEN:
            return _check_unary_operator(node, "exit");

        default:
            return 1;
    }
}

static int _check_control_flow(tree_t* node, const char* type) {
    if (!node || node->child_count < 2) {
        printf("[ERROR] Invalid %s structure\n", type);
        return 0;
    }

    tree_t* condition = node->first_child;
    tree_t* body = condition->next_sibling->first_child;
    
    if (!_check_expression(condition)) return 0;
    
    int result = 1;
    while (body && result) {
        result &= _check_expression(body);
        body = body->next_sibling;
    }
    return result;
}

static int _check_while(tree_t* node) {
    return _check_control_flow(node, "while-loop");
}

static int _check_if(tree_t* node) {
    return _check_control_flow(node, "if-statement");
}

static int _check_syscall(tree_t* node) {
    if (!node || node->child_count == 0) {
        printf("[ERROR] Empty syscall (line %d)\n", node->token->line_number);
        return 0;
    }

    int result = 1;
    tree_t* arg = node->first_child;
    while (arg && result) {
        result &= _check_expression(arg);
        arg = arg->next_sibling;
    }
    return result;
}

int check_semantic(tree_t* node) {
    return 1;
    if (!node || !node->token) return 0;
    
    if (node->token->t_type == START_TOKEN) {
        if (node->child_count != 2 || !node->first_child->next_sibling) {
            printf("[ERROR] Program structure violation\n");
            return 0;
        }

        tree_t* scope = node->first_child;
        for (tree_t* child = scope->first_child; child; child = child->next_sibling) {
            if (!_check_expression(child)) return 0;
        }
        return _check_expression(node->first_child->next_sibling);
    }
    
    return _check_expression(node);
}

#include "../include/parser.h"


int command_markup(token_t* head) {
    token_t* curr = head;

    while (curr) {
        if (curr->t_type == UNKNOWN_STRING_TOKEN) {
            // this is string command or string value
            if (!str_strcmp((char*)curr->value, START_COMMAND)) curr->t_type = START_TOKEN;

            // Variables and types
            else if (!str_strcmp((char*)curr->value, INT_VARIABLE)) curr->t_type = INT_TYPE_TOKEN;
            else if (!str_strcmp((char*)curr->value, STR_VARIABLE)) curr->t_type = STRING_TYPE_TOKEN;
            else if (!str_strcmp((char*)curr->value, ARR_VARIABLE)) curr->t_type = ARRAY_TYPE_TOKEN;

            // While
            else if (!str_strcmp((char*)curr->value, WHILE_COMAND)) curr->t_type = WHILE_TOKEN;
            else if (!str_strcmp((char*)curr->value, WHILE_START_COMMAND)) curr->t_type = WHILE_START_TOKEN;
            else if (!str_strcmp((char*)curr->value, WHILE_END_COMMAND)) curr->t_type = WHILE_END_TOKEN;

            // End
            else if (!str_strcmp((char*)curr->value, EXIT_COMMAND)) curr->t_type = EXIT_TOKEN;
        }
        else if (curr->t_type == UNKNOWN_SYMBOL_TOKEN) {
            // this is symbol. Now determining what's symbol is
            if (!str_strcmp((char*)curr->value, ASIGN_STATEMENT)) curr->t_type = ASIGN_TOKEN;
            else if (!str_strcmp((char*)curr->value, COMPARE_STATEMENT)) curr->t_type = COMPARE_TOKEN;
            else if (!str_strcmp((char*)curr->value, PLUS_STATEMENT)) curr->t_type = PLUS_TOKEN;
            else if (!str_strcmp((char*)curr->value, MINUS_STATEMENT)) curr->t_type = MINUS_TOKEN;
            else if (!str_strcmp((char*)curr->value, LARGER_STATEMENT)) curr->t_type = LARGER_TOKEN;
            else if (!str_strcmp((char*)curr->value, LOWER_STATEMENT)) curr->t_type = LOWER_TOKEN;
        }

        curr = curr->next;
    }

    return 1;
}

static tree_t* create_tree_node(token_t* token) {
    tree_t* node = mm_malloc(sizeof(tree_t));
    if (!node) return NULL;
    
    node->token        = token;
    node->parent       = NULL;
    node->first_child  = NULL;
    node->next_sibling = NULL;
    node->child_count  = 0;
    return node;
}

static void add_child_node(tree_t* parent, tree_t* child) {
    if (!parent || !child) return;
    
    child->parent = parent;
    if (!parent->first_child) parent->first_child = child;
    else {
        tree_t* sibling = parent->first_child;
        while (sibling->next_sibling) sibling = sibling->next_sibling;
        sibling->next_sibling = child;
    }

    parent->child_count++;
}

static int is_expression_terminator(token_t* t) {
    return t->t_type == DELIMITER_TOKEN ||
           t->t_type == WHILE_START_TOKEN ||
           t->t_type == WHILE_END_TOKEN ||
           t->t_type == EXIT_TOKEN;
}

static tree_t* parse_expression(token_t** curr) {
    if (!*curr) return NULL;
    
    tree_t* expr_node = create_tree_node(*curr);
    *curr = (*curr)->next;
    
    while (*curr && !is_expression_terminator(*curr)) {
        tree_t* node = create_tree_node(*curr);
        add_child_node(expr_node, node);
        *curr = (*curr)->next;
    }
    
    return expr_node;
}

static tree_t* parse_variable_declaration(token_t** curr) {
    token_t* type_token   = *curr;
    token_t* name_token   = (*curr)->next;
    token_t* assign_token = name_token->next;
    
    if (
        !name_token || name_token->t_type != UNKNOWN_STRING_TOKEN ||
        !assign_token || assign_token->t_type != ASIGN_TOKEN
    ) {
        return NULL;
    }
    
    tree_t* decl_node  = create_tree_node(type_token);
    tree_t* name_node  = create_tree_node(name_token);
    tree_t* value_node = create_tree_node(assign_token->next);
    
    add_child_node(decl_node, name_node);
    add_child_node(decl_node, value_node);
    
    *curr = assign_token->next->next;
    return decl_node;
}

static tree_t* parse_array_declaration(token_t** curr) {
    token_t* arr_token    = *curr;
    token_t* size_token   = (*curr)->next;
    token_t* type_token   = size_token->next;
    token_t* name_token   = type_token->next;
    token_t* assign_token = name_token->next;
    
    if (
        !size_token || size_token->t_type != UNKNOWN_NUMERIC_TOKEN ||
        !type_token || (type_token->t_type != INT_TYPE_TOKEN && type_token->t_type != STRING_TYPE_TOKEN) ||
        !name_token || name_token->t_type != UNKNOWN_STRING_TOKEN ||
        !assign_token || assign_token->t_type != ASIGN_TOKEN
        ) {
        return NULL;
    }
    
    tree_t* arr_node  = create_tree_node(arr_token);
    tree_t* size_node = create_tree_node(size_token);
    tree_t* type_node = create_tree_node(type_token);
    tree_t* name_node = create_tree_node(name_token);
    
    add_child_node(arr_node, size_node);
    add_child_node(arr_node, type_node);
    add_child_node(arr_node, name_node);
    
    int arr_size = str_atoi((char*)size_token->value);
    token_t* val_token = assign_token->next;
    
    for (int i = 0; i < arr_size && val_token; i++) {
        tree_t* val_node = create_tree_node(val_token);
        add_child_node(arr_node, val_node);
        val_token = val_token->next;
    }
    
    *curr = val_token;
    return arr_node;
}

static tree_t* parse_assignment(token_t** curr) {
    token_t* name_token = *curr;
    token_t* assign_token = name_token->next;
    
    if (!assign_token || assign_token->t_type != ASIGN_TOKEN) {
        return NULL;
    }
    
    tree_t* assign_node = create_tree_node(assign_token);
    tree_t* name_node = create_tree_node(name_token);
    tree_t* expr_node = parse_expression(&(assign_token->next));
    
    add_child_node(assign_node, name_node);
    if (expr_node) {
        add_child_node(assign_node, expr_node);
    }
    
    *curr = assign_token->next;
    while (
        *curr && (*curr)->t_type != WHILE_END_TOKEN && (*curr)->t_type != EXIT_TOKEN
        ) {
        (*curr) = (*curr)->next;
    }
    
    return assign_node;
}

static tree_t* parse_while_loop(token_t** curr) {
    token_t* while_token = *curr;
    tree_t* while_node = create_tree_node(while_token);
    *curr = (*curr)->next;
    
    tree_t* condition_node = parse_expression(curr);
    add_child_node(while_node, condition_node);
    
    if (*curr && (*curr)->t_type == WHILE_START_TOKEN) {
        tree_t* body_node = create_tree_node(*curr);
        add_child_node(while_node, body_node);
        *curr = (*curr)->next;
        
        while (*curr && (*curr)->t_type != WHILE_END_TOKEN) {
            tree_t* stmt_node = NULL;
            
            if (
                (*curr)->t_type == INT_TYPE_TOKEN || 
                (*curr)->t_type == STRING_TYPE_TOKEN
            ) {
                stmt_node = parse_variable_declaration(curr);
            }
            else if ((*curr)->t_type == ARRAY_TYPE_TOKEN) {
                stmt_node = parse_array_declaration(curr);
            }
            else if ((*curr)->t_type == UNKNOWN_STRING_TOKEN) {
                stmt_node = parse_assignment(curr);
            }
            
            if (stmt_node) add_child_node(body_node, stmt_node);
            else *curr = (*curr)->next;
        }
        
        if (*curr) *curr = (*curr)->next;
    }
    
    return while_node;
}

tree_t* create_parse_tree(token_t* head) {
    token_t* curr = head;
    tree_t* root = NULL;
    while (curr && curr->t_type != START_TOKEN) {
        curr = curr->next;
    }

    if (!curr) return NULL;
    root = create_tree_node(curr);
    curr = curr->next;
    
    while (curr && curr->t_type != EXIT_TOKEN) {
        tree_t* node = NULL;
        
        if (
            curr->t_type == INT_TYPE_TOKEN || 
            curr->t_type == STRING_TYPE_TOKEN
        ) {
            // Single variable
            node = parse_variable_declaration(&curr);
        }
        else if (curr->t_type == ARRAY_TYPE_TOKEN) {
            // Multiply variable (array)
            node = parse_array_declaration(&curr);
        }
        else if (curr->t_type == WHILE_TOKEN) {
            // Loop cycle
            node = parse_while_loop(&curr);
        }
        else if (curr->t_type == UNKNOWN_STRING_TOKEN) {
            // Variable name
            node = parse_assignment(&curr);
        }
        
        if (node) add_child_node(root, node);
        else curr = curr->next;
    }
    
    if (curr && curr->t_type == EXIT_TOKEN) {
        tree_t* exit_node = create_tree_node(curr);
        add_child_node(root, exit_node);
        if (curr->next && (curr->next->t_type == UNKNOWN_NUMERIC_TOKEN || curr->next->t_type == UNKNOWN_STRING_TOKEN)) {
            tree_t* exit_code = create_tree_node(curr->next);
            add_child_node(exit_node, exit_code);
            curr = curr->next->next;
        }
    }

    return root;
}

int unload_parse_tree(tree_t* node) {
    if (!node) return 0;
    unload_parse_tree(node->first_child);
    unload_parse_tree(node->next_sibling);
    mm_free(node);
    return 1;
}

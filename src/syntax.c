#include "../include/syntax.h"


static tree_t* _parse_function_call(token_t** curr);
static tree_t* _parse_function_declaration(token_t** curr);
static tree_t* _parse_variable_declaration(token_t** curr);
static tree_t* _parse_array_declaration(token_t** curr);
static tree_t* _parse_while_loop(token_t** curr);
static tree_t* _parse_if_statement(token_t** curr);
static tree_t* _parse_syscall(token_t** curr);
static tree_t* _parse_scope(token_t** curr_ptr, token_type_t exit_token);
static tree_t* _parse_array_expression(token_t** curr);
static tree_t* _parse_expression(token_t** curr);
static tree_t* _parse_exit(token_t** curr);

typedef struct {
    token_type_t type;
    tree_t* (*parser)(token_t**);
} command_handler_t;

static const command_handler_t _command_handlers[] = {
    { INT_TYPE_TOKEN,       _parse_variable_declaration },
    { FUNC_TOKEN,           _parse_function_declaration },
    { STRING_TYPE_TOKEN,    _parse_variable_declaration },
    { ARRAY_TYPE_TOKEN,     _parse_array_declaration    },
    { WHILE_TOKEN,          _parse_while_loop           },
    { IF_TOKEN,             _parse_if_statement         },
    { SYSCALL_TOKEN,        _parse_syscall              },
    { STR_VARIABLE_TOKEN,   _parse_expression           },
    { INT_VARIABLE_TOKEN,   _parse_expression           },
    { ARR_VARIABLE_TOKEN,   _parse_expression           },
    { UNKNOWN_STRING_TOKEN, _parse_expression           },
    { CALL_TOKEN,           _parse_function_call        },
    { -1, NULL }
};


static tree_t* _create_tree_node(token_t* token) {
    tree_t* node = mm_malloc(sizeof(tree_t));
    if (!node) return NULL;
    node->token        = token;
    node->parent       = NULL;
    node->first_child  = NULL;
    node->next_sibling = NULL;
    node->child_count  = 0;
    return node;
}

static int _add_child_node(tree_t* parent, tree_t* child) {
    if (!parent || !child) return 0;
    
    child->parent = parent;
    if (!parent->first_child) parent->first_child = child;
    else {
        tree_t* sibling = parent->first_child;
        while (sibling->next_sibling) sibling = sibling->next_sibling;
        sibling->next_sibling = child;
    }

    parent->child_count++;
    return 1;
}

static tree_t* _parse_function_call(token_t** curr) {
    if (!curr || !*curr || (*curr)->t_type != CALL_TOKEN) return NULL;
    tree_t* call_node = _create_tree_node(*curr);
    *curr = (*curr)->next;

    tree_t* name_node = _create_tree_node(*curr);
    _add_child_node(call_node, name_node);

    tree_t* args_node = _create_tree_node(NULL);
    *curr = (*curr)->next;
    while (*curr && (*curr)->t_type != DELIMITER_TOKEN) {
        tree_t* arg_name_node = _create_tree_node((*curr));
        _add_child_node(args_node, arg_name_node);
        *curr = (*curr)->next;
    }

    _add_child_node(call_node, args_node);
    return call_node;
}

static tree_t* _parse_function_declaration(token_t** curr) {
    if (!curr || !*curr || (*curr)->t_type != FUNC_TOKEN) return NULL;

    tree_t* func_node = _create_tree_node(*curr);
    *curr = (*curr)->next;
    if (!*curr) {
        unload_syntax_tree(func_node);
        return NULL;
    }

    tree_t* name_node = _create_tree_node(*curr);
    _add_child_node(func_node, name_node);

    tree_t* args_node = _create_tree_node(NULL);
    *curr = (*curr)->next;
    while (*curr && (*curr)->t_type != DELIMITER_TOKEN) {
        if ((*curr) && (*curr)->next) {
            tree_t* arg_type_node = _create_tree_node(*curr);
            tree_t* arg_name_node = _create_tree_node((*curr)->next);
            _add_child_node(arg_type_node, arg_name_node);
            _add_child_node(args_node, arg_type_node);
            *curr = (*curr)->next->next;
        }
        else {
            break;
        }
    }
    
    *curr = (*curr)->next;
    _add_child_node(func_node, args_node);
    if (!*curr || (*curr)->t_type != FUNC_START_TOKEN) {
        return NULL;
    }

    *curr = (*curr)->next;
    tree_t* body_node = _parse_scope(curr, FUNC_END_TOKEN);
    _add_child_node(func_node, body_node);
    
    tree_t* return_node = _parse_expression(curr);
    _add_child_node(func_node, return_node);
    
    return func_node;
}

static tree_t* _parse_variable_declaration(token_t** curr) {
    token_t* type_token   = *curr;
    token_t* name_token   = (*curr)->next;
    token_t* assign_token = name_token->next;
    
    if (
        !type_token || !name_token || !assign_token ||
        assign_token->t_type != ASIGN_TOKEN
    ) return NULL;
    
    tree_t* decl_node  = _create_tree_node(type_token);
    tree_t* name_node  = _create_tree_node(name_token);
    tree_t* value_node = _parse_expression(&assign_token->next);
    while (*curr && (*curr)->t_type != DELIMITER_TOKEN) {
        (*curr) = (*curr)->next;
    }

    _add_child_node(decl_node, name_node);
    _add_child_node(decl_node, value_node);
    return decl_node;
}

static tree_t* _parse_array_declaration(token_t** curr) {
    token_t* arr_token  = *curr;
    token_t* name_token = (*curr)->next;
    token_t* size_token = name_token->next;
    token_t* elem_size_token = size_token->next;
    token_t* assign_token    = elem_size_token->next;
    
    if (
        !size_token || !elem_size_token || !name_token || !assign_token ||
        size_token->t_type != UNKNOWN_NUMERIC_TOKEN || 
        name_token->t_type != ARR_VARIABLE_TOKEN ||
        assign_token->t_type != ASIGN_TOKEN
        ) {
        return NULL;
    }
    
    tree_t* arr_node  = _create_tree_node(arr_token);
    tree_t* size_node = _create_tree_node(size_token);
    tree_t* elem_size_node = _create_tree_node(elem_size_token);
    tree_t* name_node = _create_tree_node(name_token);
    
    _add_child_node(arr_node, size_node);
    _add_child_node(arr_node, elem_size_node);
    _add_child_node(arr_node, name_node);
    
    int arr_size = str_atoi((char*)size_token->value);
    token_t* val_token = assign_token->next;
    for (int i = 0; i < arr_size && val_token; i++) {
        if (val_token->t_type != UNKNOWN_NUMERIC_TOKEN && val_token->t_type != UNKNOWN_STRING_TOKEN) break;
        tree_t* val_node = _create_tree_node(val_token);
        _add_child_node(arr_node, val_node);
        val_token = val_token->next;
    }
    
    *curr = val_token;
    return arr_node;
}

static tree_t* _parse_expression(token_t** curr) {
    if (!curr || !*curr) return NULL;
    token_t* left = *curr;
    if ((*curr)->t_type == ARR_VARIABLE_TOKEN || (*curr)->t_type == STR_VARIABLE_TOKEN) {
        return _parse_array_expression(curr);
    }
    else if ((*curr)->t_type == CALL_TOKEN) {
        return _parse_function_call(curr);
    }

    tree_t* left_node = _create_tree_node(left);
    
    *curr = left->next;
    token_t* assign_token = left->next;
    if (!assign_token || assign_token->t_type == DELIMITER_TOKEN || assign_token->t_type == CLOSE_INDEX_TOKEN) {
        return left_node;
    }

    tree_t* right = NULL;
    tree_t* assign_node = _create_tree_node(assign_token);

    if (assign_token->next->t_type == CALL_TOKEN) right = _parse_function_call(&(assign_token->next));
    else right = _parse_expression(&(assign_token->next));
    
    _add_child_node(assign_node, left_node);
    if (right) _add_child_node(assign_node, right);
    return assign_node;
}

static tree_t* _parse_array_expression(token_t** curr) {
    if (!curr || !*curr) return NULL;
    token_t* arr_name = *curr;
    tree_t* arr_name_node = _create_tree_node(arr_name);
    
    *curr = arr_name->next;
    if ((*curr)->t_type == OPEN_INDEX_TOKEN) {
        tree_t* offset_exp = _parse_expression(&((*curr)->next));
        while (*curr && (*curr)->t_type != CLOSE_INDEX_TOKEN) {
            (*curr) = (*curr)->next;
        }

        _add_child_node(arr_name_node, offset_exp);

        *curr = (*curr)->next;
        token_t* assign_token = (*curr);
        if (!assign_token || assign_token->t_type == DELIMITER_TOKEN) {
            return arr_name_node;
        }

        tree_t* assign_node = _create_tree_node(assign_token);
        tree_t* right = _parse_expression(&(assign_token->next));

        _add_child_node(assign_node, arr_name_node);
        if (right) _add_child_node(assign_node, right);
        return assign_node;
    }

    return arr_name_node;
}

static tree_t* _parse_scope(token_t** curr_ptr, token_type_t exit_token) {
    if (!curr_ptr || !*curr_ptr) return NULL;
    tree_t* scope_node = _create_tree_node(NULL);
    token_t* curr = *curr_ptr;

    while (curr && curr->t_type != exit_token && curr->t_type != EXIT_TOKEN) {
        int handled = 0;
        for (const command_handler_t* handler = _command_handlers; handler->parser; handler++) {
            if (curr && handler->type == curr->t_type) {
                tree_t* node = handler->parser(&curr);
                if (node) {
                    _add_child_node(scope_node, node);
                    handled = 1;
                    break;
                }
            }
        }

        if (!handled && curr) {
            curr = curr->next;
        }
    }
    
    if (!curr) *curr_ptr = NULL; 
    else {
        if (curr->t_type != EXIT_TOKEN) *curr_ptr = curr->next;
        else *curr_ptr = curr;
    }

    return scope_node;
}

static tree_t* __parse_statement_scope(token_t** curr, token_type_t start, token_type_t end) {
    if (!curr || !*curr) return NULL;
    
    token_t* body_token = *curr;
    tree_t* body_node = _create_tree_node(body_token);
    if (!body_node) return NULL;
    
    *curr = (*curr)->next;
    tree_t* cond = _parse_expression(curr);
    if (cond) _add_child_node(body_node, cond);

    *curr = (*curr)->next->next;
    if (*curr && (*curr)->t_type == start) {
        token_t* body_start = *curr;
        *curr = body_start->next;
        
        tree_t* body = _parse_scope(curr, end);
        if (body) _add_child_node(body_node, body);
        if (*curr && (*curr)->t_type == end) {
            *curr = (*curr)->next;
        }
    }

    return body_node;
}

static tree_t* _parse_while_loop(token_t** curr) {
    return __parse_statement_scope(curr, WHILE_START_TOKEN, WHILE_END_TOKEN);
}

static tree_t* _parse_if_statement(token_t** curr) {
    return __parse_statement_scope(curr, IF_START_TOKEN, IF_END_TOKEN);;
}

static tree_t* _parse_syscall(token_t** curr) {
    if (!curr || !*curr || (*curr)->t_type != SYSCALL_TOKEN) return NULL;

    token_t* syscall_token = *curr;
    tree_t* syscall_node = _create_tree_node(syscall_token);
    *curr = (*curr)->next;

    for (int i = 0; *curr && (*curr)->t_type != DELIMITER_TOKEN; i++) {
        tree_t* arg_node = _create_tree_node(*curr);
        _add_child_node(syscall_node, arg_node);
        *curr = (*curr)->next;
    }

    if (*curr && (*curr)->t_type == DELIMITER_TOKEN) *curr = (*curr)->next;
    return syscall_node;
}

static tree_t* _parse_exit(token_t** curr) {
    if (!curr || !*curr || (*curr)->t_type != EXIT_TOKEN) return NULL;

    tree_t* exit_node = _create_tree_node(*curr);
    *curr = (*curr)->next;

    tree_t* exp = _parse_expression(curr);
    _add_child_node(exit_node, exp);

    if (*curr && (*curr)->t_type == DELIMITER_TOKEN) *curr = (*curr)->next;
    return exit_node;
}

tree_t* create_syntax_tree(token_t* head) {
    token_t* curr = head;
    while (curr && curr->t_type != START_TOKEN) {
        curr = curr->next;
    }

    if (!curr) return NULL;
    tree_t* root = _create_tree_node(curr);
    curr = curr->next;

    tree_t* body = _parse_scope(&curr, EXIT_TOKEN);
    if (body) _add_child_node(root, body);

    if (curr) {
        tree_t* exit_node = _parse_exit(&curr);
        _add_child_node(root, exit_node);
    }

    return root;
}

int unload_syntax_tree(tree_t* node) {
    if (!node) return 0;
    unload_syntax_tree(node->first_child);
    unload_syntax_tree(node->next_sibling);

    if (node->token) {
        mm_free(node->token);
    }

    mm_free(node);
    return 1;
}

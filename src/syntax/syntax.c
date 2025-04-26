#include "../../include/syntax.h"


static char* _current_function_name = NULL;
static tree_t* _parser_dummy(token_t**);
static tree_t* _parse_import(token_t**);
static tree_t* _parse_variable_declaration(token_t**);
static tree_t* _parse_array_declaration(token_t**);
static tree_t* _parse_function_call(token_t**);
static tree_t* _parse_function_declaration(token_t**);
static tree_t* _parse_return_declaration(token_t**);
static tree_t* _parse_condition_scope(token_t**);
static tree_t* _parse_syscall(token_t**);
static tree_t* _parse_expression(token_t**);
static tree_t* _parse_array_expression(token_t**);

static tree_t* (*_get_parser(token_type_t t_type))(token_t**) {
    switch (t_type) {
        case INT_TYPE_TOKEN:
        case SHORT_TYPE_TOKEN:
        case CHAR_TYPE_TOKEN:
        case STR_TYPE_TOKEN:        return _parse_variable_declaration;
        case WHILE_TOKEN:
        case IF_TOKEN:              return _parse_condition_scope;
        case INT_VARIABLE_TOKEN:
        case STR_VARIABLE_TOKEN:
        case ARR_VARIABLE_TOKEN:
        case UNKNOWN_STRING_TOKEN:  return _parse_expression;
        case SYSCALL_TOKEN:         return _parse_syscall;
        case IMPORT_SELECT_TOKEN:   return _parse_import;
        case ARRAY_TYPE_TOKEN:      return _parse_array_declaration;
        case CALL_TOKEN:            return _parse_function_call;
        case FUNC_TOKEN:            return _parse_function_declaration;
        case RETURN_TOKEN:          return _parse_return_declaration;
        default: return _parser_dummy;
    }
}

static int _fill_variable(tree_t* val_node) {
    variable_info_t varinfo;
    if (!get_var_info((char*)val_node->token->value, _current_function_name, &varinfo)) return 0;
    val_node->variable_offset = varinfo.offset;
    val_node->variable_size   = varinfo.size;
    return 1;
}

tree_t* create_tree_node(token_t* token) {
    tree_t* node = mm_malloc(sizeof(tree_t));
    if (!node) return NULL;

    node->token           = token;
    node->parent          = NULL;
    node->first_child     = NULL;
    node->next_sibling    = NULL;
    node->variable_size   = 0;
    node->variable_offset = 0;
    return node;
}

int add_child_node(tree_t* parent, tree_t* child) {
    if (!parent || !child) return 0;
    child->parent = parent;
    if (!parent->first_child) parent->first_child = child;
    else {
        tree_t* sibling = parent->first_child;
        while (sibling->next_sibling) sibling = sibling->next_sibling;
        sibling->next_sibling = child;
    }

    return 1;
}

int remove_child_node(tree_t* parent, tree_t* child) {
    if (!parent || !child) return 0;

    tree_t* prev = NULL;
    tree_t* current = parent->first_child;

    while (current) {
        if (current == child) {
            if (prev) prev->next_sibling = current->next_sibling;
            else parent->first_child = current->next_sibling;
            break;
        }

        prev = current;
        current = current->next_sibling;
    }

    return 1;
}

static tree_t* _parser_dummy(token_t** curr) {
    return NULL;
}

/*
Main parse function, that parse whole scope.
For parsing we have registered parsers for every token.
*/
static tree_t* _parse_scope(token_t** curr, token_type_t exit_token) {
    if (!curr || !*curr) return NULL;
    tree_t* scope_node = create_tree_node(NULL);
    if (!scope_node) return NULL;
    
    while ((*curr) && (*curr)->t_type != exit_token) {
        tree_t* node = _get_parser((*curr)->t_type)(curr);
        if (!node) *curr = (*curr)->next;
        else add_child_node(scope_node, node);
    }
    
    return scope_node;
}

/*
from "src.cpl" import func1 func2;
Where "from" - import_node.
"src.cpl" - source_node.
And then adding to source_node func1 and func2.
*/
static tree_t* _parse_import(token_t** curr) {
    if (!curr || !*curr || (*curr)->t_type != IMPORT_SELECT_TOKEN) return NULL;
    tree_t* import_node = create_tree_node(*curr);
    if (!import_node) return NULL;
    
    *curr = (*curr)->next;
    tree_t* source_node = create_tree_node(*curr);
    if (!source_node) {
        unload_syntax_tree(import_node);
        return NULL;
    }

    *curr = (*curr)->next->next;
    while (*curr && (*curr)->t_type != DELIMITER_TOKEN) {
        tree_t* function_name = create_tree_node(*curr);
        if (!function_name) {
            unload_syntax_tree(import_node);
            unload_syntax_tree(source_node);
            return NULL;
        }

        add_child_node(source_node, function_name);
        *curr = (*curr)->next;
    }

    add_child_node(import_node, source_node);
    return import_node;
}

/*
func1 arg1 arg2;
"func1" - call_node.
And then adding to scope arg1 and arg2.
*/
static tree_t* _parse_function_call(token_t** curr) {
    if (!curr || !*curr || (*curr)->t_type != CALL_TOKEN) return NULL;
    tree_t* call_node = create_tree_node(*curr);
    if (!call_node) return NULL;
    
    *curr = (*curr)->next;
    tree_t* args_node = create_tree_node(NULL);
    if (!args_node) {
        unload_syntax_tree(call_node);
        return NULL;
    }

    while (*curr && (*curr)->t_type != DELIMITER_TOKEN) {
        tree_t* arg_name_node = create_tree_node((*curr));
        if (!arg_name_node) {
            unload_syntax_tree(call_node);
            unload_syntax_tree(args_node);
            return NULL;
        }
        
        _fill_variable(arg_name_node);
        add_child_node(args_node, arg_name_node);
        *curr = (*curr)->next;
    }

    add_child_node(call_node, args_node);
    return call_node;
}

static tree_t* _parse_return_declaration(token_t** curr) {
    if (!curr || !*curr || (*curr)->t_type != RETURN_TOKEN) return NULL;
    tree_t* return_node = create_tree_node(*curr);
    if (!return_node) return NULL;
    
    *curr = (*curr)->next;
    tree_t* exp_node = _parse_expression(curr);
    if (!exp_node) {
        unload_syntax_tree(return_node);
        return NULL;
    }

    add_child_node(return_node, exp_node);
    return return_node;
}

static tree_t* _parse_function_declaration(token_t** curr) {
    if (!curr || !*curr || (*curr)->t_type != FUNC_TOKEN) return NULL;

    int temp_off = get_vars_offset();
    char* temp_fname = _current_function_name;
    set_vars_offset(0);

    tree_t* func_node = create_tree_node(*curr);
    if (!func_node) return NULL;
    
    *curr = (*curr)->next;
    tree_t* name_node = create_tree_node(*curr);
    if (!name_node) {
        unload_syntax_tree(func_node);
        return NULL;
    }

    _current_function_name = (char*)name_node->token->value;
    add_child_node(func_node, name_node);

    *curr = (*curr)->next;
    tree_t* args_node = create_tree_node(NULL);
    if (!args_node) {
        unload_syntax_tree(func_node);
        unload_syntax_tree(name_node);
        return NULL;
    }

    while (!*curr || (*curr)->t_type != OPEN_BLOCK_TOKEN) {
        if (is_variable((*curr)->t_type)) {
            tree_t* param_node = _parse_variable_declaration(curr);
            if (!param_node) {
                unload_syntax_tree(func_node);
                unload_syntax_tree(name_node);
                unload_syntax_tree(args_node);
                return NULL;
            }

            add_child_node(args_node, param_node);
        }

        *curr = (*curr)->next;
    }

    add_child_node(func_node, args_node);

    tree_t* body_node = _parse_scope(curr, CLOSE_BLOCK_TOKEN);
    if (!body_node) {
        unload_syntax_tree(func_node);
        unload_syntax_tree(name_node);
        unload_syntax_tree(args_node);
        return NULL;
    }

    add_child_node(func_node, body_node);
    _current_function_name = temp_fname;
    set_vars_offset(temp_off);

    return func_node;
}

static tree_t* _parse_variable_declaration(token_t** curr) {
    token_t* type_token = *curr;
    token_t* name_token = type_token->next;
    if (!type_token || !name_token) return NULL;
    
    token_t* assign_token = name_token->next;
    if (!assign_token) return NULL;

    token_t* value_token = assign_token->next;
    if (!value_token) return NULL;

    tree_t* decl_node = create_tree_node(type_token);
    if (!decl_node) return NULL;
    
    tree_t* name_node = create_tree_node(name_token);
    if (!name_node) {
        unload_syntax_tree(decl_node);
        return NULL;
    }
    
    while (*curr && (*curr)->t_type != DELIMITER_TOKEN) {
        (*curr) = (*curr)->next;
    }
    
    if ((name_token->ptr || get_variable_type(name_token) != 1) && !decl_node->token->ro && !decl_node->token->glob) {
        int var_size = 4;
        if (name_token->ptr) {
            if (!str_strcmp((char*)type_token->value, CHAR_VARIABLE)) var_size = 1;
            else if (!str_strcmp((char*)type_token->value, SHORT_VARIABLE)) var_size = 2;
            else if (!str_strcmp((char*)type_token->value, INT_VARIABLE)) var_size = 4;
        }

        decl_node->variable_offset = add_variable_info((char*)name_node->token->value, var_size, _current_function_name);
        decl_node->variable_size = var_size;
        _fill_variable(name_node);
    }

    add_child_node(decl_node, name_node);
    if (!assign_token || assign_token->t_type != ASIGN_TOKEN) return decl_node;

    tree_t* value_node = _parse_expression(&value_token);
    if (!value_node) {
        unload_syntax_tree(decl_node);
        unload_syntax_tree(name_node);
        return NULL;
    }

    if (type_token->t_type == STR_TYPE_TOKEN && !decl_node->token->ro && !decl_node->token->glob) {
        int str_size = str_strlen((char*)value_node->token->value);
        decl_node->variable_offset = add_variable_info((char*)name_node->token->value, str_size, _current_function_name);
        decl_node->variable_size = ALIGN_TO(str_size, 4);
        add_array_info((char*)name_node->token->value, _current_function_name, 1, decl_node->variable_size);
        _fill_variable(name_node);
    }

    add_child_node(decl_node, value_node);
    return decl_node;
}

static tree_t* _parse_array_declaration(token_t** curr) {
    token_t* arr_token       = *curr;
    token_t* name_token      = arr_token->next;
    token_t* size_token      = name_token->next;
    token_t* elem_size_token = size_token->next;
    token_t* assign_token    = elem_size_token->next;
    
    if (!size_token || !elem_size_token || !name_token || !assign_token ||
        size_token->t_type != UNKNOWN_NUMERIC_TOKEN || 
        name_token->t_type != ARR_VARIABLE_TOKEN ||
        assign_token->t_type != ASIGN_TOKEN) {
        return NULL;
    }
    
    tree_t* arr_node  = create_tree_node(arr_token);
    if (!arr_node) return NULL;
    
    int array_size = str_atoi((char*)size_token->value);
    tree_t* size_node = create_tree_node(size_token);
    if (!size_node) {
        unload_syntax_tree(arr_node);
        return NULL;
    }
    
    int el_size = 1;
    if (!str_strcmp((char*)elem_size_token->value, SHORT_VARIABLE)) el_size = 2;
    else if (!str_strcmp((char*)elem_size_token->value, INT_VARIABLE)) el_size = 4;
    
    tree_t* elem_size_node = create_tree_node(elem_size_token);
    if (!elem_size_node) {
        unload_syntax_tree(arr_node);
        unload_syntax_tree(size_node);
        return NULL;
    }

    tree_t* name_node = create_tree_node(name_token);
    if (!name_node) {
        unload_syntax_tree(arr_node);
        unload_syntax_tree(size_node);
        unload_syntax_tree(name_node);
        return NULL;
    }
    
    add_child_node(arr_node, size_node);
    add_child_node(arr_node, elem_size_node);
    add_child_node(arr_node, name_node);

    add_array_info((char*)name_token->value, _current_function_name, el_size, array_size);
    if (!arr_token->ro && !arr_token->glob) {
        arr_node->variable_size = ALIGN_TO(array_size * el_size, 4);
        arr_node->variable_offset = add_variable_info((char*)name_token->value, arr_node->variable_size, _current_function_name);
    }

    int arr_size = str_atoi((char*)size_token->value);
    token_t* val_token = assign_token->next;
    for (int i = 0; i < arr_size && val_token && val_token->t_type != DELIMITER_TOKEN; i++) {
        tree_t* val_node = create_tree_node(val_token);
        if (!val_node) {
            unload_syntax_tree(arr_node);
            unload_syntax_tree(size_node);
            unload_syntax_tree(name_node);
            return NULL;
        }

        _fill_variable(val_node);
        add_child_node(arr_node, val_node);
        val_token = val_token->next;
    }
    
    *curr = val_token;
    return arr_node;
}

static tree_t* _parse_expression(token_t** curr) {
    if (!curr || !*curr) return NULL;
    token_t* left = *curr;
    if ((*curr)->t_type == ARR_VARIABLE_TOKEN || (*curr)->t_type == STR_VARIABLE_TOKEN || (*curr)->ptr) return _parse_array_expression(curr);
    else if ((*curr)->t_type == CALL_TOKEN) return _parse_function_call(curr);
    else if ((*curr)->t_type == SYSCALL_TOKEN) return _parse_syscall(curr);
    
    tree_t* left_node = create_tree_node(left);
    if (!left_node) return NULL;
    _fill_variable(left_node);
    
    *curr = left->next;
    token_t* assign_token = *curr;
    if (!assign_token || assign_token->t_type == DELIMITER_TOKEN || assign_token->t_type == CLOSE_INDEX_TOKEN) {
        return left_node;
    }

    tree_t* right = NULL;
    tree_t* assign_node = create_tree_node(assign_token);
    if (!assign_node) {
        unload_syntax_tree(left_node);
        return NULL;
    }

    *curr = (*curr)->next;
    if ((*curr)->t_type == SYSCALL_TOKEN)   right = _parse_syscall(curr);
    else if ((*curr)->t_type == CALL_TOKEN) right = _parse_function_call(curr);
    else                                    right = _parse_expression(curr);
    
    if (!right) {
        unload_syntax_tree(left_node);
        unload_syntax_tree(assign_node);
        return NULL;
    }

    add_child_node(assign_node, left_node);
    add_child_node(assign_node, right);
    return assign_node;
}

static tree_t* _parse_array_expression(token_t** curr) {
    if (!curr || !*curr) return NULL;
    tree_t* arr_name_node = create_tree_node(*curr);
    if (!arr_name_node) return NULL;
    _fill_variable(arr_name_node);
    
    *curr = (*curr)->next;
    if ((*curr)->t_type == OPEN_INDEX_TOKEN) {
        token_t* offset_token = (*curr)->next;
        tree_t* offset_exp = _parse_expression(&offset_token);
        if (!offset_exp) {
            unload_syntax_tree(arr_name_node);
            return NULL;
        }

        while (*curr && (*curr)->t_type != CLOSE_INDEX_TOKEN) (*curr) = (*curr)->next;
        add_child_node(arr_name_node, offset_exp);

        *curr = (*curr)->next;
        token_t* assign_token = *curr;
        if (!assign_token || assign_token->t_type == DELIMITER_TOKEN) {
            return arr_name_node;
        }

        tree_t* assign_node = create_tree_node(assign_token);
        if (!assign_node) {
            unload_syntax_tree(arr_name_node);
            unload_syntax_tree(offset_exp);
            return NULL;
        }

        *curr = (*curr)->next;
        tree_t* right = _parse_expression(curr);
        if (!right) {
            unload_syntax_tree(arr_name_node);
            unload_syntax_tree(offset_exp);
            unload_syntax_tree(assign_node);
            return NULL;
        }

        add_child_node(assign_node, arr_name_node);
        add_child_node(assign_node, right);
        return assign_node;
    }

    return arr_name_node;
}

static tree_t* _parse_condition_scope(token_t** curr) {
    if (!curr || !*curr) return NULL;
    tree_t* body_node = create_tree_node(*curr);
    if (!body_node) return NULL;
    
    *curr = (*curr)->next;
    tree_t* cond = _parse_expression(curr);
    if (!cond) {
        unload_syntax_tree(body_node);
        return NULL;
    }

    add_child_node(body_node, cond);

    *curr = (*curr)->next;
    if (*curr && (*curr)->t_type == OPEN_BLOCK_TOKEN) {
        *curr = (*curr)->next;
        tree_t* if_body = _parse_scope(curr, CLOSE_BLOCK_TOKEN);
        if (!if_body) {
            unload_syntax_tree(body_node);
            unload_syntax_tree(cond);
            return NULL;
        }

        add_child_node(body_node, if_body);
        if (*curr && (*curr)->t_type == CLOSE_BLOCK_TOKEN) *curr = (*curr)->next;
    }

    if (*curr && (*curr)->t_type == ELSE_TOKEN) {
        *curr = (*curr)->next->next;
        tree_t* else_body = _parse_scope(curr, CLOSE_BLOCK_TOKEN);
        if (!else_body) {
            unload_syntax_tree(body_node);
            unload_syntax_tree(cond);
        }

        add_child_node(body_node, else_body);
        if (*curr && (*curr)->t_type == CLOSE_BLOCK_TOKEN) *curr = (*curr)->next;
    }
    
    return body_node;
}

static tree_t* _parse_syscall(token_t** curr) {
    if (!curr || !*curr || (*curr)->t_type != SYSCALL_TOKEN) return NULL;
    tree_t* syscall_node = create_tree_node(*curr);
    if (!syscall_node) return NULL;

    *curr = (*curr)->next;
    for (int i = 0; *curr && (*curr)->t_type != DELIMITER_TOKEN; i++) {
        tree_t* arg_node = create_tree_node(*curr);
        if (!arg_node) {
            unload_syntax_tree(syscall_node);
            return NULL;
        }

        _fill_variable(arg_node);
        add_child_node(syscall_node, arg_node);
        *curr = (*curr)->next;
    }

    return syscall_node;
}

tree_t* create_syntax_tree(token_t* head) {
    token_t* curr_head = head;
    tree_t* root = create_tree_node(NULL);
    if (!root) return NULL;

    tree_t* prestart = _parse_scope(&curr_head, START_TOKEN);
    if (!prestart) {
        print_error("Prestart parse error!");
        unload_syntax_tree(root);
        return NULL;
    }

    tree_t* body = _parse_scope(&curr_head, EXIT_TOKEN);
    if (!body) print_warn("No body in file!");
    else {
        tree_t* exit_node = create_tree_node(create_token(EXIT_TOKEN, NULL, 0, curr_head->line_number));
        if (!exit_node) {
            print_error("Exit parse error!");
            unload_syntax_tree(root);
            unload_syntax_tree(prestart);
            return NULL;
        }

        token_t* exit_token = curr_head->next;
        tree_t* exit_exp = _parse_expression(&exit_token);
        if (!exit_exp) {
            print_error("Exit expression parse error!");
            unload_syntax_tree(root);
            unload_syntax_tree(prestart);
            unload_syntax_tree(exit_node);
            return NULL;
        }

        add_child_node(exit_node, exit_exp);
        add_child_node(body, exit_node);
    }

    add_child_node(root, prestart);
    add_child_node(root, body);
    return root;
}

int unload_syntax_tree(tree_t* node) {
    if (!node) return 0;
    unload_syntax_tree(node->first_child);
    unload_syntax_tree(node->next_sibling);
    mm_free(node);
    return 1;
}

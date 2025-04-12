#include "../include/generator.h"


static int _label_counter = 0;
static int _generate_function(tree_t* node, FILE* output);
static int _generate_if(tree_t* node, FILE* output);
static int _generate_while(tree_t* node, FILE* output);
static int _generate_syscall(tree_t* node, FILE* output);
static int _generate_assignment(tree_t* node, FILE* output);

typedef struct {
    char name[TOKEN_MAX_SIZE];
    int el_size;
} array_info_t;

static array_info_t _arrays_info[ARRAYS_MAX_TOKEN];
static int _array_count = 0;


static array_info_t* _find_array_info(const char* name) {
    for(int i = 0; i < _array_count; i++) {
        if(str_strcmp(_arrays_info[i].name, (char*)name) == 0) {
            return &_arrays_info[i];
        }
    }
    return NULL;
}

static int _generate_data_section(tree_t* node, FILE* output) {
    if (!node) return 0;
    for (tree_t* child = node->first_child; child; child = child->next_sibling) {
        if (!child->token) {
            _generate_data_section(child, output);
            continue;
        }
        
        /*
        Not global function.
        */
        if (child->function > 1) continue;
        switch (child->token->t_type) {
            case INT_TYPE_TOKEN: {
                // TODO: Constants instead integers
                break;
            }
            case ARRAY_TYPE_TOKEN: {
                tree_t* size   = child->first_child;
                tree_t* t_type = size->next_sibling;
                tree_t* name   = t_type->next_sibling;
            
                array_info_t info;
                str_strncpy(info.name, (char*)name->token->value, 32);
                info.el_size = str_atoi((char*)t_type->token->value);
                _arrays_info[_array_count++] = info;

                const char* directive = NULL;
                if (str_strcmp((char*)t_type->token->value, "1") == 0) directive = "db";
                else if (str_strcmp((char*)t_type->token->value, "2") == 0) directive = "dw";
                else if (str_strcmp((char*)t_type->token->value, "4") == 0) directive = "dd";
                else if (str_strcmp((char*)t_type->token->value, "8") == 0) directive = "dq";
                else directive = "dd";
            
                if (!name->next_sibling) fprintf(output, "%s times %s %s 0\n", name->token->value, size->token->value, directive);
                else {
                    fprintf(output, "%s %s ", name->token->value, directive);
                    for (tree_t* elem = name->next_sibling; elem; elem = elem->next_sibling) {
                        if (elem->token->t_type == UNKNOWN_NUMERIC_TOKEN) fprintf(output, "%s%s", elem->token->value, elem->next_sibling ? "," : "\n");
                        else {
                            int value = 0;
                            char* token_start = (char*)elem->token->value;
                            while (*token_start) {
                                value += *token_start;
                                token_start++;
                            }

                            fprintf(output, "%i%s", value, elem->next_sibling ? "," : "\n");
                        }
                    }
                }

                break;
            }
            case STRING_TYPE_TOKEN: {
                tree_t* name  = child->first_child;
                tree_t* value = name->next_sibling;
                fprintf(output, "%s db '%s', 0\n", name->token->value, value->token->value);
                break;
            }
            default: _generate_data_section(child, output); break;
        }
    }

    return 1;
}

static int _generate_expression(tree_t* node, FILE* output) {
    if (!node) return 0;

    if (node->token->t_type == IF_TOKEN) _generate_if(node, output);
    else if (node->token->t_type == FUNC_TOKEN) _generate_function(node, output);
    else if (node->token->t_type == WHILE_TOKEN) _generate_while(node, output);
    else if (node->token->t_type == SYSCALL_TOKEN) _generate_syscall(node, output);
    else if (node->token->t_type == ASIGN_TOKEN) _generate_assignment(node, output);
    else if (node->token->t_type == UNKNOWN_NUMERIC_TOKEN) {
        /*
        If it unknown num, we store it in eax register for other operations.
        */
        fprintf(output, "mov eax, %s\n", node->token->value);
    }
    else if (node->token->t_type == INT_TYPE_TOKEN) {
        /*
        Init and saving variable in stack.
        Getting variable name and memory offset.
        */
        tree_t* name_node = node->first_child;
        const char* var_name = (char*)name_node->token->value;
        
        /*
        Put value from eax to stack with variable offset.
        EAX register is shared point of all recursive _generate calls.
        */
        int var_offset = node->variable_offset;
        if (name_node->next_sibling) {
            _generate_expression(name_node->next_sibling, output);
            fprintf(output, "mov [ebp - %d], eax ; int %s = eax\n", var_offset, var_name);
        } 
        else {
            fprintf(output, "mov dword [ebp - %d], 0 ; int %s = 0\n", var_offset, var_name);
        }
    }
    else if (node->token->t_type == INT_VARIABLE_TOKEN) {
        /*
        Getting integer from stack, and saving in EAX shared register.
        */
        fprintf(output, "mov eax, [ebp - %d] ; %s\n", node->variable_offset, node->token->value);
        _generate_expression(node->first_child, output);
    }
    else if (node->token->t_type == ARR_VARIABLE_TOKEN || node->token->t_type == STR_VARIABLE_TOKEN) {
        if (node->first_child) {
            array_info_t* arr_info = _find_array_info((char*)node->token->value);
            int elem_size = arr_info ? arr_info->el_size : 1;
            _generate_expression(node->first_child, output);
            fprintf(output, "imul eax, %d\n", elem_size); 
            fprintf(output, "add eax, %s\n", node->token->value);
            if (elem_size == 1) fprintf(output, "mov al, [eax]\n");
            else if (elem_size == 4) fprintf(output, "mov eax, [eax]\n");
        }
        else {
            fprintf(output, "mov eax, %s\n", node->token->value);
        }
    }
    else if (node->token->t_type == PLUS_TOKEN) {
        _generate_expression(node->first_child, output);
        fprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output);
        fprintf(output, "pop ebx\n");
        fprintf(output, "add eax, ebx\n");
    }
    else if (node->token->t_type == MINUS_TOKEN) {
        _generate_expression(node->first_child, output);
        fprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output);
        fprintf(output, "pop ebx\n");
        fprintf(output, "sub ebx, eax\n");
        fprintf(output, "mov eax, ebx\n");
    }
    else if (node->token->t_type == MULTIPLY_TOKEN) {
        _generate_expression(node->first_child, output);
        fprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output);
        fprintf(output, "pop ebx\n");
        fprintf(output, "imul eax, ebx\n");
    }
    else if (node->token->t_type == DIVIDE_TOKEN) {
        _generate_expression(node->first_child, output);
        fprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output);
        fprintf(output, "mov ebx, eax\n");
        fprintf(output, "pop eax\n");
        fprintf(output, "cdq\n");
        fprintf(output, "idiv ebx\n");
    }
    else if (node->token->t_type == LARGER_TOKEN) {
        _generate_expression(node->first_child, output);
        fprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output);
        fprintf(output, "pop ebx\n");
        fprintf(output, "cmp ebx, eax\n");
        fprintf(output, "setg al\n");
        fprintf(output, "movzx eax, al\n");
    }
    else if (node->token->t_type == LOWER_TOKEN) {
        _generate_expression(node->first_child, output);
        fprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output);
        fprintf(output, "pop ebx\n");
        fprintf(output, "cmp ebx, eax\n");
        fprintf(output, "setl al\n");
        fprintf(output, "movzx eax, al\n");
    }
    else if (node->token->t_type == COMPARE_TOKEN) {
        _generate_expression(node->first_child, output);
        fprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output);
        fprintf(output, "pop ebx\n");
        fprintf(output, "cmp ebx, eax\n");
        fprintf(output, "sete al\n");
        fprintf(output, "movzx eax, al\n");
    }
    else if (node->token->t_type == NCOMPARE_TOKEN) {
        _generate_expression(node->first_child, output);
        fprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output);
        fprintf(output, "pop ebx\n");
        fprintf(output, "cmp ebx, eax\n");
        fprintf(output, "setne al\n");
        fprintf(output, "movzx eax, al\n");
    }
    else if (node->token->t_type == CALL_TOKEN) {
        /*
        Generating function preparations.
        1) Getting function name and args.
        2) Put args to the stack.
        */
        tree_t* func_name_node = node->first_child;
        tree_t* args_node = func_name_node->next_sibling;
        int arg_count = 0;
        for (tree_t* arg = args_node->first_child; arg; arg = arg->next_sibling) {
            if (arg->token->t_type == INT_VARIABLE_TOKEN) {
                fprintf(output, "mov eax, [ebp - %d]\n", arg->variable_offset);
            }
            else {
                fprintf(output, "mov eax, %s\n", arg->token->value);
            }

            fprintf(output, "push eax\n");
            arg_count++;
        }
    
        fprintf(output, "call %s\n", func_name_node->token->value);
        if (arg_count > 0) {
            fprintf(output, "add esp, %d\n", arg_count * 4);
        }
    }
    else if (node->token->t_type == EXIT_TOKEN) {
        _generate_expression(node->first_child, output);
        /*
        Restore stack frame after programm.
        */
        fprintf(output, "mov ebx, eax\n");
        fprintf(output, "mov eax, 1\n");
        fprintf(output, "mov esp, ebp\n");
        fprintf(output, "pop ebp\n");
        fprintf(output, "int 0x80\n");
    }

    return 1;
}

static int _generate_function(tree_t* node, FILE* output) {
    tree_t* name_node = node->first_child;
    tree_t* params_node = name_node->next_sibling;
    tree_t* body_node = params_node->next_sibling;
    tree_t* return_node = body_node->next_sibling;

    fprintf(output, "jmp _end_%s\n", name_node->token->value);
    fprintf(output, "%s:\n", name_node->token->value);
    fprintf(output, "push ebp\n");
    fprintf(output, "mov ebp, esp\n");

    /*
    Reserving stack memory for local variables. (Creating stack frame).
    We should go into function body and find all local variables.
    */
    int local_vars_size = 0;
    for (tree_t* arg = params_node->first_child; arg; arg = arg->next_sibling) {
        if (arg->token->t_type == INT_TYPE_TOKEN) local_vars_size += 4;
    }

    for (tree_t* expression = body_node->first_child; expression; expression = expression->next_sibling) {
        if (expression->token->t_type == INT_TYPE_TOKEN) local_vars_size += 4;
    }

    fprintf(output, "sub esp, %d\n", local_vars_size);

    /*
    Loading input args to stack.
    */
    int param_offset = 8;
    for (tree_t* param = params_node->first_child; param; param = param->next_sibling) {
        char* param_name = (char*)param->first_child->token->value;
        fprintf(output, "mov eax, [ebp + %d]\n", param_offset);
        fprintf(output, "mov [ebp - %d], eax\n", param_offset - 4);
        param_offset += 4;
    }

    /*
    Function body without return statement.
    All expressions will use one shared register EAX.
    */
    for (tree_t* part = body_node->first_child; part; part = part->next_sibling) {
        _generate_expression(part, output);
    }

    /*
    Move to eax return expresion, and restore stack.
    Now, result of function stored at EAX register.
    */
    _generate_expression(return_node->first_child, output);

    fprintf(output, "mov esp, ebp\n");
    fprintf(output, "pop ebp\n");
    fprintf(output, "ret\n");
    fprintf(output, "_end_%s:\n", name_node->token->value);
    return 1;
}

static int _generate_while(tree_t* node, FILE* output) {
    if (!node) return 0;

    int current_label = _label_counter++;
    tree_t* condition = node->first_child;
    tree_t* body = condition->next_sibling->first_child;

    fprintf(output, "while_%d:\n", current_label);

    _generate_expression(condition, output);
    fprintf(output, "cmp eax, 0\n");
    fprintf(output, "je end_while_%d\n", current_label);

    while (body) {
        _generate_expression(body, output);
        body = body->next_sibling;
    }

    fprintf(output, "jmp while_%d\n", current_label);
    fprintf(output, "end_while_%d:\n", current_label);
    return 1;
}

static int _generate_if(tree_t* node, FILE* output) {
    int current_label = _label_counter++;
    tree_t* condition = node->first_child;
    tree_t* body = condition->next_sibling->first_child;

    _generate_expression(condition, output);
    fprintf(output, "cmp eax, 0\n");
    fprintf(output, "je end_if_%d\n", current_label);


    while (body) {
        _generate_expression(body, output);
        body = body->next_sibling;
    }

    fprintf(output, "end_if_%d:\n", current_label);
    return 1;
}

static int _generate_syscall(tree_t* node, FILE* output) {
    char* registers[] = {
        "eax", "ebx", "ecx", "edx", "esx", "edi"
    };

    int argument_index = 0;
    tree_t* args = node->first_child;
    while (args) {
        if (args->token->t_type == INT_VARIABLE_TOKEN) fprintf(output, "mov %s, [ebp - %d]\n", registers[argument_index++], args->variable_offset);
        else fprintf(output, "mov %s, %s\n", registers[argument_index++], args->token->value);
        args = args->next_sibling;
    }

    fprintf(output, "int 0x80\n");
    return 1;
}

static int _generate_assignment(tree_t* node, FILE* output) {
    tree_t* left = node->first_child;
    tree_t* right = left->next_sibling;
    
    /*
    We store right result to EAX, and move it to stack with offset of left.
    */
    if ((left->token->t_type == ARR_VARIABLE_TOKEN || left->token->t_type == STR_VARIABLE_TOKEN) && left->first_child) {
        /*
        If left is array or string (array too) with elem size info.
        */
        array_info_t* arr_info = _find_array_info((char*)left->token->value);
        int elem_size = arr_info ? arr_info->el_size : 1;

        _generate_expression(left->first_child, output);
        fprintf(output, "imul eax, %d\n", elem_size);
        fprintf(output, "add eax, %s\n", left->token->value);
        fprintf(output, "push eax\n");
        
        _generate_expression(right, output);
        fprintf(output, "pop ebx\n");
        if (elem_size == 1) fprintf(output, "mov byte [ebx], al\n");
        else if (elem_size == 4) fprintf(output, "mov [ebx], eax\n");
    } 
    else {
        /*
        Move to eax result of right operation, and store it in stack with offset.
        */
        _generate_expression(right, output);
        fprintf(output, "mov [ebp - %d], eax\n", left->variable_offset);
    }

    return 1;
}

static int __get_variables_size(tree_t* head) {
    if (!head) return 0;
    int size = 0;
    for (tree_t* expression = head; expression; expression = expression->next_sibling) {
        if (!expression->token) continue;
        if (expression->token->t_type == INT_TYPE_TOKEN) size += expression->variable_size;
        else if (expression->token->t_type == WHILE_TOKEN || expression->token->t_type == IF_TOKEN) {
            size += __get_variables_size(expression->first_child->next_sibling->first_child);
        }
    }
    
    return size;
}

static int _generate_text_section(tree_t* node, FILE* output) {
    if (!node) return 0;
    /*
    Before start, find all "global" variables and reserve stack frame.
    */
    fprintf(output, "push ebp\n");
    fprintf(output, "mov ebp, esp\n");
    fprintf(output, "sub esp, %d\n", __get_variables_size(node->first_child));

    for (tree_t* child = node->first_child; child; child = child->next_sibling) _generate_expression(child, output);
    _generate_expression(node->next_sibling, output);
    return 1;
}

int generate_asm(tree_t* root, FILE* output) {
    fprintf(output, "section .data\n");
    _generate_data_section(root->first_child, output);
    fprintf(output, "\nsection .text\n");
    fprintf(output, "global _start\n\n");
    fprintf(output, "_start:\n");
    _generate_text_section(root->first_child, output);
    return 1;
}
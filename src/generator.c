#include "../include/generator.h"


#pragma region [Misc]

    static int _label_counter = 0;
    static int _generate_function(tree_t*, FILE*);
    static int _generate_if(tree_t*, FILE*);
    static int _generate_while(tree_t*, FILE*);
    static int _generate_syscall(tree_t*, FILE*);
    static int _generate_assignment(tree_t*, FILE*);

    typedef struct {
        char name[TOKEN_MAX_SIZE];
        int el_size;
    } array_info_t;

    static array_info_t _arrays_info[ARRAYS_MAX_TOKEN];
    static int _array_count = 0;
    static int _current_depth = 1;

    static array_info_t* _find_array_info(const char* name) {
        for (int i = 0; i < _array_count; i++) {
            if (str_strcmp(_arrays_info[i].name, (char*)name) == 0) {
                return &_arrays_info[i];
            }
        }

        return NULL;
    }

#pragma endregion


static int _generate_rodata_section(tree_t* node, FILE* output) {
    if (!node) return 0;
    static int num = 0;
    for (tree_t* child = node->first_child; child; child = child->next_sibling) {
        if (!child->token) {
            _generate_rodata_section(child, output);
            continue;
        }

        if (child->token->ro) {
            tree_t* name  = child->first_child;
            tree_t* value = name->next_sibling;
            switch (child->token->t_type) {
                case STR_TYPE_TOKEN:   iprintf(output, "%s db '%s'\n", name->token->value, value->token->value); break;
                case PTR_TYPE_TOKEN:   iprintf(output, "%s dd %s\n", name->token->value, value->token->value); break;
                case INT_TYPE_TOKEN:   iprintf(output, "%s dd %s\n", name->token->value, value->token->value); break;
                case SHORT_TYPE_TOKEN: iprintf(output, "%s dw %s\n", name->token->value, value->token->value); break;
                case CHAR_TYPE_TOKEN:  iprintf(output, "%s db %s\n", name->token->value, value->token->value); break;
                default: break;
            }
        }
        else if (!child->token->glob) {
            switch (child->token->t_type) {
                case IF_TOKEN:
                case CALL_TOKEN:
                case WHILE_TOKEN: _generate_rodata_section(child, output); break;
                case STRING_VALUE_TOKEN: 
                    iprintf(output, "string_%d db '%s'\n", num, child->token->value);
                    sprintf((char*)child->token->value, "string_%d", num++);
                    break;
                default: break;
            }
        }
    }

    return 1;
}

static int _generate_data_section(tree_t* node, FILE* output) {
    if (!node) return 0;
    for (tree_t* child = node->first_child; child; child = child->next_sibling) {
        if (!child->token) {
            _generate_data_section(child, output);
            continue;
        }

        if (child->token->glob) {
            tree_t* name  = child->first_child;
            tree_t* value = name->next_sibling;
            switch (child->token->t_type) {
                case STR_TYPE_TOKEN:   iprintf(output, "%s db '%s'\n", name->token->value, value->token->value); break;
                case PTR_TYPE_TOKEN:   iprintf(output, "%s dd %s\n", name->token->value, value->token->value); break;
                case INT_TYPE_TOKEN:   iprintf(output, "%s dd %s\n", name->token->value, value->token->value); break;
                case SHORT_TYPE_TOKEN: iprintf(output, "%s dw %s\n", name->token->value, value->token->value); break;
                case CHAR_TYPE_TOKEN:  iprintf(output, "%s db %s\n", name->token->value, value->token->value); break;
                default: break;
            }
        }
        else if (!child->token->ro) {
            switch (child->token->t_type) {
                case IF_TOKEN:
                case FUNC_TOKEN:
                case WHILE_TOKEN: _generate_data_section(child, output); break;
                case ARRAY_TYPE_TOKEN: {
                    tree_t* size   = child->first_child;
                    tree_t* t_type = size->next_sibling;
                    tree_t* name   = t_type->next_sibling;
                
                    array_info_t info;
                    const char* directive = NULL;
                    str_strncpy(info.name, (char*)name->token->value, 32);
                    if (!str_strcmp((char*)t_type->token->value, CHAR_VARIABLE)) {
                        directive = "db";
                        info.el_size = 1;
                    }
                    else if (!str_strcmp((char*)t_type->token->value, SHORT_VARIABLE)) {
                        directive = "dw";
                        info.el_size = 2;
                    }
                    else if (!str_strcmp((char*)t_type->token->value, INT_VARIABLE)) {
                        directive = "dd";
                        info.el_size = 4;
                    }
                    else {
                        directive = "dq";
                        info.el_size = 8;
                    }
                
                    _arrays_info[_array_count++] = info;
                    if (!name->next_sibling) iprintf(output, "%s times %s %s 0\n", name->token->value, size->token->value, directive);
                    else {
                        iprintf(output, "%s %s ", name->token->value, directive);
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

                case STR_TYPE_TOKEN: 
                    tree_t* name  = child->first_child;
                    tree_t* value = name->next_sibling;
                    iprintf(output, "%s db '%s', 0\n", name->token->value, value->token->value);
                    break;

                default: break;
            }
        }
    }

    return 1;
}

static int _generate_expression(tree_t* node, FILE* output) {
    if (!node) return 0;
    if (node->token->t_type == IF_TOKEN)            _generate_if(node, output);
    else if (node->token->t_type == FUNC_TOKEN)     _generate_function(node, output);
    else if (node->token->t_type == WHILE_TOKEN)    _generate_while(node, output);
    else if (node->token->t_type == SYSCALL_TOKEN)  _generate_syscall(node, output);
    else if (node->token->t_type == ASIGN_TOKEN)    _generate_assignment(node, output);
    else if (node->token->t_type == UNKNOWN_NUMERIC_TOKEN) {
        /*
        If it unknown num, we store it in eax register for other operations.
        */
       iprintf(output, "mov eax, %s\n", node->token->value);
    }
    else if (node->token->t_type == INT_TYPE_TOKEN) {
        /*
        Init and saving variable in stack.
        Getting variable name and memory offset.
        Put value from eax to stack with variable offset.
        EAX register is shared point of all recursive _generate calls.
        */
        tree_t* name_node = node->first_child;
        if (name_node->next_sibling) {
            _generate_expression(name_node->next_sibling, output);
            iprintf(output, "mov [ebp - %d], eax ; int %s = eax\n", node->variable_offset, (char*)name_node->token->value);
        }
    }
    else if (node->token->t_type == INT_VARIABLE_TOKEN) {
        /*
        Getting integer from stack, and saving in EAX shared register.
        */
       iprintf(output, "mov eax, [ebp - %d] ; int %s\n", node->variable_offset, node->token->value);
        _generate_expression(node->first_child, output);
    }
    else if (node->token->t_type == SHORT_TYPE_TOKEN) {
        tree_t* name_node = node->first_child;
        if (name_node->next_sibling) {
            _generate_expression(name_node->next_sibling, output);
            iprintf(output, "mov [ebp - %d], ax ; short %s = eax\n", node->variable_offset, (char*)name_node->token->value);
        }
    }
    else if (node->token->t_type == SHORT_VARIABLE_TOKEN) {
        iprintf(output, "mov ax, [ebp - %d] ; short %s\n", node->variable_offset, node->token->value);
        _generate_expression(node->first_child, output);
    }
    else if (node->token->t_type == CHAR_TYPE_TOKEN) {
        tree_t* name_node = node->first_child;
        if (name_node->next_sibling) {
            _generate_expression(name_node->next_sibling, output);
            iprintf(output, "mov [ebp - %d], al ; char %s = eax\n", node->variable_offset, (char*)name_node->token->value);
        }
    }
    else if (node->token->t_type == CHAR_VARIABLE_TOKEN) {
        iprintf(output, "mov al, [ebp - %d] ; char %s\n", node->variable_offset, node->token->value);
        _generate_expression(node->first_child, output);
    }
    else if (node->token->t_type == ARR_VARIABLE_TOKEN || node->token->t_type == STR_VARIABLE_TOKEN) {
        if (!node->first_child) iprintf(output, "mov eax, %s\n", node->token->value);
        else {
            array_info_t* arr_info = _find_array_info((char*)node->token->value);
            int elem_size = arr_info ? arr_info->el_size : 1;
            _generate_expression(node->first_child, output);
            if (elem_size > 1) iprintf(output, "imul eax, %d\n", elem_size); 
            iprintf(output, "add eax, %s\n", node->token->value);
            if (elem_size == 1) iprintf(output, "mov al, [eax]\n");
            else if (elem_size == 4) iprintf(output, "mov eax, [eax]\n");
        }
    }
    else if (node->token->t_type == PTR_TYPE_TOKEN) {
        tree_t* name_node = node->first_child;
        if (name_node->next_sibling) {
            _generate_expression(name_node->next_sibling, output);
            iprintf(output, "mov [ebp - %d], eax ; ptr %s = eax\n", node->variable_offset, (char*)name_node->token->value);
        }
    }
    else if (node->token->t_type == PTR_VARIABLE_TOKEN) {
        if (!node->first_child) iprintf(output, "mov eax, [ebp - %d]\n", node->variable_offset);
        else {
            _generate_expression(node->first_child, output);
            iprintf(output, "add eax, [ebp - %d]\n", node->variable_offset);
            iprintf(output, "mov eax, [eax]\n");
        }
    }
    else if (node->token->t_type == BITMOVE_LEFT_TOKEN) {
        _generate_expression(node->first_child, output);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output);
        iprintf(output, "pop ebx\n");
        iprintf(output, "mov ecx, eax\n");
        iprintf(output, "shl ebx, cl\n");
        iprintf(output, "mov eax, ebx\n");
    }
    else if (node->token->t_type == BITMOVE_RIGHT_TOKEN) {
        _generate_expression(node->first_child, output);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output);
        iprintf(output, "pop ebx\n");
        iprintf(output, "mov ecx, eax\n");
        iprintf(output, "shr ebx, cl\n");
        iprintf(output, "mov eax, ebx\n");
    }
    else if (node->token->t_type == BITAND_TOKEN) {
        _generate_expression(node->first_child, output);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output);
        iprintf(output, "pop ebx\n");
        iprintf(output, "and eax, ebx\n");
    }
    else if (node->token->t_type == BITOR_TOKEN) {
        _generate_expression(node->first_child, output);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output);
        iprintf(output, "pop ebx\n");
        iprintf(output, "or eax, ebx\n");
    }
    else if (node->token->t_type == PLUS_TOKEN) {
        _generate_expression(node->first_child, output);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output);
        iprintf(output, "pop ebx\n");
        iprintf(output, "add eax, ebx\n");
    }
    else if (node->token->t_type == MINUS_TOKEN) {
        _generate_expression(node->first_child, output);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output);
        iprintf(output, "pop ebx\n");
        iprintf(output, "sub ebx, eax\n");
        iprintf(output, "mov eax, ebx\n");
    }
    else if (node->token->t_type == MULTIPLY_TOKEN) {
        _generate_expression(node->first_child, output);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output);
        iprintf(output, "pop ebx\n");
        iprintf(output, "imul eax, ebx\n");
    }
    else if (node->token->t_type == DIVIDE_TOKEN) {
        _generate_expression(node->first_child, output);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output);
        iprintf(output, "mov ebx, eax\n");
        iprintf(output, "pop eax\n");
        iprintf(output, "cdq\n");
        iprintf(output, "idiv ebx\n");
    }
    else if (node->token->t_type == LARGER_TOKEN) {
        _generate_expression(node->first_child, output);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output);
        iprintf(output, "pop ebx\n");
        iprintf(output, "cmp ebx, eax\n");
        iprintf(output, "setg al\n");
        iprintf(output, "movzx eax, al\n");
    }
    else if (node->token->t_type == LOWER_TOKEN) {
        _generate_expression(node->first_child, output);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output);
        iprintf(output, "pop ebx\n");
        iprintf(output, "cmp ebx, eax\n");
        iprintf(output, "setl al\n");
        iprintf(output, "movzx eax, al\n");
    }
    else if (node->token->t_type == COMPARE_TOKEN) {
        _generate_expression(node->first_child, output);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output);
        iprintf(output, "pop ebx\n");
        iprintf(output, "cmp ebx, eax\n");
        iprintf(output, "sete al\n");
        iprintf(output, "movzx eax, al\n");
    }
    else if (node->token->t_type == NCOMPARE_TOKEN) {
        _generate_expression(node->first_child, output);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output);
        iprintf(output, "pop ebx\n");
        iprintf(output, "cmp ebx, eax\n");
        iprintf(output, "setne al\n");
        iprintf(output, "movzx eax, al\n");
    }
    else if (node->token->t_type == CALL_TOKEN) {
        /*
        Generating function preparations.
        1) Getting function name and args.
        2) Put args to the stack.
        */
        int variables_size = 0;
        tree_t* func_name_node = node;
        tree_t* args_node = func_name_node->first_child;

        /*
        Saving params in stack.
        */
        int arg_count = 0;
        tree_t* args[128] = { NULL };
        for (tree_t* arg = args_node->first_child; arg; arg = arg->next_sibling) {
            args[arg_count++] = arg;
        }

        fprintf(output, "\n ; --------------- Call function %s --------------- \n", func_name_node->token->value);

        for (int i = arg_count - 1; i >= 0; --i) {
            tree_t* arg = args[i];
            switch (get_variable_type(arg->token->t_type)) {
                case 32: iprintf(output, "mov eax, [ebp - %d] ; uint32 %s \n", arg->variable_offset, arg->token->value); break;
                case 16: iprintf(output, "mov ax, [ebp - %d] ; uint16 %s \n", arg->variable_offset, arg->token->value); break;
                case 8:  iprintf(output, "mov al, [ebp - %d] ; uint8 %s \n", arg->variable_offset, arg->token->value); break;
                case 1:  iprintf(output, "mov eax, %s ; ptr32 %s\n", arg->token->value, arg->token->value); break;
                default: break;
            }
            
            iprintf(output, "push eax\n");
        }
    
        iprintf(output, "call %s\n", func_name_node->token->value);
        if (arg_count > 0) iprintf(output, "add esp, %d\n", arg_count * 4);
        fprintf(output, " ; --------------- \n");
    }
    else if (node->token->t_type == EXIT_TOKEN) {
        /*
        Restore stack frame after programm.
        */
        fprintf(output, "\n ; --------------- Exit --------------- \n");
        _generate_expression(node->first_child, output);
        iprintf(output, "mov ebx, eax\n");
        iprintf(output, "mov eax, 1\n");
        iprintf(output, "mov esp, ebp\n");
        iprintf(output, "pop ebp\n");
        iprintf(output, "int 0x80\n");
    }
    else if (node->token->t_type == RETURN_TOKEN) {
        fprintf(output, "\n ; --------------- Return --------------- \n");
        /*
        Move to eax return expresion, and restore stack.
        Now, result of function stored at EAX register.
        */
        _generate_expression(node->first_child, output);
        iprintf(output, "mov esp, ebp\n");
        iprintf(output, "pop ebp\n");
        iprintf(output, "ret\n");
    }

    return 1;
}

static int __get_variables_size(tree_t* head) {
    int size = 0;
    if (!head) return 0;
    for (tree_t* expression = head; expression; expression = expression->next_sibling) {
        if (!expression->token) continue;
        if (
            expression->token->t_type == WHILE_TOKEN || 
            expression->token->t_type == IF_TOKEN
        ) size += __get_variables_size(expression->first_child->next_sibling->first_child);
        else size += expression->variable_size;
    }
    
    return size;
}

static int _generate_function(tree_t* node, FILE* output) {
    tree_t* name_node   = node->first_child;
    tree_t* params_node = name_node->next_sibling;
    tree_t* body_node   = params_node->next_sibling;
    tree_t* return_node = body_node->next_sibling;

    fprintf(output, "\n ; --------------- Function %s --------------- \n", name_node->token->value);
    iprintf(output, "jmp _end_%s\n", name_node->token->value);
    iprintf(output, "%s:\n", name_node->token->value);

    _current_depth += 1;
    iprintf(output, "push ebp\n");
    iprintf(output, "mov ebp, esp\n");

    /*
    Reserving stack memory for local variables. (Creating stack frame).
    We should go into function body and find all local variables.
    Also we remember input variables.
    */
    int local_vars_size = __get_variables_size(params_node->first_child) + __get_variables_size(body_node->first_child);
    iprintf(output, "sub esp, %d\n", local_vars_size);

    /*
    Loading input args to stack.
    */
    int param_offset = 8;
    for (tree_t* param = params_node->first_child; param; param = param->next_sibling) {
        int param_size = param->variable_size;
        char* param_name = (char*)param->first_child->token->value;
        switch (get_variable_type(param->first_child->token->t_type)) {
            case 32:
                iprintf(output, "mov eax, [ebp + %d] ; int %s \n", param_offset, param_name);
                iprintf(output, "mov [ebp - %d], eax\n", param_offset - param_size);
                break;

            case 16:
                iprintf(output, "mov ax, [ebp + %d] ; short %s \n", param_offset, param_name);
                iprintf(output, "mov [ebp - %d], ax\n", param_offset - param_size);
                break;

            case 8:
                iprintf(output, "mov al, [ebp + %d] ; char %s \n", param_offset, param_name);
                iprintf(output, "mov [ebp - %d], al\n", param_offset - param_size);
                break;

            case 1:
                iprintf(output, "mov [ebp - %d], %s\n", param_offset - param_size, param_name);
                break;
            default: break;
        }

        param_offset += param_size;
    }

    /*
    Function body without return statement.
    All expressions will use one shared register EAX.
    */
    for (tree_t* part = body_node->first_child; part; part = part->next_sibling) {
        _generate_expression(part, output);
    }

    _current_depth -= 1;
    fprintf(output, " ; --------------- \n");
    iprintf(output, "_end_%s:\n", name_node->token->value);
    return 1;
}

static int _generate_while(tree_t* node, FILE* output) {
    if (!node) return 0;
    int current_label = _label_counter++;
    tree_t* condition = node->first_child;
    tree_t* body = condition->next_sibling->first_child;

    fprintf(output, "\n ; --------------- while cycle [%i] --------------- \n", current_label);
    iprintf(output, "while_%d:\n", current_label);
    _current_depth += 1;

    _generate_expression(condition, output);
    iprintf(output, "cmp eax, 0\n");
    iprintf(output, "je end_while_%d\n", current_label);

    while (body) {
        _generate_expression(body, output);
        body = body->next_sibling;
    }

    iprintf(output, "jmp while_%d\n", current_label);
    
    _current_depth -= 1;
    fprintf(output, " ; --------------- \n");
    iprintf(output, "end_while_%d:\n", current_label);
    return 1;
}

static int _generate_if(tree_t* node, FILE* output) {
    if (!node) return 0;
    int current_label = _label_counter++;
    tree_t* condition = node->first_child;
    tree_t* body = condition->next_sibling;
    tree_t* else_body = body->next_sibling;

    fprintf(output, "\n ; --------------- if statement [%i] --------------- \n", current_label);
    _generate_expression(condition, output);
    iprintf(output, "cmp eax, 0\n");
    iprintf(output, "je end_if_%d\n", current_label);
    _current_depth += 1;

    tree_t* body_exp = body->first_child;
    while (body_exp) {
        _generate_expression(body_exp, output);
        body_exp = body_exp->next_sibling;
    }

    fprintf(output, " ; --------------- \n");
    iprintf(output, "end_if_%d:\n", current_label);
    if (else_body) {
        tree_t* else_body_exp = else_body->first_child;
        while (else_body_exp) {
            _generate_expression(else_body_exp, output);
            else_body_exp = else_body_exp->next_sibling;
        }
    }
    
    _current_depth -= 1;
    return 1;
}

static int _generate_syscall(tree_t* node, FILE* output) {
    if (!node) return 0;
    char* registers_32[] =  { "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp"  };
    char* registers_16[] =  { "ax", "bx", "cx", "dx", "si", "di", "bp"         };
    char* registers_8[] =   { "al", "bl", "cl", "dl", "sl", "dil", "bpl"       };

    fprintf(output, "\n ; --------------- system call --------------- \n");

    int argument_index = 0;
    tree_t* args = node->first_child;
    while (args) {
        switch (get_variable_type(args->token->t_type)) {
            case 32: iprintf(output, "mov %s, [ebp - %d]\n", registers_32[argument_index++], args->variable_offset); break;
            case 16: iprintf(output, "mov %s, [ebp - %d]\n", registers_16[argument_index++], args->variable_offset); break;
            case 8:  iprintf(output, "mov %s, [ebp - %d]\n", registers_8[argument_index++], args->variable_offset); break;
            case 1:  iprintf(output, "mov %s, %s\n", registers_32[argument_index++], args->token->value); break;
            default: break;
        }
        
        args = args->next_sibling;
    }

    iprintf(output, "int 0x80\n");
    fprintf(output, " ; --------------- \n");
    return 1;
}

static int _generate_assignment(tree_t* node, FILE* output) {
    if (!node) return 0;
    tree_t* left  = node->first_child;
    tree_t* right = left->next_sibling;
    
    fprintf(output, "\n; --------------- Assignment: %s = %s --------------- \n", left->token->value, right->token->value);

    /*
    We store right result to EAX, and move it to stack with offset of left.
    Pointer assignment.
    */
    if ((get_variable_size(left->token->t_type) == 32) && left->first_child) {
        /*
        If left is array or string (array too) with elem size info.
        */
        array_info_t* arr_info = _find_array_info((char*)left->token->value);
        int elem_size = arr_info ? arr_info->el_size : 1;

        /*
        Generate offset movement in this array-like data type.
        Then multiply it by arr el_size.
        */
        _generate_expression(left->first_child, output);
        if (elem_size > 1) iprintf(output, "imul eax, %d\n", elem_size);

        if (left->token->t_type == PTR_VARIABLE_TOKEN) iprintf(output, "add eax, [ebp - %d]\n", left->variable_offset);
        else iprintf(output, "add eax, %s\n", left->token->value);
        iprintf(output, "push eax\n");
        
        _generate_expression(right, output);
        iprintf(output, "pop ebx\n");
        if (elem_size == 1)      iprintf(output, "mov byte [ebx], al\n");
        else if (elem_size == 2) iprintf(output, "mov word [ebx], ax\n");
        else if (elem_size == 4) iprintf(output, "mov [ebx], eax\n");
    } 
    else {
        /*
        Move to eax result of right operation, and store it in stack with offset in the left.
        */
        _generate_expression(right, output);
        iprintf(output, "mov [ebp - %d], eax\n", left->variable_offset);
    }

    fprintf(output, " ; --------------- \n");

    return 1;
}

static int _generate_text_section(tree_t* node, FILE* output) {
    if (!node) return 0;
    for (tree_t* child = node->first_child; child; child = child->next_sibling) _generate_expression(child, output);
    return 1;
}

int generate_asm(tree_t* root, FILE* output) {
    tree_t* program_body = root->first_child;
    tree_t* prestart = program_body;
    tree_t* main_body = prestart->next_sibling;

    /*
    Generate data section. Here we store static arrays,
    static strings and etc. 
    Also we store here global vars.
    */
    fprintf(output, "\nsection .data\n");
    _generate_data_section(prestart, output);
    _generate_data_section(main_body, output);

    /*
    Generate rodata section. Here we store strings, that
    not assign to any variable.
    */
    fprintf(output, "\nsection .rodata\n");
    _generate_rodata_section(prestart, output);
    _generate_rodata_section(main_body, output);

    /*
    Generate text sction were placed while program code.
    */
    fprintf(output, "\nsection .text\n");
    if (prestart) {
        for (tree_t* child = prestart->first_child; child; child = child->next_sibling) {
            if (child->token) switch (child->token->t_type) {
                case IMPORT_SELECT_TOKEN: 
                    for (tree_t* func = child->first_child->first_child; func; func = func->next_sibling)
                        fprintf(output, "    extern %s\n", func->token->value);
                    break;

                case FUNC_TOKEN: 
                    fprintf(output, "    global %s\n", child->first_child->token->value);
                    break;

                default: break;
            }
        }

        _generate_text_section(prestart, output);
    }

    if (main_body) {
        fprintf(output, "\nglobal _start\n\n");
        fprintf(output, "    _start:\n");

        /*
        Before start, find all "global" variables and reserve stack frame.
        */
        iprintf(output, "push ebp\n");
        iprintf(output, "mov ebp, esp\n");
        iprintf(output, "sub esp, %d\n", __get_variables_size(main_body->first_child));
        _generate_text_section(main_body, output);
    }

    return 1;
}
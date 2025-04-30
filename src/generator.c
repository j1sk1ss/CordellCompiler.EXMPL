#include "../include/generator.h"


#pragma region [Misc]

    static int _label_counter = 0;
    static int _current_depth = 1;
    static int _generate_function(tree_t*, FILE*, const char*);
    static int _generate_switch(tree_t*, FILE*, const char*);
    static int _generate_if(tree_t*, FILE*, const char*);
    static int _generate_while(tree_t*, FILE*, const char*);
    static int _generate_syscall(tree_t*, FILE*, const char*);
    static int _generate_assignment(tree_t*, FILE*, const char*);

#pragma endregion


static int _generate_global(token_type_t t_type, tree_t* entry, FILE* output) {
    switch (t_type) {
        case STR_TYPE_TOKEN:   iprintf(output, "__%s__ db '%s', 0\n", (char*)entry->first_child->token->value, (char*)entry->first_child->next_sibling->token->value); break;
        case INT_TYPE_TOKEN:   iprintf(output, "__%s__ dd %s\n", (char*)entry->first_child->token->value, (char*)entry->first_child->next_sibling->token->value); break;
        case SHORT_TYPE_TOKEN: iprintf(output, "__%s__ dw %s\n", (char*)entry->first_child->token->value, (char*)entry->first_child->next_sibling->token->value); break;
        case CHAR_TYPE_TOKEN:  iprintf(output, "__%s__ db %s\n", (char*)entry->first_child->token->value, (char*)entry->first_child->next_sibling->token->value); break;
        case ARRAY_TYPE_TOKEN:
            tree_t* size   = entry->first_child;
            tree_t* t_type = size->next_sibling;
            tree_t* name   = t_type->next_sibling;
        
            int el_size = 1;
            const char* directive = NULL;
            if (!str_strcmp((char*)t_type->token->value, CHAR_VARIABLE)) directive = "db";
            else if (!str_strcmp((char*)t_type->token->value, SHORT_VARIABLE)) {
                directive = "dw";
                el_size = 2;
            }
            else if (!str_strcmp((char*)t_type->token->value, INT_VARIABLE)) {
                directive = "dd";
                el_size = 4;
            }
            else {
                directive = "dq";
                el_size = 8;
            }
        
            if (!name->next_sibling) iprintf(output, "__%s__ times %s %s 0\n", name->token->value, size->token->value, directive);
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
        default: break;
    }

    return 1;
}

static int _generate_rodata_section(tree_t* node, FILE* output) {
    if (!node) return 0;
    static int num = 0;
    for (tree_t* child = node->first_child; child; child = child->next_sibling) {
        if (!child->token) {
            _generate_rodata_section(child, output);
            continue;
        }

        if (child->token->ro) {
            _generate_global(child->token->t_type, child, output);
        }
        else if (!child->token->glob) {
            switch (child->token->t_type) {
                case IF_TOKEN:
                case CALL_TOKEN:
                case SYSCALL_TOKEN:
                case WHILE_TOKEN:  _generate_rodata_section(child, output); break;
                case SWITCH_TOKEN: _generate_rodata_section(child->first_child->next_sibling, output); break;
                case DEFAULT_TOKEN:
                case CASE_TOKEN:   _generate_rodata_section(child->first_child->first_child, output); break;
                case FUNC_TOKEN:   _generate_rodata_section(child->first_child->next_sibling->next_sibling, output); break;
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
            _generate_global(child->token->t_type, child, output);
        }
        else if (!child->token->ro) {
            switch (child->token->t_type) {
                case IF_TOKEN:
                case SYSCALL_TOKEN:
                case WHILE_TOKEN: _generate_data_section(child, output); break;
                case SWITCH_TOKEN: _generate_data_section(child->first_child->next_sibling, output); break;
                case DEFAULT_TOKEN:
                case CASE_TOKEN: _generate_data_section(child->first_child->first_child, output); break;
                case FUNC_TOKEN: _generate_data_section(child->first_child->next_sibling->next_sibling, output); break;
                default: break;
            }
        }
    }

    return 1;
}

static int _generate_expression(tree_t* node, FILE* output, const char* func) {
    if (!node) return 0;
    if (node->token->t_type == IF_TOKEN)            _generate_if(node, output, func);
    else if (node->token->t_type == SWITCH_TOKEN)   _generate_switch(node, output, func);
    else if (node->token->t_type == WHILE_TOKEN)    _generate_while(node, output, func);
    else if (node->token->t_type == FUNC_TOKEN)     _generate_function(node, output, func);
    else if (node->token->t_type == SYSCALL_TOKEN)  _generate_syscall(node, output, func);
    else if (node->token->t_type == ASIGN_TOKEN)    _generate_assignment(node, output, func);
    else if (node->token->t_type == UNKNOWN_NUMERIC_TOKEN) iprintf(output, "mov eax, %s\n", node->token->value);
    else if (node->token->t_type == CHAR_VALUE_TOKEN) iprintf(output, "mov eax, %i\n", *node->token->value);
    else if (node->token->ptr && is_variable(node->token->t_type)) {
        tree_t* name_node = node->first_child;
        if (name_node->next_sibling && !name_node->token->ro && !name_node->token->glob) {
            _generate_expression(name_node->next_sibling, output, func);
            iprintf(output, "mov %s, eax ; ptr %s = eax\n", GET_ASMVAR(name_node), (char*)name_node->token->value);
        }
    }
    else if (node->token->ptr && !is_variable(node->token->t_type) && get_variable_type(node->token)) {
        if (!node->first_child) iprintf(output, "mov eax, %s\n", GET_ASMVAR(node));
        else {
            variable_info_t info;
            if (get_var_info((char*)node->token->value, func, &info)) {
                _generate_expression(node->first_child, output, func);
                if (info.size > 1) iprintf(output, "imul eax, %d\n", info.size);
                iprintf(output, "add eax, %s\n", GET_ASMVAR(node));

                if (info.size == 1) iprintf(output, "movzx eax, byte [eax]\n");
                else if (info.size == 2) iprintf(output, "movzx eax, word [eax]\n");
                else iprintf(output, "mov eax, [eax]\n");
            }
        }
    }
    else if (node->token->t_type == INT_TYPE_TOKEN) {
        tree_t* name_node = node->first_child;
        if (name_node->next_sibling && !name_node->token->ro && !name_node->token->glob) {
            _generate_expression(name_node->next_sibling, output, func);
            iprintf(output, "mov %s, eax ; int %s = eax\n", GET_ASMVAR(name_node), (char*)name_node->token->value);
        }
    }
    else if (node->token->t_type == INT_VARIABLE_TOKEN) {
        iprintf(output, "mov eax, %s ; int %s\n", GET_ASMVAR(node), node->token->value);
        _generate_expression(node->first_child, output, func);
    }
    else if (node->token->t_type == SHORT_TYPE_TOKEN) {
        tree_t* name_node = node->first_child;
        if (name_node->next_sibling && !name_node->token->ro && !name_node->token->glob) {
            _generate_expression(name_node->next_sibling, output, func);
            iprintf(output, "mov word %s, ax ; short %s = ax\n", GET_ASMVAR(name_node), (char*)name_node->token->value);
        }
    }
    else if (node->token->t_type == SHORT_VARIABLE_TOKEN) {
        iprintf(output, "mov ax, %s ; short %s\n", GET_ASMVAR(node), node->token->value);
        _generate_expression(node->first_child, output, func);
    }
    else if (node->token->t_type == CHAR_TYPE_TOKEN) {
        tree_t* name_node = node->first_child;
        if (name_node->next_sibling && !name_node->token->ro && !name_node->token->glob) {
            _generate_expression(name_node->next_sibling, output, func);
            iprintf(output, "mov byte %s, al ; char %s = al\n", GET_ASMVAR(name_node), (char*)name_node->token->value);
        }
    }
    else if (node->token->t_type == CHAR_VARIABLE_TOKEN) {
        iprintf(output, "mov al, %s ; char %s\n", GET_ASMVAR(node), node->token->value);
        _generate_expression(node->first_child, output, func);
    }
    else if (node->token->t_type == ARRAY_TYPE_TOKEN) {
        if (node->first_child && !node->token->ro && !node->token->glob) {
            tree_t* size   = node->first_child;
            tree_t* t_type = size->next_sibling;
            tree_t* name   = t_type->next_sibling;
            
            array_info_t arr_info = { .el_size = 1 };
            if (get_array_info((char*)name->token->value, func, &arr_info)) {
                tree_t* vals = name->next_sibling;
                if (vals && vals->token->t_type != DELIMITER_TOKEN) {
                    fprintf(output, "\n ; --------------- Array setup %s --------------- \n", name->token->value);
                    
                    char* reg = NULL;
                    char* mov_op = NULL;
                    if (arr_info.el_size == 1) {
                        reg = "al";
                        mov_op = " byte ";
                    }
                    else if (arr_info.el_size == 2) {
                        reg = "ax";
                        mov_op = " word ";
                    }
                    else {
                        reg = "eax";
                        mov_op = " ";
                    }
                    
                    int base_off = name->variable_offset;
                    for (tree_t* v = vals; v && v->token->t_type != DELIMITER_TOKEN; v = v->next_sibling) {
                        if (v->token->t_type == UNKNOWN_NUMERIC_TOKEN) iprintf(output, "mov%s[ebp - %d], %d\n", mov_op, base_off, str_atoi((char*)v->token->value));
                        else if (v->token->t_type == CHAR_VALUE_TOKEN) iprintf(output, "mov%s[ebp - %d], %d\n", mov_op, base_off, *v->token->value);    
                        else {
                            int is_ptr = get_array_info((char*)v->token->value, func, NULL) && !(v->token->ro || v->token->glob);
                            iprintf(output, "%s %s, %s ; uint32 %s \n", !is_ptr ? "mov" : "lea", reg, GET_ASMVAR(v), v->token->value); 
                            iprintf(output, "mov%s[ebp - %d], %s\n", mov_op, base_off, reg);
                        }
    
                        base_off -= arr_info.el_size;
                    }

                    fprintf(output, " ; --------------- \n");
                }
            }
        }
    }
    else if (node->token->t_type == STR_TYPE_TOKEN) {
        tree_t* name_node = node->first_child;
        if (name_node->next_sibling && !name_node->token->ro && !name_node->token->glob) {
            fprintf(output, "\n ; --------------- String setup %s --------------- \n", name_node->token->value);
            tree_t* val_node = name_node->next_sibling;
            char* val_head = (char*)val_node->token->value;
            int base_off = name_node->variable_offset;
            while (*val_head) {
                iprintf(output, "mov byte [ebp - %d], %i\n", base_off--, *val_head);
                val_head++;
            }

            iprintf(output, "mov byte [ebp - %d], 0\n", base_off);
            fprintf(output, " ; --------------- \n");
        }
    }
    else if (node->token->t_type == ARR_VARIABLE_TOKEN || node->token->t_type == STR_VARIABLE_TOKEN) {
        if (!node->first_child) iprintf(output, "mov eax, %s\n", node->token->value);
        else {
            array_info_t arr_info = { .el_size = 1 };
            get_array_info((char*)node->token->value, func, &arr_info);
            _generate_expression(node->first_child, output, func);
            if (arr_info.el_size > 1) iprintf(output, "imul eax, %d\n", arr_info.el_size);
            iprintf(output, "%s ebx, %s\n", (node->token->ro || node->token->glob) ? "mov" : "lea",  GET_ASMVAR(node));
            iprintf(output, "add eax, ebx\n");

            if (arr_info.el_size == 1) iprintf(output, "movzx eax, byte [eax]\n");
            else if (arr_info.el_size == 2) iprintf(output, "movzx eax, word [eax]\n");
            else iprintf(output, "mov eax, [eax]\n");
        }
    }
    else if (node->token->t_type == BITMOVE_LEFT_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop ebx\n");
        iprintf(output, "mov ecx, eax\n");
        iprintf(output, "shl ebx, cl\n");
        iprintf(output, "mov eax, ebx\n");
    }
    else if (node->token->t_type == BITMOVE_RIGHT_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop ebx\n");
        iprintf(output, "mov ecx, eax\n");
        iprintf(output, "shr ebx, cl\n");
        iprintf(output, "mov eax, ebx\n");
    }
    else if (node->token->t_type == BITAND_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop ebx\n");
        iprintf(output, "and eax, ebx\n");
    }
    else if (node->token->t_type == BITOR_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop ebx\n");
        iprintf(output, "or eax, ebx\n");
    }
    else if (node->token->t_type == PLUS_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop ebx\n");
        iprintf(output, "add eax, ebx\n");
    }
    else if (node->token->t_type == MINUS_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop ebx\n");
        iprintf(output, "sub ebx, eax\n");
        iprintf(output, "mov eax, ebx\n");
    }
    else if (node->token->t_type == MULTIPLY_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop ebx\n");
        iprintf(output, "imul eax, ebx\n");
    }
    else if (node->token->t_type == DIVIDE_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "mov ebx, eax\n");
        iprintf(output, "pop eax\n");
        iprintf(output, "cdq\n");
        iprintf(output, "idiv ebx\n");
    }
    else if (node->token->t_type == MODULO_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "mov ebx, eax\n");
        iprintf(output, "pop eax\n");
        iprintf(output, "cdq\n");
        iprintf(output, "idiv ebx\n");
        iprintf(output, "mov eax, edx\n");
    }
    else if (node->token->t_type == LARGER_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop ebx\n");
        iprintf(output, "cmp ebx, eax\n");
        iprintf(output, "setg al\n");
        iprintf(output, "movzx eax, al\n");
    }
    else if (node->token->t_type == LOWER_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop ebx\n");
        iprintf(output, "cmp ebx, eax\n");
        iprintf(output, "setl al\n");
        iprintf(output, "movzx eax, al\n");
    }
    else if (node->token->t_type == COMPARE_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop ebx\n");
        iprintf(output, "cmp ebx, eax\n");
        iprintf(output, "sete al\n");
        iprintf(output, "movzx eax, al\n");
    }
    else if (node->token->t_type == NCOMPARE_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
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
            switch (get_variable_type(arg->token)) {
                case 1: 
                case 32:
                    int is_ptr = get_array_info((char*)arg->token->value, func, NULL) && !(arg->token->ro || arg->token->glob);
                    iprintf(output, "%s eax, %s ; uint32 %s \n", !is_ptr ? "mov" : "lea", GET_ASMVAR(arg), arg->token->value); 
                    break;
                case 16: iprintf(output, "mov ax, %s ; uint16 %s \n", GET_ASMVAR(arg), arg->token->value); break;
                case 8:  iprintf(output, "mov al, %s ; uint8 %s \n", GET_ASMVAR(arg), arg->token->value); break;
                default: break;
            }
            
            iprintf(output, "push eax\n");
        }
    
        iprintf(output, "call __%s__\n", func_name_node->token->value);
        if (arg_count > 0) iprintf(output, "add esp, %d\n", arg_count * 4);
        fprintf(output, " ; --------------- \n");
    }
    else if (node->token->t_type == EXIT_TOKEN) {
        /*
        Restore stack frame after programm.
        */
        fprintf(output, "\n ; --------------- Exit --------------- \n");
        _generate_expression(node->first_child, output, func);
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
        _generate_expression(node->first_child, output, func);
        iprintf(output, "mov esp, ebp\n");
        iprintf(output, "pop ebp\n");
        iprintf(output, "ret\n");
    }

    return 1;
}

static int _get_variables_size(tree_t* head, const char* func) {
    int size = 0;
    if (!head) return 0;
    for (tree_t* expression = head; expression; expression = expression->next_sibling) {
        if (!expression->token) continue;
        if (expression->token->ro || expression->token->glob) continue;
        if (expression->token->t_type == ARRAY_TYPE_TOKEN) {
            array_info_t arr_info = { .el_size = 1 };
            if (get_array_info((char*)expression->first_child->next_sibling->next_sibling->token->value, func, &arr_info)) {
                size += arr_info.size * arr_info.el_size;
            }
        }
        else if (expression->token->t_type == STR_TYPE_TOKEN) {
            array_info_t arr_info = { .el_size = 1 };
            if (get_array_info((char*)expression->first_child->token->value, func, &arr_info)) {
                size += arr_info.size * arr_info.el_size;
            }
        }
        else if (
            expression->token->t_type == SWITCH_TOKEN ||
            expression->token->t_type == WHILE_TOKEN || 
            expression->token->t_type == IF_TOKEN
        ) size += _get_variables_size(expression->first_child->next_sibling->first_child, func);
        else if (expression->token->t_type == CASE_TOKEN) size += _get_variables_size(expression->first_child->first_child, func);
        else size += expression->variable_size;
    }
    
    return size;
}

static int _generate_function(tree_t* node, FILE* output, const char* func) {
    tree_t* name_node   = node->first_child;
    tree_t* params_node = name_node->next_sibling;
    tree_t* body_node   = params_node->next_sibling;
    tree_t* return_node = body_node->next_sibling;

    fprintf(output, "\n ; --------------- Function %s --------------- \n", name_node->token->value);
    iprintf(output, "jmp __end_%s__\n", name_node->token->value);
    iprintf(output, "__%s__:\n", name_node->token->value);

    _current_depth += 1;
    iprintf(output, "push ebp\n");
    iprintf(output, "mov ebp, esp\n");

    /*
    Reserving stack memory for local variables. (Creating stack frame).
    We should go into function body and find all local variables.
    Also we remember input variables.
    */
    int local_vars_size = _get_variables_size(params_node->first_child, (char*)name_node->token->value) + _get_variables_size(body_node->first_child, (char*)name_node->token->value);
    iprintf(output, "sub esp, %d\n", ALIGN_TO(local_vars_size, 4));

    /*
    Loading input args to stack.
    */
    int param_offset = 8;
    for (tree_t* param = params_node->first_child; param; param = param->next_sibling) {
        int param_size = ALIGN_TO(param->variable_size, 4);
        char* param_name = (char*)param->first_child->token->value;
        switch (get_variable_type(param->first_child->token)) {
            case 1:
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
            default: break;
        }

        param_offset += param_size;
    }

    /*
    Function body without return statement.
    All expressions will use one shared register EAX.
    */
    for (tree_t* part = body_node->first_child; part; part = part->next_sibling) {
        _generate_expression(part, output, (char*)name_node->token->value);
    }

    _current_depth -= 1;
    fprintf(output, " ; --------------- \n");
    iprintf(output, "__end_%s__:\n", name_node->token->value);
    return 1;
}

static int _generate_while(tree_t* node, FILE* output, const char* func) {
    int current_label = _label_counter++;
    tree_t* condition = node->first_child;
    tree_t* body = condition->next_sibling->first_child;

    fprintf(output, "\n ; --------------- while cycle [%i] --------------- \n", current_label);
    iprintf(output, "__while_%d__:\n", current_label);
    _current_depth += 1;

    _generate_expression(condition, output, func);
    iprintf(output, "cmp eax, 0\n");
    iprintf(output, "je __end_while_%d__\n", current_label);

    while (body) {
        _generate_expression(body, output, func);
        body = body->next_sibling;
    }

    iprintf(output, "jmp __while_%d__\n", current_label);
    
    _current_depth -= 1;
    fprintf(output, " ; --------------- \n");
    iprintf(output, "__end_while_%d__:\n", current_label);
    return 1;
}

static int _cmp(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

static int _generate_case_binary_jump(FILE* output, int* values, int left, int right, int label_id, int default_scope) {
    if (left > right) {
        if (default_scope) iprintf(output, "jmp __default_%d__\n", label_id);
        else iprintf(output, "jmp __end_switch_%d__\n", label_id);
        return 0;
    }

    int mid = (left + right) / 2;
    int val = values[mid];

    iprintf(output, "cmp eax, %d\n", val);
    iprintf(output, "jl __case_l_%d_%d__\n", val, label_id);
    iprintf(output, "jg __case_r_%d_%d__\n", val, label_id);
    iprintf(output, "jmp __case_%d_%d__\n", val, label_id);

    iprintf(output, "__case_l_%d_%d__:\n", val, label_id);
    _generate_case_binary_jump(output, values, left, mid - 1, label_id, default_scope);

    iprintf(output, "__case_r_%d_%d__:\n", val, label_id);
    _generate_case_binary_jump(output, values, mid + 1, right, label_id, default_scope);
    return 1;
}

static int _generate_switch(tree_t* node, FILE* output, const char* func) {
    int current_label = _label_counter++;
    tree_t* stmt = node->first_child;
    tree_t* cases = stmt->next_sibling;

    int cases_count = 0;
    int values[128] = { -1 };

    fprintf(output, "\n ; --------------- switch [%i] --------------- \n", current_label);
    _current_depth += 1;

    int have_default = 0;
    iprintf(output, "jmp __end_cases_%d__\n", current_label);
    for (tree_t* curr_case = cases->first_child; curr_case; curr_case = curr_case->next_sibling) {
        if (curr_case->token->t_type == DEFAULT_TOKEN) {
            have_default = 1;
            iprintf(output, "__default_%d__:\n", current_label);
        } 
        else {
            int case_value = str_atoi((char*)curr_case->token->value);
            iprintf(output, "__case_%d_%d__:\n", case_value, current_label);
            values[cases_count++] = case_value;   
        }

        _current_depth += 1;
        for (tree_t* part = curr_case->first_child->first_child; part; part = part->next_sibling) {
            _generate_expression(part, output, func);
        }

        _current_depth -= 1;
        iprintf(output, "jmp __end_switch_%d__\n", current_label);
    }

    qsort(values, cases_count, sizeof(int), _cmp);
    iprintf(output, "__end_cases_%d__:\n", current_label);
    _generate_expression(stmt, output, func);
    _generate_case_binary_jump(output, values, 0, cases_count - 1, current_label, have_default);

    _current_depth -= 1;
    iprintf(output, "__end_switch_%d__:\n", current_label);
    fprintf(output, " ; --------------- \n");
    return 1;
}

static int _generate_if(tree_t* node, FILE* output, const char* func) {
    int current_label = _label_counter++;
    tree_t* condition = node->first_child;
    tree_t* body = condition->next_sibling;
    tree_t* else_body = body->next_sibling;

    fprintf(output, "\n ; --------------- if statement [%i] --------------- \n", current_label);
    _generate_expression(condition, output, func);
    iprintf(output, "cmp eax, 0\n");
    if (else_body) iprintf(output, "je __else_%d__\n", current_label);
    else iprintf(output, "je __end_if_%d__\n", current_label);
    _current_depth += 1;

    tree_t* body_exp = body->first_child;
    while (body_exp) {
        _generate_expression(body_exp, output, func);
        body_exp = body_exp->next_sibling;
    }

    iprintf(output, "jmp __end_if_%d__\n", current_label);

    if (else_body) {
        iprintf(output, "__else_%d__:\n", current_label);
        tree_t* else_body_exp = else_body->first_child;
        while (else_body_exp) {
            _generate_expression(else_body_exp, output, func);
            else_body_exp = else_body_exp->next_sibling;
        }
    }

    fprintf(output, " ; --------------- \n");
    iprintf(output, "__end_if_%d__:\n", current_label);
    
    _current_depth -= 1;
    return 1;
}

static int _generate_syscall(tree_t* node, FILE* output, const char* func) {
    char* registers_32[] =  { "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp"  };
    char* registers_16[] =  { "ax", "bx", "cx", "dx", "si", "di", "bp"         };
    char* registers_8[] =   { "al", "bl", "cl", "dl", "sl", "dil", "bpl"       };

    fprintf(output, "\n ; --------------- system call --------------- \n");

    int argument_index = 0;
    tree_t* args = node->first_child;
    while (args) {
        switch (get_variable_type(args->token)) {
            case 1: 
            case 32:
                int is_ptr = get_array_info((char*)node->first_child->token->value, func, NULL) && !(node->first_child->token->ro || node->first_child->token->glob);
                iprintf(output, "%s %s, %s\n", !is_ptr ? "mov" : "lea", registers_32[argument_index++], GET_ASMVAR(args)); 
                break;
            case 16: iprintf(output, "mov %s, %s\n", registers_16[argument_index++], GET_ASMVAR(args)); break;
            case 8:  iprintf(output, "mov %s, %s\n", registers_8[argument_index++], GET_ASMVAR(args)); break;
            default: break;
        }
        
        args = args->next_sibling;
    }

    iprintf(output, "int 0x80\n");
    fprintf(output, " ; --------------- \n");
    return 1;
}

static int _generate_assignment(tree_t* node, FILE* output, const char* func) {
    tree_t* left  = node->first_child;
    tree_t* right = left->next_sibling;
    
    fprintf(output, "\n; --------------- Assignment: %s = %s --------------- \n", left->token->value, right->token->value);

    /*
    We store right result to EAX, and move it to stack with offset of left.
    Pointer assignment.
    */
    if ((get_variable_size(left->token) == 32) && left->first_child) {
        /*
        If left is array or string (array too) with elem size info.
        */
        array_info_t arr_info = { .el_size = 1 };
        int is_ptr = get_array_info((char*)node->first_child->token->value, func, NULL) && !(node->first_child->token->ro || node->first_child->token->glob);

        /*
        Generate offset movement in this array-like data type.
        Then multiply it by arr el_size.
        */
        _generate_expression(left->first_child, output, func);
        if (arr_info.el_size > 1) iprintf(output, "imul eax, %d\n", arr_info.el_size);
        iprintf(output, "%s ebx, %s\n", !is_ptr ? "mov" : "lea", GET_ASMVAR(left));

        iprintf(output, "add eax, ebx\n");
        iprintf(output, "push eax\n");
        
        _generate_expression(right, output, func);
        iprintf(output, "pop ebx\n");
        if (arr_info.el_size == 1)      iprintf(output, "mov byte [ebx], al\n");
        else if (arr_info.el_size == 2) iprintf(output, "mov word [ebx], ax\n");
        else if (arr_info.el_size == 4) iprintf(output, "mov [ebx], eax\n");
    } 
    else {
        /*
        Move to eax result of right operation, and store it in stack with offset in the left.
        */
        _generate_expression(right, output, func);
        iprintf(output, "mov %s, eax\n", GET_ASMVAR(left));
    }

    fprintf(output, " ; --------------- \n");

    return 1;
}

static int _generate_text_section(tree_t* node, FILE* output) {
    if (!node) return 0;
    for (tree_t* child = node->first_child; child; child = child->next_sibling) _generate_expression(child, output, NULL);
    return 1;
}

int generate_asm(tree_t* root, FILE* output) {
    tree_t* program_body = root->first_child;
    tree_t* prestart     = program_body;
    tree_t* main_body    = prestart->next_sibling;

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
                        fprintf(output, "    extern __%s__\n", func->token->value);
                    break;

                case FUNC_TOKEN: 
                    fprintf(output, "    global __%s__\n", child->first_child->token->value);
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
        iprintf(output, "sub esp, %d\n", ALIGN_TO(_get_variables_size(main_body->first_child, NULL), 4));
        _generate_text_section(main_body, output);
    }

    return 1;
}
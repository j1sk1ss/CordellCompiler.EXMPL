#include "../include/generator.h"


#pragma region [Misc]

    static int _label_counter = 0;
    static int _current_depth = 1;
    static int _generate_declaration(tree_t*, FILE*, const char*);
    static int _generate_function(tree_t*, FILE*, const char*);
    static int _generate_switch(tree_t*, FILE*, const char*);
    static int _generate_if(tree_t*, FILE*, const char*);
    static int _generate_while(tree_t*, FILE*, const char*);
    static int _generate_syscall(tree_t*, FILE*, const char*);
    static int _generate_assignment(tree_t*, FILE*, const char*);

#pragma endregion


static int _generate_raw(token_type_t t_type, tree_t* entry, FILE* output) {
    switch (t_type) {
        case LONG_TYPE_TOKEN:  if (!entry->first_child) iprintf(output, "__%s__: resq 1\n", (char*)entry->first_child->token->value); break;
        case INT_TYPE_TOKEN:   if (!entry->first_child) iprintf(output, "__%s__: resd 1\n", (char*)entry->first_child->token->value); break;
        case SHORT_TYPE_TOKEN: if (!entry->first_child) iprintf(output, "__%s__: resw 1\n", (char*)entry->first_child->token->value); break;
        case CHAR_TYPE_TOKEN:  if (!entry->first_child) iprintf(output, "__%s__: resb 1\n", (char*)entry->first_child->token->value); break;
        case ARRAY_TYPE_TOKEN:
            tree_t* size   = entry->first_child;
            tree_t* t_type = size->next_sibling;
            tree_t* name   = t_type->next_sibling;
        
            int el_size = 1;
            const char* directive = NULL;
            if (t_type->token->t_type == CHAR_TYPE_TOKEN) directive = "resb";
            else if (t_type->token->t_type == SHORT_TYPE_TOKEN) {
                directive = "resw";
                el_size = 2;
            }
            else if (t_type->token->t_type == INT_TYPE_TOKEN) {
                directive = "resd";
                el_size = 4;
            }
            else if (t_type->token->t_type == LONG_TYPE_TOKEN) {
                directive = "resq";
                el_size = 8;
            }
        
            if (!name->next_sibling) {
                iprintf(output, "__%s__: %s %s\n", name->token->value, directive, size->token->value);
            }

            break;
        default: break;
    }

    return 1;
}

static int _generate_init(token_type_t t_type, tree_t* entry, FILE* output) {
    switch (t_type) {
        case STR_TYPE_TOKEN:   iprintf(output, "__%s__ db '%s', 0\n", (char*)entry->first_child->token->value, (char*)entry->first_child->next_sibling->token->value); break;
        case LONG_TYPE_TOKEN:  iprintf(output, "__%s__ dq %s\n", (char*)entry->first_child->token->value, (char*)entry->first_child->next_sibling->token->value); break;
        case INT_TYPE_TOKEN:   iprintf(output, "__%s__ dd %s\n", (char*)entry->first_child->token->value, (char*)entry->first_child->next_sibling->token->value); break;
        case SHORT_TYPE_TOKEN: iprintf(output, "__%s__ dw %s\n", (char*)entry->first_child->token->value, (char*)entry->first_child->next_sibling->token->value); break;
        case CHAR_TYPE_TOKEN:  iprintf(output, "__%s__ db %s\n", (char*)entry->first_child->token->value, (char*)entry->first_child->next_sibling->token->value); break;
        case ARRAY_TYPE_TOKEN:
            tree_t* size   = entry->first_child;
            tree_t* t_type = size->next_sibling;
            tree_t* name   = t_type->next_sibling;
        
            int el_size = 1;
            const char* directive = NULL;
            if (t_type->token->t_type == CHAR_TYPE_TOKEN) directive = "db";
            else if (t_type->token->t_type == SHORT_TYPE_TOKEN) {
                directive = "dw";
                el_size = 2;
            }
            else if (t_type->token->t_type == INT_TYPE_TOKEN) {
                directive = "dd";
                el_size = 4;
            }
            else if (t_type->token->t_type == LONG_TYPE_TOKEN) {
                directive = "dq";
                el_size = 8;
            }
        
            if (name->next_sibling) {
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

/*
section 1 is a data section
section 2 is a rodata section
*/
static int _generate_data_section(tree_t* node, FILE* output, int section, int (*data_gen)(token_type_t, tree_t*, FILE*)) {
    if (!node) return 0;
    for (tree_t* child = node->first_child; child; child = child->next_sibling) {
        if (!child->token) {
            _generate_data_section(child, output, section, data_gen);
            continue;
        }

        if ((section == 1 && child->token->glob) || (section == 2 && child->token->ro)) {
            data_gen(child->token->t_type, child, output);
        }
        else if (!child->token->ro && !child->token->glob) {
            switch (child->token->t_type) {
                case IF_TOKEN:
                case SYSCALL_TOKEN:
                case WHILE_TOKEN:  _generate_data_section(child, output, section, data_gen); break;
                case SWITCH_TOKEN: _generate_data_section(child->first_child->next_sibling, output, section, data_gen); break;
                case DEFAULT_TOKEN:
                case CASE_TOKEN:   _generate_data_section(child->first_child->first_child, output, section, data_gen); break;
                case FUNC_TOKEN:   _generate_data_section(child->first_child->next_sibling->next_sibling, output, section, data_gen); break;
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
    else if (node->token->t_type == UNKNOWN_NUMERIC_TOKEN)  iprintf(output, "mov rax, %s\n", node->token->value);
    else if (node->token->t_type == CHAR_VALUE_TOKEN)       iprintf(output, "mov rax, %i\n", *node->token->value);
    else if (node->token->t_type == STRUCT_VARIABLE_TOKEN) {
        tree_t* struct_node = node->first_child;
        if (node->token->ptr) {
            struct_info_t* used_struct = get_associated_struct((char*)struct_node->token->value);
            _generate_expression(struct_node->next_sibling, output, func);
            iprintf(output, "mov [rbp - %d], rax\n", used_struct->offset);
        }
    }
    else if (node->token->ptr && is_variable_decl(node->token->t_type)) _generate_declaration(node, output, func);
    else if (node->token->ptr && !is_variable_decl(node->token->t_type) && get_variable_type(node->token)) {
        if (!node->first_child) iprintf(output, "mov rax, %s\n", GET_ASMVAR(node));
        else {
            variable_info_t info;
            if (get_var_info((char*)node->token->value, func, &info)) {
                _generate_expression(node->first_child, output, func);

                if (node->token->t_type == SHORT_VARIABLE_TOKEN) iprintf(output, "imul rax, 2\n");
                else if (node->token->t_type == INT_VARIABLE_TOKEN) iprintf(output, "imul rax, 4\n");
                else if (node->token->t_type == LONG_VARIABLE_TOKEN) iprintf(output, "imul rax, 8\n");
                iprintf(output, "add rax, %s\n", GET_ASMVAR(node));

                if (node->token->t_type == CHAR_VARIABLE_TOKEN) iprintf(output, "movzx rax, byte [rax]\n");
                else if (node->token->t_type == SHORT_VARIABLE_TOKEN) iprintf(output, "movzx rax, word [rax]\n");
                else iprintf(output, "mov rax, [rax]\n");
            }
        }
    }
    else if (node->token->t_type == DOT_TOKEN) {
        tree_t* struct_node = node->first_child;
        tree_t* field_name  = struct_node->next_sibling;
        struct_info_t* used_struct = get_associated_struct((char*)struct_node->token->value);

        int field_offset = 0;
        for (struct_field_info_t* field = used_struct->field; field; field = field->next) {
            if (!str_strcmp(field->name, (char*)field_name->token->value)) {
                field_offset = field->offset;
                break;
            }
        }

        if (!struct_node->token->ptr) {
            iprintf(output, "mov rax, [rbp - %d]\n", used_struct->offset + field_offset);
        }
        else {
            iprintf(output, "lea rax, [rbp - %d]\n", used_struct->offset);
            iprintf(output, "add rax, %i\n", field_offset);
            iprintf(output, "mov rax, [rax]\n");

        }
    }
    else if (node->token->t_type == LONG_TYPE_TOKEN) _generate_declaration(node, output, func);
    else if (node->token->t_type == LONG_VARIABLE_TOKEN) {
        iprintf(output, "mov rax, %s ; int64 %s\n", GET_ASMVAR(node), node->token->value);
        _generate_expression(node->first_child, output, func);
    }
    else if (node->token->t_type == INT_TYPE_TOKEN) _generate_declaration(node, output, func);
    else if (node->token->t_type == INT_VARIABLE_TOKEN) {
        iprintf(output, "mov rax, %s ; int32 %s\n", GET_ASMVAR(node), node->token->value);
        _generate_expression(node->first_child, output, func);
    }
    else if (node->token->t_type == SHORT_TYPE_TOKEN) _generate_declaration(node, output, func);
    else if (node->token->t_type == SHORT_VARIABLE_TOKEN) {
        iprintf(output, "mov ax, %s ; int16 %s\n", GET_ASMVAR(node), node->token->value);
        _generate_expression(node->first_child, output, func);
    }
    else if (node->token->t_type == CHAR_TYPE_TOKEN) _generate_declaration(node, output, func);
    else if (node->token->t_type == CHAR_VARIABLE_TOKEN) {
        iprintf(output, "mov al, %s ; int8 %s\n", GET_ASMVAR(node), node->token->value);
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
                    else if (arr_info.el_size == 4) {
                        reg = "eax";
                        mov_op = " dword ";
                    }
                    else {
                        reg = "rax";
                        mov_op = " ";
                    }

                    int base_off = name->variable_offset;
                    for (tree_t* v = vals; v && v->token->t_type != DELIMITER_TOKEN; v = v->next_sibling) {
                        if (v->token->t_type == UNKNOWN_NUMERIC_TOKEN) iprintf(output, "mov%s[rbp - %d], %d\n", mov_op, base_off, str_atoi((char*)v->token->value));
                        else if (v->token->t_type == CHAR_VALUE_TOKEN) iprintf(output, "mov%s[rbp - %d], %d\n", mov_op, base_off, *v->token->value);
                        else {
                            int is_ptr = (get_array_info((char*)v->token->value, func, NULL) && !(v->token->ro || v->token->glob)); // || v->token->ptr;
                            iprintf(output, "%s %s, %s ; int64 %s \n", !is_ptr ? "mov" : "lea", reg, GET_ASMVAR(v), v->token->value); 
                            iprintf(output, "mov%s[rbp - %d], %s\n", mov_op, base_off, reg);
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
                iprintf(output, "mov byte [rbp - %d], %i\n", base_off--, *val_head);
                val_head++;
            }

            fprintf(output, " ; --------------- \n");
        }
    }
    else if (node->token->t_type == ARR_VARIABLE_TOKEN || node->token->t_type == STR_VARIABLE_TOKEN) {
        if (!node->first_child) iprintf(output, "mov rax, __%s__\n", node->token->value);
        else {
            array_info_t arr_info = { .el_size = 1 };
            get_array_info((char*)node->token->value, func, &arr_info);
            _generate_expression(node->first_child, output, func);
            if (arr_info.el_size > 1) iprintf(output, "imul rax, %d\n", arr_info.el_size);
            iprintf(output, "%s rbx, %s\n", (node->token->ro || node->token->glob) ? "mov" : "lea",  GET_ASMVAR(node));
            iprintf(output, "add rax, rbx\n");

            if (arr_info.el_size == 1) iprintf(output, "movzx rax, byte [rax]\n");
            else if (arr_info.el_size == 2) iprintf(output, "movzx rax, word [rax]\n");
            else if (arr_info.el_size == 4) iprintf(output, "movzx rax, dword [rax]\n");
            else iprintf(output, "mov rax, [rax]\n");
        }
    }
    else if (node->token->t_type == BITMOVE_LEFT_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push rax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop rbx\n");
        iprintf(output, "mov rcx, rax\n");
        iprintf(output, "shl rbx, cl\n");
        iprintf(output, "mov rax, rbx\n");
    }
    else if (node->token->t_type == BITMOVE_RIGHT_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push rax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop rbx\n");
        iprintf(output, "mov rcx, rax\n");
        iprintf(output, "shr rbx, cl\n");
        iprintf(output, "mov rax, rbx\n");
    }
    else if (node->token->t_type == BITAND_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push rax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop rbx\n");
        iprintf(output, "and rax, rbx\n");
    }
    else if (node->token->t_type == BITOR_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push rax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop rbx\n");
        iprintf(output, "or rax, rbx\n");
    }
    else if (node->token->t_type == BITXOR_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push rax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop rbx\n");
        iprintf(output, "xor rax, rbx\n");
    }
    else if (node->token->t_type == AND_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push rax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "mov rbx, rax\n");
        iprintf(output, "pop rax\n");
        iprintf(output, "and rax, rbx\n");
    }
    else if (node->token->t_type == OR_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push rax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "mov rbx, rax\n");
        iprintf(output, "pop rax\n");
        iprintf(output, "or rax, rbx\n"); 
    }
    else if (node->token->t_type == PLUS_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push rax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop rbx\n");
        iprintf(output, "add rax, rbx\n");
    }
    else if (node->token->t_type == MINUS_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push rax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop rbx\n");
        iprintf(output, "sub rbx, rax\n");
        iprintf(output, "mov rax, rbx\n");
    }
    else if (node->token->t_type == MULTIPLY_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push rax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop rbx\n");
        iprintf(output, "imul rax, rbx\n");
    }
    else if (node->token->t_type == DIVIDE_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push rax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "mov rbx, rax\n");
        iprintf(output, "pop rax\n");
        iprintf(output, "cdq\n");
        iprintf(output, "idiv rbx\n");
    }
    else if (node->token->t_type == MODULO_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push rax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "mov rbx, rax\n");
        iprintf(output, "pop rax\n");
        iprintf(output, "cdq\n");
        iprintf(output, "idiv rbx\n");
        iprintf(output, "mov rax, rdx\n");
    }
    else if (node->token->t_type == LARGER_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push rax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop rbx\n");
        iprintf(output, "cmp rbx, rax\n");
        iprintf(output, "setg al\n");
        iprintf(output, "movzx rax, al\n");
    }
    else if (node->token->t_type == LOWER_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push rax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop rbx\n");
        iprintf(output, "cmp rbx, rax\n");
        iprintf(output, "setl al\n");
        iprintf(output, "movzx rax, al\n");
    }
    else if (node->token->t_type == COMPARE_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push rax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop rbx\n");
        iprintf(output, "cmp rbx, rax\n");
        iprintf(output, "sete al\n");
        iprintf(output, "movzx rax, al\n");
    }
    else if (node->token->t_type == NCOMPARE_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push rax\n");
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop rbx\n");
        iprintf(output, "cmp rbx, rax\n");
        iprintf(output, "setne al\n");
        iprintf(output, "movzx rax, al\n");
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
        const char* args_regs64[] = { "rdi", "rsi", "rdx", "rcx", "r8",  "r9"  };
        const char* args_regs32[] = { "edi", "esi", "edx", "ecx", "r8d", "r9d" };
        const char* args_regs16[] = { "di",  "si",  "dx",  "cx",  "r8w", "r9w" };
        const char* args_regs8[]  = { "sil", "sil", "dl",  "cl",  "r8b", "r9b" };

        int pushed_args = 0;
        for (pushed_args = 0; pushed_args < MIN(arg_count, 6); pushed_args++) {
            tree_t* arg = args[pushed_args];
            _generate_expression(arg, output, func);
            switch (get_variable_type(arg->token)) {
                case 1: 
                case 64:
                    int is_ptr = (get_array_info((char*)arg->token->value, func, NULL) && !(arg->token->ro || arg->token->glob));
                    iprintf(output, "%s %s, rax ; int64 %s \n", !is_ptr ? "mov" : "lea", args_regs64[pushed_args], arg->token->value); 
                    break;
                case 32: iprintf(output, "mov dword %s, eax ; int32 %s \n", args_regs32[pushed_args], arg->token->value); break;
                case 16: iprintf(output, "mov word %s, ax ; int16 %s \n", args_regs16[pushed_args], arg->token->value); break;
                case 8:  iprintf(output, "mov byte %s, al ; int8 %s \n", args_regs8[pushed_args], arg->token->value); break;
                default: break;
            }
        }
    
        int stack_args = arg_count - pushed_args;
        while (pushed_args < arg_count) {
            tree_t* arg = args[pushed_args++];

            _generate_expression(arg, output, func);
            iprintf(output, "mav rbx, rax\n");

            switch (get_variable_type(arg->token)) {
                case 1: 
                case 64:
                    int is_ptr = (get_array_info((char*)arg->token->value, func, NULL) && !(arg->token->ro || arg->token->glob));
                    iprintf(output, "%s rax, rbx ; uint64 %s \n", !is_ptr ? "mov" : "lea", arg->token->value);
                    break;
                case 32: iprintf(output, "mov rax, rbx ; int16 %s \n", arg->token->value); break;
                case 16: iprintf(output, "mov rax, eax ; int16 %s \n", arg->token->value); break;
                case 8:  iprintf(output, "mov rax, bl ; int8 %s \n", arg->token->value); break;
                default: break;
            }

            iprintf(output, "push rax\n");
        }

        iprintf(output, "call __%s__\n", func_name_node->token->value);
        if (stack_args > 0) iprintf(output, "add rsp, %d\n", stack_args * 8);
        fprintf(output, " ; --------------- \n");
    }
    else if (node->token->t_type == EXIT_TOKEN) {
        /*
        Restore stack frame before exiting program.
        */
        fprintf(output, "\n ; --------------- Exit --------------- \n");
        _generate_expression(node->first_child, output, func);
        iprintf(output, "mov rdi, rax\n");
        iprintf(output, "mov rax, 60\n");
        iprintf(output, "syscall\n");
    }
    else if (node->token->t_type == RETURN_TOKEN) {
        fprintf(output, "\n ; --------------- Return --------------- \n");
        /*
        Move to rax return expresion, and restore stack.
        Now, result of function stored at EAX register.
        */
        _generate_expression(node->first_child, output, func);
        iprintf(output, "mov rsp, rbp\n");
        iprintf(output, "pop rbp\n");
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
                size +=  ALIGN_TO(arr_info.size * arr_info.el_size, 8);
            }
        }
        else if (expression->token->t_type == STR_TYPE_TOKEN) {
            array_info_t arr_info = { .el_size = 1 };
            if (get_array_info((char*)expression->first_child->token->value, func, &arr_info)) {
                size += ALIGN_TO(arr_info.size * arr_info.el_size, 8);
            }
        }
        else if (
            expression->token->t_type == SWITCH_TOKEN ||
            expression->token->t_type == WHILE_TOKEN || 
            expression->token->t_type == IF_TOKEN
        ) size += _get_variables_size(expression->first_child->next_sibling->first_child, func);
        else if (expression->token->t_type == STRUCT_VARIABLE_TOKEN) {
            struct_info_t* used_struct = get_associated_struct((char*)expression->token->value);
            if (used_struct && expression->token->ptr) {
                for (struct_field_info_t* field = used_struct->field; field; field = field->next) {
                    size += field->size;
                }
            }
            else {
                size += 8;
            }
        }
        else if (expression->token->t_type == CASE_TOKEN) size += _get_variables_size(expression->first_child->first_child, func);
        else size += expression->variable_size;
    }
    
    return size;
}

static int _generate_declaration(tree_t* node, FILE* output, const char* func) {
    int val = 0;
    int type = 0;
    char* derictive = " ";
    
    tree_t* name_node = node->first_child;
    if (name_node->token->ro || name_node->token->glob) return 0;

    if (name_node->next_sibling->token->t_type != UNKNOWN_NUMERIC_TOKEN && name_node->next_sibling->token->t_type != CHAR_VALUE_TOKEN) {
        type = 0;
        _generate_expression(name_node->next_sibling, output, func);
    }
    else {
        type = 1;
        switch (get_variable_size(name_node->token)) {
            default:
            case 64: derictive = " qword "; break;
            case 32: derictive = " dword "; break;
            case 16: derictive = " word "; break;
            case 8: derictive = " byte "; break;
        }

        if (name_node->next_sibling->token->t_type == UNKNOWN_NUMERIC_TOKEN) val = str_atoi((char*)name_node->next_sibling->token->value);
        else if (name_node->next_sibling->token->t_type == CHAR_VALUE_TOKEN) val = *name_node->next_sibling->token->value;
    }
    
    char source[36] = { 0 };
    if (type) sprintf(source, "%d", val);
    else sprintf(source, "%s", "rax");

    iprintf(output, "mov%s%s, %s ; decl %s = %s\n", derictive, GET_ASMVAR(name_node), source, (char*)name_node->token->value, source);
    return 1;
}

static int _generate_function(tree_t* node, FILE* output, const char* func) {
    tree_t* name_node   = node->first_child;
    tree_t* params_node = name_node->next_sibling;
    tree_t* body_node   = params_node->next_sibling;

    fprintf(output, "\n ; --------------- Function %s --------------- \n", name_node->token->value);
    iprintf(output, "jmp __end_%s__\n", name_node->token->value);
    iprintf(output, "__%s__:\n", name_node->token->value);

    _current_depth += 1;
    iprintf(output, "push rbp\n");
    iprintf(output, "mov rbp, rsp\n");

    /*
    Reserving stack memory for local variables. (Creating stack frame).
    We should go into function body and find all local variables.
    Also we remember input variables.
    */
    int local_vars_size = _get_variables_size(params_node->first_child, (char*)name_node->token->value) + _get_variables_size(body_node->first_child, (char*)name_node->token->value);
    iprintf(output, "sub rsp, %d\n", ALIGN_TO(local_vars_size, 8));

    /*
    Loading input args to stack.
    */
    int pop_params = 0;
    int param_offset = 16;
    int stack_offset = 8;

    const char* args_regs64[] = { "rdi", "rsi", "rdx", "rcx", "r8",  "r9"  };
    const char* args_regs32[] = { "edi", "esi", "edx", "ecx", "r8d", "r9d" };
    const char* args_regs16[] = { "di",  "si",  "dx",  "cx",  "r8w", "r9w" };
    const char* args_regs8[]  = { "sil", "sil", "dl",  "cl",  "r8b", "r9b" };

    for (tree_t* param = params_node->first_child; param; param = param->next_sibling) {
        int param_size = ALIGN_TO(param->variable_size, 8);
        char* param_name = (char*)param->first_child->token->value;

        if (pop_params >= 6) {
            switch (get_variable_type(param->first_child->token)) {
                case 1:
                case 64:
                    iprintf(output, "mov rax, [rbp + %d] ; int64 %s \n", stack_offset, param_name);
                    iprintf(output, "mov [rbp - %d], rax\n", param_offset - param_size);
                    break;

                case 32:
                    iprintf(output, "mov eax, [rbp + %d] ; int32 %s \n", stack_offset, param_name);
                    iprintf(output, "mov dword [rbp - %d], eax\n", param_offset - param_size);
                    break;

                case 16:
                    iprintf(output, "mov ax, [rbp + %d] ; int16 %s \n", stack_offset, param_name);
                    iprintf(output, "mov word [rbp - %d], ax\n", param_offset - param_size);
                    break;

                case 8:
                    iprintf(output, "mov al, [rbp + %d] ; int8 %s \n", stack_offset, param_name);
                    iprintf(output, "mov byte [rbp - %d], al\n", param_offset - param_size);
                    break;
                default: break;
            }

            stack_offset += param_size;
        }
        else {
            switch (get_variable_type(param->first_child->token)) {
                case 1:
                case 64: iprintf(output, "mov [rbp - %d], %s\n", param_offset - param_size, args_regs64[pop_params]); break;
                case 32: iprintf(output, "mov dword [rbp - %d], %s\n", param_offset - param_size, args_regs32[pop_params]); break;
                case 16: iprintf(output, "mov word [rbp - %d], %s\n", param_offset - param_size, args_regs16[pop_params]); break;
                case 8: iprintf(output, "mov byte [rbp - %d], %s\n", param_offset - param_size, args_regs8[pop_params]); break;
                default: break;
            }
        }

        pop_params++;
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
    iprintf(output, "cmp rax, 0\n");
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

    iprintf(output, "cmp rax, %d\n", val);
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

    sort_qsort(values, cases_count, sizeof(int), _cmp);
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
    iprintf(output, "cmp rax, 0\n");
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

/* https://blog.rchapman.org/posts/Linux_System_Call_Table_for_x86_64/ */
/* https://math.hws.edu/eck/cs220/f22/registers.html */
static int _generate_syscall(tree_t* node, FILE* output, const char* func) {
    char* registers_64[] =  { "rax", "rdi", "rsi", "rdx", "r10",  "r8",  "r9"   };
    char* registers_32[] =  { "eax", "edi", "esi", "edx", "r10d", "r8d", "r9d"  };
    char* registers_16[] =  { "ax",  "di",  "si",  "dx",  "r10w", "r8w", "r9w"  };
    char* registers_8[]  =  { "al",  "sil", "sil", "dl",  "r10b", "r8b", "r9b"  };

    fprintf(output, "\n ; --------------- system call --------------- \n");

    int arg_index = 0;
    tree_t* args = node->first_child;
    while (args) {
        _generate_expression(args, output, func);
        switch (get_variable_type(args->token)) {
            case 1: 
            case 64:
                int is_ptr = (get_array_info((char*)node->first_child->token->value, func, NULL) && !(node->first_child->token->ro || node->first_child->token->glob));
                iprintf(output, "%s %s, rax\n", !is_ptr ? "mov" : "lea", registers_64[arg_index++]); 
                break;
            case 32: iprintf(output, "mov dword %s, eax\n", registers_32[arg_index++]); break;
            case 16: iprintf(output, "mov word %s, ax\n", registers_16[arg_index++]); break;
            case 8:  iprintf(output, "mov byte %s, al\n", registers_8[arg_index++]); break;
            default: break;
        }
        
        args = args->next_sibling;
    }

    iprintf(output, "syscall\n");
    fprintf(output, " ; --------------- \n");

    return 1;
}

static int _generate_assignment(tree_t* node, FILE* output, const char* func) {
    tree_t* left  = node->first_child;
    tree_t* right = left->next_sibling;

    fprintf(output, "\n; --------------- Assignment: %s = %s --------------- \n", left->token->value, right->token->value);

    /*
    We store right result to RAX, and move it to stack with offset of left.
    Pointer assignment. Also we check if this variable is ptr, array or etc.
    Markers are 64 bits size and first child.
    */
    if (left->token->t_type == DOT_TOKEN) {
        tree_t* struct_node = left->first_child;
        tree_t* field_name  = struct_node->next_sibling;
        struct_info_t* used_struct = get_associated_struct((char*)struct_node->token->value);
        if (!used_struct) return 0;

        int field_offset = 0;
        for (struct_field_info_t* field = used_struct->field; field; field = field->next) {
            if (!str_strcmp(field->name, (char*)field_name->token->value)) {
                field_offset = field->offset;
                break;
            }
        }

        _generate_expression(right, output, func);
        if (!struct_node->token->ptr) {
            iprintf(output, "mov [rbp - %d], rax\n", used_struct->offset + field_offset);
        }
        else {
            iprintf(output, "lea rbx, [rbp - %d]\n", used_struct->offset);
            iprintf(output, "add rbx, %i\n", field_offset);
            iprintf(output, "mov [rbx], rax\n");
        }
    }
    else if (get_variable_size(left->token) == 64 && left->first_child) {
        /*
        If left is array or string (array too) with elem size info.
        */
        array_info_t arr_info = { .el_size = 1 };
        int is_ptr = (get_array_info((char*)node->first_child->token->value, func, NULL) && 
                     !(node->first_child->token->ro || node->first_child->token->glob));

        /*
        Generate offset movement in this array-like data type.
        Then multiply it by arr el_size.
        */
        _generate_expression(left->first_child, output, func);
        if (arr_info.el_size > 1) iprintf(output, "imul rax, %d\n", arr_info.el_size);
        iprintf(output, "%s rbx, %s\n", !is_ptr ? "mov" : "lea", GET_ASMVAR(left));

        iprintf(output, "add rax, rbx\n");
        iprintf(output, "push rax\n");
        
        _generate_expression(right, output, func);
        iprintf(output, "pop rbx\n");
        if (arr_info.el_size == 1)      iprintf(output, "mov byte [rbx], al\n");
        else if (arr_info.el_size == 2) iprintf(output, "mov word [rbx], ax\n");
        else if (arr_info.el_size == 4) iprintf(output, "mov dword [rbx], eax\n");
        else if (arr_info.el_size == 8) iprintf(output, "mov [rbx], rax\n");
    }
    else {
        /*
        Move to rax result of right operation, and store it in stack with offset in the left.
        */
        _generate_expression(right, output, func);
        iprintf(output, "mov %s, rax\n", GET_ASMVAR(left));
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
    _generate_data_section(prestart, output, 1, _generate_init);
    _generate_data_section(main_body, output, 1, _generate_init);

    /*
    Generate rodata section. Here we store strings, that
    not assign to any variable.
    */
    fprintf(output, "\nsection .rodata\n");
    _generate_data_section(prestart, output, 2, _generate_init);
    _generate_data_section(main_body, output, 2, _generate_init);

    /*
    Generate bss section for not pre-init arrays.
    */
    fprintf(output, "\nsection .bss\n");
    _generate_data_section(prestart, output, 1, _generate_raw);
    _generate_data_section(main_body, output, 1, _generate_raw);

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

        iprintf(output, "push rbp\n");
        iprintf(output, "mov rbp, rsp\n");
        iprintf(output, "sub rsp, %d\n", ALIGN_TO(_get_variables_size(main_body->first_child, NULL), 8));
        _generate_text_section(main_body, output);
    }

    return 1;
}
#include "../include/generator.h"


static int _label_counter = 0;
static int _current_depth = 1;
static int _generate_declaration(const tree_t*, FILE*, const char*);
static int _generate_assignment( const tree_t*, FILE*, const char*);
static int _generate_function(   const tree_t*, FILE*, const char*);
static int _generate_syscall(    const tree_t*, FILE*, const char*);
static int _generate_switch(     const tree_t*, FILE*, const char*);
static int _generate_while(      const tree_t*, FILE*, const char*);
static int _generate_if(         const tree_t*, FILE*, const char*);

static int _generate_raw(token_type_t t_type, const tree_t* entry, FILE* output) {
    if (!entry->first_child) return 0;
    switch (t_type) {
        case LONG_TYPE_TOKEN:  iprintf(output, "__%s__: resq 1\n", (char*)entry->first_child->token->value); break;
        case INT_TYPE_TOKEN:   iprintf(output, "__%s__: resd 1\n", (char*)entry->first_child->token->value); break;
        case SHORT_TYPE_TOKEN: iprintf(output, "__%s__: resw 1\n", (char*)entry->first_child->token->value); break;
        case CHAR_TYPE_TOKEN:  iprintf(output, "__%s__: resb 1\n", (char*)entry->first_child->token->value); break;
        case ARRAY_TYPE_TOKEN: {
            const tree_t* size   = entry->first_child;
            const tree_t* t_type = size->next_sibling;
            const tree_t* name   = t_type->next_sibling;

            char* directive = "resb";
            if (t_type->token->t_type == SHORT_TYPE_TOKEN)      directive = "resw";
            else if (t_type->token->t_type == INT_TYPE_TOKEN)   directive = "resd";
            else if (t_type->token->t_type == LONG_TYPE_TOKEN)  directive = "resq";

            if (!name->next_sibling) {
                iprintf(output, "__%s__: %s %s\n", name->token->value, directive, size->token->value);
            }
        }
        break;
        default: break;
    }

    return 1;
}

static int _generate_init(token_type_t t_type, const tree_t* entry, FILE* output) {
    switch (t_type) {
        case STR_TYPE_TOKEN:   iprintf(output, "__%s__ db '%s', 0\n", (char*)entry->first_child->token->value, (char*)entry->first_child->next_sibling->token->value); break;
        case LONG_TYPE_TOKEN:  iprintf(output, "__%s__ dq %s\n", (char*)entry->first_child->token->value, (char*)entry->first_child->next_sibling->token->value); break;
        case INT_TYPE_TOKEN:   iprintf(output, "__%s__ dd %s\n", (char*)entry->first_child->token->value, (char*)entry->first_child->next_sibling->token->value); break;
        case SHORT_TYPE_TOKEN: iprintf(output, "__%s__ dw %s\n", (char*)entry->first_child->token->value, (char*)entry->first_child->next_sibling->token->value); break;
        case CHAR_TYPE_TOKEN:  iprintf(output, "__%s__ db %s\n", (char*)entry->first_child->token->value, (char*)entry->first_child->next_sibling->token->value); break;
        case ARRAY_TYPE_TOKEN: {
            const tree_t* size   = entry->first_child;
            const tree_t* t_type = size->next_sibling;
            const tree_t* name   = t_type->next_sibling;

            char* directive = "db";
            if (t_type->token->t_type == SHORT_TYPE_TOKEN)      directive = "dw";
            else if (t_type->token->t_type == INT_TYPE_TOKEN)   directive = "dd";
            else if (t_type->token->t_type == LONG_TYPE_TOKEN)  directive = "dq";

            if (name->next_sibling) {
                iprintf(output, "%s %s ", name->token->value, directive);
                for (const tree_t* elem = name->next_sibling; elem; elem = elem->next_sibling) {
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
        default: break;
    }

    return 1;
}

/*
section 1 is a data section
section 2 is a rodata section
*/
static int _generate_data_section(const tree_t* node, FILE* output, int section, int (*data_gen)(token_type_t, const tree_t*, FILE*)) {
    if (!node) return 0;
    for (const tree_t* child = node->first_child; child; child = child->next_sibling) {
        if (!child->token) {
            _generate_data_section(child, output, section, data_gen);
            continue;
        }

        if ((section == 1 && child->token->glob) || (section == 2 && child->token->ro)) data_gen(child->token->t_type, child, output);
        else if (!child->token->ro && !child->token->glob) {
            switch (child->token->t_type) {
                case IF_TOKEN:
                case SYSCALL_TOKEN:
                case WHILE_TOKEN:  _generate_data_section(child, output, section, data_gen);                                          break;
                case SWITCH_TOKEN: _generate_data_section(child->first_child->next_sibling, output, section, data_gen);               break;
                case DEFAULT_TOKEN:
                case CASE_TOKEN:   _generate_data_section(child->first_child->first_child, output, section, data_gen);                break;
                case FUNC_TOKEN:   _generate_data_section(child->first_child->next_sibling->next_sibling, output, section, data_gen); break;
                default: break;
            }
        }
    }

    return 1;
}

static int _generate_expression(const tree_t* node, FILE* output, const char* func) {
    if (!node) return 0;
    if (node->token->t_type == IF_TOKEN)           _generate_if(node, output, func);
    else if (node->token->t_type == SWITCH_TOKEN)  _generate_switch(node, output, func);
    else if (node->token->t_type == WHILE_TOKEN)   _generate_while(node, output, func);
    else if (node->token->t_type == FUNC_TOKEN)    _generate_function(node, output, func);
    else if (node->token->t_type == SYSCALL_TOKEN) _generate_syscall(node, output, func);
    else if (node->token->t_type == ASIGN_TOKEN)   _generate_assignment(node, output, func);
    else if (node->token->t_type == UNKNOWN_NUMERIC_TOKEN) iprintf(output, "mov %s, %s\n", GET_RAW_REG(BASE_BITNESS, RAX), node->token->value);
    else if (node->token->t_type == CHAR_VALUE_TOKEN)      iprintf(output, "mov %s, %i\n", GET_RAW_REG(8, RAX), *node->token->value);
    else if (node->token->ptr && is_variable_decl(node->token->t_type)) _generate_declaration(node, output, func);
    else if (node->token->ptr && !is_variable_decl(node->token->t_type) && get_variable_type(node->token)) {
        if (!node->first_child) iprintf(output, "mov %s, %s\n", GET_RAW_REG(BASE_BITNESS, RAX), GET_ASMVAR(node));
        else {
            variable_info_t info;
            if (get_var_info((char*)node->token->value, func, &info)) {
                _generate_expression(node->first_child, output, func);

                int ptr_type_size = get_variable_size_wt(node->token) / 8;
                if (ptr_type_size > 1) iprintf(output, "imul %s, %d\n", GET_RAW_REG(BASE_BITNESS, RAX), ptr_type_size);
                iprintf(output, "add %s, %s\n", GET_RAW_REG(BASE_BITNESS, RAX), GET_ASMVAR(node));

                regs_t reg;
                get_reg(&reg, ptr_type_size, RAX, 0);
                iprintf(
                    output, "%s %s,%s[%s]\n", ptr_type_size <= 2 ? "movzx" : "mov",
                    GET_RAW_REG(BASE_BITNESS, RAX), reg.operation, GET_RAW_REG(BASE_BITNESS, RAX)
                );
            }
        }
    }
    else if (node->token->t_type == LONG_TYPE_TOKEN)  _generate_declaration(node, output, func);
    else if (node->token->t_type == INT_TYPE_TOKEN)   _generate_declaration(node, output, func);
    else if (node->token->t_type == SHORT_TYPE_TOKEN) _generate_declaration(node, output, func);
    else if (node->token->t_type == CHAR_TYPE_TOKEN)  _generate_declaration(node, output, func);
    else if (node->token->t_type == CHAR_VARIABLE_TOKEN  ||
             node->token->t_type == SHORT_VARIABLE_TOKEN ||
             node->token->t_type == INT_VARIABLE_TOKEN   ||
             node->token->t_type == LONG_VARIABLE_TOKEN
    ) {
        iprintf(output, "mov %s, %s ; int %s\n", GET_RAW_REG(get_variable_size(node->token), RAX), GET_ASMVAR(node), node->token->value);
        _generate_expression(node->first_child, output, func);
    }
    else if (node->token->t_type == ARRAY_TYPE_TOKEN) {
        if (node->first_child && !node->token->ro && !node->token->glob) {
            const tree_t* size   = node->first_child;
            const tree_t* t_type = size->next_sibling;
            const tree_t* name   = t_type->next_sibling;

            array_info_t arr_info = { .el_size = 1 };
            if (get_array_info((char*)name->token->value, func, &arr_info)) {
                tree_t* vals = name->next_sibling;
                if (vals && vals->token->t_type != DELIMITER_TOKEN) {
                    fprintf(output, "\n ; --------------- Array setup %s --------------- \n", name->token->value);

                    regs_t reg;
                    get_reg(&reg, arr_info.el_size, RAX, 0);

                    int base_off = name->variable_offset;
                    for (tree_t* v = vals; v && v->token->t_type != DELIMITER_TOKEN; v = v->next_sibling) {
                        if (v->token->t_type == UNKNOWN_NUMERIC_TOKEN) iprintf(
                            output, "mov%s[%s - %d], %d\n", reg.operation, GET_RAW_REG(BASE_BITNESS, RBP), base_off, str_atoi((char*)v->token->value)
                        );
                        else if (v->token->t_type == CHAR_VALUE_TOKEN) iprintf(
                            output, "mov%s[%s - %d], %d\n", reg.operation, GET_RAW_REG(BASE_BITNESS, RBP), base_off, *v->token->value
                        );
                        else {
                            int is_ptr = (get_array_info((char*)v->token->value, func, NULL) && !(v->token->ro || v->token->glob));
                            iprintf(output, "%s %s, %s ; int %s \n", !is_ptr ? "mov" : "lea", reg.name, GET_ASMVAR(v), v->token->value);
                            iprintf(output, "mov%s[%s - %d], %s\n", reg.operation, GET_RAW_REG(BASE_BITNESS, RBP), base_off, reg.name);
                        }

                        base_off -= arr_info.el_size;
                    }

                    fprintf(output, " ; --------------- \n");
                }
            }
        }
    }
    else if (node->token->t_type == STR_TYPE_TOKEN) {
        const tree_t* name_node = node->first_child;
        if (name_node->next_sibling && !name_node->token->ro && !name_node->token->glob) {
            fprintf(output, "\n ; --------------- String setup %s --------------- \n", name_node->token->value);
            tree_t* val_node = name_node->next_sibling;
            char* val_head = (char*)val_node->token->value;
            int base_off = name_node->variable_offset;
            while (*val_head) {
                iprintf(output, "mov byte [%s - %d], %i\n", GET_RAW_REG(BASE_BITNESS, RBP), base_off--, *val_head);
                val_head++;
            }

            iprintf(output, "mov byte [ebp - %d], 0\n", base_off);
            fprintf(output, " ; --------------- \n");
        }
    }
    else if (node->token->t_type == ARR_VARIABLE_TOKEN || node->token->t_type == STR_VARIABLE_TOKEN) {
        if (!node->first_child) iprintf(output, "mov %s, __%s__\n", GET_RAW_REG(BASE_BITNESS, RAX), node->token->value);
        else {
            array_info_t arr_info = { .el_size = 1 };
            get_array_info((char*)node->token->value, func, &arr_info);
            _generate_expression(node->first_child, output, func);

            regs_t reg;
            get_reg(&reg, 8, RAX, !(node->token->ro || node->token->glob));

            if (arr_info.el_size > 1) iprintf(output, "imul %s, %d\n", GET_RAW_REG(BASE_BITNESS, RAX), arr_info.el_size);
            iprintf(output, "%s %s, %s\n", reg.move, GET_RAW_REG(BASE_BITNESS, RBX), GET_ASMVAR(node));
            iprintf(output, "add %s, %s\n", GET_RAW_REG(BASE_BITNESS, RAX), GET_RAW_REG(BASE_BITNESS, RBX));
            iprintf(
                output, "%s %s,%s[%s]\n", arr_info.el_size <= 2 ? "movzx" : "mov",
                GET_RAW_REG(BASE_BITNESS, RAX), GET_OPERATION_TYPE(arr_info.el_size), GET_RAW_REG(BASE_BITNESS, RAX)
            );
        }
    }
    else if (
        node->token->t_type == BITMOVE_LEFT_TOKEN ||
        node->token->t_type == BITMOVE_RIGHT_TOKEN
    ) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push %s\n",         GET_RAW_REG(BASE_BITNESS, RAX));
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop %s\n",          GET_RAW_REG(BASE_BITNESS, RBX));
        iprintf(output, "mov %s, %s\n",      GET_RAW_REG(BASE_BITNESS, RCX), GET_RAW_REG(BASE_BITNESS, RAX));
        if (node->token->t_type == BITMOVE_LEFT_TOKEN) iprintf(output, "shl %s, cl\n", GET_RAW_REG(BASE_BITNESS, RBX));
        else iprintf(output, "shr %s, cl\n", GET_RAW_REG(BASE_BITNESS, RBX));
        iprintf(output, "mov %s, %s\n",      GET_RAW_REG(BASE_BITNESS, RAX), GET_RAW_REG(BASE_BITNESS, RBX));
    }
    else if (
        node->token->t_type == BITAND_TOKEN ||
        node->token->t_type == BITOR_TOKEN ||
        node->token->t_type == BITXOR_TOKEN
    ) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push %s\n",        GET_RAW_REG(BASE_BITNESS, RAX));
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop %s\n",         GET_RAW_REG(BASE_BITNESS, RBX));
        if (node->token->t_type == BITAND_TOKEN) iprintf(output, "and %s, %s\n", GET_RAW_REG(BASE_BITNESS, RAX), GET_RAW_REG(BASE_BITNESS, RBX));
        else if (node->token->t_type == BITOR_TOKEN) iprintf(output, "or %s, %s\n", GET_RAW_REG(BASE_BITNESS, RAX), GET_RAW_REG(BASE_BITNESS, RBX));
        else iprintf(output, "xor %s, %s\n", GET_RAW_REG(BASE_BITNESS, RAX), GET_RAW_REG(BASE_BITNESS, RBX));
    }
    else if (
        node->token->t_type == AND_TOKEN ||
        node->token->t_type == OR_TOKEN
    ) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push %s\n",        GET_RAW_REG(BASE_BITNESS, RAX));
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "mov %s, %s\n",     GET_RAW_REG(BASE_BITNESS, RBX), GET_RAW_REG(BASE_BITNESS, RAX));
        iprintf(output, "pop %s\n",         GET_RAW_REG(BASE_BITNESS, RAX));
        if (node->token->t_type == AND_TOKEN) iprintf(output, "and %s, %s\n", GET_RAW_REG(BASE_BITNESS, RAX), GET_RAW_REG(BASE_BITNESS, RBX));
        else iprintf(output, "or %s, %s\n", GET_RAW_REG(BASE_BITNESS, RAX), GET_RAW_REG(BASE_BITNESS, RBX));
    }
    else if (node->token->t_type == PLUS_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push %s\n",        GET_RAW_REG(BASE_BITNESS, RAX));
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop %s\n",         GET_RAW_REG(BASE_BITNESS, RBX));
        iprintf(output, "add %s, %s\n",     GET_RAW_REG(BASE_BITNESS, RAX), GET_RAW_REG(BASE_BITNESS, RBX));
    }
    else if (node->token->t_type == MINUS_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push %s\n",        GET_RAW_REG(BASE_BITNESS, RAX));
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop %s\n",         GET_RAW_REG(BASE_BITNESS, RBX));
        iprintf(output, "sub %s, %s\n",     GET_RAW_REG(BASE_BITNESS, RBX), GET_RAW_REG(BASE_BITNESS, RAX));
        iprintf(output, "mov %s, %s\n",     GET_RAW_REG(BASE_BITNESS, RAX), GET_RAW_REG(BASE_BITNESS, RBX));
    }
    else if (node->token->t_type == MULTIPLY_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push %s\n",        GET_RAW_REG(BASE_BITNESS, RAX));
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop %s\n",         GET_RAW_REG(BASE_BITNESS, RBX));
        iprintf(output, "imul %s, %s\n",    GET_RAW_REG(BASE_BITNESS, RAX), GET_RAW_REG(BASE_BITNESS, RBX));
    }
    else if (node->token->t_type == DIVIDE_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push %s\n",        GET_RAW_REG(BASE_BITNESS, RAX));
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "mov %s, %s\n",     GET_RAW_REG(BASE_BITNESS, RBX), GET_RAW_REG(BASE_BITNESS, RAX));
        iprintf(output, "pop %s\n",         GET_RAW_REG(BASE_BITNESS, RAX));
        iprintf(output, "cdq\n");
        iprintf(output, "idiv %s\n",        GET_RAW_REG(BASE_BITNESS, RBX));
    }
    else if (node->token->t_type == MODULO_TOKEN) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push %s\n",        GET_RAW_REG(BASE_BITNESS, RAX));
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "mov %s, %s\n",     GET_RAW_REG(BASE_BITNESS, RBX), GET_RAW_REG(BASE_BITNESS, RAX));
        iprintf(output, "pop %s\n",         GET_RAW_REG(BASE_BITNESS, RAX));
        iprintf(output, "cdq\n");
        iprintf(output, "idiv %s\n",        GET_RAW_REG(BASE_BITNESS, RBX));
        iprintf(output, "mov %s, %s\n",     GET_RAW_REG(BASE_BITNESS, RAX), GET_RAW_REG(BASE_BITNESS, RDX));
    }
    else if (
        node->token->t_type == LARGER_TOKEN ||
        node->token->t_type == LOWER_TOKEN ||
        node->token->t_type == COMPARE_TOKEN ||
        node->token->t_type == NCOMPARE_TOKEN
    ) {
        _generate_expression(node->first_child, output, func);
        iprintf(output, "push %s\n",        GET_RAW_REG(BASE_BITNESS, RAX));
        _generate_expression(node->first_child->next_sibling, output, func);
        iprintf(output, "pop %s\n",         GET_RAW_REG(BASE_BITNESS, RBX));
        iprintf(output, "cmp %s, %s\n",     GET_RAW_REG(BASE_BITNESS, RBX), GET_RAW_REG(BASE_BITNESS, RAX));
        if (node->token->t_type == LARGER_TOKEN)       iprintf(output, "setg al\n");
        else if (node->token->t_type == LOWER_TOKEN)   iprintf(output, "setl al\n");
        else if (node->token->t_type == COMPARE_TOKEN) iprintf(output, "sete al\n");
        else iprintf(output, "setne al\n");
        iprintf(output, "movzx %s, al\n", GET_RAW_REG(BASE_BITNESS, RAX));
    }
    else if (node->token->t_type == CALL_TOKEN) {
        /*
        Generating function preparations.
        1) Getting function name and args.
        2) Put args to the stack.
        */
        int variables_size = 0;
        const tree_t* func_name_node = node;
        const tree_t* args_node = func_name_node->first_child;

        /*
        Saving params in stack.
        */
        int arg_count = 0;
        tree_t* args[128] = { NULL };
        for (tree_t* arg = args_node->first_child; arg; arg = arg->next_sibling) {
            args[arg_count++] = arg;
        }

        fprintf(output, "\n ; --------------- Call function %s --------------- \n", func_name_node->token->value);
        int pushed_args = 0;

#if (BASE_BITNESS == BIT64)
        static const int args_regs[] = { RDI, RSI, RDX, RCX, R8, R9 };
        for (pushed_args = 0; pushed_args < MIN(arg_count, 6); pushed_args++) {
            tree_t* arg = args[pushed_args];

            regs_t reg;
            int is_ptr = (get_array_info((char*)arg->token->value, func, NULL) && !(arg->token->ro || arg->token->glob));
            get_reg(&reg, get_variable_size(arg->token) / 8, args_regs[pushed_args], is_ptr);
            iprintf(
                output, "%s%s%s, %s ; int %s \n",
                reg.move, reg.operation, reg.name, GET_ASMVAR(arg), arg->token->value
            );
        }
#endif

        int stack_args = arg_count - pushed_args;
        while (pushed_args < arg_count) {
            regs_t reg;
            tree_t* arg = args[pushed_args++];
            int variable_type = get_variable_size(arg->token);
            int is_ptr = (get_array_info((char*)arg->token->value, func, NULL) && !(arg->token->ro || arg->token->glob));
            get_reg(&reg, variable_type / 8, RAX, is_ptr);

            iprintf(output, "%s%s%s, %s ; int %s \n", reg.move, reg.operation, reg.name, GET_ASMVAR(arg), arg->token->value);
            iprintf(output, "push %s\n", GET_RAW_REG(BASE_BITNESS, RAX));
        }

        iprintf(output, "call __%s__\n", func_name_node->token->value);
        if (stack_args > 0) iprintf(output, "add %s, %d\n", GET_RAW_REG(BASE_BITNESS, RSP), stack_args * 4);
        fprintf(output, " ; --------------- \n");
    }
    else if (node->token->t_type == EXIT_TOKEN) {
        fprintf(output, "\n ; --------------- Exit --------------- \n");
        _generate_expression(node->first_child, output, func);
        iprintf(output, "mov %s, %s\n", GET_RAW_REG(BASE_BITNESS, RDI), GET_RAW_REG(BASE_BITNESS, RAX));
        iprintf(output, "mov %s, 60\n", GET_RAW_REG(BASE_BITNESS, RAX));
        iprintf(output, "%s\n", SYSCALL);
    }
    else if (node->token->t_type == RETURN_TOKEN) {
        fprintf(output, "\n ; --------------- Return --------------- \n");
        _generate_expression(node->first_child, output, func);
        iprintf(output, "mov %s, %s\n", GET_RAW_REG(BASE_BITNESS, RSP), GET_RAW_REG(BASE_BITNESS, RBP));
        iprintf(output, "pop %s\n",     GET_RAW_REG(BASE_BITNESS, RBP));
        iprintf(output, "ret\n");
    }

    return 1;
}

static int _get_variables_size(tree_t* head, const char* func) {
    int size = 0;
    if (!head) return 0;
    for (const tree_t* expression = head; expression; expression = expression->next_sibling) {
        if (!expression->token) continue;
        if (expression->token->ro || expression->token->glob) continue;
        if (expression->token->t_type == ARRAY_TYPE_TOKEN) {
            array_info_t arr_info = { .el_size = 1 };
            if (get_array_info((char*)expression->first_child->next_sibling->next_sibling->token->value, func, &arr_info)) {
                size +=  ALIGN_TO(arr_info.size * arr_info.el_size, 4);
            }
        }
        else if (expression->token->t_type == STR_TYPE_TOKEN) {
            array_info_t arr_info = { .el_size = 1 };
            if (get_array_info((char*)expression->first_child->token->value, func, &arr_info)) {
                size += ALIGN_TO(arr_info.size * arr_info.el_size, 4);
            }
        }
        else if (
            expression->token->t_type == SWITCH_TOKEN ||
            expression->token->t_type == WHILE_TOKEN ||
            expression->token->t_type == IF_TOKEN
        ) size += _get_variables_size(expression->first_child->next_sibling->first_child, func);
        else if (expression->token->t_type == CASE_TOKEN) size += _get_variables_size(expression->first_child->first_child, func);
        else size += ALIGN_TO(expression->variable_size, 4);
    }

    return size;
}

static int _generate_declaration(const tree_t* node, FILE* output, const char* func) {
    int val = 0;
    int type = 0;

    const tree_t* name_node = node->first_child;
    if (name_node->token->ro || name_node->token->glob) return 0;

    char* derictive = " ";
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
            case 16: derictive = " word ";  break;
            case 8: derictive  = " byte ";  break;
        }

        if (name_node->next_sibling->token->t_type == UNKNOWN_NUMERIC_TOKEN) val = str_atoi((char*)name_node->next_sibling->token->value);
        else if (name_node->next_sibling->token->t_type == CHAR_VALUE_TOKEN) val = *name_node->next_sibling->token->value;
    }

    char source[36] = { 0 };
    if (type) sprintf(source, "%d", val);
    else sprintf(source, "%s", GET_RAW_REG(BASE_BITNESS, RAX));

    iprintf(output, "mov%s%s, %s ; decl %s = %s\n", derictive, GET_ASMVAR(name_node), source, (char*)name_node->token->value, source);
    return 1;
}

static int _generate_function(const tree_t* node, FILE* output, const char* func) {
    const tree_t* name_node   = node->first_child;
    const tree_t* params_node = name_node->next_sibling;
    const tree_t* body_node   = params_node->next_sibling;

    fprintf(output, "\n ; --------------- Function %s --------------- \n", name_node->token->value);
    iprintf(output, "jmp __end_%s__\n", name_node->token->value);
    iprintf(output, "__%s__:\n", name_node->token->value);

    _current_depth += 1;
    iprintf(output, "push %s\n", GET_RAW_REG(BASE_BITNESS, RBP));
    iprintf(output, "mov %s, %s\n", GET_RAW_REG(BASE_BITNESS, RBP), GET_RAW_REG(BASE_BITNESS, RSP));

    /*
    Reserving stack memory for local variables. (Creating stack frame).
    We should go into function body and find all local variables.
    Also we remember input variables.
    */
    int local_vars_size = _get_variables_size(params_node->first_child, (char*)name_node->token->value) + _get_variables_size(body_node->first_child, (char*)name_node->token->value);
    iprintf(output, "sub %s, %d\n", GET_RAW_REG(BASE_BITNESS, RSP), ALIGN_TO(local_vars_size, 4));

    /*
    Loading input args to stack.
    */
    int pop_params = 0;
    int stack_offset = 8;

    static const int args_regs[] = { RDI, RSI, RDX, RCX, R8, R9 };
    for (const tree_t* param = params_node->first_child; param; param = param->next_sibling) {
        int param_size = param->variable_size;
        char* param_name = (char*)param->first_child->token->value;

        regs_t reg;
#if (BASE_BITNESS == BIT64)
        get_reg(&reg, get_variable_size(param->first_child->token) / 8, args_regs[pop_params], 0);
        if (pop_params < 6) iprintf(output, "mov%s[%s - %d], %s\n", reg.operation, GET_RAW_REG(BASE_BITNESS, RBP), param->first_child->variable_offset, reg.name);
        else
#else
        get_reg(&reg, get_variable_size(param->first_child->token) / 8, RAX, 0);
#endif
        {
            iprintf(output, "mov %s, [%s + %d] ; int64 %s \n", reg.name, GET_RAW_REG(BASE_BITNESS, RBP), stack_offset, param_name);
            iprintf(output, "mov%s[%s - %d], %s\n", reg.operation, GET_RAW_REG(BASE_BITNESS, RBP), param->first_child->variable_offset, reg.name);
            stack_offset += param_size;
        }

        pop_params++;
    }

    /*
    Function body without return statement.
    All expressions will use one shared register EAX.
    */
    for (const tree_t* part = body_node->first_child; part; part = part->next_sibling) {
        _generate_expression(part, output, (char*)name_node->token->value);
    }

    _current_depth -= 1;
    fprintf(output, " ; --------------- \n");
    iprintf(output, "__end_%s__:\n", name_node->token->value);
    return 1;
}

static int _generate_while(const tree_t* node, FILE* output, const char* func) {
    int current_label = _label_counter++;
    const tree_t* condition = node->first_child;
    const tree_t* body = condition->next_sibling->first_child;

    fprintf(output, "\n ; --------------- while cycle [%i] --------------- \n", current_label);
    iprintf(output, "__while_%d__:\n", current_label);
    _current_depth += 1;

    _generate_expression(condition, output, func);
    iprintf(output, "cmp %s, 0\n", GET_RAW_REG(BASE_BITNESS, RAX));
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

    iprintf(output, "cmp %s, %d\n", GET_RAW_REG(BASE_BITNESS, RAX), val);
    iprintf(output, "jl __case_l_%d_%d__\n", val, label_id);
    iprintf(output, "jg __case_r_%d_%d__\n", val, label_id);
    iprintf(output, "jmp __case_%d_%d__\n", val, label_id);

    iprintf(output, "__case_l_%d_%d__:\n", val, label_id);
    _generate_case_binary_jump(output, values, left, mid - 1, label_id, default_scope);

    iprintf(output, "__case_r_%d_%d__:\n", val, label_id);
    _generate_case_binary_jump(output, values, mid + 1, right, label_id, default_scope);
    return 1;
}

static int _generate_switch(const tree_t* node, FILE* output, const char* func) {
    int current_label = _label_counter++;
    const tree_t* stmt = node->first_child;
    const tree_t* cases = stmt->next_sibling;

    int cases_count = 0;
    int values[128] = { -1 };

    fprintf(output, "\n ; --------------- switch [%i] --------------- \n", current_label);
    _current_depth += 1;

    int have_default = 0;
    iprintf(output, "jmp __end_cases_%d__\n", current_label);
    for (const tree_t* curr_case = cases->first_child; curr_case; curr_case = curr_case->next_sibling) {
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
        for (const tree_t* part = curr_case->first_child->first_child; part; part = part->next_sibling) {
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

static int _generate_if(const tree_t* node, FILE* output, const char* func) {
    int current_label = _label_counter++;
    const tree_t* condition = node->first_child;
    const tree_t* body = condition->next_sibling;
    const tree_t* else_body = body->next_sibling;

    fprintf(output, "\n ; --------------- if statement [%i] --------------- \n", current_label);
    _generate_expression(condition, output, func);
    iprintf(output, "cmp %s, 0\n", GET_RAW_REG(BASE_BITNESS, RAX));
    if (else_body) iprintf(output, "je __else_%d__\n", current_label);
    else iprintf(output, "je __end_if_%d__\n", current_label);
    _current_depth += 1;

    const tree_t* body_exp = body->first_child;
    while (body_exp) {
        _generate_expression(body_exp, output, func);
        body_exp = body_exp->next_sibling;
    }

    iprintf(output, "jmp __end_if_%d__\n", current_label);

    if (else_body) {
        iprintf(output, "__else_%d__:\n", current_label);
        const tree_t* else_body_exp = else_body->first_child;
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
/* https://gist.github.com/GabriOliv/a9411fa771a1e5d94105cb05cbaebd21 */
/* https://math.hws.edu/eck/cs220/f22/registers.html */
static int _generate_syscall(const tree_t* node, FILE* output, const char* func) {
    static const int args_regs[] = { 
#if (BASE_BITNESS == BIT64)
        RAX, RDI, RSI, RDX, R10, R8, R9
#else
        RAX, RBX, RCX, RDX, RSI, RDI, RBP
#endif
    };

    fprintf(output, "\n ; --------------- system call --------------- \n");

    int arg_index = 0;
    const tree_t* args = node->first_child;

    while (args) {
        regs_t reg;
        int is_ptr = (get_array_info((char*)node->first_child->token->value, func, NULL) && !(node->first_child->token->ro || node->first_child->token->glob));
        get_reg(&reg, get_variable_size(args->token) / 8, args_regs[arg_index++], is_ptr);
        iprintf(output, "%s%s%s, %s\n", reg.move, reg.operation, reg.name, GET_ASMVAR(args));
        args = args->next_sibling;
    }

    iprintf(output, "%s\n", SYSCALL);
    fprintf(output, " ; --------------- \n");

    return 1;
}

static int _generate_assignment(const tree_t* node, FILE* output, const char* func) {
    const tree_t* left  = node->first_child;
    const tree_t* right = left->next_sibling;

    fprintf(output, "\n; --------------- Assignment: %s = %s --------------- \n", left->token->value, right->token->value);

    /*
    We store right result to RAX, and move it to stack with offset of left.
    Pointer assignment. Also we check if this variable is ptr, array or etc.
    Markers are 64 bits size and first child.
    */
    if ((get_variable_size(left->token) == 32) && left->first_child) {
        /*
        If left is array or string (array too) with elem size info.
        */
        array_info_t arr_info = { .el_size = 1 };
        int is_ptr = (get_array_info((char*)node->first_child->token->value, func, &arr_info) && !(node->first_child->token->ro || node->first_child->token->glob));

        /*
        Generate offset movement in this array-like data type.
        Then multiply it by arr el_size.
        */
        _generate_expression(left->first_child, output, func);
        if (arr_info.el_size > 1) iprintf(output, "imul %s, %d\n", GET_RAW_REG(BASE_BITNESS, RAX), arr_info.el_size);
        iprintf(output, "%s %s, %s\n", !is_ptr ? "mov" : "lea", GET_RAW_REG(BASE_BITNESS, RBX), GET_ASMVAR(left));

        iprintf(output, "add %s, %s\n", GET_RAW_REG(BASE_BITNESS, RAX), GET_RAW_REG(BASE_BITNESS, RBX));
        iprintf(output, "push %s\n", GET_RAW_REG(BASE_BITNESS, RAX));

        _generate_expression(right, output, func);
        iprintf(output, "pop %s\n", GET_RAW_REG(BASE_BITNESS, RBX));

        regs_t reg;
        get_reg(&reg, arr_info.el_size, RAX, 0);
        iprintf(output, "mov%s[%s], %s\n", reg.operation, GET_RAW_REG(BASE_BITNESS, RBX), reg.name);
    }
    else {
        _generate_expression(right, output, func);
        iprintf(output, "mov %s, %s\n", GET_ASMVAR(left), GET_RAW_REG(BASE_BITNESS, RAX));
    }

    fprintf(output, " ; --------------- \n");
    return 1;
}

static int _generate_text_section(const tree_t* node, FILE* output) {
    if (!node) return 0;
    for (const tree_t* child = node->first_child; child; child = child->next_sibling) _generate_expression(child, output, NULL);
    return 1;
}

int generate_asm(const tree_t* root, FILE* output) {
    const tree_t* program_body = root->first_child;
    const tree_t* prestart     = program_body;
    const tree_t* main_body    = prestart->next_sibling;

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
        for (const tree_t* child = prestart->first_child; child; child = child->next_sibling) {
            if (child->token) switch (child->token->t_type) {
                case IMPORT_SELECT_TOKEN:
                    for (const tree_t* func = child->first_child->first_child; func; func = func->next_sibling)
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

        iprintf(output, "push %s\n", GET_RAW_REG(BASE_BITNESS, RBP));
        iprintf(output, "mov %s, %s\n", GET_RAW_REG(BASE_BITNESS, RBP), GET_RAW_REG(BASE_BITNESS, RSP));
        iprintf(output, "sub %s, %d\n", GET_RAW_REG(BASE_BITNESS, RSP), ALIGN_TO(_get_variables_size(main_body->first_child, NULL), 16));
        _generate_text_section(main_body, output);
    }

    return 1;
}

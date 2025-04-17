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
        switch (child->token->t_type) {
            case INT_TYPE_TOKEN:
            case SHORT_TYPE_TOKEN:
            case CHAR_TYPE_TOKEN: break;
            case ARRAY_TYPE_TOKEN: {
                tree_t* size   = child->first_child;
                tree_t* t_type = size->next_sibling;
                tree_t* name   = t_type->next_sibling;
            
                array_info_t info;
                str_strncpy(info.name, (char*)name->token->value, 32);
                info.el_size = str_atoi((char*)t_type->token->value);
                _arrays_info[_array_count++] = info;

                const char* directive = NULL;
                if (str_strcmp((char*)t_type->token->value, CHAR_VARIABLE) == 0) directive = "db";
                else if (str_strcmp((char*)t_type->token->value, SHORT_VARIABLE) == 0) directive = "dw";
                else if (str_strcmp((char*)t_type->token->value, INT_VARIABLE) == 0) directive = "dd";
                else directive = "dq";
            
                if (!name->next_sibling) fprintf(output, "%*s%s times %s %s 0\n", _current_depth * 4, "", name->token->value, size->token->value, directive);
                else {
                    fprintf(output, "%*s%s %s ", _current_depth * 4, "", name->token->value, directive);
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
                fprintf(output, "%*s%s db '%s', 0\n", _current_depth * 4, "", name->token->value, value->token->value);
                break;
            }
            default: _generate_data_section(child, output); break;
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
        fprintf(output, "%*smov eax, %s\n", _current_depth * 4, "", node->token->value);
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
            fprintf(output, "%*smov [ebp - %d], eax ; int %s = eax\n", _current_depth * 4, "", node->variable_offset, (char*)name_node->token->value);
        }
    }
    else if (node->token->t_type == INT_VARIABLE_TOKEN) {
        /*
        Getting integer from stack, and saving in EAX shared register.
        */
        fprintf(output, "%*smov eax, [ebp - %d] ; int %s\n", _current_depth * 4, "", node->variable_offset, node->token->value);
        _generate_expression(node->first_child, output);
    }
    else if (node->token->t_type == SHORT_TYPE_TOKEN) {
        tree_t* name_node = node->first_child;
        if (name_node->next_sibling) {
            _generate_expression(name_node->next_sibling, output);
            fprintf(output, "%*smov [ebp - %d], ax ; short %s = eax\n", _current_depth * 4, "", node->variable_offset, (char*)name_node->token->value);
        }
    }
    else if (node->token->t_type == SHORT_VARIABLE_TOKEN) {
        fprintf(output, "%*smov ax, [ebp - %d] ; short %s\n", _current_depth * 4, "", node->variable_offset, node->token->value);
        _generate_expression(node->first_child, output);
    }
    else if (node->token->t_type == CHAR_TYPE_TOKEN) {
        tree_t* name_node = node->first_child;
        if (name_node->next_sibling) {
            _generate_expression(name_node->next_sibling, output);
            fprintf(output, "%*smov [ebp - %d], al ; char %s = eax\n", _current_depth * 4, "", node->variable_offset, (char*)name_node->token->value);
        }
    }
    else if (node->token->t_type == CHAR_VARIABLE_TOKEN) {
        fprintf(output, "%*smov al, [ebp - %d] ; char %s\n", _current_depth * 4, "", node->variable_offset, node->token->value);
        _generate_expression(node->first_child, output);
    }
    else if (node->token->t_type == ARR_VARIABLE_TOKEN || node->token->t_type == STR_VARIABLE_TOKEN) {
        if (!node->first_child) fprintf(output, "%*smov eax, %s\n", _current_depth * 4, "", node->token->value);
        else {
            array_info_t* arr_info = _find_array_info((char*)node->token->value);
            int elem_size = arr_info ? arr_info->el_size : 1;
            _generate_expression(node->first_child, output);
            fprintf(output, "%*simul eax, %d\n", _current_depth * 4, "", elem_size); 
            fprintf(output, "%*sadd eax, %s\n", _current_depth * 4, "", node->token->value);
            if (elem_size == 1) fprintf(output, "%*smov al, [eax]\n", _current_depth * 4, "");
            else if (elem_size == 4) fprintf(output, "%*smov eax, [eax]\n", _current_depth * 4, "");
        }
    }
    else if (node->token->t_type == PTR_TYPE_TOKEN) {
        tree_t* name_node = node->first_child;
        if (name_node->next_sibling) {
            _generate_expression(name_node->next_sibling, output);
            fprintf(output, "%*smov [ebp - %d], eax ; ptr %s = eax\n", _current_depth * 4, "", node->variable_offset, (char*)name_node->token->value);
        }
    }
    else if (node->token->t_type == PTR_VARIABLE_TOKEN) {
        fprintf(output, "%*smov eax, [ebp - %d] ; ptr %s\n", _current_depth * 4, "", node->variable_offset, node->token->value);
        if (node->first_child) {
            _generate_expression(node->first_child, output);
            fprintf(output, "%*smov eax, [eax]\n", _current_depth * 4, "");
        }
    }
    else if (node->token->t_type == BITMOVE_LEFT_TOKEN) {
        _generate_expression(node->first_child, output);
        fprintf(output, "%*spush eax\n", _current_depth * 4, "");
        _generate_expression(node->first_child->next_sibling, output);
        fprintf(output, "%*spop ebx\n", _current_depth * 4, "");
        fprintf(output, "%*smov ecx, eax\n", _current_depth * 4, "");
        fprintf(output, "%*sshl ebx, cl\n", _current_depth * 4, "");
        fprintf(output, "%*smov eax, ebx\n", _current_depth * 4, "");
    }
    else if (node->token->t_type == BITMOVE_RIGHT_TOKEN) {
        _generate_expression(node->first_child, output);
        fprintf(output, "%*spush eax\n", _current_depth * 4, "");
        _generate_expression(node->first_child->next_sibling, output);
        fprintf(output, "%*spop ebx\n", _current_depth * 4, "");
        fprintf(output, "%*smov ecx, eax\n", _current_depth * 4, "");
        fprintf(output, "%*sshr ebx, cl\n", _current_depth * 4, "");
        fprintf(output, "%*smov eax, ebx\n", _current_depth * 4, "");
    }
    else if (node->token->t_type == BITAND_TOKEN) {
        _generate_expression(node->first_child, output);
        fprintf(output, "%*spush eax\n", _current_depth * 4, "");
        _generate_expression(node->first_child->next_sibling, output);
        fprintf(output, "%*spop ebx\n", _current_depth * 4, "");
        fprintf(output, "%*sand eax, ebx\n", _current_depth * 4, "");
    }
    else if (node->token->t_type == BITOR_TOKEN) {
        _generate_expression(node->first_child, output);
        fprintf(output, "%*spush eax\n", _current_depth * 4, "");
        _generate_expression(node->first_child->next_sibling, output);
        fprintf(output, "%*spop ebx\n", _current_depth * 4, "");
        fprintf(output, "%*sor eax, ebx\n", _current_depth * 4, "");
    }
    else if (node->token->t_type == PLUS_TOKEN) {
        _generate_expression(node->first_child, output);
        fprintf(output, "%*spush eax\n", _current_depth * 4, "");
        _generate_expression(node->first_child->next_sibling, output);
        fprintf(output, "%*spop ebx\n", _current_depth * 4, "");
        fprintf(output, "%*sadd eax, ebx\n", _current_depth * 4, "");
    }
    else if (node->token->t_type == MINUS_TOKEN) {
        _generate_expression(node->first_child, output);
        fprintf(output, "%*spush eax\n", _current_depth * 4, "");
        _generate_expression(node->first_child->next_sibling, output);
        fprintf(output, "%*spop ebx\n", _current_depth * 4, "");
        fprintf(output, "%*ssub ebx, eax\n", _current_depth * 4, "");
        fprintf(output, "%*smov eax, ebx\n", _current_depth * 4, "");
    }
    else if (node->token->t_type == MULTIPLY_TOKEN) {
        _generate_expression(node->first_child, output);
        fprintf(output, "%*spush eax\n", _current_depth * 4, "");
        _generate_expression(node->first_child->next_sibling, output);
        fprintf(output, "%*spop ebx\n", _current_depth * 4, "");
        fprintf(output, "%*simul eax, ebx\n", _current_depth * 4, "");
    }
    else if (node->token->t_type == DIVIDE_TOKEN) {
        _generate_expression(node->first_child, output);
        fprintf(output, "%*spush eax\n", _current_depth * 4, "");
        _generate_expression(node->first_child->next_sibling, output);
        fprintf(output, "%*smov ebx, eax\n", _current_depth * 4, "");
        fprintf(output, "%*spop eax\n", _current_depth * 4, "");
        fprintf(output, "%*scdq\n", _current_depth * 4, "");
        fprintf(output, "%*sidiv ebx\n", _current_depth * 4, "");
    }
    else if (node->token->t_type == LARGER_TOKEN) {
        _generate_expression(node->first_child, output);
        fprintf(output, "%*spush eax\n", _current_depth * 4, "");
        _generate_expression(node->first_child->next_sibling, output);
        fprintf(output, "%*spop ebx\n", _current_depth * 4, "");
        fprintf(output, "%*scmp ebx, eax\n", _current_depth * 4, "");
        fprintf(output, "%*ssetg al\n", _current_depth * 4, "");
        fprintf(output, "%*smovzx eax, al\n", _current_depth * 4, "");
    }
    else if (node->token->t_type == LOWER_TOKEN) {
        _generate_expression(node->first_child, output);
        fprintf(output, "%*spush eax\n", _current_depth * 4, "");
        _generate_expression(node->first_child->next_sibling, output);
        fprintf(output, "%*spop ebx\n", _current_depth * 4, "");
        fprintf(output, "%*scmp ebx, eax\n", _current_depth * 4, "");
        fprintf(output, "%*ssetl al\n", _current_depth * 4, "");
        fprintf(output, "%*smovzx eax, al\n", _current_depth * 4, "");
    }
    else if (node->token->t_type == COMPARE_TOKEN) {
        _generate_expression(node->first_child, output);
        fprintf(output, "%*spush eax\n", _current_depth * 4, "");
        _generate_expression(node->first_child->next_sibling, output);
        fprintf(output, "%*spop ebx\n", _current_depth * 4, "");
        fprintf(output, "%*scmp ebx, eax\n", _current_depth * 4, "");
        fprintf(output, "%*ssete al\n", _current_depth * 4, "");
        fprintf(output, "%*smovzx eax, al\n", _current_depth * 4, "");
    }
    else if (node->token->t_type == NCOMPARE_TOKEN) {
        _generate_expression(node->first_child, output);
        fprintf(output, "%*spush eax\n", _current_depth * 4, "");
        _generate_expression(node->first_child->next_sibling, output);
        fprintf(output, "%*spop ebx\n", _current_depth * 4, "");
        fprintf(output, "%*scmp ebx, eax\n", _current_depth * 4, "");
        fprintf(output, "%*ssetne al\n", _current_depth * 4, "");
        fprintf(output, "%*smovzx eax, al\n", _current_depth * 4, "");
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
            if (
                arg->token->t_type == INT_VARIABLE_TOKEN || 
                arg->token->t_type == PTR_VARIABLE_TOKEN
            ) fprintf(output, "%*smov eax, [ebp - %d] ; uint32 %s \n", _current_depth * 4, "", arg->variable_offset, arg->token->value);
            else if (arg->token->t_type == SHORT_VARIABLE_TOKEN) fprintf(output, "%*smov ax, [ebp - %d] ; uint16 %s \n", _current_depth * 4, "", arg->variable_offset, arg->token->value);
            else if (arg->token->t_type == CHAR_VARIABLE_TOKEN) fprintf(output, "%*smov al, [ebp - %d] ; uint8 %s \n", _current_depth * 4, "", arg->variable_offset, arg->token->value);
            else fprintf(output, "%*smov eax, %s ; uint32 %s\n", _current_depth * 4, "", arg->token->value, arg->token->value);
            fprintf(output, "%*spush eax\n", _current_depth * 4, "");
        }
    
        fprintf(output, "%*scall %s\n", _current_depth * 4, "", func_name_node->token->value);
        if (arg_count > 0) fprintf(output, "%*sadd esp, %d\n", _current_depth * 4, "", arg_count * 4);
        fprintf(output, " ; --------------- \n");
    }
    else if (node->token->t_type == EXIT_TOKEN) {
        /*
        Restore stack frame after programm.
        */
        fprintf(output, "\n ; --------------- Exit --------------- \n");
        _generate_expression(node->first_child, output);
        fprintf(output, "%*smov ebx, eax\n", _current_depth * 4, "");
        fprintf(output, "%*smov eax, 1\n", _current_depth * 4, "");
        fprintf(output, "%*smov esp, ebp\n", _current_depth * 4, "");
        fprintf(output, "%*spop ebp\n", _current_depth * 4, "");
        fprintf(output, "%*sint 0x80\n", _current_depth * 4, "");
    }
    else if (node->token->t_type == RETURN_TOKEN) {
        fprintf(output, "\n ; --------------- Return --------------- \n");
        /*
        Move to eax return expresion, and restore stack.
        Now, result of function stored at EAX register.
        */
        _generate_expression(node->first_child, output);

        fprintf(output, "%*smov esp, ebp\n", _current_depth * 4, "");
        fprintf(output, "%*spop ebp\n", _current_depth * 4, "");
        fprintf(output, "%*sret\n", _current_depth * 4, "");
    }

    return 1;
}

static int __get_variables_size(tree_t* head) {
    int size = 0;
    if (!head) return 0;
    for (tree_t* expression = head; expression; expression = expression->next_sibling) {
        if (!expression->token) continue;
        if (expression->token->t_type == WHILE_TOKEN || expression->token->t_type == IF_TOKEN) size += __get_variables_size(expression->first_child->next_sibling->first_child);
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
    fprintf(output, "%*sjmp _end_%s\n", _current_depth * 4, "", name_node->token->value);
    fprintf(output, "%*s%s:\n", _current_depth * 4, "", name_node->token->value);

    _current_depth += 1;

    fprintf(output, "%*spush ebp\n", _current_depth * 4, "");
    fprintf(output, "%*smov ebp, esp\n", _current_depth * 4, "");

    /*
    Reserving stack memory for local variables. (Creating stack frame).
    We should go into function body and find all local variables.
    Also we remember input variables.
    */
    int local_vars_size = __get_variables_size(params_node->first_child) + __get_variables_size(body_node->first_child);
    fprintf(output, "%*ssub esp, %d\n", _current_depth * 4, "", local_vars_size);

    /*
    Loading input args to stack.
    */
    int param_offset = 8;
    for (tree_t* param = params_node->first_child; param; param = param->next_sibling) {
        char* param_name = (char*)param->first_child->token->value;
        int param_size = param->variable_size;

        if (
            param->first_child->token->t_type == INT_VARIABLE_TOKEN || 
            param->first_child->token->t_type == PTR_VARIABLE_TOKEN || 
            param->first_child->token->t_type == STR_VARIABLE_TOKEN ||
            param->first_child->token->t_type == ARR_VARIABLE_TOKEN
        ) {
            fprintf(output, "%*smov eax, [ebp + %d] ; int %s \n", _current_depth * 4, "", param_offset, param_name);
            fprintf(output, "%*smov [ebp - %d], eax\n", _current_depth * 4, "", param_offset - param_size);
        }
        else if (param->first_child->token->t_type == SHORT_VARIABLE_TOKEN) {
            fprintf(output, "%*smov ax, [ebp + %d] ; short %s \n", _current_depth * 4, "", param_offset, param_name);
            fprintf(output, "%*smov [ebp - %d], ax\n", _current_depth * 4, "", param_offset - param_size);
        }
        else if (param->first_child->token->t_type == CHAR_VARIABLE_TOKEN) {
            fprintf(output, "%*smov al, [ebp + %d] ; char %s \n", _current_depth * 4, "", param_offset, param_name);
            fprintf(output, "%*smov [ebp - %d], al\n", _current_depth * 4, "", param_offset - param_size);
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

    fprintf(output, " ; --------------- \n");
    _current_depth -= 1;

    fprintf(output, "%*s_end_%s:\n", _current_depth * 4, "", name_node->token->value);
    return 1;
}

static int _generate_while(tree_t* node, FILE* output) {
    if (!node) return 0;
    int current_label = _label_counter++;
    tree_t* condition = node->first_child;
    tree_t* body = condition->next_sibling->first_child;

    fprintf(output, "\n ; --------------- while cycle [%i] --------------- \n", current_label);
    fprintf(output, "%*swhile_%d:\n", _current_depth * 4, "", current_label);
    _current_depth += 1;

    _generate_expression(condition, output);
    fprintf(output, "%*scmp eax, 0\n", _current_depth * 4, "");
    fprintf(output, "%*sje end_while_%d\n", _current_depth * 4, "", current_label);

    while (body) {
        _generate_expression(body, output);
        body = body->next_sibling;
    }

    fprintf(output, "%*sjmp while_%d\n", _current_depth * 4, "", current_label);
    fprintf(output, " ; --------------- \n");
    _current_depth -= 1;

    fprintf(output, "%*send_while_%d:\n", _current_depth * 4, "", current_label);
    return 1;
}

static int _generate_if(tree_t* node, FILE* output) {
    if (!node) return 0;
    int current_label = _label_counter++;
    tree_t* condition = node->first_child;
    tree_t* body = condition->next_sibling->first_child;

    _current_depth += 1;
    fprintf(output, "\n ; --------------- if statement [%i] --------------- \n", current_label);
    _generate_expression(condition, output);
    fprintf(output, "%*scmp eax, 0\n", _current_depth * 4, "");
    fprintf(output, "%*sje end_if_%d\n", _current_depth * 4, "", current_label);


    while (body) {
        _generate_expression(body, output);
        body = body->next_sibling;
    }

    fprintf(output, " ; --------------- \n");
    _current_depth -= 1;
    fprintf(output, "%*send_if_%d:\n", _current_depth * 4, "", current_label);
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
        if (
            args->token->t_type == INT_VARIABLE_TOKEN || args->token->t_type == PTR_VARIABLE_TOKEN
        ) fprintf(output, "%*smov %s, [ebp - %d]\n", _current_depth * 4, "", registers_32[argument_index++], args->variable_offset);
        else if (args->token->t_type == SHORT_VARIABLE_TOKEN) fprintf(output, "%*smov %s, [ebp - %d]\n", _current_depth * 4, "", registers_16[argument_index++], args->variable_offset);
        else if (args->token->t_type == CHAR_VARIABLE_TOKEN) fprintf(output, "%*smov %s, %s\n", _current_depth * 4, "", registers_8[argument_index++], args->token->value);
        else fprintf(output, "%*smov %s, %s\n", _current_depth * 4, "", registers_32[argument_index++], args->token->value);
        args = args->next_sibling;
    }

    fprintf(output, "%*sint 0x80\n", _current_depth * 4, "");
    fprintf(output, " ; --------------- \n");
    return 1;
}

static int _generate_assignment(tree_t* node, FILE* output) {
    if (!node) return 0;
    tree_t* left = node->first_child;
    tree_t* right = left->next_sibling;
    
    fprintf(output, "\n; --------------- Assignment: %s = %s --------------- \n", left->token->value, right->token->value);

    /*
    We store right result to EAX, and move it to stack with offset of left.
    Pointer assignment.
    */
    if ((
        left->token->t_type == ARR_VARIABLE_TOKEN || left->token->t_type == STR_VARIABLE_TOKEN || left->token->t_type == PTR_VARIABLE_TOKEN
    ) && left->first_child) {
        /*
        If left is array or string (array too) with elem size info.
        */
        array_info_t* arr_info = _find_array_info((char*)left->token->value);
        int elem_size = arr_info ? arr_info->el_size : 1;

        _generate_expression(left->first_child, output);
        fprintf(output, "%*simul eax, %d\n", _current_depth * 4, "", elem_size);
        if (left->token->t_type == PTR_VARIABLE_TOKEN) fprintf(output, "%*sadd eax, [ebp - %d]\n", _current_depth * 4, "", left->variable_offset);
        else fprintf(output, "%*sadd eax, %s\n", _current_depth * 4, "", left->token->value);
        fprintf(output, "%*spush eax\n", _current_depth * 4, "");
        
        _generate_expression(right, output);
        fprintf(output, "%*spop ebx\n", _current_depth * 4, "");
        if (elem_size == 1) fprintf(output, "%*smov byte [ebx], al\n", _current_depth * 4, "");
        else if (elem_size == 4) fprintf(output, "%*smov [ebx], eax\n", _current_depth * 4, "");
    } 
    else {
        /*
        Move to eax result of right operation, and store it in stack with offset.
        */
        _generate_expression(right, output);
        fprintf(output, "%*smov [ebp - %d], eax\n", _current_depth * 4, "", left->variable_offset);
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
    fprintf(output, "section .data\n");

    tree_t* program_body = root->first_child;
    tree_t* prestart = program_body;
    tree_t* main_body = prestart->next_sibling;
    if (prestart)  _generate_data_section(prestart, output);
    if (main_body) _generate_data_section(main_body, output);

    fprintf(output, "\nsection .text\n");
    if (prestart) {
        for (tree_t* child = prestart->first_child; child; child = child->next_sibling) {
            if (child->token) switch (child->token->t_type) {
                case IMPORT_SELECT_TOKEN: {
                    tree_t* module_name = child->first_child;
                    for (tree_t* func = module_name->first_child; func; func = func->next_sibling)
                    fprintf(output, "    extern %s\n", func->token->value);
                    break;
                }

                case FUNC_TOKEN: {
                    fprintf(output, "    global %s\n", child->first_child->token->value);
                    break;
                }

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
        fprintf(output, "%*spush ebp\n", _current_depth * 4, "");
        fprintf(output, "%*smov ebp, esp\n", _current_depth * 4, "");
        fprintf(output, "%*ssub esp, %d\n", _current_depth * 4, "", __get_variables_size(main_body->first_child));
        _generate_text_section(main_body, output);
    }

    return 1;
}
#include "../include/generator.h"


static int _generate_if(tree_t* node, FILE* output);
static int _generate_while(tree_t* node, FILE* output);
static int _generate_syscall(tree_t* node, FILE* output);
static int _generate_assignment(tree_t* node, FILE* output);
static int _label_counter = 0;


static int _generate_data_section(tree_t* node, FILE* output) {
    if (!node) return 0;
    if (node->token->t_type == START_TOKEN) {
        tree_t* scope = node->first_child;
        for (tree_t* child = scope->first_child; child; child = child->next_sibling) {
            switch (child->token->t_type) {
                case INT_TYPE_TOKEN: {
                    tree_t* name = child->first_child;
                    tree_t* value = name->next_sibling;
                    if (value->token->t_type == INT_VALUE_TOKEN) fprintf(output, "%s dd %s\n", name->token->value, value->token->value);
                    else fprintf(output, "%s dd 0\n", name->token->value);
                    break;
                }
                case ARRAY_TYPE_TOKEN: {
                    tree_t* size = child->first_child;
                    tree_t* t_type = size->next_sibling;
                    tree_t* name = t_type->next_sibling;
                    fprintf(output, "%s dd ", name->token->value);
                    for (tree_t* elem = name->next_sibling; elem; elem = elem->next_sibling) {
                        fprintf(output, "%s%s", elem->token->value, elem->next_sibling ? ", " : "\n");
                    }
                    break;
                }
                case STRING_TYPE_TOKEN: {
                    tree_t* name = child->first_child;
                    tree_t* value = name->next_sibling;
                    fprintf(output, "%s db '%s', 0\n", name->token->value, value->token->value);
                    break;
                }
                default: break;
            }
        }
    }

    return 1;
}

static int _generate_expression(tree_t* node, FILE* output) {
    if (!node) return 0;

    if (node->token->t_type == IF_TOKEN) _generate_if(node, output);
    else if (node->token->t_type == WHILE_TOKEN) _generate_while(node, output);
    else if (node->token->t_type == SYSCALL_TOKEN) _generate_syscall(node, output);
    else if (node->token->t_type == ASIGN_TOKEN) _generate_assignment(node, output);
    else if (node->token->t_type == INT_TYPE_TOKEN) {
        if (node->first_child->next_sibling->token->t_type != INT_VALUE_TOKEN) {
            _generate_expression(node->first_child->next_sibling, output);
            fprintf(output, "mov [%s], eax\n", node->first_child->token->value);
        }
    }
    else if (node->token->t_type == STRING_TYPE_TOKEN) {
        if (node->first_child->next_sibling->token->t_type != STRING_VALUE_TOKEN) {
            _generate_expression(node->first_child->next_sibling, output);
            fprintf(output, "mov %s, eax\n", node->first_child->token->value);
        }
    }
    else if (node->token->t_type == STR_VARIABLE_TOKEN || node->token->t_type == UNKNOWN_NUMERIC_TOKEN) {
        fprintf(output, "mov eax, %s\n", node->token->value);
        _generate_expression(node->first_child, output);
    }
    else if (node->token->t_type == INT_VARIABLE_TOKEN) {
        fprintf(output, "mov eax, [%s]\n", node->token->value);
        _generate_expression(node->first_child, output);
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
    else if (node->token->t_type == COMPARE_TOKEN) {
        _generate_expression(node->first_child, output);
        fprintf(output, "push eax\n");
        _generate_expression(node->first_child->next_sibling, output);
        fprintf(output, "pop ebx\n");
        fprintf(output, "cmp ebx, eax\n");
        fprintf(output, "sete al\n");
        fprintf(output, "movzx eax, al\n");
    }
    else if (node->token->t_type == EXIT_TOKEN) {
        _generate_expression(node->first_child, output);
        fprintf(output, "xor ebx, ebx\n");
        fprintf(output, "int 0x80\n");
    }

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
        if (args->token->t_type == UNKNOWN_NUMERIC_TOKEN || args->token->t_type == STR_VARIABLE_TOKEN) fprintf(output, "mov %s, %s\n", registers[argument_index++], args->token->value);
        else fprintf(output, "mov %s, [%s]\n", registers[argument_index++], args->token->value);
        args = args->next_sibling;
    }

    fprintf(output, "int 0x80\n");
    return 1;
}

static int _generate_assignment(tree_t* node, FILE* output) {
    tree_t* target = node->first_child;
    tree_t* value = target->next_sibling;
    
    _generate_expression(value, output);
    fprintf(output, "mov [%s], eax\n", target->token->value);
    return 1;
}

static int _generate_text_section(tree_t* node, FILE* output) {
    if (!node) return 0;
    if (node->token->t_type == START_TOKEN) {
        tree_t* scope = node->first_child;
        for (tree_t* child = scope->first_child; child; child = child->next_sibling) {
            _generate_expression(child, output);
        }

        _generate_expression(node->first_child->next_sibling, output);
    }

    return 1;
}

int generate_asm(tree_t* root, FILE* output) {
    fprintf(output, "section .data\n");
    _generate_data_section(root, output);
    fprintf(output, "\nsection .text\n");
    fprintf(output, "global _start\n\n");
    fprintf(output, "_start:\n");
    _generate_text_section(root, output);
    return 1;
}
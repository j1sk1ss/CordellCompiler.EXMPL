#include "include/tknz.h"
#include "include/parser.h"
#include "include/logg.h"
#include "include/mm.h"


static const char* __token_type_to_str(token_type_t type) {
    switch(type) {
        case UNKNOWN_STRING_TOKEN: return "UNKNOWN_STR";
        case UNKNOWN_NUMERIC_TOKEN: return "UNKNOWN_NUM";
        case UNKNOWN_SYMBOL_TOKEN: return "UNKNOWN_SYM";
        case START_TOKEN: return "START";
        case INT_TYPE_TOKEN: return "INT_DECL";
        case STRING_TYPE_TOKEN: return "STR_DECL";
        case ARRAY_TYPE_TOKEN: return "ARR_DECL";
        case EXIT_TOKEN: return "EXIT";
        case WHILE_TOKEN: return "WHILE";
        case WHILE_START_TOKEN: return "WHILE_START";
        case WHILE_END_TOKEN: return "WHILE_END";
        case IF_TOKEN: return "IF";
        case IF_START_TOKEN: return "IF_START";
        case IF_END_TOKEN: return "IF_END";
        case SYSCALL_TOKEN: return "SYSCALL";
        case ASIGN_TOKEN: return "ASSIGN";
        case COMPARE_TOKEN: return "COMPARE";
        case DELIMITER_TOKEN: return "DELIMITER";
        default: return "UNKNOWN";
    }
}

static void __print_node(tree_t* node, int depth) {
    if (!node) return;
    for (int i = 0; i < depth; i++) 
        printf("│   ");

    if (node->token) {
        printf("├─ [%s] %s\n", 
              __token_type_to_str(node->token->t_type),
              node->token->value);
    } else {
        printf("├─ [SCOPE]\n");
    }

    __print_node(node->first_child, depth + 1);
    if (depth > 0) {
        __print_node(node->next_sibling, depth);
    }
}

void print_parse_tree(tree_t* root) {
    if (!root) {
        printf("Parse tree is empty!\n");
        return;
    }
    
    printf("\nPARSE TREE STRUCTURE:\n");
    __print_node(root, 0);
    printf("\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_error("Input files required!");
        return -1;
    }

    mm_init();

    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY);
        token_t* tokens = tknz_tokenize(fd);
        int markup_res = command_markup(tokens);
        if (!markup_res) {
            unload_tokens(tokens);
            return -1;
        }

        tree_t* parse_tree = create_parse_tree(tokens);

        print_parse_tree(parse_tree);
        unload_parse_tree(parse_tree);
        unload_tokens(tokens);
    }

    return 1;
}
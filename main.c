#include "include/generator.h"
#include "include/syntax.h"
#include "include/token.h"
#include "include/logg.h"
#include "include/mm.h"


void print_parse_tree(tree_t* node, int depth) {
    if (!node) return;
    for (int i = 0; i < depth; i++) printf("  ");
    if (node->token) printf("Token(type=%d, value=%s)\n", node->token->t_type, (char*)node->token->value);
    else printf("scope\n");
    
    tree_t* child = node->first_child;
    while (child) {
        print_parse_tree(child, depth + 1);
        child = child->next_sibling;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_error("Input files required!");
        return -1;
    }

    mm_init();

    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY);
        if (fd < 0) return -1;

        token_t* tokens = tokenize(fd);
        if (!tokens) return -1;

        int markup_res = command_markup(tokens);
        if (!markup_res) {
            unload_tokens(tokens);
            return -1;
        }

        tree_t* parse_tree = create_syntax_tree(tokens);

        // print_parse_tree(parse_tree, 0);

        FILE* output = fopen("output.asm", "w");
        generate_asm(parse_tree, output);
        fclose(output);
        unload_syntax_tree(parse_tree);
        unload_tokens(tokens);
    }

    return 1;
}
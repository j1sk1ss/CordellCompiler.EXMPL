#include "include/generator.h"
#include "include/semantic.h"
#include "include/syntax.h"
#include "include/token.h"
#include "include/logg.h"
#include "include/mm.h"


#ifdef PRINT_PARSE
void print_parse_tree(tree_t* node, int depth) {
    if (!node) return;
    for (int i = 0; i < depth; i++) printf("  ");
    if (node->token) printf("[%s] (t=%d, size=%i, off=%i, f=%i)\n", (char*)node->token->value, node->token->t_type, node->variable_size, node->variable_offset, node->function);
    else printf("scope\n");
    
    tree_t* child = node->first_child;
    while (child) {
        print_parse_tree(child, depth + 1);
        child = child->next_sibling;
    }
}
#else
void print_parse_tree(tree_t* node, int depth) {}
#endif


int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_error("Input files required!\nFor usage use -h.");
        return -1;
    }
    
    mm_init();

    int print_syntax = 0;
    char* output_location = NULL;
    for (int i = 1; i < argc; i++) {
        if (!str_strcmp(argv[i], "-h")) {
            print_info("Usage:");
            print_info("Print syntax tree: --syntax");
            print_info("Output location: -o");
            print_info("Show this message: -h");
        }
        else if (!str_strcmp(argv[i], "--syntax")) {
            print_syntax = 1;
        }
        else if (!str_strcmp(argv[i], "-o")) {
            output_location = argv[++i];
        }
        else {
            if (!output_location) {
                print_error("Output location not provided!");
                return 0;
            }

            int fd = open(argv[i], O_RDONLY);
            if (fd < 0) return -1;

            token_t* tokens = tokenize(fd);
            if (!tokens) return -1;

            int markup_res = command_markup(tokens);
            markup_res = variable_markup(tokens);
            if (!markup_res) {
                unload_tokens(tokens);
                return -1;
            }

            tree_t* parse_tree = create_syntax_tree(tokens);
            if (print_syntax) print_parse_tree(parse_tree, 0);
            int semantic_res = check_semantic(parse_tree);
            if (semantic_res) {
                FILE* output = fopen(output_location, "w");
                generate_asm(parse_tree, output);
                fclose(output);
            }

            unload_syntax_tree(parse_tree);
            unload_tokens(tokens);
        }
    }

    return 1;
}
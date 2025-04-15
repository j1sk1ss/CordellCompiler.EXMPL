#include "../include/builder.h"


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


static params_t __params = {
    .syntax = 0
};

static object_t __files[MAX_FILES];
static int __current_file = 0;


int __add_object(char* path, int is_main) {
    __files[__current_file].main = is_main;
    __files[__current_file].path = path;
    __current_file++;
    return 1;
}

int __compile(object_t* obj) {
    int fd = open(obj->path, O_RDONLY);
    if (fd < 0) return -1;
    
    token_t* tokens = tokenize(fd);
    if (!tokens) return -2;

    int markup_res = command_markup(tokens);
    markup_res = variable_markup(tokens);
    if (!markup_res) {
        unload_tokens(tokens);
        return -3;
    }

    tree_t* parse_tree = create_syntax_tree(tokens);
    if (__params.syntax) print_parse_tree(parse_tree, 0);
    int semantic_res = check_semantic(parse_tree);
    if (semantic_res) {
        char save_path[128] = { 0 };
        sprintf(save_path, "%s.asm", obj->path);
        FILE* output = fopen(save_path, "w");
        generate_asm(parse_tree, output);
        fclose(output);

        char compile_command[128] = { 0 };
        sprintf(compile_command, "%s -f%s %s -o %s.o", DEFAULT_ASM_COMPILER, DEFAULT_ARCH, save_path, save_path);
        system(compile_command);
    }

    unload_syntax_tree(parse_tree);
    unload_tokens(tokens);
    return 1;
}

int build(char* path, int is_main) {
    return __add_object(path, is_main);
}

int build_all(char* output) {
    if (__current_file == 0) return 0;

    /*
    Production of .asm files with temporary saving in files directory.
    */
    for (int i = __current_file - 1; i >= 0; i--) {
        int res = __compile(&__files[i]);
        if (!res) return res;
    }

    /*
    Linking output files
    */
    char link_command[256] = { 0 };
    sprintf(link_command, "%s -m %s ", DEFAULT_LINKER, DEFAULT_LINKER_ARCH);

    for (int i = __current_file - 1; i >= 0; i--) {
        char object_path[128] = { 0 };
        sprintf(object_path, " %s.asm.o", __files[i].path);
        str_strcat(link_command, object_path);
    }

    str_strcat(link_command, " -o ");
    str_strcat(link_command, output);
    system(link_command);

    /*
    Cleanup
    */
    for (int i = __current_file - 1; i >= 0; i--) {
        char delete_command[128] = { 0 };
        sprintf(delete_command, "rm %s.asm %s.asm.o", __files[i].path, __files[i].path);
        system(delete_command);
    }

    return 1;
}

int set_params(params_t* params) {
    str_memcpy(&__params, params, sizeof(params_t));
    return 1;
}

#include "../include/builder.h"


static int _print_parse_tree(tree_t* node, int depth) {
    if (!node) return 0;
    for (int i = 0; i < depth; i++) printf("  ");
    if (node->token) printf(
        "[%s] (t=%d, size=%i, is_ptr=%i, off=%i, ro=%i glob=%i)\n", 
        (char*)node->token->value, node->token->t_type, node->variable_size, node->token->ptr, node->variable_offset, node->token->ro, node->token->glob
    );
    else printf("scope\n");
    
    tree_t* child = node->first_child;
    while (child) {
        _print_parse_tree(child, depth + 1);
        child = child->next_sibling;
    }
    
    return 1;
}


static params_t _params = { .syntax = 0, .save_asm = 0 };
static object_t _files[MAX_FILES];
static int _current_file = 0;


static int _generate_raw_ast(object_t* obj) {
    int fd = open(obj->path, O_RDONLY);
    if (fd < 0) return -1;

    token_t* tokens = tokenize(fd);
    if (!tokens) {
        close(fd);
        return -2;
    }

    int markup_res = command_markup(tokens);
    markup_res = variable_markup(tokens);
    if (!markup_res) {
        unload_tokens(tokens);
        close(fd);
        return -3;
    }

    tree_t* parse_tree = create_syntax_tree(tokens);
    if (!check_semantic(parse_tree)) {
        unload_syntax_tree(parse_tree);
        unload_tokens(tokens);
        close(fd);
        return -4;
    }

    obj->ast = parse_tree;

    obj->ast_arrinfo = get_arrmap_head();
    set_arrmap_head(NULL);

    obj->ast_varinfo = get_varmap_head();
    set_varmap_head(NULL);
    return 1;
}

static int _add_object(char* path) {
    _files[_current_file].path = path;
    _generate_raw_ast(&_files[_current_file]);
    _current_file++;
    return 1;
}

static int _compile_object(object_t* obj) {
    string_optimization(obj->ast);

    int is_fold_vars = 0;
    do {
        assign_optimization(obj->ast);
        is_fold_vars = muldiv_optimization(obj->ast);
    } while (is_fold_vars);
    
    unload_varmap(obj->ast_varinfo);
    varuse_optimization(obj->ast);
    offset_optimization(obj->ast);

    char save_path[128] = { 0 };
    sprintf(save_path, "%s.asm", obj->path);
    FILE* output = fopen(save_path, "w");
    if (_params.syntax) _print_parse_tree(obj->ast, 0);
    generate_asm(obj->ast, output);
    fclose(output);

    char compile_command[128] = { 0 };
    sprintf(compile_command, "%s -f%s %s -o %s.o", DEFAULT_ASM_COMPILER, DEFAULT_ARCH, save_path, save_path);
    system(compile_command);

    unload_syntax_tree(obj->ast);
    unload_arrmap(obj->ast_arrinfo);
    unload_varmap(obj->ast_varinfo);
    return 1;
}

int builder_add_file(char* input) {
    return _add_object(input);
}

int builder_compile(char* output) {
    if (_current_file == 0) return 0;

    /*
    Production of .asm files with temporary saving in files directory.
    */
    for (int i = _current_file - 1; i >= 0; i--) {
        int res = _compile_object(&_files[i]);
        if (!res) return res;
    }

    /*
    Linking output files
    */
    char link_command[256] = { 0 };
    sprintf(link_command, "%s -m %s %s ", DEFAULT_LINKER, DEFAULT_LINKER_ARCH, LINKER_FLAGS);

    for (int i = _current_file - 1; i >= 0; i--) {
        char object_path[128] = { 0 };
        sprintf(object_path, " %s.asm.o", _files[i].path);
        str_strcat(link_command, object_path);
    }

    str_strcat(link_command, " -o ");
    str_strcat(link_command, output);
    system(link_command);

    /*
    Cleanup
    */
    for (int i = _current_file - 1; i >= 0; i--) {
        char delete_command[128] = { 0 };
        if (!_params.save_asm) sprintf(delete_command, "rm %s.asm %s.asm.o", _files[i].path, _files[i].path);
        system(delete_command);
    }

    return 1;
}

int set_params(params_t* params) {
    str_memcpy(&_params, params, sizeof(params_t));
    return 1;
}

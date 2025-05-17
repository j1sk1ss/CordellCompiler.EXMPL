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


static params_t _params = { };
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

/*
First markup will mark all primitives and struct declarations.
*/
    markup_res = variable_markup(tokens);
/*
Second markup will mark all complicated objects like struct instances.
*/
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

    obj->tokens = tokens;
    obj->ast = parse_tree;

    obj->ast_arrinfo = get_arrmap_head();
    set_arrmap_head(NULL);

    obj->ast_varinfo = get_varmap_head();
    set_varmap_head(NULL);
    close(fd);
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
    stmt_optimization(obj->ast);
    varuse_optimization(obj->ast);
    offset_optimization(obj->ast);

    char save_path[128] = { 0 };
    sprintf(save_path, "%s.asm", obj->path);
    FILE* output = fopen(save_path, "w");
    if (_params.syntax) _print_parse_tree(obj->ast, 0);

    set_varmap_head(obj->ast_varinfo);
    set_arrmap_head(obj->ast_arrinfo);
    generate_asm(obj->ast, output);
    fclose(output);

    char compile_command[128] = { 0 };
    sprintf(compile_command, "%s -f%s %s -g -o %s.o", _params.asm_compiler, _params.arch, save_path, save_path);

    print_debug("COMPILING: system(%s)", compile_command);
    system(compile_command);

    unload_syntax_tree(obj->ast);
    unload_tokens(obj->tokens);
    unload_arrmap(obj->ast_arrinfo);
    unload_varmap(obj->ast_varinfo);
    return 1;
}

int builder_add_file(char* input) {
    return _add_object(input);
}

int builder_compile() {
    if (_current_file == 0) return 0;

    for (int i = 0; i < _current_file; i++) funcopt_add_ast(_files[i].ast);
    func_optimization();

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
    sprintf(link_command, "%s -m %s %s ", _params.linker, _params.linker_arch, _params.linker_flags);

    for (int i = _current_file - 1; i >= 0; i--) {
        char object_path[128] = { 0 };
        sprintf(object_path, " %s.asm.o", _files[i].path);
        str_strcat(link_command, object_path);
    }

    str_strcat(link_command, " -o ");
    str_strcat(link_command, _params.save_path);

    print_debug("LINKING: system(%s)", link_command);
    system(link_command);

    /*
    Cleanup
    */
    for (int i = _current_file - 1; i >= 0; i--) {
        char delete_command[128] = { 0 };
        if (!_params.save_asm) sprintf(delete_command, "rm %s.asm %s.asm.o", _files[i].path, _files[i].path);

        print_debug("CLEANUP: system(%s)", delete_command);
        system(delete_command);
    }

    return 1;
}

int set_params(params_t* params) {
    str_memcpy(&_params, params, sizeof(params_t));
    return 1;
}

#include "../../include/optimization.h"


typedef struct string_info {
    char str_body[TOKEN_MAX_SIZE];
    char str_name[TOKEN_MAX_SIZE];
    struct string_info* next;
} string_info_t;

static int _str_num = 0;
static string_info_t* _str_h = NULL;

static string_info_t* _create_string_info(const char* body) {
    string_info_t* node = (string_info_t*)mm_malloc(sizeof(string_info_t));
    if (!node) return NULL;

    str_memset(node->str_name, 0, TOKEN_MAX_SIZE);
    str_memset(node->str_body, 0, TOKEN_MAX_SIZE);
    node->next = NULL;

    str_strncpy(node->str_body, body, TOKEN_MAX_SIZE);
    sprintf(node->str_name, "__str_%d__", _str_num++);
    return node;
}

static int _add_string(const char* body) {
    string_info_t* node = _create_string_info(body);
    if (!node) return 0;
    
    string_info_t* h = _str_h;
    if (!h) {
        _str_h = node;
        return 1;
    }

    while (h->next) {
        h = h->next;
    }

    h->next = node;
    return 1;
}

static int _get_string(const char* body, string_info_t* info) {
    string_info_t* h = _str_h;
    while (h) {
        if (!str_strcmp(h->str_body, body)) {
            if (info) str_memcpy(info, h, sizeof(string_info_t));
            return 1;
        }

        h = h->next;
    }

    return 0;
}

static int _unload_stringmap() {
    while (_str_h) {
        string_info_t* n = _str_h->next;
        mm_free(_str_h);
        _str_h = n;
    }

    return 1;
}


static int _find_string(tree_t* root) {
    if (!root) return 0;
    for (tree_t* t = root->first_child; t; t = t->next_sibling) {
        if (!t->token) {
            _find_string(t);
            continue;
        }

        switch (t->token->t_type) {
            case CALL_TOKEN:
            case SYSCALL_TOKEN:
            case RETURN_TOKEN: _find_string(t); continue;
            case IF_TOKEN:
            case WHILE_TOKEN: _find_string(t->first_child->next_sibling); continue;
            case FUNC_TOKEN: _find_string(t->first_child->next_sibling->next_sibling); continue;
            default: break;
        }
        
        if (t->token->t_type == STRING_VALUE_TOKEN) {
            string_info_t info;
            if (_get_string((char*)t->token->value, &info)) {
                t->token->t_type = STR_VARIABLE_TOKEN;
                sprintf((char*)t->token->value, "%s", info.str_name);
            }
            else {
                _add_string((char*)t->token->value);
                _get_string((char*)t->token->value, &info);
                sprintf((char*)t->token->value, "%s", info.str_name);
            }
            
            t->token->ro = 1;
        }
    }

    return 1;
}

static int _declare_strings(tree_t* root) {
    string_info_t* h = _str_h;
    while (h) {
        tree_t* decl_root = create_tree_node(create_token(STR_TYPE_TOKEN, (unsigned char*)STR_VARIABLE, str_strlen(STR_VARIABLE), 0));
        if (!decl_root) return 0;
        
        tree_t* name_node = create_tree_node(create_token(STR_VARIABLE_TOKEN, (unsigned char*)h->str_name, str_strlen(h->str_name), 0));
        if (!name_node) return 0;
        
        tree_t* value_node = create_tree_node(create_token(STRING_VALUE_TOKEN, (unsigned char*)h->str_body, str_strlen(h->str_body), 0));
        if (!value_node) return 0;

        add_child_node(decl_root, name_node);
        add_child_node(decl_root, value_node);
        
        name_node->token->ro = 1;
        decl_root->token->ro = 1;

        tree_t* old = root->first_child;
        root->first_child = decl_root;
        decl_root->next_sibling = old;

        h = h->next;
    }

    return 1;
}

int string_optimization(tree_t* root) {
    tree_t* program_body = root->first_child;
    tree_t* prestart     = program_body;
    tree_t* main_body    = prestart->next_sibling;
    _find_string(prestart);
    _find_string(main_body);
    if (_str_h) _declare_strings(prestart);
    _unload_stringmap();
    return 1;
}

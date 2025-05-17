#include "../../include/optimization.h"


typedef struct code_node {
    tree_t* start;
    tree_t* end;
    struct code_node* parent;
    struct code_node* first_child;
    struct code_node* next_sibling;
    int is_reachable;
    int is_function;
} code_node_t;

static code_node_t* _code_block_h = NULL;

typedef struct func_code_node {
    char name[TOKEN_MAX_SIZE];
    code_node_t* head;
    struct func_code_node* next;
} func_code_node_t;

static func_code_node_t* _func_code_block_h = NULL;


static code_node_t* _create_code_node(tree_t* start) {
    if (!start) return NULL;
    code_node_t* node = (code_node_t*)mm_malloc(sizeof(code_node_t));
    if (!node) return NULL;

    node->start = start;
    node->end = NULL;

    node->is_reachable = 0;
    node->parent       = NULL;
    node->first_child  = NULL;
    node->next_sibling = NULL;

    return node;
}

static int _add_child_to_block(code_node_t* parent, code_node_t* child) {
    if (!parent || !child) return 0;
    child->parent = parent;
    if (!parent->first_child) parent->first_child = child;
    else {
        code_node_t* sibling = parent->first_child;
        while (sibling->next_sibling) sibling = sibling->next_sibling;
        sibling->next_sibling = child;
    }

    return 1;
}

static int _unload_code_block(code_node_t* node) {
    if (!node) return 0;
    _unload_code_block(node->first_child);
    _unload_code_block(node->next_sibling);
    mm_free(node);
    return 1;
}

static func_code_node_t* _create_func_code_block(char* name, code_node_t* head) {
    if (!name || !head) return NULL;
    func_code_node_t* node = (func_code_node_t*)mm_malloc(sizeof(func_code_node_t));
    if (!node) return NULL;

    str_strncpy(node->name, name, TOKEN_MAX_SIZE);
    node->head = head;
    node->next = NULL;
    return node;
}

static int _add_func_code_block(char* name, code_node_t* head) {
    func_code_node_t* node = _create_func_code_block(name, head);
    if (!node) return 0;

    if (!_func_code_block_h) {
        _func_code_block_h = node;
    }
    else {
        func_code_node_t* h = _func_code_block_h;
        while (h->next) h = h->next;
        h->next = node;
    }

    return 1;
}

static func_code_node_t* _find_func_block(char* name) {
    func_code_node_t* h = _func_code_block_h;
    while (h) {
        if (!str_strncmp(h->name, name, TOKEN_MAX_SIZE)) return h;
        h = h->next;
    }

    return NULL;
}

static int _unload_func_node_map() {
    while (_func_code_block_h) {
        func_code_node_t* n = _func_code_block_h->next;
        _unload_code_block(_func_code_block_h->head);
        mm_free(_func_code_block_h);
        _func_code_block_h = n;
    }

    return 1;
}


static code_node_t* _generate_blocks(tree_t* curr) {
    if (!curr) return NULL;
    code_node_t* head = _create_code_node(curr);
    if (!head) return NULL;
    
    /*
    We don't go deeper, and only catch changes in flow.
    */
    tree_t* curr_node = curr;
    for (tree_t* t = curr; t; t = t->next_sibling, curr_node = curr_node->next_sibling) {
        if (!t->token) continue;
        switch (t->token->t_type) {
            /*
            Generating block and linking to function name.
            */
            case FUNC_TOKEN:
                tree_t* name_node   = t->first_child;
                tree_t* params_node = name_node->next_sibling;
                tree_t* body_node   = params_node->next_sibling;
                code_node_t* func_blocks = _generate_blocks(body_node);
                if (func_blocks) {
                    _add_func_code_block((char*)name_node->token->value, func_blocks);
                }
            break;

            /*
            Flow go deeper without flow changing.
            */
            case SWITCH_TOKEN:
            case WHILE_TOKEN:
            case IF_TOKEN: break;
            case CALL_TOKEN:
                _add_child_to_block(head, _find_func_block((char*)t->token->value)->head);
            break;

            /*
            Flow kill point.
            */
            case RETURN_TOKEN:
            case EXIT_TOKEN: goto dead_flow;
            default: break;
        }
    }

dead_flow:
    head->end = curr_node;
    return head;
}

int deadcode_optimization(tree_t* root) {
    code_node_t* program = _generate_blocks(root->first_child);
    _unload_func_node_map();
    _unload_code_block(program);
    return 1;
}

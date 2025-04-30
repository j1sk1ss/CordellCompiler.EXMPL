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


static int _generate_blocks(code_node_t* block) {
    if (!block || !block->start) return 0;
    tree_t* entry = block->start;
    
    for (tree_t* t = entry; t; t = t->next_sibling) {
        if (!t->token) {
            // Scope tree node.
            continue;
        }
    }

    return 1;
}

int deadcode_optimization(tree_t* root) {
    code_node_t* program = _create_code_node(root);
    _code_block_h = _create_code_node(root->first_child);
    _code_block_h->is_reachable = 1;
    _generate_blocks(_code_block_h);
    _unload_code_block(_code_block_h);
    return 1;
}

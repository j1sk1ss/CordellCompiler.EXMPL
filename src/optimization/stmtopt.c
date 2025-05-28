#include "../../include/optimization.h"


static int _find_stmt(tree_t* root) {
    if (!root) return 0;
    for (tree_t* t = root->first_child; t; t = t->next_sibling) {
        if (!t->token) {
            _find_stmt(t);
            continue;
        }

        switch (t->token->t_type) {
            case IF_TOKEN: {
                tree_t* condition = t->first_child;
                tree_t* body = condition->next_sibling;
                tree_t* else_body = body->next_sibling;
                if (condition->token->t_type == UNKNOWN_NUMERIC_TOKEN) {
                    int true_or_false = str_atoi((char*)condition->token->value);
                    if (true_or_false && else_body) {
                        remove_child_node(t, else_body);
                        unload_syntax_tree(else_body);
                    }
                    else if (!true_or_false && else_body) {
                        remove_child_node(t, body);
                        unload_syntax_tree(body);
                    }
                    else if (!true_or_false && !else_body) {
                        remove_child_node(root, t);
                        unload_syntax_tree(t);
                        _find_stmt(root);
                        return 1;
                    }
                }

                break;
            }

            case WHILE_TOKEN: {
                tree_t* condition = t->first_child;
                tree_t* body = condition->next_sibling->first_child;
                if (condition->token->t_type == UNKNOWN_NUMERIC_TOKEN) {
                    int true_or_false = str_atoi((char*)condition->token->value);
                    if (!true_or_false) {
                        remove_child_node(root, t);
                        unload_syntax_tree(t);
                        _find_stmt(root);
                        return 1;
                    }
                }

                break;
            }

            case SWITCH_TOKEN: {
                tree_t* stmt  = t->first_child;
                tree_t* cases = stmt->next_sibling;
                if (stmt->token->t_type == UNKNOWN_NUMERIC_TOKEN) {
                    int option_case = str_atoi((char*)stmt->token->value);
                    for (tree_t* curr_case = cases->first_child; curr_case; curr_case = curr_case->next_sibling) {
                        if (curr_case->token->t_type == DEFAULT_TOKEN) continue;
                        int case_value = str_atoi((char*)curr_case->token->value);
                        if (option_case != case_value) {
                            remove_child_node(cases, curr_case);
                            unload_syntax_tree(curr_case);
                            _find_stmt(root);
                            return 1;
                        }
                    }
                }
                
                break;
            }
            
            case FUNC_TOKEN: _find_stmt(t->first_child->next_sibling->next_sibling); continue;
            default: break;
        }

    }

    return 1;
}

int stmt_optimization(tree_t* root) {
    if (!root) return 0;
    return _find_stmt(root);
}

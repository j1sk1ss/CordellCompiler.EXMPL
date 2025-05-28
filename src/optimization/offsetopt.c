#include "../../include/optimization.h"


static int _recalc_offs(tree_t* root, const char* func) {
    if (!root) return 0;
    for (tree_t* t = root->first_child; t; t = t->next_sibling) {
        if (!t->token) {
            _recalc_offs(t, func);
            continue;
        }

        t->variable_offset = 0;
        switch (t->token->t_type) {
            case FUNC_TOKEN:
                int tmp_off = get_vars_offset();
                set_vars_offset(0);
                _recalc_offs(t, (char*)t->first_child->token->value);
                set_vars_offset(tmp_off);
            break;
            case ARR_VARIABLE_TOKEN:
            case STR_VARIABLE_TOKEN:
            case LONG_VARIABLE_TOKEN:
            case INT_VARIABLE_TOKEN:
            case SHORT_VARIABLE_TOKEN:
            case CHAR_VARIABLE_TOKEN:
            if (!t->token->ro && !t->token->glob) {
                variable_info_t info;
                if (get_var_info((char*)t->token->value, func, &info)) t->variable_offset = info.offset;
                else t->variable_offset = add_variable_info((char*)t->token->value, t->variable_size, func);
                }

                _recalc_offs(t, func);
            break;
            default: _recalc_offs(t, func); break;
        }
    }

    return 1;
}


int offset_optimization(tree_t* root) {
    if (!root) return 0;
    _recalc_offs(root, NULL);
    return 1;
}


#include "../include/parser.h"


int command_markup(token_t* head) {
    token_t* curr = head;
    while (curr) {
        if (curr->t_type == UNKNOWN_STRING_TOKEN) {
            // this is string command or string value
            if (!str_strcmp((char*)curr->value, START_COMMAND)) curr->t_type = START_TOKEN;

            // Variables and types
            else if (!str_strcmp((char*)curr->value, INT_VARIABLE)) curr->t_type = INT_TYPE_TOKEN;
            else if (!str_strcmp((char*)curr->value, STR_VARIABLE)) curr->t_type = STRING_TYPE_TOKEN;
            else if (!str_strcmp((char*)curr->value, ARR_VARIABLE)) curr->t_type = ARRAY_TYPE_TOKEN;

            // While
            else if (!str_strcmp((char*)curr->value, WHILE_COMAND)) curr->t_type = WHILE_TOKEN;
            else if (!str_strcmp((char*)curr->value, WHILE_START_COMMAND)) curr->t_type = WHILE_START_TOKEN;
            else if (!str_strcmp((char*)curr->value, WHILE_END_COMMAND)) curr->t_type = WHILE_END_TOKEN;

            // If
            else if (!str_strcmp((char*)curr->value, IF_COMMAND)) curr->t_type = IF_TOKEN;
            else if (!str_strcmp((char*)curr->value, IF_START_COMMAND)) curr->t_type = IF_START_TOKEN;
            else if (!str_strcmp((char*)curr->value, IF_END_COMMAND)) curr->t_type = IF_END_TOKEN;

            else if (!str_strcmp((char*)curr->value, SYSCALL_COMMAND)) curr->t_type = SYSCALL_TOKEN;

            // End
            else if (!str_strcmp((char*)curr->value, EXIT_COMMAND)) curr->t_type = EXIT_TOKEN;
        }
        else if (curr->t_type == UNKNOWN_SYMBOL_TOKEN) {
            // this is symbol. Now determining what's symbol is
            if (!str_strcmp((char*)curr->value, ASIGN_STATEMENT)) curr->t_type = ASIGN_TOKEN;
            else if (!str_strcmp((char*)curr->value, COMPARE_STATEMENT)) curr->t_type = COMPARE_TOKEN;
            else if (!str_strcmp((char*)curr->value, PLUS_STATEMENT)) curr->t_type = PLUS_TOKEN;
            else if (!str_strcmp((char*)curr->value, MINUS_STATEMENT)) curr->t_type = MINUS_TOKEN;
            else if (!str_strcmp((char*)curr->value, LARGER_STATEMENT)) curr->t_type = LARGER_TOKEN;
            else if (!str_strcmp((char*)curr->value, LOWER_STATEMENT)) curr->t_type = LOWER_TOKEN;
        }

        curr = curr->next;
    }

    return 1;
}

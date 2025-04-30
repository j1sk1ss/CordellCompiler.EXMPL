#include "../include/token.h"


#pragma region [Misc]

    static char_type_t _get_char_type(unsigned char ch) {
        if (isalpha(ch)) return CHAR_ALPHA;
        else if (str_isdigit(ch) || ch == '-') return CHAR_DIGIT;
        else if (ch == '"')  return CHAR_QUOTE;
        else if (ch == '\'') return CHAR_SING_QUOTE;
        else if (ch == '\n') return CHAR_NEWLINE;
        else if (ch == ' ')  return CHAR_SPACE;
        else if (ch == ';')  return CHAR_DELIMITER;
        else if (ch == ':')  return CHAR_COMMENT;
        else if (ch == '[')  return CHAR_OPEN_INDEX;
        else if (ch == ']')  return CHAR_CLOSE_INDEX;
        return CHAR_OTHER;
    }

    static int _add_token(token_t** head, token_t** tail, token_type_t type, const unsigned char* buffer, size_t len, int line) {
        token_t* new_token = create_token(type, buffer, len, line);
        if (!new_token) return 0;
        
        if (!*head) *head = new_token;
        else (*tail)->next = new_token;
        *tail = new_token; 
        return 1;
    }

#pragma endregion


token_t* create_token(token_type_t type, const unsigned char* value, size_t len, int line) {
    if (len > TOKEN_MAX_SIZE) return NULL;

    token_t* token = mm_malloc(sizeof(token_t));
    if (!token) return NULL;

    token->t_type = type;
    if (value) {
        str_strncpy((char*)token->value, (char*)value, len);
        token->value[len] = '\0';
    }
    
    token->next = NULL;
    token->line_number = line;
    
    token->ro = 0;
    token->ptr = 0;
    if (type == UNKNOWN_NUMERIC_TOKEN) token->glob = 1;
    else token->glob = 0;
    return token;
}

token_t* tokenize(int fd) {
    token_t* head = NULL;
    token_t* tail = NULL;
    unsigned char buffer[BUFFER_SIZE]       = { 0 };
    unsigned char token_buf[TOKEN_MAX_SIZE] = { 0 };
    
    int in_token       = 0;
    int file_offset    = 0;
    size_t token_len   = 0;
    ssize_t bytes_read = 0;
    token_type_t current_type = UNKNOWN_STRING_TOKEN;

    int line = 1;
    while ((bytes_read = pread(fd, buffer, BUFFER_SIZE, file_offset)) > 0) {
        file_offset += bytes_read;

        int comment_open = 0;
        int quotes_open  = 0;
        int sing_quotes_open = 0;
        
        for (ssize_t i = 0; i < bytes_read; ++i) {
            unsigned char ch = buffer[i];
            char_type_t ct = _get_char_type(ch);
            if (ct == CHAR_SING_QUOTE) {
                sing_quotes_open = !sing_quotes_open;
                continue;
            }
            else if (ct == CHAR_QUOTE) {
                quotes_open = !quotes_open;
                continue;
            }
            else if (ct == CHAR_COMMENT && !quotes_open && !sing_quotes_open) {
                comment_open = !comment_open;
                continue;
            }

            if (comment_open && !quotes_open) continue;
            if ((ct != CHAR_SPACE && ct != CHAR_NEWLINE) || quotes_open || sing_quotes_open) {
                token_type_t new_type = UNKNOWN_STRING_TOKEN;

                /*
                If current type is UNKNOWS_STRING, we know, that this can be variable name.
                */
                //if (current_type != UNKNOWN_STRING_TOKEN) {
                    if (quotes_open) new_type = STRING_VALUE_TOKEN;
                    else if (sing_quotes_open) new_type = CHAR_VALUE_TOKEN;
                    else {
                        if (ct == CHAR_ALPHA)            new_type = UNKNOWN_STRING_TOKEN;
                        else if (ct == CHAR_DIGIT)       new_type = UNKNOWN_NUMERIC_TOKEN;
                        else if (ct == CHAR_DELIMITER)   new_type = DELIMITER_TOKEN;
                        else if (ct == CHAR_OPEN_INDEX)  new_type = OPEN_INDEX_TOKEN;
                        else if (ct == CHAR_CLOSE_INDEX) new_type = CLOSE_INDEX_TOKEN;
                        else new_type = UNKNOWN_STRING_TOKEN;
                    }
                //}
                
                if (in_token) {
                    if (current_type != new_type && new_type != STRING_VALUE_TOKEN) {
                        if (!_add_token(&head, &tail, current_type, token_buf, token_len, line)) goto error;
                        in_token = 0;
                    }
                }
                
                if (!in_token) {
                    current_type = new_type;
                    token_len = 0;
                    in_token = 1;
                }
                
                if (token_len >= TOKEN_MAX_SIZE) goto error;
                token_buf[token_len++] = ch;
            } 
            else {
                // current_type = UNKNOWN_COMMAND_TOKEN;
                if (in_token) {
                    if (ct == CHAR_NEWLINE) line++;
                    if (!_add_token(&head, &tail, current_type, token_buf, token_len, line)) goto error;
                    in_token = 0;
                }
            }
        }
    }

    if (bytes_read < 0) goto error;
    if (in_token) {
        if (!_add_token(&head, &tail, current_type, token_buf, token_len, line)) goto error;
    }
    
    return head;

error:
    unload_tokens(head);
    return NULL;
}

int unload_tokens(token_t* head) {
    while (head) {
        token_t* next = head->next;
        mm_free(head);
        head = next;
    }
    
    return 1;
}
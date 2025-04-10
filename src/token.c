#include "../include/token.h"


static char_type_t _get_char_type(unsigned char ch) {
    if (isalpha(ch)) return CHAR_ALPHA;
    if (isdigit(ch)) return CHAR_DIGIT;
    if (ch == '"')   return CHAR_QUOTE;
    if (ch == '\n')  return CHAR_NEWLINE;
    if (ch == ' ')   return CHAR_SPACE;
    if (ch == ';')   return CHAR_DELIMITER;
    return CHAR_OTHER;
}

static token_t* _create_token(token_type_t type, const unsigned char* value, size_t len) {
    if (len > TOKEN_MAX_SIZE) return NULL;
    token_t* token = mm_malloc(sizeof(token_t));
    if (!token) return NULL;
    
    token->t_type = type;
    str_strncpy((char*)token->value, (char*)value, len);
    token->value[len] = '\0';
    token->next = NULL;
    return token;
}

static int _add_token(token_t** head, token_t** tail, token_type_t type, const unsigned char* buffer, size_t len) {
    token_t* new_token = _create_token(type, buffer, len);
    if (!new_token) return 0;
    
    if (!*head) *head = new_token;
    else (*tail)->next = new_token;
    
    *tail = new_token;
    return 1;
}

token_t* tokenize(int fd) {
    token_t* head = NULL;
    token_t* tail = NULL;
    unsigned char buffer[BUFFER_SIZE] = { 0 };
    unsigned char token_buf[TOKEN_MAX_SIZE + 1] = {0};
    
    int file_offset = 0;
    size_t token_len = 0;
    token_type_t current_type = UNKNOWN_STRING_TOKEN;
    int in_token = 0;
    ssize_t bytes_read = 0;

    while ((bytes_read = pread(fd, buffer, BUFFER_SIZE, file_offset)) > 0) {
        file_offset += bytes_read;
        int quotes_open = 0;
        
        for (ssize_t i = 0; i < bytes_read; ++i) {
            unsigned char ch = buffer[i];
            char_type_t ct = _get_char_type(ch);
            if (ct == CHAR_QUOTE) {
                quotes_open = !quotes_open;
                continue;
            }

            if ((ct != CHAR_SPACE && ct != CHAR_NEWLINE) || quotes_open) {
                token_type_t new_type;
                if (ct == CHAR_ALPHA) new_type = UNKNOWN_STRING_TOKEN;
                else if (ct == CHAR_DIGIT) new_type = UNKNOWN_NUMERIC_TOKEN;
                else if (ct == CHAR_DELIMITER) new_type = DELIMITER_TOKEN;
                else new_type = UNKNOWN_SYMBOL_TOKEN;
                
                if (quotes_open) new_type = STRING_VALUE_TOKEN;
                if (in_token) {
                    if (current_type != new_type && current_type != UNKNOWN_STRING_TOKEN && new_type != STRING_VALUE_TOKEN) {
                        if (!_add_token(&head, &tail, current_type, token_buf, token_len)) goto error;
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
            } else {
                if (in_token) {
                    if (!_add_token(&head, &tail, current_type, token_buf, token_len)) goto error;
                    in_token = 0;
                }
            }
        }
    }

    if (bytes_read < 0) goto error;
    if (in_token) {
        if (!_add_token(&head, &tail, current_type, token_buf, token_len)) goto error;
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
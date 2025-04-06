#include "../include/tknz.h"


token_t* tknz_tokenize(int fd) {
    token_t* head = NULL;
    token_t* tail = NULL;

    int file_seek = 0;
    unsigned char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_read;

    unsigned char token_buf[TOKEN_MAX_SIZE];
    size_t token_len = 0;
    token_type_t current_type = UNKNOWN_STRING;

    int in_token = 0;

    while ((bytes_read = pread(fd, buffer, BUFFER_SIZE, file_seek)) > 0) {
        file_seek += bytes_read;
        for (ssize_t i = 0; i < bytes_read; ++i) {
            unsigned char ch = buffer[i];

            if (isalpha(ch)) {
                if (!in_token || current_type != UNKNOWN_STRING) {
                    if (in_token) {
                        token_t* new_token = (token_t*)mm_malloc(sizeof(token_t));
                        new_token->t_type = current_type;
                        str_memcpy(new_token->value, token_buf, token_len);
                        new_token->value[token_len] = '\0';
                        new_token->next = NULL;
                        if (!head) head = new_token;
                        else tail->next = new_token;
                        tail = new_token;
                        token_len = 0;
                    }

                    in_token = 1;
                    current_type = UNKNOWN_STRING;
                }

                if (token_len < TOKEN_MAX_SIZE - 1)
                    token_buf[token_len++] = ch;
            }
            else if (isdigit(ch)) {
                if (!in_token || current_type != UNKNOWN_NUMERIC) {
                    if (in_token) {
                        token_t* new_token = (token_t*)mm_malloc(sizeof(token_t));
                        new_token->t_type = current_type;
                        str_memcpy(new_token->value, token_buf, token_len);
                        new_token->value[token_len] = '\0';
                        new_token->next = NULL;
                        if (!head) head = new_token;
                        else tail->next = new_token;
                        tail = new_token;
                        token_len = 0;
                    }

                    in_token = 1;
                    current_type = UNKNOWN_NUMERIC;
                }

                if (token_len < TOKEN_MAX_SIZE - 1)
                    token_buf[token_len++] = ch;
            }
            else {
                if (in_token) {
                    token_t* new_token = (token_t*)mm_malloc(sizeof(token_t));
                    new_token->t_type = current_type;
                    str_memcpy(new_token->value, token_buf, token_len);
                    new_token->value[token_len] = '\0';
                    new_token->next = NULL;
                    if (!head) head = new_token;
                    else tail->next = new_token;
                    tail = new_token;
                    token_len = 0;
                    in_token = 0;
                }
            }
        }
    }

    if (in_token && token_len > 0) {
        token_t* new_token = (token_t*)mm_malloc(sizeof(token_t));
        new_token->t_type = current_type;
        str_memcpy(new_token->value, token_buf, token_len);
        new_token->value[token_len] = '\0';
        new_token->next = NULL;
        if (!head) head = new_token;
        else tail->next = new_token;
        tail = new_token;
    }

    return head;
}

int unload_tokens(token_t* head) {
    token_t* curr = head;
    while (curr) {
        token_t* prev = curr;
        curr = curr->next;
        mm_free(prev);
    }

    return 1;
}

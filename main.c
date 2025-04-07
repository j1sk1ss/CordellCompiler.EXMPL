#include "include/tknz.h"
#include "include/logg.h"
#include "include/mm.h"


int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_error("Input files required!");
        return -1;
    }

    mm_init();

    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY);
        token_t* tokens = tknz_tokenize(fd);
        token_t* curr = tokens;
        while (curr) {
            print_log("t_type=%i, val=%s", curr->t_type, curr->value);
            curr = curr->next;
        }

        unload_tokens(tokens);
    }

    return 1;
}
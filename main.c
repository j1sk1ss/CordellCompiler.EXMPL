#include "include/builder.h"
#include "include/mm.h"


int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_error("Input files required!");
        print_info("For usage use -h.");
        return EXIT_FAILURE;
    }
    
    mm_init();
    params_t p = { .save_asm = 0, .syntax = 0 };

    int first = 1;
    char* output_location = NULL;
    for (int i = 1; i < argc; i++) {
        if (!str_strcmp(argv[i], "-h")) {
            printf("|===============================\n"         );
            printf("|| Usage:\n"                                );
            printf("|| Print syntax tree: --syntax\n"           );
            printf("|| Output location: -o\n"                   );
            printf("|| Show this message: -h\n"                 );
            printf("|| Save output generated asm code: --asm\n" );
            printf("|===============================\n"         );
        }
        else if (!str_strcmp(argv[i], "--syntax")) p.syntax = 1;
        else if (!str_strcmp(argv[i], "--asm")) p.save_asm = 1;
        else if (!str_strcmp(argv[i], "-o")) output_location = argv[++i];
        else {
            if (!output_location) {
                print_error("Output location not provided!");
                return EXIT_FAILURE;
            }

            build(argv[i], first);
            first = 0;
        }
    }

    set_params(&p);
    int build_res = build_all(output_location);
    if (!build_res) {
        print_error("Error via compilation! Code: %i", build_res);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
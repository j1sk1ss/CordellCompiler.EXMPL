#include "include/builder.h"
#include "include/mm.h"


int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_error("Input files required!\nFor usage use -h.");
        return -1;
    }
    
    mm_init();

    int first = 1;
    char* output_location = NULL;
    for (int i = 1; i < argc; i++) {
        if (!str_strcmp(argv[i], "-h")) {
            print_info("Usage:");
            print_info("Print syntax tree: --syntax");
            print_info("Output location: -o");
            print_info("Show this message: -h");
        }
        else if (!str_strcmp(argv[i], "--syntax")) {
            params_t p = { .syntax = 1 };
            set_params(&p);
        }
        else if (!str_strcmp(argv[i], "-o")) {
            output_location = argv[++i];
        }
        else {
            if (!output_location) {
                print_error("Output location not provided!");
                return 0;
            }

            build(argv[i], first);
            first = 0;
        }
    }

    int build_res = build_all();
    if (!build_res) {
        print_error("Error via compilation! Code: %i", build_res);
    }

    return 1;
}
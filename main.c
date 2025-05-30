#include "include/builder.h"
#include "include/mm.h"


int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_error("Input files required!");
        print_info("For usage use -h.");
        return EXIT_FAILURE;
    }
    
    mm_init();
    params_t p = { 
        .save_asm = 0, .syntax = 0, 
        .asm_compiler = DEFAULT_ASM_COMPILER, .arch = DEFAULT_ARCH,
        .linker = DEFAULT_LINKER, .linker_arch = DEFAULT_LINKER_ARCH, .linker_flags = LINKER_FLAGS, 
        .save_path = NULL 
    };

    for (int i = 1; i < argc; i++) {
        if (!str_strcmp(argv[i], "-h")) {
            printf("|==============================================\n");
            printf("|| Usage:\n"                                     );
            printf("|| Print syntax tree: --syntax\n"                );
            printf("|| Output location: -o / --output\n"             );
            printf("|| Save output generated asm code: --save-asm\n" );
            printf("|| Change microcode compiler: --asm\n"           );
            printf("|| Change target microcode arch: --arch\n"       );
            printf("|| Change linker: -l / --linker\n"               );
            printf("|| Change linker flags: --lf\n"                  );
            printf("|| Change linker target arch: --la\n"            );
            printf("|| Show this message: -h\n"                      );
            printf("|=============================================\n");
            return EXIT_SUCCESS;
        }
        else if (!str_strcmp(argv[i], "--syntax")) p.syntax = 1;
        else if (!str_strcmp(argv[i], "--save-asm")) p.save_asm = 1;
        else if (!str_strcmp(argv[i], "-o") || !str_strcmp(argv[i], "--output")) {
            if (i + 1 < argc) p.save_path = argv[++i];
            else print_warn("Wrong usage of -o command. Usage: -o <path>.");
        }
        else if (!str_strcmp(argv[i], "--asm")) {
            if (i + 1 < argc) p.asm_compiler = argv[++i];
            else print_warn("Wrong usage of --asm command. Usage: --asm <asm compiler (default - nasm)>.");
        }
        else if (!str_strcmp(argv[i], "--arch")) {
            if (i + 1 < argc) p.arch = argv[++i];
            else print_warn("Wrong usage of --arch command. Usage: --arch <architecture (default - elf64)>.");
        }
        else if (!str_strcmp(argv[i], "-l") || !str_strcmp(argv[i], "--linker")) {
            if (i + 1 < argc) p.linker = argv[++i];
            else print_warn("Wrong usage of --linker command. Usage: -l / --linker <linker (default - ld)>.");
        }
        else if (!str_strcmp(argv[i], "--lf")) {
            if (i + 1 < argc) p.linker_flags = argv[++i];
            else print_warn("Wrong usage of --lf command. Usage: --lf <flags (default - \"-z relro -z now\")>.");
        }
        else if (!str_strcmp(argv[i], "--la")) {
            if (i + 1 < argc) p.linker_arch = argv[++i];
            else print_warn("Wrong usage of --la command. Usage: --la <linker arch (default - elf_x86_64)>.");
        }
        else {
            builder_add_file(argv[i]);
        }
    }

    set_params(&p);
    int build_res = builder_compile();
    if (!build_res) {
        print_error("Error via compilation! Code: %i", build_res);
        return EXIT_FAILURE;
    }

    print_info("Executable generated by [%s] path.", p.save_path);
    return EXIT_SUCCESS;
}
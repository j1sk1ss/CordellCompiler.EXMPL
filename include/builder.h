#ifndef BUILDER_H_
#define BUILDER_H_

#include <stdlib.h>

#include "optimization.h"
#include "generator.h"
#include "semantic.h"
#include "syntax.h"
#include "arrmem.h"
#include "varmem.h"
#include "token.h"
#include "logg.h"

#define MAX_FILES   100
#define DEFAULT_ASM_COMPILER    "nasm"
#define DEFAULT_LINKER          "ld"
#define DEFAULT_ARCH            "elf32"
#define DEFAULT_LINKER_ARCH     "elf_i386"
#define LINKER_FLAGS            "-z relro -z now"

typedef struct {
    tree_t*          ast;
    token_t*         tokens;
    array_info_t*    ast_arrinfo;
    variable_info_t* ast_varinfo;
    char*            path;
} object_t;

typedef struct {
    int syntax;
    int save_asm;
    
    char* arch;
    char* linker;
    char* linker_arch;
    char* linker_flags;
    char* asm_compiler;

    char* save_path;
} params_t;


int builder_add_file(char* input);
int builder_compile();
int set_params(params_t* params);

#endif
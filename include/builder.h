#ifndef BUILDER_H_
#define BUILDER_H_

#include "generator.h"
#include "semantic.h"
#include "syntax.h"
#include "token.h"
#include "logg.h"

#define MAX_FILES   100

typedef struct {
    char* path;
    int main;
} object_t;

typedef struct {
    int syntax;
} params_t;


int build(char* path, int is_main);
int build_all();
int set_params(params_t* params);

#endif
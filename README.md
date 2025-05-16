# Cordell Compiler Reference

## Navigation
- [Summary](#summary)
- [Architecture](#architecture)
    - [Optimization summary](#frontend-optimization-and-backend)
    - [AST generating](#ast)
- [Documentation](#documentation)
    - [Structure](#program-structure)
    - [Variables and Types](#variables-and-types)
    - [Operations](#operations)
    - [Loops and Conditions](#loops-and-conditions)
    - [Functions](#functions)
    - [System calls](#inputoutput-via-system-calls)
- [Links](#links)

# Summary

**Cordell Compiler** is a compact hobby compiler for `Cordell Programming Language` with a simple syntax, inspired by C and assembly. It is designed for studying compilation, code optimization, translation, and low-level microcode generation. </br>
**Main goal** of this project is learning of compilers architecture and porting one to `CordellOS` project.

---

# Architecture

## Frontend, optimization and Backend

Main idea of this compiler is simplification of architecture of real compilers like `gcc` or `CMake`. This project splitted into two parts: `frontend` and `backend`. </br>
`frontend` part, like frontend part in real compilers, takes files with .CPL extention, generates abstract syntax tree (AST), and optimize code with next algorithms: </br>
- funcopt - First optimization algorithm, that takes care about unused functions, iterate through files from input, register all used functions, and delete all unregistered.
- stropt - Second optimization algorithm takes care about strings that read only. If user uses strings with ro flag somewhere, this algorithm will allocate memory for them in the .rodata section.
- assignopt - Third optimization works in group with fourth. This is a constant folding algorithm implementation. If we can use value of the variable, instead the variable itself, we replace it and remove the variable declaration.
- muldivopt - The fourth algorithm always called after the third, and extends the constant folding cycle by convolution of constants. For example, if the expression was looks like: `5 + 5`, it convolve it into `10`.
- stmtopt - The fifth algorithm always works only with `switch`, `if` and `while`, and looks similar to `funcopt`. The main idea is to remove all unreacheble code. For example, we can always say, that the code in `while (0)` never called.
- varuseopt - The sixth algorithm continues code cleaning. At this point we try to remove all unused variables.
- offsetopt - The seventh algorithm completes all the work described above work by recalculating local and global offsets for variables and arrays.
- deadopt - [WIP algorithm. See description here: https://en.wikipedia.org/wiki/Control_flow]

After compiler optimization, we go into the `backend`.</br>
In backend we generate ASM microcode (WIP opcodes and ELF executable), whose architecture depends on the target system architecture, such as `x86_64` or `x86_32`.

## AST

A number of compilers generate an Abstract Syntax Tree (next `AST`), and this one of them. The alghorithm is simple. Before generating the tree, we already tokenize the entire file, and now we only need register a bunch of parsers for each token type (We will speak about several types of tokens): </br>
- `LONG_TYPE_TOKEN` - This token indicates, that the following sequence of tokens is an expression that will be placed into the variable of type `long`. If we print this structure, it will looks like: </br>

        [LONG_TYPE_TOKEN]
            [NAME]
            [expression]
                [...]

- `CALL_TOKEN` - This token tells us, that the next few tokens are the function name and the function's input arguments. Token itself is the function name: </br>

        [CALL_TOKEN (name)]
            [SCOPE]
                [ARG1 expression]
                [ARG2 expression]
                ...

- `WHILE_TOKEN` - This token similar to `IF_TOKEN`, and tells us about the structure of following tokens: </br>

        [WHILE_TOKEN]
            [STMT]
            [SCOPE]
                [BODY]

- `PLUS_TOKEN` - This is binary operator token, that has next structure: </br>

        [PLUS_TOKEN]
            [LEFT expression]
            [RIGHT expression]

---

# .CPL Documentation

## Program Structure

Every program begins with the `start` entrypoint and ends with the `exit [return_code];` statement.

```
    start 
        ... // code 
    exit 0;
```

Also every program can contain `pre-implemented` code blocks and data segments:

```
    function a ; { }
    glob int b = 0;

    start
    exit 0;
```

## Variables and Types

The following types are supported:

- `long` — Integer (64-bit).
- `int` — Integer (32-bit).
- `short` — Integer (16-bit).
- `char` — Integer (8-bit).
- `str` — String (Array of characters).
- `arr` — Array.

### Declaring Variables

```
    int a = 5;
    ro int aReadOnly = 5; : Const and global :
    glob int aGlobal = 5; : Global :

    short b = 1234 + (432 * (2 + 12)) / 87;
    char c = 'X';

    str name = "Hello, World!";
    ptr char strPtr = name; : Pointer to name string :
    : Pointers can be used as arrays :
    strPtr[0] = 'B';

    arr farr 100 char =; // Will allocate array with size 100 and elem size 1 byte
    arr sarr 5 int = 1 2 3 4 5; // Will allocate array for provided elements
```

## Operations

Basic arithmetic and logical operations are supported:

| Operation | Description         |
|-----------|---------------------|
| `+`       | Addition            |
| `-`       | Subtraction         |
| `*`       | Multiplication      |
| `/`       | Division (int)      |
| `%`       | Module (int)        |
| `==`      | Equality            |
| `!=`      | Inequality          |
| `>` `<`   | Comparison          |
| `&&` `\|\|`             | Logic operations |
| `>>` `<<` `&`  `\|` `^` | Bit operations   |

## Loops and Conditions

### Switch expression

```
    switch (a) {
        case 1; {
        }
        case 111; {
        }
        case 1111; {
        }
        : ... :
    }
```

**Note:** Switch statement based on binary search algorithm, thats why, prefer switch in situations with many cases. In other hand, with three or less options, use if for avoiding overhead.

### If Condition

```
    if a > b; {
        : ... if code :
    }
    else {
        : ... else code :
    }
```

### While Loop

```
    while (x < 10) && (y > 20); {
        : ... loop body : 
    }
```

## Functions

Functions are declared using the `function` keyword.

### Function Signature:

```
    function [name] [type1] arg1; [type2] arg2; ...; {
        : function body :
        return something;
    }
```

### Example:

```
    function sumfunc int a; int b; {
        return a + b;
    }
```

### Calling the function

```
    int result = sumfunc(5, 10);
    : Functions without return values can be called directly :
    printStr(strptr, size);
```

## Input/Output (via system calls)

### String Input/Output

```
    syscall(4, 1, ptr, size);
    syscall(3, 0, ptr, size);
```

### Wrapping in a function:

```
    function printStr ptr char buffer; int size; {
        return syscall(4, 1, buffer, size); 
    }

    function getStr ptr char buffer; int size; {
        return syscall(3, 0, buffer, size); 
    }
```

## Comments

Comments are written as annotations `:` within functions and code blocks:

```
    : Comment in one line :

    :
    Function description.
    Params
        - name Description
    :
```

## Examples

If you want see more examples, please look into the folder `examples`. 

### Example of Printing a Number:

```
    function itoa ptr char buffer; int dsize; int num; {
        int index = dsize - 1;
        int tmp = 0;

        int isNegative = 0;
        if num < 0; {
            isNegative = 1;
            num = num * -1;
        }

        while num > 0; {
            tmp = num % 10;
            buffer[index] = tmp + 48;
            index = index - 1;
            num = num / 10;
        }

        if isNegative; {
            buffer[index - 1] = '-';
        }

        return 1;
    }
```

### Example of Fibonacci N-number print:

```
    start
        int a = 0;
        int b = 1;
        int c = 0;
        int count = 0;
        while count < 20; {
            c = a + b;
            a = b;
            b = c;
            
            arr buffer 40 char =;
            itoa(buffer, 40, c);
            prints(buffer, 40);

            count = count + 1;
        }
    exit 1;
```

---

# Links

- [Compiler architecture](https://cs.lmu.edu/~ray/notes/compilerarchitecture/) </br>
- [GCC architecture](https://en.wikibooks.org/wiki/GNU_C_Compiler_Internals/GNU_C_Compiler_Architecture) </br>
- [AST tips](https://dev.to/balapriya/abstract-syntax-tree-ast-explained-in-plain-english-1h38) </br>
- [Control flow algorithm](https://en.wikipedia.org/wiki/Control_flow) </br>
- [Summary about optimization](https://en.wikipedia.org/wiki/Optimizing_compiler) </br>

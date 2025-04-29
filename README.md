# Cordell Compiler Reference

**Cordell Compiler** is a compact hobby compile for Cordell Programming Language with a simple syntax, inspired by C and assembly. It is designed for studying compilation, interpreter creation, translation, and low-level code generation.

---

## Program Structure

Every program begins with the `start` keyword and ends with the `exit [return_code];` statement.

            start 
                ... // code 
            exit 0;

---

## Variables and Types

The following types are supported:

- `ptr` — Pointer (32-bit).
- `int` — Integer (typically 32-bit).
- `short` — Short integer (16-bit).
- `char` — 8-bit integer.
- `str` — String (array of characters).
- `arr` — Array.

### Declaring Variables

            int a = 5;
            ro int aReadOnly = 5; : Const and global :
            glob int aGlobal = 5; : Global :
            
            short b = 1234;
            char c = 48;
            
            str name = "Hello, World!";
            ptr char strPtr = name; : Pointer to name string :
            : Pointers can be used as arrays :
            strPtr[0] = 'B';
            
            arr farr 100 char =; // Will allocate array with size 100 and elem size 1 byte
            arr sarr 5 4 = 1 2 3 4 5; // Will allocate array for provided elements

---

## Operations

Basic arithmetic and logical operations are supported:

| Operation | Description         |
|-----------|---------------------|
| `+`       | Addition            |
| `-`       | Subtraction         |
| `*`       | Multiplication      |
| `/`       | Division (int)      |
| `==`      | Equality            |
| `!=`      | Inequality          |
| `>` `<`   | Comparison          |
| `>>` `<<` `&`  `\|` | Bit operations |

---

## Loops and Conditions

### Switch expression

            switch (a) {
                 case 1; {
                 }
                 case 111; {
                 }
                 case 1111; {
                 }
                 : ... :
            }

**Note:** Switch statement based on binary search algorithm, thats why, prefer switch in situations with many cases. In other hand, with three or less options, use if for avoiding overhead.

### If Condition

            if a > b; {
                : ... if code :
            }
            else {
                : ... else code :
            }

### While Loop

            while x < 10; {
                : ... loop body : 
            }

---

## Functions

Functions are declared using the `function` keyword and defined within `fstart ... fend`.

### Function Signature:
            function [name] [type1] arg1; [type2] arg2; ...; {
                : function body :
                return something;
            }

### Example:

            function sumfunc int a; int b; {
                return a + b;
            }

---

## Function Calls

            int result = sumfunc 5 10;
            : Functions without return values can be called directly :
            printStr strptr size;

---

## Input/Output (via system calls)

### String Input/Output

            syscall 4 1 ptr size;
            syscall 3 0 ptr size;

### Wrapping in a function:

            function printStr int ptr; int size; {
                return syscall 4 1 ptr size; 
            }
            
            function getStr int ptr; int size; {
                return syscall 3 0 ptr size; 
            }

---

## Comments

Comments are written as annotations `:` within functions and code blocks:

            : Comment in one line :

            :
            Function description.
            Params
                - name Description
            :

---

## Standard Structures

### Example of Printing a Number:

            function itoa ptr buffer; int dsize; int num; {
                int index = dsize - 1;
                int tmp = 0;

                int isNegative = 0;
                if num < 0; {
                    isNegative = 1;
                    num = num * -1;
                }

                while num > 0; {
                    tmp = num / 10;
                    tmp = tmp * 10;
                    tmp = num - tmp;
                    tmp = tmp + 48;
                    buffer[index] = tmp;
                    index = index - 1;
                    num = num / 10;
                }

                if isNegative == 1; {
                    char minus = 45;
                    buffer[index - 1] = minus;
                }

                return 1;
            }

### Example of Fibonacci N-number print:

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
                    itoa buffer 40 c;
                    printf buffer 40;

                    count = count + 1;
                }
            exit 1;

---

## Built-in Features

| Feature               | Support |
|-----------------------|---------|
| Arithmetic Operations | V       |
| Functions             | V       |
| Strings               | V       |
| Conditions            | V       |
| Loops                 | V       |
| Arrays (`arr[...]`)   | V       |
| Byte Operations       | V       |
| Input/Output          | V (via syscall) |
| Structures            | In development  |

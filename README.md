# Cordell Compiler Language Reference

**Cordell Compiler** is a compact hobby compile for language with a simple syntax, inspired by C and assembly. It is designed for studying compilation, interpreter creation, translation, and low-level code generation.

---

## Program Structure

Every program begins with the `start` keyword and ends with the `exit [return_code];` statement.

            start 
                        ... // code 
            exit 0;

---

## Variables and Types

The following types are supported:

- `int` — Integer (typically 32-bit).
- `short` — Short integer (16-bit).
- `char` — 8-bit integer.
- `str` — String (array of characters).
- `arr` — Array.

### Declaring Variables

            int a = 5;
            short b = 1234;
            char c = 48;
            str name = "Hello, World!";
            arr farr 100 1 =; // Will allocate array with size 100 and elem size 1
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

---

## Loops and Conditions

### If Condition

            if a > b; ifstart 
                        ... // code 
            ifend

### While Loop

            while x < 10; lstart ... 
                        // loop body 
            lend

---

## Functions

Functions are declared using the `function` keyword and defined within `fstart ... fend`.

### Function Signature:
            function [name] [type1] arg1; [type2] arg2; ...; fstart 
                        // function body 
            fend [expression]; // to return a value

### Example:

            function sumfunc int a; int b; 
            fstart fend a + b;

---

## Function Calls

            int result = callfunc sumfunc 5 10;
            
            // Functions without return values can be called directly:
            callfunc printStr strptr size;

---

## Input/Output (via system calls)

### String Input/Output

            syscall 4 1 ptr size;
            syscall 3 0 ptr size;


### Wrapping in a function:

            function printStr int ptr; int size; fstart 
                syscall 4 1 ptr size; 
            fend 1;
            
            function mult int a; int b; fstart 
            fend a * b;

---

## Comments

Comments are written as annotations `:` within functions and code blocks:

            :
            Function description.
            Params:
                - name: Description
            :

---

## Standard Structures

### Example of Printing a Number:

            function printNum int num; fstart 
                int buffIndex = 19; 
                str buff = " "; int 
                tmp = 0; 
            
                while num > 0; lstart 
                    tmp = num / 10; 
                    tmp = tmp * 10; 
                    tmp = num - tmp; 
                    tmp = tmp + 48;
                    buff[buffIndex] = tmp;
                    buffIndex = buffIndex - 1;
                    num = num / 10;
                lend
            
                syscall 4 1 buff 20;
            fend 1;

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
| Input/Output          | V (via syscall) |
| Type Casting          | In development  |
| Structures            | In development  |
| Byte Operations       | In development  |

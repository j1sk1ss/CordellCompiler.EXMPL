start
    int stdin = 3;
    int stdout = 4;

    int a = 10 + 1;                        : Comment line. Here we init a variable :
    int b = 20 / 2;                        : Init works! :
    int c = a + b;

    :
        Print result of arithmetic operations.
    :
    str buffer = "   ";
    char new = c + 48;
    buffer[0] = new;
    syscall stdout 1 buffer 3;

    :
        Getting value on index 2 and printing on the screen.
    :
    arr array 7 4 = 1 2 3 4 5 6 7;
    int thirdelem = array[3];
    thirdelem = thirdelem + 48;
    str input = "         ";
    input[0] = thirdelem;
    syscall stdout 1 input 9;
    
    :
        Printing user input.
    :
    syscall stdin 0 input 9;
    syscall stdout 1 input 9;
    
    :
        Compare a with 100 and print statement.
    :
    a = 100;
    str output = "equals 100";
    if a == 100; ifstart
        syscall stdout 1 output 10;
    ifend

:
    Exit code with a value.
:
exit a;

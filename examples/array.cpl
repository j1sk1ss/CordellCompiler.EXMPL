:
    Arrays and strings usage example
:

start

:
    Constants
:
    int stdin = 3;
    int stdout = 4;

:
    String usage example.
    Changing first symbol to zero.
:
    str asd = "              ";
    syscall stdin 0 asd 14;
    asd[0] = 48;
    syscall stdout 1 asd 14;

:
    Array usage example.
    Print all elements from array.
:
    int c = 0;
    arr letters 10 1 = e k l m n o p r s t;
    arr numbers 11 4 = 48 49 50 51 52 53 54 55 56 57 58;
    arr stringBuffer 30 1 =;

    while c < 10 ; lstart
        str buffer = "XXXX";
        syscall stdout 1 buffer 4;
        c = c + 1;
    lend
exit 0;
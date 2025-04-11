:
    Arrays and strings usage example
:

start

:
    Constants
:
    int stdin = 3 ;
    int stdout = 4 ;

:
    String usage example.
    Changing first symbol to zero.
:
    str asd = "              ";
    syscall stdin 0 asd 14 ;
    asd[0] = 48 ;
    syscall stdout 1 asd 14 ;

:
    Array usage example.
    Print all elements from array.
:
    int c = 0 ;
    str buffer = "XXXX" ;
    arr letters 10 1 = a b c d e f g k m n ;
    arr numbers 11 4 = 48 49 50 51 52 53 54 55 56 57 58 ;
    while c < 10 ; lstart
        : buffer[0] = numbers[c] ; :
        : buffer[0] = letters[c] ; :
        syscall stdout 1 buffer 4 ;
        c = c + 1 ;
    lend
exit 0 ;
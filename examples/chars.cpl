start
    function printStr int ptr; int size; fstart
    :
        Print string to console.
        Params 
            - ptr - Pointer to string.
            - size - String size.
    :
        syscall 4 1 ptr size;
    fend 1;
    
    str debugMessage = "equals";

    char zero = 48;
    str buff = "0123456789";

    if buff[0] == zero; ifstart
        callfunc printStr debugMessage 6;
    ifend
exit 1;
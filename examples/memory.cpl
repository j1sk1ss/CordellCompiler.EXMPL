start
    function prints ptr strPtr; int size; fstart
        syscall 4 1 strPtr size;
    fend 1;

    function gets ptr buffPtr; int size; fstart
    fend syscall 3 0 buffPtr size;

    function malloc int size; fstart
        ptr old = syscall 45 0;
        int new = old + size;
        syscall 45 new;
    fend old;

    ptr buffer = callfunc malloc 50;
    int inputSize = callfunc gets buffer 50;
    callfunc prints buffer inputSize;
exit 1;
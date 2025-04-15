function prints ptr string; int size; fstart
fend syscall 4 1 string size;

function gets ptr buffer; int size; fstart
fend syscall 3 0 buffer size;

function strlen ptr string; fstart
    char brk = 0;
    int index = 0;
    while string[index] != brk; lstart
        index = index + 1;
    lend
fend index;

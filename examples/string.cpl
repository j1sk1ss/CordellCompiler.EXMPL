function prints ptr string; int size; {
    if size == 0; {
        return 1;
    }
    
    return syscall 4 1 string size;
}

function gets ptr buffer; int size; {
    return syscall 3 0 buffer size;
}

function strlen ptr string; {
    char brk = 0;
    int index = 0;
    while string[index] != brk; {
        index = index + 1;
    }

    return index;
}

start
    str val = "Hello!";
    prints val 6;
exit 1;

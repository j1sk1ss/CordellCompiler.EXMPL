function prints ptr string; int size; {
    if size == 0; {
        return 1;
    }
    
    return syscall 4 1 string size;
}

function gets ptr buffer; int size; {
    return syscall 3 0 buffer size;
}

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

    return 1;
}

function strlen ptr string; {
    char brk = 0;
    int index = 0;
    while string[index] != brk; {
        index = index + 1;
    }

    return index;
}

function strlen ptr char string; {
    char brk = 0;
    int index = 0;
    while string[index] != brk; {
        index = index + 1;
    }

    return index;
}

function prints ptr char string; {
    int strSize = strlen string;
    return syscall 1 1 string strSize;
}

function gets ptr char buffer; int size; {
    return syscall 0 0 buffer size;
}

function atoi ptr char buffer; int bsize; {
    int num = 0;
    bsize = bsize - 1;

    while bsize > 0; {
        num = num * 10;
        char sym = buffer[bsize];
        num = num + sym - 48;
    }

    return num;
}

function itoa ptr char buffer; int dsize; int num; {
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

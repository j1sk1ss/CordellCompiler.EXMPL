function strlen ptr char string; {
    char brk = 0;
    long index = 0;
    while string[index] != brk; {
        index = index + 1;
    }

    return index;
}

function reverse ptr char string; int size; {
    int l = 0;
    int r = size;
    while l < r; {
        if string[r] != 0; {
            char tmp = string[l];
            string[l] = string[r];
            string[r] = tmp;
            l = l + 1;
        }

        r = r - 1;
    }

    return 1;
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

function itoa ptr char buffer; long buffer_size; int num; {
    int index = 0;
    int isNegative = 0;
    if num == 0; {
        buffer[0] = '0';
        return 1;
    }

    if num < 0; {
        isNegative = 1;
        num = num * -1;
    }

    int tmp = 0;
    while (num > 0) && (index < buffer_size); {
        tmp = num % 10;
        buffer[index] = tmp + 48;
        index = index + 1;
        num = num / 10;
    }

    if isNegative == 1; {
        buffer[0] = 'n';
    }

    return index;
}

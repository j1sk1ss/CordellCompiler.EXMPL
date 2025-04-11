start
    str buffer      = "             ";
    int a           = 0;
    int b           = 1;
    int c           = 0;
    int count       = 0;
    int bufferindex = 0;

    while count < 100; lstart
        c = a + b;
        a = b;
        b = c;

        bufferindex = 10;
        while c > 0; lstart
            int tmp = c / 10;
            tmp = tmp * 10;
            tmp = c - tmp;
            tmp = tmp + 48;

            buffer[bufferindex] = tmp;
            bufferindex = bufferindex - 1;

            c = c / 10;
        lend

        syscall 4 1 buffer 13;
        count = count + 1;
    lend
exit 1;

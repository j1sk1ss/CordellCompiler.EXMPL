start
    function printNum ptr buffer; int num; int size; fstart
        int buffIndex = size - 1;
        int tmp = 0;
        while num > 0; lstart
            tmp = num / 10;
            tmp = tmp * 10;
            tmp = num - tmp;
            tmp = tmp + 48;

            buffer[buffIndex] = tmp;
            buffIndex = buffIndex - 1;

            num = num / 10;
        lend
        
        syscall 4 1 buffer size;
    fend 1;
    
    arr printBuffer 20 1 =;
    ptr bufferPtr = printBuffer;

    int a = 0;
    int b = 1;
    int c = 0;
    int count = 0;
    while count < 20; lstart
        c = a + b;
        a = b;
        b = c;
        callfunc printNum bufferPtr c 20;
        count = count + 1;
    lend
exit 1;

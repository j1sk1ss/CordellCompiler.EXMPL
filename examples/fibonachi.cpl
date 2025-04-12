start
    function printNum int num; fstart
        int buffIndex = 19;
        str buff = "                    ";
        int tmp = 0;
        while num > 0; lstart
            tmp = num / 10;
            tmp = tmp * 10;
            tmp = num - tmp;
            tmp = tmp + 48;

            buff[buffIndex] = tmp;
            buffIndex = buffIndex - 1;

            num = num / 10;
        lend

        syscall 4 1 buff 20;
    fend 1;
    
    int a = 0;
    int b = 1;
    int c = 0;
    int count = 0;
    while count < 20; lstart
        c = a + b;
        a = b;
        b = c;
        callfunc printNum c;
        count = count + 1;
    lend
exit 1;

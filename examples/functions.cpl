start
    int c = 24;
    int g = 12;
    
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

    function sumfunc int a; int b; fstart
    fend a + b;

    function divfunc int a; int b; fstart
    fend a - b;

    int ffuncres = callfunc sumfunc g c; : 24 + 12 = 36 :
    int sfuncres = callfunc divfunc g c; : 24 - 12 = 12 :

    callfunc printNum ffuncres;
    callfunc printNum sfuncres;
exit 1;
start
    int c = 24;
    int g = 12;
    
    function printNum int num; fstart
    :
        Print number to console.
        Params
            - num - number for print.
    :
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

    function printStr ptr strPtr; int size; fstart
    :
        Print string to console.
        Params 
            - ptr - Pointer to string.
            - size - String size.
    :
        syscall 4 1 strPtr size;
    fend 1;

    function sumfunc int a; int b; fstart
    fend a + b;

    function subfunc int a; int b; fstart
    fend a - b;

    int ffuncres = callfunc sumfunc g c;
    int sfuncres = callfunc subfunc c g;

    callfunc printNum ffuncres;
    callfunc printNum sfuncres;

    str printVal = "Hello from function!";
    callfunc printStr printVal 20;
exit 1;
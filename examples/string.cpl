start
    function printc char c; fstart
        str tmpBuff = " ";
        tmpBuff[0] = c;
        syscall 4 1 tmpBuff 1;
    fend 1;
    
    function prints int strPtr; int size; fstart
    :
        Print string to console.
        Params 
            - strPtr - Pointer to string.
            - size - String size.
    :
        syscall 4 1 strPtr size;
    fend 1;

    function gets int buffPtr; int size; fstart
    :
        Get input string to buffer.
        Params 
            - buffPtr - Pointer to string.
            - size - String size.
        Return input string size.
    :
    fend syscall 3 2 buffPtr size;

    function fopen int pathPtr; int flags; int mode; fstart
    :
        Open file and return fd.
        Params 
            - pathPtr - Path to file.
            - flags - Open flags.
            - mode - Open mode.
        Return fd.
    :
    fend syscall 5 pathPtr flags mode;

    function fread int fd; int buffPtr; int size; fstart
    :
        Read data from a file descriptor into a buffer.
        Params
            - fd       - File descriptor to read from.
            - buffPtr  - Pointer to buffer for storing read data.
            - size     - Number of bytes to read.
        Return number of bytes read.
    :
    fend syscall 3 fd buffPtr size;

    function fclose int fd; fstart
    :
        Close file.
        Params 
            - fd - File fd.
        Return operation result.
    :
    fend syscall 6 fd;

:
    Main logic of file processing
:

    arr loadPath 50 1 =;
    callfunc gets loadPath 50;
    int fd = callfunc fopen loadPath 0 0;

    if fd < 0; ifstart
        str error = "File not found!";
        callfunc prints error 15;
        callfunc printc 10;
        callfunc prints loadPath 13;
        callfunc printc 10;
    ifend

    if fd > 0; ifstart
        arr buffer 100 1 =;
        callfunc fread fd buffer 100;
        callfunc prints buffer 100;
        callfunc fclose fd;
    ifend
exit 1;
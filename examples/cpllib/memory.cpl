glob arr _mm_head 100000 char =;
glob long _head = 0;

function alloc_arena long size; {
    ptr int mm = _mm_head + _head;
    _head = _head + size;
    return mm;
}

function memset ptr char buffer; char val; long size; {
    long index = 0;
    while index < size; {
        buffer[index] = val;
        index = index + 1;
    }

    return 1;
}

function free_arena ; {
    _head = 0;
    return 1;
}

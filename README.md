# Cordell Compiler
-----------------------------------

Little hobby code compiler

# TODO list:
-----------------------------------
- lexic module      [V]
- syntax tree       [V]
- semantic module   [ ]
- optimisation      [ ]
- code generator    [?]



# Code example:
-----------------------------------

        start
            int a = 10 + 1 ;
            int b = 20 / 2 ;
            int c = a + b ;
            str hello = "hello there!" ;
            while c > 0 ; lstart
                c = c - 1 ;
                syscall 4 1 hello 12 ;
            lend

            arr array 7 4 = 1 2 3 4 5 6 7 ;

            str input = "         " ;
            str output = "equals 100" ;
            syscall 3 0 input 9 ;
            syscall 4 1 input 9 ;
            
            if input == 10 ; ifstart
                a = 100 ;
                if a == 100 ; ifstart
                    syscall 3 1 output 10 ;
                ifend
            ifend
        exit a ;

# Syntax:
-----------------------------------

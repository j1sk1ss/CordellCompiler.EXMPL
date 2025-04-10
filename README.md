# Cordell Compiler
-----------------------------------

Little hobby code compiler

# TODO list:
-----------------------------------
- lexic module      [V]
- syntax tree       [V]
- semantic module   [ ]
- optimisation      [ ]
- code generator    [ ]



# Code example:
-----------------------------------

      start
          int a = 10 + 1 ;
          int b = 20 / 2 ;
          int c = a + b ;
          while c > 0 ; lstart
              c = c - 1 ;
          lend
  
          arr 7 int array = 1 2 3 4 5 6 7 ;
  
          str input = 0 ;
          syscall 10 10 10 10 input ;
          if input == hello ; ifstart
              a = 100 ;
          ifend
      exit a

# Syntax:
-----------------------------------

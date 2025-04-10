nasm -f elf32 output.asm -o builds/output.o
ld -m elf_i386 builds/output.o -o builds/output
./builds/output
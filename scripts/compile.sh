#!/bin/bash
../../Kompilator/cross/bin/i686-elf-as ../source/boot.s -o ../build/boot.o
../../Kompilator/cross/bin/i686-elf-gcc -c ../source/kernel.c -o ../build/kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
../../Kompilator/cross/bin/i686-elf-gcc -T ../source/linker.ld -o ../build/mkos.bin -ffreestanding -O2 -nostdlib ../build/boot.o ../build/kernel.o -lgcc

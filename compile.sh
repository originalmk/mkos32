#!/bin/bash
../Kompilator/cross/bin/i686-elf-as boot.s -o boot.o
../Kompilator/cross/bin/i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
../Kompilator/cross/bin/i686-elf-gcc -T linker.ld -o mkos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc

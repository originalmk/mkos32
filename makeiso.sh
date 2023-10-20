#!/bin/bash
cp mkos.bin isodir/boot/mkos.bin
cp grub.cfg isodir/boot/grub/grub.cfg
grub-mkrescue -o mkos.iso isodir

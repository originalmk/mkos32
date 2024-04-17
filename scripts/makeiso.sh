#!/bin/bash
cp ../build/mkos.bin ../isodir/boot/mkos.bin
cp ../config/grub.cfg ../isodir/boot/grub/grub.cfg
grub-mkrescue -o ../build/mkos.iso ../isodir

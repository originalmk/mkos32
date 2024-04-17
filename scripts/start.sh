#!/bin/bash
./compile.sh
./makeiso.sh
# Remove lock in case of bad shutdown, it makes debugging easier
rm ../filesystem/dysk.img.lock
bochs -f ../config/bochsrc

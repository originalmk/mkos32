#!/bin/bash
./compile.sh
./makeiso.sh
# Remove lock in case of bad shutdown, it makes debugging easier
rm dysk.img.lock
bochs

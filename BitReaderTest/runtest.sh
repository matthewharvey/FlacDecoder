#!/bin/bash -e

#if there's no output, then it passed

BINFILE=input.bin

make
make -C ..
echo -ne "49485243F0AF3142E0EC5B5555" | ./writeHexToFile $BINFILE
cat $BINFILE | ./test
rm $BINFILE

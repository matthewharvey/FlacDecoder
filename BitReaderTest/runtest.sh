#!/bin/bash -e

#if there's no output, then it passed

BINFILE=input.bin

make -q
make -q -C ..
echo -ne "49485243F0AF3142E0EC5B00" | ./writeHexToFile $BINFILE
cat $BINFILE | ./test
rm $BINFILE

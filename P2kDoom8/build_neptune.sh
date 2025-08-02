#!/usr/bin/env bash

make -f Makefile.eg1 clean

make PHONE=E1 -f Makefile.eg1
make -f Makefile.eg1 clean_objs

make PHONE=C650 -f Makefile.eg1
make -f Makefile.eg1 clean_objs

md5sum *.elf

cp *.elf /home/exl/Downloads/Shared/V3x/Doom/
cp *.elf /home/exl/Storage/Projects/Git/MotoLab/simtech/arch/linux

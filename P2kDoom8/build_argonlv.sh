#!/usr/bin/env bash

make -f Makefile.eg1 clean

make ARGON=1 RES=240x160 QUALITY=MID -f Makefile.eg1
make -f Makefile.eg1 clean_objs

make ARGON=1 RES=240x320 QUALITY=MID -f Makefile.eg1
make -f Makefile.eg1 clean_objs

make ARGON=1 RES=320x240 QUALITY=MID -f Makefile.eg1
make -f Makefile.eg1 clean_objs


md5sum *.elf

#!/usr/bin/env bash

make -f Makefile.em2 clean

make RES=240x160 -f Makefile.em2
make -f Makefile.em2 clean_objs

make RES=240x320 -f Makefile.em2
make -f Makefile.em2 clean_objs

make RES=320x240 -f Makefile.em2
make -f Makefile.em2 clean_objs

make RES=176x220 -f Makefile.em2
make -f Makefile.em2 clean_objs

make RES=220x176 -f Makefile.em2
make -f Makefile.em2 clean_objs

md5sum *.elf

cp *.elf /home/exl/Downloads/Shared/V3x/Doom/

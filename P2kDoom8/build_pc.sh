#!/usr/bin/env bash

make -f Makefile.sdl2 clean

make -f Makefile.sdl2
make -f Makefile.sdl2 clean_objs

md5sum P2kDoom8

./P2kDoom8

#!/usr/bin/env bash

set -e

setup_directories() {
	echo -e "\033[32mSetup directories...\033[0m"
	rm -rf Release
	mkdir -p Release/Neptune Release/Rainbow Release/Argon
	echo -e "\033[32mSetup directories...done!\033[0m"
}

build_sdl() {
	local version=$1
	local makefile="Makefile.sdl${version}"
	echo -e "\033[32mBuilding for SDL${version}...\033[0m"
	make -f "$makefile" clean
	make -f "$makefile"
	mv P2kDoom8_SDL* "Release/" 2>/dev/null || true
	make -f "$makefile" clean
	echo -e "\033[32mBuilding for SDL${version}...done!\033[0m"
}

build_neptune() {
	echo -e "\033[32mBuilding for P2K/Neptune...\033[0m"
	for phone in "C650" "E1"; do
		make -f Makefile.eg1 clean
		PHONE="$phone" make -f Makefile.eg1
		mv *.elf "Release/Neptune" 2>/dev/null || true
		make -f Makefile.eg1 clean
	done
	echo -e "\033[32mBuilding for P2K/Neptune...done!\033[0m"
}

build_neptune_iram() {
	echo -e "\033[32mBuilding for P2K/Neptune (IRAM)...\033[0m"
	for phone in "C650" "E1"; do
		make -f Makefile.eg1 clean
		NEPTUNE=IRAM_YES PHONE="$phone" make -f Makefile.eg1
		rm -f P2kDoom8_IRAM.elf P2kDoom8_"$phone".elf
		mv *.elf "Release/Neptune" 2>/dev/null || true
		make -f Makefile.eg1 clean
	done
	echo -e "\033[32mBuilding for P2K/Neptune (IRAM)...done!\033[0m"
}

build_rainbow() {
	echo -e "\033[32mBuilding for P2K/Rainbow...\033[0m"
	for res in "240x160" "240x320" "320x240" "176x220" "220x176"; do
		make -f Makefile.em2 clean
		RES="$res" make -f Makefile.em2
		mv *.elf "Release/Rainbow" 2>/dev/null || true
		make -f Makefile.em2 clean
	done
	echo -e "\033[32mBuilding for P2K/Rainbow...done!\033[0m"
}

build_argon() {
	echo -e "\033[32mBuilding for P2K/Argon...\033[0m"
	for res in "240x160" "240x320" "320x240"; do
		make -f Makefile.ea1 clean
		RES="$res" GFX=NVIDIA QUALITY=MID make -f Makefile.ea1
		mv *.elf "Release/Argon" 2>/dev/null || true
		make -f Makefile.ea1 clean
	done
	echo -e "\033[32mBuilding for P2K/Argon...done!\033[0m"
}

get_files_info() {
	echo -e "\033[32mGenerating checksums, permissions, and file sizes...\033[0m"
	find Release -type f | sort | while read -r file; do
		md5=$(md5sum "$file" | cut -d' ' -f1)
		perm=$(stat -c %a "$file")
		perm=$(printf "%04d" "$perm")
		size=$(stat -c %s "$file")
		hrs=$(numfmt --to=iec-i --suffix=B "$size" 2>/dev/null)
		hrs="${hrs%???} ${hrs: -3}"
		printf "\nFILE:   %s\nSIZE:   %s bytes (%s)\nRIGHTS: %s\nMD5SUM: %s\n" "$file" "$size" "$hrs" "$perm" "$md5"
	done
	echo -e "\n\033[32mGenerating checksums, permissions, and file sizes...done!\033[0m"
}

usage() {
	echo "Usage: $0 [all|rainbow|neptune|neptune-iram|argon|sdl [1|2|3]]"
	echo
	echo "Examples:"
	echo "  $0                  # Build all"
	echo "  $0 all              # Build all"
	echo "  $0 rainbow          # Build Rainbow only"
	echo "  $0 neptune          # Build Neptune only"
	echo "  $0 neptune-iram     # Build Neptune IRAM only"
	echo "  $0 argon            # Build Argon only"
	echo "  $0 sdl              # Build SDL 1, 2, 3"
	echo "  $0 sdl 2            # Build SDL version 2 only"
	exit 1
}

main() {
	setup_directories

	case "$1" in
		""|"all")
			build_sdl 1
			build_sdl 2
			build_sdl 3
			build_neptune
			build_neptune_iram
			build_rainbow
			build_argon
			;;
		"rainbow")
			build_rainbow
			;;
		"neptune")
			build_neptune
			;;
		"neptune-iram")
			build_neptune_iram
			;;
		"argon")
			build_argon
			;;
		"sdl")
			if [ -z "$2" ]; then
				build_sdl 1
				build_sdl 2
				build_sdl 3
			else
				case "$2" in
					1|2|3)
						build_sdl "$2"
						;;
					*)
						usage
						;;
				esac
			fi
			;;
		*)
			usage
			;;
	esac

	get_files_info
}

main "$@"

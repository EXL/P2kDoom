#!/usr/bin/env python3

import os
import sys
import struct

def align_file(data, alignment=16, fill=0xFF):
	'''Aligns the data to the next multiple of alignment, filling with fill byte.'''
	pad_len = (alignment - (len(data) % alignment)) % alignment
	if pad_len:
		data += bytes([fill] * pad_len)
	return data

def main():
	if len(sys.argv) != 4:
		print('Usage: python postlinker.py <input.elf> <input.bin> <address_hex>')
		sys.exit(1)

	elf_filename = sys.argv[1]
	bin_filename = sys.argv[2]
	address = int(sys.argv[3], 16)

	# Read files
	with open(elf_filename, 'rb') as f:
		elf_data = bytearray(f.read())
	with open(bin_filename, 'rb') as f:
		bin_data = f.read()

	# Step 4: Replace byte at offset 0x0B with 0x02
	elf_data[0x0B] = 0x02

	# Step 5: Align ELF file by 16 bytes with 0xFF
	elf_data = align_file(elf_data, 16, 0xFF)

	# Step 6: Replace 0x0C-0x0F bytes in ELF to offset of end of aligned file (Big-Endian)
	end_offset = len(elf_data)
	elf_data[0x0C:0x10] = struct.pack('>I', end_offset)

	# Step 7: Insert address (4 bytes, Big-Endian) at end
	elf_data += struct.pack('>I', address)

	# Step 8: Insert BIN file size (4 bytes, Big-Endian)
	bin_size = len(bin_data)
	elf_data += struct.pack('>I', bin_size)

	# Step 9: Insert BIN file
	elf_data += bin_data

	# Output file (add "i" before extension)
	base, ext = os.path.splitext(elf_filename)
	patched_filename = f'{base}I{ext}'
	with open(patched_filename, 'wb') as f:
		f.write(elf_data)

	print(f'Patched ELF file \'{patched_filename}\' successfully.')

if __name__ == '__main__':
	main()

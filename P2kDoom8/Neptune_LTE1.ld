/* Use `IRAM_ADDRESS` address defined in the build system. */
IRAM_PATCH_ORIGIN = IRAM_ADDRESS;

/* Memory layout for Neptune LTE SoC by Motorola. */
MEMORY {
	IROM  (RX)  : ORIGIN = 0x00000000, LENGTH = 0x001C0000 /* 1.75 MiB     */
	IRAM  (RWX) : ORIGIN = 0x03FC0000, LENGTH = 0x00040000 /* 256  KiB     */
	ROM   (RX)  : ORIGIN = 0x10000000, LENGTH = 0x02000000 /* 32   MiB     */
	RAM   (RWX) : ORIGIN = 0x12000000, LENGTH = 0x00800000 /* 8    MiB     */

	/* Entry point origin address. */
	IRAM_PATCH (RWX) : ORIGIN = IRAM_PATCH_ORIGIN, LENGTH = ORIGIN(IRAM) - IRAM_PATCH_ORIGIN
}

/* Generate Big-Endian ARM ELFs. */
OUTPUT_FORMAT("elf32-bigarm")

/* Entry point function. */
ENTRY(IRAM_Globals_Init)

/* Combine all relevant section to one. */
SECTIONS {
	.text : {
		*(.text .text.*)
		*(.rodata .rodata.*)
		*(.data .data.*)
		*(.bss .bss.*)
		*(COMMON COMMON.*)
	} > IRAM_PATCH
}

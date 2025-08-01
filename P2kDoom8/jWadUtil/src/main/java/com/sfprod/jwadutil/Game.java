package com.sfprod.jwadutil;

import java.nio.ByteOrder;

public enum Game {
	DOOM8088("Doom8088", ByteOrder.LITTLE_ENDIAN, "DOOM1.WAD"), //
	DOOM8088_BIG_ENDIAN("Doom8088", ByteOrder.BIG_ENDIAN, "DOOM1_BE.WAD"), //
	DOOM8088_2_COLOR_TEXT_MODE("Doom8088", ByteOrder.LITTLE_ENDIAN, "DOOM2T.WAD"), //
	DOOM8088_4_COLOR("Doom8088", ByteOrder.LITTLE_ENDIAN, "DOOM4.WAD"), //
	DOOM8088_16_COLOR_DITHERED("Doom8088", ByteOrder.LITTLE_ENDIAN, "DOOM16D.WAD"), //
	DOOM8088_16_COLOR_DITHERED_TEXT_MODE("Doom8088", ByteOrder.LITTLE_ENDIAN, "DOOM16DT.WAD"), //
	DOOM8088_AMIGA_16_COLOR("Doom8088: Amiga Edition", ByteOrder.BIG_ENDIAN, "DOOMAM16.WAD"), //
	DOOM8088_ATARI_ST_2_COLOR("Doom8088: Atari ST Edition", ByteOrder.BIG_ENDIAN, "DOOMST2.WAD"), //
	DOOM8088_ATARI_ST_16_COLOR("Doom8088: Atari ST Edition", ByteOrder.BIG_ENDIAN, "DOOMST16.WAD"), //
	DOOMTD3_BIG_ENDIAN("doomtd3", ByteOrder.BIG_ENDIAN, "DOOMTD3B.WAD"), //
	DOOMTD3_LITTLE_ENDIAN("doomtd3", ByteOrder.LITTLE_ENDIAN, "DOOMTD3L.WAD"), //
	ELKSDOOM("ELKSDOOM", ByteOrder.LITTLE_ENDIAN, "elksdoom.wad");

	private final String title;
	private final ByteOrder byteOrder;
	private final String wadFile;

	Game(String title, ByteOrder byteOrder, String wadFile) {
		this.title = title;
		this.byteOrder = byteOrder;
		this.wadFile = wadFile;
	}

	public String getTitle() {
		return title;
	}

	public ByteOrder getByteOrder() {
		return byteOrder;
	}

	public String getWadFile() {
		return wadFile;
	}
}

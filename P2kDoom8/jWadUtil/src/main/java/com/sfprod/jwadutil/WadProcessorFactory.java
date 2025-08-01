package com.sfprod.jwadutil;

public interface WadProcessorFactory {

	static WadProcessor getWadProcessor(Game game, WadFile wadFile) {
		return switch (game) {
		case DOOM8088_2_COLOR_TEXT_MODE ->
			new WadProcessor2ColorsTextMode(game.getTitle(), game.getByteOrder(), wadFile);
		case DOOM8088_4_COLOR -> new WadProcessor4Colors(game.getTitle(), game.getByteOrder(), wadFile);
		case DOOM8088_AMIGA_16_COLOR ->
			new WadProcessor16ColorsDitheredAmiga(game.getTitle(), game.getByteOrder(), wadFile);
		case DOOM8088_ATARI_ST_2_COLOR -> new WadProcessor2ColorsAtariST(game.getTitle(), game.getByteOrder(), wadFile);
		case DOOM8088_ATARI_ST_16_COLOR ->
			new WadProcessor16ColorsDitheredAtariST(game.getTitle(), game.getByteOrder(), wadFile);
		case DOOM8088_16_COLOR_DITHERED ->
			new WadProcessor16ColorsDitheredPC(game.getTitle(), game.getByteOrder(), wadFile);
		case DOOM8088_16_COLOR_DITHERED_TEXT_MODE ->
			new WadProcessor16ColorsDitheredTextMode(game.getTitle(), game.getByteOrder(), wadFile);
		case DOOMTD3_BIG_ENDIAN, DOOMTD3_LITTLE_ENDIAN ->
			new WadProcessorDoomtd3(game.getTitle(), game.getByteOrder(), wadFile);
		default -> new WadProcessor(game.getTitle(), game.getByteOrder(), wadFile);
		};
	}

}

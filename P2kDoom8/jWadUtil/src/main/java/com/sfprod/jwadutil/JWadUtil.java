package com.sfprod.jwadutil;

import java.util.Arrays;

public class JWadUtil {

	public static void main(String[] args) {
		// createWad(Game.DOOMTD3_BIG_ENDIAN);
		Arrays.stream(Game.values()).forEach(JWadUtil::createWad);
	}

	static void createWad(Game game) {
		System.out.println("Creating WAD file for " + game);

		WadFile wadFile = new WadFile("/doom1.wad");

		WadProcessor wadProcessor = WadProcessorFactory.getWadProcessor(game, wadFile);
		wadProcessor.processWad();

		wadFile.saveWadFile(game.getByteOrder(), game.getWadFile());

		System.out.println();
	}

}

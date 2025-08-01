package com.sfprod.jwadutil;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.ValueSource;

/**
 * This class tests {@link WadProcessor}
 *
 */
class WadProcessorTest {

	@ParameterizedTest
	@ValueSource(ints = { 1, 2, 3, 4, 5, 6, 7, 8, 9 })
	void processBlockmap(int map) {
		String mapName = "E1M" + map;

		WadFile wadFile = new WadFile("/doom1.wad");
		int e1m1LabelPre = wadFile.getLumpNumByName(mapName);
		Lump blockmapPre = wadFile.getLumpByNum(e1m1LabelPre + 10);
		int blockmapSizePre = blockmapPre.length();
		Map<Integer, List<Short>> mapOfLinenosPre = getMapOfLinenos(blockmapPre);

		WadProcessor wadProcessor = WadProcessorFactory.getWadProcessor(Game.DOOM8088, wadFile);
		wadProcessor.processWad();

		int e1m1LabelPost = wadFile.getLumpNumByName(mapName);
		Lump blockmapPost = wadFile.getLumpByNum(e1m1LabelPost + 9);
		int blockmapSizePost = blockmapPost.length();
		Map<Integer, List<Short>> mapOfLinenosPost = getMapOfLinenos(blockmapPost);

		assertTrue(blockmapSizePost < blockmapSizePre);
		assertEquals(mapOfLinenosPre, mapOfLinenosPost);
	}

	private Map<Integer, List<Short>> getMapOfLinenos(Lump blockmap) {
		ByteBuffer blockmapByteBuffer = blockmap.dataAsByteBuffer();
		blockmapByteBuffer.getShort(); // bmaporgx
		blockmapByteBuffer.getShort(); // bmaporgy
		short bmapwidth = blockmapByteBuffer.getShort();
		short bmapheight = blockmapByteBuffer.getShort();

		List<Short> offsets = new ArrayList<>();
		for (int i = 0; i < bmapwidth * bmapheight; i++) {
			offsets.add(blockmapByteBuffer.getShort());
		}

		Map<Integer, List<Short>> mapOfLinenos = new HashMap<>();
		for (int i = 0; i < bmapwidth * bmapheight; i++) {
			short offset = offsets.get(i);
			List<Short> linenos = new ArrayList<>();
			blockmapByteBuffer.position(offset * 2);
			blockmapByteBuffer.getShort(); // always 0
			short lineno = blockmapByteBuffer.getShort();
			while (lineno != -1) {
				linenos.add(lineno);
				lineno = blockmapByteBuffer.getShort();
			}
			mapOfLinenos.put(i, linenos);
		}

		return mapOfLinenos;
	}
}

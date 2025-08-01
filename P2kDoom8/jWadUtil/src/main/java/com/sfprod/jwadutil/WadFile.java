package com.sfprod.jwadutil;

import static com.sfprod.utils.ByteBufferUtils.newByteBuffer;
import static com.sfprod.utils.ByteBufferUtils.toArray;
import static com.sfprod.utils.NumberUtils.toShort;
import static com.sfprod.utils.StringUtils.toByteArray;

import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class WadFile {

	private static final List<String> FILE_SIGNATURES = List.of("IWAD", "PWAD");

	private final List<Lump> lumps = new ArrayList<>();

	private static record Filelump(int filepos, int size, byte[] name) {
	}

	public WadFile(String wadPath) {
		ByteBuffer byteBuffer;

		try {
			byteBuffer = ByteBuffer.wrap(WadFile.class.getResourceAsStream(wadPath).readAllBytes());
			byteBuffer.order(ByteOrder.LITTLE_ENDIAN);
		} catch (IOException e) {
			throw new UncheckedIOException(e);
		}

		byte[] identification = new byte[4];
		byteBuffer.get(identification);

		String identificationAsString = new String(identification);
		if (!FILE_SIGNATURES.contains(identificationAsString)) {
			throw new IllegalArgumentException(wadPath + " is not a WAD file");
		}

		int numlumps = byteBuffer.getInt();
		int infotableofs = byteBuffer.getInt();

		byteBuffer.position(infotableofs);

		List<Filelump> filelumps = new ArrayList<>();

		for (int i = 0; i < numlumps; i++) {
			int filepos = byteBuffer.getInt();
			int size = byteBuffer.getInt();
			byte[] name = new byte[8];
			byteBuffer.get(name);
			filelumps.add(new Filelump(filepos, size, name));
		}

		for (Filelump filelump : filelumps) {
			byte[] data = new byte[filelump.size];
			byteBuffer.position(filelump.filepos());
			byteBuffer.get(data);
			lumps.add(new Lump(filelump.name(), data, ByteOrder.LITTLE_ENDIAN));
		}
	}

	public void saveWadFile(ByteOrder byteOrder, String wadPath) {
		int filepos = 4 + 4 + 4 + lumps.size() * (4 + 4 + 8);
		int filesize = filepos + lumps.stream().mapToInt(Lump::length).sum();

		ByteBuffer byteBuffer = newByteBuffer(byteOrder, filesize);

		byteBuffer.put(toByteArray("IWAD"));
		byteBuffer.putShort(toShort(lumps.size()));
		byteBuffer.putShort(toShort(0));
		byteBuffer.putInt(4 + 4 + 4);

		// WAD lump merging
		Map<String, Integer> duplicateDataMap = new HashMap<>();
		int duplicateDataCount = 0;
		for (int lumpnum = 0; lumpnum < lumps.size(); lumpnum++) {
			Lump lump = lumps.get(lumpnum);
			if (lump.length() == 0) {
				byteBuffer.putInt(0);
			} else {
				String key = Arrays.toString(lump.data());
				if (duplicateDataMap.containsKey(key)) {
					int previousfilepos = duplicateDataMap.get(key);
					byteBuffer.putInt(previousfilepos);
					Lump newLump = new Lump(lump.name(), new byte[] {}, byteOrder);
					lumps.set(lumpnum, newLump);
					duplicateDataCount++;
				} else {
					duplicateDataMap.put(key, filepos);
					byteBuffer.putInt(filepos);
					filepos += lump.length();
				}
			}

			byteBuffer.putShort(toShort(lump.length()));
			byteBuffer.putShort(toShort(0));
			byteBuffer.put(lump.name());
		}
		System.out.println("Removed " + duplicateDataCount + " duplicate lumps");

		for (Lump lump : lumps) {
			byteBuffer.put(lump.data());
		}

		Path path = Path.of("target", wadPath);
		int filesizeWithoutDuplicates = 4 + 4 + 4 + lumps.size() * (4 + 4 + 8)
				+ lumps.stream().mapToInt(Lump::length).sum();

		try {
			Files.write(path, toArray(byteBuffer, filesizeWithoutDuplicates));
		} catch (IOException e) {
			throw new UncheckedIOException(e);
		}

		System.out.println("WAD file of size " + filesizeWithoutDuplicates + " written to " + path.toAbsolutePath());
	}

	public Lump getLumpByName(String name) {
		List<Lump> lumpsWithName = lumps.stream().filter(l -> l.nameAsString().equalsIgnoreCase(name)).toList();

		if (lumpsWithName.size() != 1) {
			throw new IllegalArgumentException("Found " + lumpsWithName.size() + " lumps with the name " + name);
		}

		return lumpsWithName.get(0);
	}

	public List<Lump> getLumpsByName(String prefix) {
		return lumps.stream().filter(l -> l.nameAsString().startsWith(prefix)).toList();
	}

	public List<Lump> getLumpsBetween(String start, String end) {
		int startIndex = getLumpNumByName(start);
		int endIndex = getLumpNumByName(end);
		return lumps.subList(startIndex + 1, endIndex);
	}

	public int getLumpNumByName(String name) {
		List<Integer> lumpNums = new ArrayList<>();
		for (int index = 0; index < lumps.size(); index++) {
			Lump lump = lumps.get(index);
			if (lump.nameAsString().equalsIgnoreCase(name)) {
				lumpNums.add(index);
			}
		}

		if (lumpNums.size() != 1) {
			throw new IllegalArgumentException("Found " + lumpNums.size() + " lumps with the name " + name);
		}

		return lumpNums.get(0);
	}

	public Lump getLumpByNum(int lumpnum) {
		return lumps.get(lumpnum);
	}

	public void replaceLump(int lumpnum, Lump newLump) {
		lumps.set(lumpnum, newLump);
	}

	public void replaceLump(Lump newLump) {
		int lumpnum = getLumpNumByName(newLump.nameAsString());
		lumps.set(lumpnum, newLump);
	}

	public void removeLump(int lumpnum) {
		lumps.remove(lumpnum);
	}

	public void removeLump(Lump lump) {
		removeLump(lump.nameAsString());
	}

	public void removeLump(String lumpname) {
		int index = 0;
		for (Lump lump : lumps) {
			if (lump.nameAsString().equals(lumpname)) {
				break;
			}
			index++;
		}
		lumps.remove(index);
	}

	public void removeLumps(String prefix) {
		lumps.removeIf(l -> l.nameAsString().startsWith(prefix));
	}

	public void addLump(Lump lump) {
		lumps.add(lump);
	}

}

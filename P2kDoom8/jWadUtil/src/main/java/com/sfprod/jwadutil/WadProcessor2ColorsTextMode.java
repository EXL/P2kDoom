package com.sfprod.jwadutil;

import static com.sfprod.utils.NumberUtils.toByte;
import static com.sfprod.utils.NumberUtils.toInt;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.sfprod.utils.ByteBufferUtils;

class WadProcessor2ColorsTextMode extends WadProcessor {

	private static final List<Integer> MDA_FONT_BIT_COUNTS = List.of( //
			0, 34, 60, 41, 25, 36, 42, 12, 100, 20, 92, 33, 32, 32, 48, 34, //
			29, 29, 30, 32, 47, 44, 21, 36, 24, 24, 15, 15, 13, 20, 31, 31, //
			0, 22, 14, 42, 41, 21, 37, 8, 18, 18, 24, 20, 8, 8, 4, 14, //
			44, 25, 30, 30, 32, 32, 32, 25, 39, 33, 8, 10, 18, 12, 18, 23, //
			43, 35, 41, 28, 38, 38, 32, 33, 39, 22, 26, 38, 28, 44, 45, 34, //
			33, 42, 40, 33, 32, 37, 34, 44, 34, 30, 36, 22, 21, 22, 12, 8, //
			6, 24, 32, 22, 32, 27, 27, 32, 33, 19, 27, 32, 21, 37, 25, 26, //
			30, 30, 22, 24, 24, 25, 22, 32, 22, 30, 26, 21, 16, 21, 10, 23, //
			35, 33, 33, 32, 32, 30, 34, 26, 35, 35, 33, 23, 25, 21, 39, 40, //
			37, 35, 41, 34, 34, 32, 35, 31, 38, 38, 37, 32, 33, 36, 46, 34, //
			30, 21, 32, 31, 35, 51, 23, 19, 23, 13, 13, 39, 43, 22, 20, 20, //
			28, 56, 84, 28, 31, 34, 58, 31, 24, 58, 56, 39, 35, 35, 22, 17, //
			20, 23, 21, 32, 9, 35, 36, 58, 35, 39, 37, 41, 58, 18, 60, 28, //
			37, 30, 33, 35, 24, 26, 31, 61, 42, 19, 18, 126, 63, 56, 70, 63, //
			28, 34, 27, 31, 32, 25, 27, 20, 36, 37, 37, 31, 24, 38, 23, 33, //
			21, 28, 20, 20, 29, 27, 16, 20, 14, 4, 2, 30, 24, 19, 30, 0);

	private static final byte[] COLORS_WALLS = toByteArray(0, 176, 179, 177, 186, 221, 222, 178, 219);
	private static final byte[] COLORS_SPRITES = toByteArray(255, 250, 249, 124, 180, 195, 197, 182, 199, 215, 10, 8);

	public static final byte[] COLORS_FLOORS = toByteArray(32, 46, 96, 39, 44, 45, 58, 95, 196, 59, 126, 7, 61, 94, 28,
			169, 170, 34, 47, 248, 26, 27, 246, 191, 60, 62, 218, 217, 9, 29, 43, 174, 175, 192, 231, 247, 22, 37, 92,
			108, 194, 240, 73, 91, 93, 99, 120, 127, 193, 238, 24, 25, 42, 236, 252, 4, 111, 226, 245, 67, 224, 16, 17,
			244, 145, 211, 5, 157, 254, 30, 31, 214, 227, 155, 228, 239, 15, 79, 88, 232, 85, 233, 237, 72, 3, 36, 6,
			35, 21, 220, 223);

	private static final Map<Integer, List<Integer>> COLORS_SHUFFLE_MAP = createColorsShuffleMap();

	private final List<Byte> lookupTableWalls;
	private final List<Byte> lookupTableSprites;

	WadProcessor2ColorsTextMode(String title, ByteOrder byteOrder, WadFile wadFile) {
		super(title, byteOrder, wadFile, new MapProcessor2ColorsTextMode(byteOrder, wadFile));

		wadFile.replaceLump(new Lump("TITLEPIC", getLump("TP80X25M").data(), ByteBufferUtils.DONT_CARE));
		wadFile.replaceLump(new Lump("WIMAP0", getLump("WM80X25M").data(), ByteBufferUtils.DONT_CARE));
		wadFile.replaceLump(new Lump("HELP2", getLump("HL80X25M").data(), ByteBufferUtils.DONT_CARE));

		List<Double> grays = vgaColors.stream().map(Color::gray).toList();
		List<Double> sortedGrays = grays.stream().sorted().toList();

		this.lookupTableWalls = createLookupTable(grays, sortedGrays, COLORS_WALLS);
		this.lookupTableSprites = createLookupTable(grays, sortedGrays, COLORS_SPRITES);
	}

	private List<Byte> createLookupTable(List<Double> grays, List<Double> sortedGrays, byte[] colors) {
		double[] bucketLimits = new double[colors.length];
		double fracstep = 256 / colors.length;
		double frac = fracstep;
		for (int i = 0; i < colors.length - 1; i++) {
			bucketLimits[i] = sortedGrays.get(((int) frac));
			frac += fracstep;
		}
		bucketLimits[colors.length - 1] = Double.MAX_VALUE;

		List<Byte> lut = new ArrayList<>();
		for (Double gray : grays) {
			int bucket = 0;
			while (gray >= bucketLimits[bucket]) {
				bucket++;
			}
			lut.add(colors[bucket]);
		}
		return lut;
	}

	private static Map<Integer, List<Integer>> createColorsShuffleMap(byte[] colors) {
		Map<Integer, List<Integer>> shuffleMap = new HashMap<>();

		for (byte colorWallsI : colors) {
			int i = toInt(colorWallsI);
			List<Integer> sameColorList = new ArrayList<>();
			int bitCount = MDA_FONT_BIT_COUNTS.get(i);
			for (byte colorWallsJ : colors) {
				int j = toInt(colorWallsJ);
				int otherBitCount = MDA_FONT_BIT_COUNTS.get(j);
				if (bitCount == otherBitCount) {
					sameColorList.add(j);
				}
			}

			shuffleMap.put(i, sameColorList);
		}

		return shuffleMap;
	}

	private static Map<Integer, List<Integer>> createColorsShuffleMap() {
		Map<Integer, List<Integer>> shuffleMap = new HashMap<>(Collections.emptyMap());
		shuffleMap.putAll(createColorsShuffleMap(COLORS_WALLS));
		shuffleMap.putAll(createColorsShuffleMap(COLORS_SPRITES));
		return shuffleMap;
	}

	private static byte[] toByteArray(int... colors) {
		byte[] result = new byte[colors.length];
		for (int i = 0; i < colors.length; i++) {
			result[i] = toByte(colors[i]);
		}
		return result;
	}

	@Override
	void changeColors() {
		// Raw graphics
		// Finale background flat
		changePaletteRaw(wadFile.getLumpByName("FLOOR4_8"));

		// Graphics in picture format
		List<String> switchesAndDoors = List.of("SW1S0", "SW1S1", "SW2S0", "SW2S1", "SW3S0", "SW3S1", "SW4S0", "SW4S1",
				"WARNA0", "WARNB0", "DOOR2_1", "DOOR2_4", "DOOR2_5", "DOOR3_6", "DOOR9_1");
		// Sprites
		List<Lump> sprites = new ArrayList<>(256);
		sprites.addAll(switchesAndDoors.stream().map(wadFile::getLumpByName).toList());
		sprites.addAll(wadFile.getLumpsBetween("S_START", "S_END"));
		sprites.forEach(this::changePaletteSprites);
		// Walls
		wadFile.getLumpsBetween("P1_START", "P1_END").stream().filter(l -> !switchesAndDoors.contains(l.nameAsString()))
				.forEach(this::changePaletteWalls);
	}

	private byte convert256to2(List<Byte> lookupTable, byte b) {
		return lookupTable.get(toInt(b));
	}

	private void changePaletteRaw(Lump lump) {
		for (int i = 0; i < lump.length(); i++) {
			lump.data()[i] = convert256to2(lookupTableWalls, lump.data()[i]);
		}
	}

	private void changePaletteSprites(Lump lump) {
		changePalettePicture(lookupTableSprites, lump);
	}

	private void changePaletteWalls(Lump lump) {
		changePalettePicture(lookupTableWalls, lump);
	}

	private void changePalettePicture(List<Byte> lookupTable, Lump lump) {
		ByteBuffer dataByteBuffer = lump.dataAsByteBuffer();
		short width = dataByteBuffer.getShort();
		dataByteBuffer.getShort(); // height
		dataByteBuffer.getShort(); // leftoffset
		dataByteBuffer.getShort(); // topoffset

		List<Integer> columnofs = new ArrayList<>();
		for (int columnof = 0; columnof < width; columnof++) {
			columnofs.add(dataByteBuffer.getInt());
		}

		for (int columnof = 0; columnof < width; columnof++) {
			int index = columnofs.get(columnof);
			byte topdelta = lump.data()[index];
			index++;
			while (topdelta != -1) {
				byte lengthByte = lump.data()[index];
				index++;
				int length = toInt(lengthByte);
				for (int i = 0; i < length + 2; i++) {
					lump.data()[index] = convert256to2(lookupTable, lump.data()[index]);
					index++;
				}
				topdelta = lump.data()[index];
				index++;
			}
		}
	}

	@Override
	void processColormap() {
		List<Byte> colormapInvulnerability = createColormapInvulnerability();

		for (int gamma = 0; gamma < 6; gamma++) {
			Lump colormapLump;
			if (gamma == 0) {
				colormapLump = wadFile.getLumpByName("COLORMAP");
			} else {
				colormapLump = new Lump("COLORMP" + gamma, new byte[34 * 256], ByteBufferUtils.DONT_CARE);
				wadFile.addLump(colormapLump);
			}

			int index = 0;

			// colormap 0-31 from bright to dark
			int colormap = 0 - (int) (gamma * (32.0 / 5));
			for (int i = 0; i < 32; i++) {
				List<Byte> colormapBytes = createColormap(colormap);
				for (byte b : colormapBytes) {
					colormapLump.data()[index] = b;
					index++;
				}
				colormap++;
			}

			// colormap 32 invulnerability powerup
			for (int i = 0; i < 256; i++) {
				colormapLump.data()[index] = colormapInvulnerability.get(i);
				index++;
			}

			// colormap 33 all black
			for (int i = 0; i < 256; i++) {
				colormapLump.data()[index] = 0;
				index++;
			}
		}
	}

	private List<Byte> createColormap(int colormap) {
		List<Byte> result = new ArrayList<>();

		for (int i = 0; i < 256; i++) {
			result.add(toByte(i));
		}

		if (colormap != 0) {
			int c = 32 - colormap;

			for (int color = 0; color < COLORS_FLOORS.length; color++) {
				int newcolor = Math.clamp(color * c / 32, 0, COLORS_FLOORS.length - 1);
				result.set(toInt(COLORS_FLOORS[color]), COLORS_FLOORS[newcolor]);
			}
			for (int color = 0; color < COLORS_WALLS.length; color++) {
				int newcolor = Math.clamp(color * c / 32, 0, COLORS_WALLS.length - 1);
				result.set(toInt(COLORS_WALLS[color]), COLORS_WALLS[newcolor]);
			}
			for (int color = 0; color < COLORS_SPRITES.length; color++) {
				int newcolor = Math.clamp(color * c / 32, 0, COLORS_SPRITES.length - 1);
				result.set(toInt(COLORS_SPRITES[color]), COLORS_SPRITES[newcolor]);
			}
		}

		result.set(205, toByte(205));
		result.set(207, toByte(207));
		result.set(209, toByte(209));

		return result;
	}

	private List<Byte> createColormapInvulnerability() {
		List<Byte> colormapInvulnerability = new ArrayList<>();

		for (int i = 0; i < 256; i++) {
			colormapInvulnerability.add(toByte(i));
		}

		for (int i = 0; i < COLORS_FLOORS.length; i++) {
			colormapInvulnerability.set(toInt(COLORS_FLOORS[i]), COLORS_FLOORS[COLORS_FLOORS.length - i - 1]);
		}
		for (int i = 0; i < COLORS_WALLS.length; i++) {
			colormapInvulnerability.set(toInt(COLORS_WALLS[i]), COLORS_WALLS[COLORS_WALLS.length - i - 1]);
		}
		for (int i = 0; i < COLORS_SPRITES.length; i++) {
			colormapInvulnerability.set(toInt(COLORS_SPRITES[i]), COLORS_SPRITES[COLORS_SPRITES.length - i - 1]);
		}

		colormapInvulnerability.set(205, toByte(209));
		colormapInvulnerability.set(207, toByte(207));
		colormapInvulnerability.set(209, toByte(205));

		return colormapInvulnerability;
	}

	private byte shuffleColor(byte b) {
		List<Integer> list = COLORS_SHUFFLE_MAP.get(toInt(b));
		return toByte(list.get(random.nextInt(list.size())));
	}

	@Override
	void shuffleColors() {
		// Raw graphics
		// Flat
		shuffleColorsRaw(wadFile.getLumpByName("FLOOR4_8"));

		// Graphics in picture format
		// Walls
		wadFile.getLumpsBetween("P1_START", "P1_END").forEach(this::shuffleColorPicture);
	}

	private void shuffleColorsRaw(Lump lump) {
		for (int i = 0; i < lump.length(); i++) {
			lump.data()[i] = shuffleColor(lump.data()[i]);
		}
	}

	private void shuffleColorPicture(Lump lump) {
		ByteBuffer dataByteBuffer = lump.dataAsByteBuffer();
		short width = dataByteBuffer.getShort();
		dataByteBuffer.getShort(); // height
		dataByteBuffer.getShort(); // leftoffset
		dataByteBuffer.getShort(); // topoffset

		List<Integer> columnofs = new ArrayList<>();
		for (int columnof = 0; columnof < width; columnof++) {
			columnofs.add(dataByteBuffer.getInt());
		}

		for (int columnof = 0; columnof < width; columnof++) {
			int index = columnofs.get(columnof);
			byte topdelta = lump.data()[index];
			index++;
			while (topdelta != -1) {
				byte lengthByte = lump.data()[index];
				index++;
				int length = toInt(lengthByte);
				for (int i = 0; i < length + 2; i++) {
					lump.data()[index] = shuffleColor(lump.data()[index]);
					index++;
				}
				topdelta = lump.data()[index];
				index++;
			}
		}
	}

	@Override
	protected void removeUnusedLumps() {
		super.removeUnusedLumps();

		wadFile.removeLumps("PLAYPAL");

		// Menu graphics
		wadFile.removeLumps("M_");

		// Status bar graphics
		List<Lump> stLumps = wadFile.getLumpsByName("ST");
		stLumps.stream().filter(l -> !("STIMA0".equals(l.nameAsString()) || l.nameAsString().startsWith("STEP")))
				.forEach(wadFile::removeLump);

		// Intermission screen graphics
		List<Lump> wiLumps = wadFile.getLumpsByName("WI");
		wiLumps.stream().filter(l -> !"WIMAP0".equals(l.nameAsString())).forEach(wadFile::removeLump);
	}

}

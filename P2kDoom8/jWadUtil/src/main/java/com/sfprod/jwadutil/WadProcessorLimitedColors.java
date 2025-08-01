package com.sfprod.jwadutil;

import static com.sfprod.utils.NumberUtils.toByte;
import static com.sfprod.utils.NumberUtils.toInt;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Function;
import java.util.stream.Collectors;

import com.sfprod.utils.ByteBufferUtils;
import com.sfprod.utils.NumberUtils;

abstract class WadProcessorLimitedColors extends WadProcessor {

	private final List<Integer> grayscaleFromDarkToBright;
	private final int divisor;

	private Map<Integer, List<Integer>> availableColorsShuffleMap;

	private List<Integer> vga256ToDitheredLUT;

	WadProcessorLimitedColors(String title, ByteOrder byteOrder, WadFile wadFile,
			List<Integer> grayscaleFromDarkToBright, int divisor) {
		super(title, byteOrder, wadFile);
		this.grayscaleFromDarkToBright = grayscaleFromDarkToBright;
		this.divisor = divisor;
	}

	protected void fillAvailableColorsShuffleMap(List<Color> colors) {
		setAvailableColors(colors);

		Map<Integer, List<Integer>> shuffleMap = new HashMap<>();
		for (int i = 0; i < 256; i++) {
			List<Integer> sameColorList = new ArrayList<>();
			Color availableColor = availableColors.get(i);
			for (int j = 0; j < 256; j++) {
				Color otherColor = availableColors.get(j);
				if (availableColor.equals(otherColor)) {
					sameColorList.add(j);
				}
			}

			shuffleMap.put(i, sameColorList);
		}

		this.availableColorsShuffleMap = shuffleMap;

		this.vga256ToDitheredLUT = createVga256ToDitheredLUT(vgaColors, availableColors);
	}

	protected abstract List<Integer> createVga256ToDitheredLUT(List<Color> vgaCols, List<Color> availableCols);

	protected byte convert256to16dithered(byte b) {
		return toByte(vga256ToDitheredLUT.get(toInt(b)));
	}

	@Override
	void changeColors() {
		// Raw graphics
		List<Lump> rawGraphics = new ArrayList<>();
		rawGraphics.add(wadFile.getLumpByName("HELP2"));
		rawGraphics.add(wadFile.getLumpByName("STBAR"));
		rawGraphics.add(wadFile.getLumpByName("TITLEPIC"));
		rawGraphics.add(wadFile.getLumpByName("WIMAP0"));
		// Finale background flat
		rawGraphics.add(wadFile.getLumpByName("FLOOR4_8"));
		rawGraphics.forEach(this::changePaletteRaw);

		// Graphics in picture format

		List<Lump> spritesAndWallsGraphics = new ArrayList<>(256);
		// Sprites
		spritesAndWallsGraphics.addAll(wadFile.getLumpsBetween("S_START", "S_END"));
		// Walls
		spritesAndWallsGraphics.addAll(wadFile.getLumpsBetween("P1_START", "P1_END"));

		spritesAndWallsGraphics.forEach(this::changePaletteSpritesAndWalls);

		List<Lump> statusBarMenuAndIntermissionGraphics = new ArrayList<>(256);
		// Status bar
		statusBarMenuAndIntermissionGraphics.addAll(wadFile.getLumpsByName("STC"));
		statusBarMenuAndIntermissionGraphics.addAll(wadFile.getLumpsByName("STF"));
		statusBarMenuAndIntermissionGraphics.addAll(wadFile.getLumpsByName("STG"));
		statusBarMenuAndIntermissionGraphics.addAll(wadFile.getLumpsByName("STK"));
		statusBarMenuAndIntermissionGraphics.addAll(wadFile.getLumpsByName("STY"));
		// Menu
		statusBarMenuAndIntermissionGraphics.addAll(wadFile.getLumpsByName("M_"));
		// Intermission
		statusBarMenuAndIntermissionGraphics
				.addAll(wadFile.getLumpsByName("WI").stream().filter(l -> !"WIMAP0".equals(l.nameAsString())).toList());

		statusBarMenuAndIntermissionGraphics.forEach(this::changePaletteStatusBarMenuAndIntermission);
	}

	protected abstract void changePaletteRaw(Lump lump);

	protected abstract byte convert256to16(byte b);

	private void changePaletteSpritesAndWalls(Lump lump) {
		changePalettePicture(lump, this::convert256to16dithered);
	}

	private void changePaletteStatusBarMenuAndIntermission(Lump lump) {
		changePalettePicture(lump, this::convert256to16);
	}

	protected void changePalettePicture(Lump lump, Function<Byte, Byte> colorConvertFunction) {
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
					lump.data()[index] = colorConvertFunction.apply(lump.data()[index]);
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

	private List<Byte> createColormapInvulnerability() {
		List<Double> grays = availableColors.stream().map(Color::gray).collect(Collectors.toSet()).stream()
				.sorted(Comparator.reverseOrder()).toList();

		return availableColors.stream().mapToDouble(Color::gray).mapToInt(grays::indexOf).map(i -> i / divisor)
				.map(grayscaleFromDarkToBright::get).mapToObj(NumberUtils::toByte).toList();
	}

	private List<Byte> createColormap(int colormap) {
		List<Byte> result = new ArrayList<>();

		if (colormap == 0) {
			for (int i = 0; i < 256; i++) {
				result.add(toByte(i));
			}
		} else {
			int c = 32 - colormap;

			for (Color color : availableColors) {
				int r = Math.clamp((long) Math.sqrt(color.r() * color.r() * c / 32), 0, 255);
				int g = Math.clamp((long) Math.sqrt(color.g() * color.g() * c / 32), 0, 255);
				int b = Math.clamp((long) Math.sqrt(color.b() * color.b() * c / 32), 0, 255);

				byte closestColor = calculateClosestColor(new Color(r, g, b));
				byte shuffledColor = shuffleColor(closestColor);
				result.add(shuffledColor);
			}
		}

		return result;
	}

	private byte calculateClosestColor(Color c) {
		int closestColor = -1;
		int closestDist = Integer.MAX_VALUE;

		for (int i = 0; i < 256; i++) {
			int dist = c.calculateDistance(availableColors.get(i));
			if (dist == 0) {
				// perfect match
				closestColor = i;
				break;
			}

			if (dist < closestDist) {
				closestDist = dist;
				closestColor = i;
			}
		}

		return toByte(closestColor);
	}

	@Override
	void shuffleColors() {
		// Graphics in picture format
		// Walls
		wadFile.getLumpsBetween("P1_START", "P1_END").forEach(this::shuffleColorPicture);
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

	protected byte shuffleColor(byte b) {
		List<Integer> list = availableColorsShuffleMap.get(toInt(b));
		return toByte(list.get(random.nextInt(list.size())));
	}

	@Override
	protected void removeUnusedLumps() {
		super.removeUnusedLumps();

		wadFile.removeLump("PLAYPAL");
		wadFile.removeLump("PLAYPAL1");
		wadFile.removeLump("PLAYPAL2");
		wadFile.removeLump("PLAYPAL3");
		wadFile.removeLump("PLAYPAL4");
		wadFile.removeLump("PLAYPAL5");
	}
}

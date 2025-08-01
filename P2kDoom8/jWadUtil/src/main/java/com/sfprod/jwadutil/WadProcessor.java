package com.sfprod.jwadutil;

import static com.sfprod.utils.ByteBufferUtils.newByteBuffer;
import static com.sfprod.utils.NumberUtils.toInt;
import static com.sfprod.utils.StringUtils.toByteArray;

import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.stream.Stream;

import com.sfprod.utils.ByteBufferUtils;

public class WadProcessor {

	public static final boolean FLAT_SPAN = true;

	private static final String CREDITS = """
			${title} by Frenkel Smeijers
			based on
			GBA PrBoom port created by doomhack
			SVN by Kippykip
			Doom II status bar by Torus Games""".replace("\n", "\r\n");

	static final short DIVISORS[] = { //
			0, //
			6818, 6628, 6449, 6279, 6087, 5906, 5736, 5575, //
			5423, 5279, 5120, 4971, 4830, 4697, 4554, 4435, //
			4307, 4186, 4058, 3950, 3836, 3728, 3615, 3519, //
			3418, 3323, 3224, 3131, 3043, 2960, 2875, 2794, //
			2711, 2633, 2560, 2485, 2415, 2348, 2281, 2213, //
			2153, 2089, 2032, 1975, 1918, 1864, 1810, 1757, //
			1709, 1659, 1612, 1565, 1521, 1478, 1435, 1395, //
			1355, 1316, 1280, 1242, 1207, 1173, 1140, 1107, //
			1075, 1045, 1015, 986, 959, 931, 905, 879, //
			854, 829, 806, 783, 760, 739, 718, 697, //
			677, 658, 640, 621, 604, 586, 570, 553, //
			538, 522, 507, 493, 479, 465, 452, 439, //
			427, 415, 403, 391, 380, 369, 359, 348, //
			339, 329, 319, 310, 302, 293, 285, 276, //
			269, 261, 253, 246, 239, 232, 226, 219, //
			213, 207, 201, 195, 190, 184, 179 //
	};

	final Random random = new Random(0x1d4a11);

	final ByteOrder byteOrder;
	final WadFile wadFile;
	private final MapProcessor mapProcessor;
	final List<Color> vgaColors;
	List<Color> availableColors;

	protected WadProcessor(String title, ByteOrder byteOrder, WadFile wadFile) {
		this(title, byteOrder, wadFile, new MapProcessor(byteOrder, wadFile));
	}

	protected WadProcessor(String title, ByteOrder byteOrder, WadFile wadFile, MapProcessor mapProcessor) {
		this.byteOrder = byteOrder;
		this.wadFile = wadFile;
		this.mapProcessor = mapProcessor;
		this.vgaColors = createVgaColors(wadFile);
		setAvailableColors(vgaColors);

		wadFile.addLump(getLump("M_ARUN"));
		wadFile.addLump(getLump("M_GAMMA"));
		wadFile.addLump(getLump("STGANUM0"));
		wadFile.addLump(getLump("STGANUM1"));
		wadFile.addLump(getLump("STGANUM2"));
		wadFile.addLump(getLump("STGANUM3"));
		wadFile.addLump(getLump("STGANUM4"));
		wadFile.addLump(getLump("STGANUM5"));
		wadFile.addLump(getLump("STGANUM6"));
		wadFile.addLump(getLump("STGANUM7"));
		wadFile.addLump(getLump("STGANUM8"));
		wadFile.addLump(getLump("STGANUM9"));
		wadFile.addLump(getLump("PLAYPAL1"));
		wadFile.addLump(getLump("PLAYPAL2"));
		wadFile.addLump(getLump("PLAYPAL3"));
		wadFile.addLump(getLump("PLAYPAL4"));
		wadFile.addLump(getLump("PLAYPAL5"));

		wadFile.addLump(
				new Lump("CREDITS", toByteArray(CREDITS.replace("${title}", title)), ByteBufferUtils.DONT_CARE));

		wadFile.replaceLump(getLump("HELP2"));
		wadFile.replaceLump(getLump("STBAR"));
		wadFile.replaceLump(getLump("TITLEPIC"));
		wadFile.replaceLump(getLump("WIMAP0"));
	}

	static List<Color> createVgaColors(WadFile wadFile) {
		Lump playpal = wadFile.getLumpByName("PLAYPAL");
		ByteBuffer bb = playpal.dataAsByteBuffer();
		List<Color> vgaColors = new ArrayList<>();
		for (int i = 0; i < 256; i++) {
			int r = toInt(bb.get());
			int g = toInt(bb.get());
			int b = toInt(bb.get());
			vgaColors.add(new Color(r, g, b));
		}
		return Collections.unmodifiableList(vgaColors);
	}

	protected Lump getLump(String lumpname) {
		try {
			byte[] data = JWadUtil.class.getResourceAsStream('/' + lumpname + ".LMP").readAllBytes();
			return new Lump(lumpname, data, ByteOrder.LITTLE_ENDIAN);
		} catch (IOException e) {
			throw new UncheckedIOException(e);
		}
	}

	protected void setAvailableColors(List<Color> availableColors) {
		this.availableColors = Collections.unmodifiableList(availableColors);
	}

	public void processWad() {
		processTexture1();
		processPNames();
		mapProcessor.processMaps(availableColors);
		changeColors();
		processColormap();
		processPlayerSprites();
		removeUnusedLumps();
		processPcSpeakerSoundEffects();
		processSprites();
		processWalls();
		compressPictures();

		shuffleColors();
		processRawGraphics();
	}

	void changeColors() {
	}

	void processColormap() {
	}

	void shuffleColors() {
	}

	void processRawGraphics() {
	}

	/**
	 * Remove unused bytes
	 */
	private void processTexture1() {
		Lump texture1 = wadFile.getLumpByName("TEXTURE1");
		ByteBuffer oldbb = texture1.dataAsByteBuffer();
		int numtextures = oldbb.getInt();
		List<Integer> oldoffsets = new ArrayList<>();
		for (int i = 0; i < numtextures; i++) {
			oldoffsets.add(oldbb.getInt());
		}

		List<Maptexture> textures = new ArrayList<>();
		for (int offset : oldoffsets) {
			oldbb.position(offset);
			byte[] name = new byte[8];
			oldbb.get(name);
			int masked = oldbb.getInt();
			short width = oldbb.getShort();
			short height = oldbb.getShort();
			int columndirectory = oldbb.getInt();
			short patchcount = oldbb.getShort();

			List<Mappatch> patches = new ArrayList<>();
			for (int i = 0; i < patchcount; i++) {
				short originx = oldbb.getShort();
				short originy = oldbb.getShort();
				short patch = oldbb.getShort();
				short stepdir = oldbb.getShort();
				short colormap = oldbb.getShort();
				patches.add(new Mappatch(originx, originy, patch, stepdir, colormap));
			}

			textures.add(new Maptexture(name, masked, width, height, columndirectory, patchcount, patches));
		}

		ByteBuffer newbb = newByteBuffer(byteOrder);
		newbb.putInt(numtextures);

		// temp offset values
		for (int i = 0; i < numtextures; i++) {
			newbb.putInt(-1);
		}

		List<Integer> newoffsets = new ArrayList<>();
		for (int i = 0; i < numtextures; i++) {
			newoffsets.add(newbb.position());

			Maptexture texture = textures.get(i);
			newbb.put(texture.name());
			newbb.putShort(texture.width());
			newbb.putShort(texture.height());
			newbb.putShort(texture.patchcount());

			for (Mappatch patch : texture.patches()) {
				newbb.putShort(patch.originx());
				newbb.putShort(patch.originy());
				newbb.putShort(patch.patch());
			}
		}

		int newsize = newbb.position();

		newbb.position(4);
		for (int newoffset : newoffsets) {
			newbb.putInt(newoffset);
		}

		Lump newLump = new Lump(texture1.name(), newsize, newbb);
		wadFile.replaceLump(newLump);
	}

	/**
	 * Capitalize patch names
	 *
	 */
	private void processPNames() {
		Lump oldPNames = wadFile.getLumpByName("PNAMES");
		ByteBuffer oldData = oldPNames.dataAsByteBuffer();
		ByteBuffer newData = newByteBuffer(byteOrder, oldPNames.length());
		int nummappatches = oldData.getInt();
		newData.putInt(nummappatches);

		for (int i = 0; i < nummappatches * 8; i++) {
			byte b = oldData.get();
			if ('a' <= b && b <= 'z') {
				b &= 0b11011111;
			}
			newData.put(b);
		}

		Lump newPNames = new Lump(oldPNames.name(), newData);
		wadFile.replaceLump(newPNames);
	}

	/**
	 * Replace player sprites by zombieman sprites
	 *
	 */
	private void processPlayerSprites() {
		List<Lump> playerSprites = wadFile.getLumpsByName("PLAY");
		List<Lump> zombiemanSprites = wadFile.getLumpsByName("POSS");

		List<String> skip = List.of( //
				"PLAYPAL", "PLAYPAL1", "PLAYPAL2", "PLAYPAL3", "PLAYPAL4", "PLAYPAL5", //
				"PLAYN0", "PLAYW0", "PLAYV0");
		for (Lump playerSprite : playerSprites) {
			if (!skip.contains(playerSprite.nameAsString())) {
				String suffix = playerSprite.nameAsString().substring(4);
				Lump zombiemanSprite = zombiemanSprites.stream().filter(z -> z.nameAsString().endsWith(suffix))
						.findAny().orElseThrow();
				Lump newLump = new Lump(playerSprite.name(), zombiemanSprite.data(), zombiemanSprite.byteOrder());
				wadFile.replaceLump(newLump);
			}
		}

		Lump playerBloodyMess = playerSprites.stream().filter(p -> "PLAYV0".equals(p.nameAsString())).findAny()
				.orElseThrow();
		Lump zombieBloodyMess = zombiemanSprites.stream().filter(z -> "POSSU0".equals(z.nameAsString())).findAny()
				.orElseThrow();
		Lump newBloodyMessLump = new Lump(playerBloodyMess.name(), zombieBloodyMess.data(),
				zombieBloodyMess.byteOrder());
		wadFile.replaceLump(newBloodyMessLump);
	}

	/**
	 * Remove unused lumps
	 *
	 */
	protected void removeUnusedLumps() {
		wadFile.removeLump("CREDIT"); // Credits screen
		wadFile.removeLump("SW18_7"); // Duplicate wall texture

		Stream.of( //
				"AMMNUM", // Status bar numbers
				"APBX", // Arachnotron projectile in flight
				"APLS", // Arachnotron projectile impact
				"BAL2", // Cacodemon projectile
				"BOSF", // Spawn cube
				"BRDR", // Border
				"D_", // MUS music
				"DEMO1", // Demo 1
				"DEMO2", // Demo 2
				"DPBD", // Blazing door sound effects
				"DPITMBK", // Item respawn sound effect in multiplayer mode
				"DS", // Sound Blaster sound effects
				"DMXGUS", // Gravis UltraSound instrument data
				"GENMIDI", // OPL instrument data
				"HELP1", // Help screen
				"IFOG", // Item respawn blue fog in multiplayer mode
				"MANF", // Mancubus projectile
				"M_DETAIL", // Graphic Detail:
				"M_DIS", // Display
				"M_EPI", // Episode names
				"M_GD", // Graphic detail high and low
				"M_LGTTL", // Load game
				"M_MSENS", // Mouse sensitivity
				"M_PAUSE", // Pause
				"M_RDTHIS", // Read This!
				"M_SCRNSZ", // Screen Size
				"M_SGTTL", // Save game
				"STARMS", // Status bar arms
				"STCDROM", // Loading icon CD-ROM
				"STCFN121", // letter
				"STDISK", // Loading icon Disk
				"STFB", // Status bar face background
				"STKEYS3", // Status bar blue skull key
				"STKEYS4", // Status bar yellow skull key
				"STKEYS5", // Status bar red skull key
				"STPB", // Status bar p? background
				"STT", // Status bar numbers
				"VERTEXES", // vertexes for a map
				"WIA", // Intermission animations
				"WIBP", // P1 - P4
				"WIFRGS", // Frgs
				"WIKILRS", // Killers
				"WILV1", // Episode 2 level names
				"WILV2", // Episode 3 level names
				"WIMINUS", // Minus
				"WIMSTAR", // You
				"WIOSTF", // F.
				"WIOSTS", // Scrt
				"WIP1", // P1
				"WIP2", // P2
				"WIP3", // P3
				"WIP4", // P4
				"WIURH1", // You are here
				"WIVCTMS" // Victims
		).forEach(prefix -> wadFile.removeLumps(prefix));

		if (FLAT_SPAN) {
			List<Lump> flats = new ArrayList<>();
			flats.add(wadFile.getLumpByName("F_START"));
			flats.add(wadFile.getLumpByName("F_END"));
			flats.addAll(wadFile.getLumpsBetween("F_START", "F_END"));
			flats.stream().filter(f -> !"FLOOR4_8".equals(f.nameAsString())).forEach(wadFile::removeLump);
		}
	}

	private void processPcSpeakerSoundEffects() {
		List<Lump> lumps = wadFile.getLumpsByName("DP");
		lumps.stream().map(this::processPcSpeakerSoundEffect).forEach(wadFile::replaceLump);
	}

	private Lump processPcSpeakerSoundEffect(Lump vanillaLump) {
		ByteBuffer vanillaData = vanillaLump.dataAsByteBuffer();
		vanillaData.getShort(); // type, 0 = PC Speaker
		short length = vanillaData.getShort();

		ByteBuffer doom8088Data = newByteBuffer(byteOrder);
		doom8088Data.putShort(length);

		short[] divisors = getDivisors();
		for (int i = 0; i < length; i++) {
			byte b = vanillaData.get();
			short d = divisors[toInt(b)];
			doom8088Data.putShort(d);
		}

		return new Lump(vanillaLump.name(), 2 + length * 2, doom8088Data);
	}

	protected short[] getDivisors() {
		return DIVISORS;
	}

	private void processSprites() {
		List<Lump> lumps = wadFile.getLumpsBetween("S_START", "S_END");
		lumps.stream().map(this::processSprite).forEach(wadFile::replaceLump);
	}

	/**
	 * Repeat every other column to cut the size in half
	 *
	 * @param vanillaLump
	 * @return
	 */
	private Lump processSprite(Lump vanillaLump) {
		ByteBuffer vanillaData = vanillaLump.dataAsByteBuffer();
		short width = vanillaData.getShort();
		short height = vanillaData.getShort();
		short leftoffset = vanillaData.getShort();
		short topoffset = vanillaData.getShort();
		List<Integer> columnofs = new ArrayList<>();
		for (int columnof = 0; columnof < width; columnof++) {
			columnofs.add(vanillaData.getInt());
		}

		List<List<Byte>> columns = new ArrayList<>();
		for (int columnof = 0; columnof < width; columnof++) {
			// get column
			List<Byte> column = new ArrayList<>();

			vanillaData.position(columnofs.get(columnof));
			byte topdelta = vanillaData.get();
			column.add(topdelta);
			while (topdelta != -1) {
				byte lengthByte = vanillaData.get();
				column.add(lengthByte);
				int length = toInt(lengthByte);
				for (int y = 0; y < length + 2; y++) {
					column.add(vanillaData.get());
				}

				topdelta = vanillaData.get();
				column.add(topdelta);
			}

			columns.add(column);
		}

		ByteBuffer doom8088Data = newByteBuffer(byteOrder);
		doom8088Data.putShort(width);
		doom8088Data.putShort(height);
		doom8088Data.putShort(leftoffset);
		doom8088Data.putShort(topoffset);

		// temp offset values
		for (int columnof = 0; columnof < width; columnof++) {
			doom8088Data.putInt(-1);
		}

		int l = width / 2;
		int c = 0;
		while (l-- != 0) {
			columnofs.set(c + 0, doom8088Data.position());
			columnofs.set(c + 1, doom8088Data.position());
			List<Byte> column = columns.get(c);
			for (byte b : column) {
				doom8088Data.put(b);
			}
			c += 2;
		}

		switch (width & 1) {
		case 1:
			columnofs.set(c, doom8088Data.position());
			for (byte b : columns.get(c)) {
				doom8088Data.put(b);
			}
		}

		int size = doom8088Data.position();

		doom8088Data.position(8);
		for (int columnof : columnofs) {
			doom8088Data.putInt(columnof);
		}

		return new Lump(vanillaLump.name(), size, doom8088Data);
	}

	private void processWalls() {
		List<Lump> lumps = wadFile.getLumpsBetween("P1_START", "P1_END");
		lumps.stream().map(this::processWall).forEach(wadFile::replaceLump);
	}

	/**
	 * Repeat every fourth column four times to leave a quarter of the size
	 *
	 * @param vanillaLump
	 * @return
	 */
	private Lump processWall(Lump vanillaLump) {
		ByteBuffer vanillaData = vanillaLump.dataAsByteBuffer();
		short width = vanillaData.getShort();
		short height = vanillaData.getShort();
		short leftoffset = vanillaData.getShort();
		short topoffset = vanillaData.getShort();
		List<Integer> columnofs = new ArrayList<>();
		for (int columnof = 0; columnof < width; columnof++) {
			columnofs.add(vanillaData.getInt());
		}

		List<List<Byte>> columns = new ArrayList<>();
		for (int columnof = 0; columnof < width; columnof++) {
			// get column
			List<Byte> column = new ArrayList<>();

			vanillaData.position(columnofs.get(columnof));
			byte topdelta = vanillaData.get();
			column.add(topdelta);
			while (topdelta != -1) {
				byte lengthByte = vanillaData.get();
				column.add(lengthByte);
				int length = toInt(lengthByte);
				for (int y = 0; y < length + 2; y++) {
					column.add(vanillaData.get());
				}

				topdelta = vanillaData.get();
				column.add(topdelta);
			}

			columns.add(column);
		}

		ByteBuffer doom8088Data = newByteBuffer(byteOrder);
		doom8088Data.putShort(width);
		doom8088Data.putShort(height);
		doom8088Data.putShort(leftoffset);
		doom8088Data.putShort(topoffset);

		// temp offset values
		for (int i = 0; i < width; i++) {
			doom8088Data.putInt(-1);
		}

		int l = width / 4;
		int c = 0;
		while (l-- != 0) {
			columnofs.set(c + 0, doom8088Data.position());
			columnofs.set(c + 1, doom8088Data.position());
			columnofs.set(c + 2, doom8088Data.position());
			columnofs.set(c + 3, doom8088Data.position());
			List<Byte> column = columns.get(c);
			for (byte b : column) {
				doom8088Data.put(b);
			}
			c += 4;
		}

		switch (width & 3) {
		case 3:
			columnofs.set(c + 0, doom8088Data.position());
			columnofs.set(c + 1, doom8088Data.position());
			columnofs.set(c + 2, doom8088Data.position());
			for (byte b : columns.get(c)) {
				doom8088Data.put(b);
			}
			break;
		case 2:
			columnofs.set(c + 0, doom8088Data.position());
			columnofs.set(c + 1, doom8088Data.position());
			for (byte b : columns.get(c)) {
				doom8088Data.put(b);
			}
			break;
		case 1:
			columnofs.set(c + 0, doom8088Data.position());
			for (byte b : columns.get(c)) {
				doom8088Data.put(b);
			}
		}

		int size = doom8088Data.position();

		doom8088Data.position(8);
		for (int columnof : columnofs) {
			doom8088Data.putInt(columnof);
		}

		return new Lump(vanillaLump.name(), size, doom8088Data);
	}

	/**
	 * Remove duplicate columns in graphics in picture format
	 *
	 */
	private void compressPictures() {
		List<Lump> pictures = new ArrayList<>(256);
		// Status bar
		pictures.addAll(wadFile.getLumpsByName("STC"));
		pictures.addAll(wadFile.getLumpsByName("STF"));
		pictures.addAll(wadFile.getLumpsByName("STG"));
		pictures.addAll(wadFile.getLumpsByName("STK"));
		pictures.addAll(wadFile.getLumpsByName("STY"));
		// Menu
		pictures.addAll(wadFile.getLumpsByName("M_"));
		// Intermission
		pictures.addAll(wadFile.getLumpsByName("WI").stream().filter(l -> !"WIMAP0".equals(l.nameAsString())).toList());
		// Sprites
		pictures.addAll(wadFile.getLumpsBetween("S_START", "S_END"));
		// Walls
		pictures.addAll(wadFile.getLumpsBetween("P1_START", "P1_END"));
		pictures.stream().map(this::compressPicture).forEach(wadFile::replaceLump);
	}

	/**
	 * Graphic squashing
	 *
	 * @param picture
	 * @return
	 */
	private Lump compressPicture(Lump picture) {
		ByteBuffer pictureData = picture.dataAsByteBuffer();
		short width = pictureData.getShort();
		short height = pictureData.getShort();
		short leftoffset = pictureData.getShort();
		short topoffset = pictureData.getShort();
		List<Integer> columnofs = new ArrayList<>();
		for (int columnof = 0; columnof < width; columnof++) {
			columnofs.add(pictureData.getInt());
		}

		List<List<Byte>> columns = new ArrayList<>();
		for (int columnof = 0; columnof < width; columnof++) {
			// get column
			List<Byte> column = new ArrayList<>();

			pictureData.position(columnofs.get(columnof));
			byte topdelta = pictureData.get();
			column.add(topdelta);
			while (topdelta != -1) {
				byte lengthByte = pictureData.get();
				column.add(lengthByte);
				int length = toInt(lengthByte);
				for (int y = 0; y < length + 2; y++) {
					column.add(pictureData.get());
				}

				topdelta = pictureData.get();
				column.add(topdelta);
			}

			columns.add(column);
		}

		ByteBuffer compressedData = newByteBuffer(byteOrder);
		compressedData.putShort(width);
		compressedData.putShort(height);
		compressedData.putShort(leftoffset);
		compressedData.putShort(topoffset);

		// temp offset values
		for (int columnof = 0; columnof < width; columnof++) {
			compressedData.putInt(-1);
		}

		Map<List<Byte>, Integer> persistedColumns = new HashMap<>();
		for (int i = 0; i < width; i++) {
			List<Byte> column = columns.get(i);
			if (persistedColumns.containsKey(column)) {
				columnofs.set(i, persistedColumns.get(column));
			} else {
				columnofs.set(i, compressedData.position());
				persistedColumns.put(column, compressedData.position());

				for (byte b : column) {
					compressedData.put(b);
				}
			}
		}

		int size = compressedData.position();

		compressedData.position(8);
		for (int columnof : columnofs) {
			compressedData.putInt(columnof);
		}

		return new Lump(picture.name(), size, compressedData);
	}

	private static record Mappatch(short originx, short originy, short patch, short stepdir, short colormap) {
	}

	private static record Maptexture(byte[] name, int masked, short width, short height, int columndirectory,
			short patchcount, List<Mappatch> patches) {
	}

}

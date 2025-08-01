package com.sfprod.jwadutil;

import static com.sfprod.jwadutil.WadProcessor.FLAT_SPAN;
import static com.sfprod.jwadutil.WadProcessor.createVgaColors;
import static com.sfprod.utils.ByteBufferUtils.newByteBuffer;
import static com.sfprod.utils.ListUtils.endsWith;
import static com.sfprod.utils.NumberUtils.toByte;
import static com.sfprod.utils.NumberUtils.toInt;
import static com.sfprod.utils.NumberUtils.toShort;
import static com.sfprod.utils.StringUtils.toStringUpperCase;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;

import com.sfprod.utils.ByteBufferUtils;

public class MapProcessor {

	// Lump order in a map WAD: each map needs a couple of lumps
	// to provide a complete scene geometry description.
	private static final int ML_THINGS = 1; // Monsters, items..
	private static final int ML_LINEDEFS = 2; // LineDefs, from editing
	private static final int ML_SIDEDEFS = 3; // SideDefs, from editing
	private static final int ML_VERTEXES = 4; // Vertices, edited and BSP splits generated
	private static final int ML_SEGS = 5; // LineSegs, from LineDefs split by BSP
	private static final int ML_SSECTORS = 6; // SubSectors, list of LineSegs
	private static final int ML_NODES = 7; // BSP nodes
	private static final int ML_SECTORS = 8; // Sectors, from editing
	private static final int ML_BLOCKMAP = 10; // LUT, motion clipping, walls/grid element

	private static final short MTF_NOTSINGLE = 16;

	private static final short NO_INDEX = (short) 0xffff;
	private static final byte NO_INDEX8 = (byte) 0xff;
	private static final byte ML_TWOSIDED = 4;

	private static final short ANG90_16 = 0x4000;

	private static final short SKY = (short) -2;
	private static final short NUKAGE = (short) -3;

	private final ByteOrder byteOrder;
	protected final WadFile wadFile;

	protected final List<Color> vgaColors;

	private final Map<String, Short> flatToColor = new HashMap<>();
	private List<Color> availableColors;
	private final Map<Short, Color> availableColorsMap = new HashMap<>();

	public MapProcessor(ByteOrder byteOrder, WadFile wadFile) {
		this.byteOrder = byteOrder;
		this.wadFile = wadFile;

		this.vgaColors = createVgaColors(wadFile);
	}

	public void processMaps(List<Color> availableColors) {
		this.availableColors = availableColors;

		for (int map = 1; map <= 9; map++) {
			String mapName = "E1M" + map;
			int lumpNum = wadFile.getLumpNumByName(mapName);
			processMap(lumpNum);
		}
	}

	private void processMap(int lumpNum) {
		processThings(lumpNum);

		processLinedefs(lumpNum);
		processSidedefs(lumpNum);
		packSidedefs(lumpNum);

		processSegs(lumpNum);
		processSsectors(lumpNum);
		processNodes(lumpNum);
		processSectors(lumpNum);
		processBlockmap(lumpNum);
	}

	/**
	 * Remove multiplayer things
	 *
	 * @param lumpNum
	 */
	private void processThings(int lumpNum) {
		int thingsLumpNum = lumpNum + ML_THINGS;
		Lump things = wadFile.getLumpByNum(thingsLumpNum);
		ByteBuffer oldByteBuffer = things.dataAsByteBuffer();
		ByteBuffer newByteBuffer = newByteBuffer(byteOrder);

		for (int i = 0; i < things.length() / (2 + 2 + 2 + 2 + 2); i++) {
			short x = oldByteBuffer.getShort();
			short y = oldByteBuffer.getShort();
			short angle = oldByteBuffer.getShort();
			short type = oldByteBuffer.getShort();
			short options = oldByteBuffer.getShort();

			if (type == 2 || type == 3 || type == 4 || type == 11) {
				// ignore starting spot for player 2, 3, 4 and Deathmatch
			} else if ((options & MTF_NOTSINGLE) != 0) {
				// ignore multiplayer things
			} else {
				newByteBuffer.putShort(x);
				newByteBuffer.putShort(y);
				newByteBuffer.putShort(type);
				newByteBuffer.put(toByte(angle / 45));
				newByteBuffer.put(toByte(options));
			}
		}

		int size = newByteBuffer.position();
		Lump newLump = new Lump(things.name(), size, newByteBuffer);
		wadFile.replaceLump(thingsLumpNum, newLump);
	}

	/**
	 * Change vertexes
	 *
	 * @param lumpNum
	 */
	private void processLinedefs(int lumpNum) {
		int lineLumpNum = lumpNum + ML_LINEDEFS;
		Lump lines = wadFile.getLumpByNum(lineLumpNum);

		int sizeofmaplinedef = 2 + 2 + 2 + 2 + 2 + 2 * 2;

		int lineCount = lines.length() / sizeofmaplinedef;

		List<Maplinedef> oldLines = new ArrayList<>(lineCount);
		ByteBuffer oldLinesByteBuffer = lines.dataAsByteBuffer();
		for (int i = 0; i < lineCount; i++) {
			short v1 = oldLinesByteBuffer.getShort();
			short v2 = oldLinesByteBuffer.getShort();
			short flags = oldLinesByteBuffer.getShort();
			short special = oldLinesByteBuffer.getShort();
			short tag = oldLinesByteBuffer.getShort();
			short[] sidenum = new short[2];
			sidenum[0] = oldLinesByteBuffer.getShort();
			sidenum[1] = oldLinesByteBuffer.getShort();
			oldLines.add(new Maplinedef(v1, v2, flags, special, tag, sidenum));
		}

		// We need vertexes for this...
		List<Vertex> vertexes = getVertexes(lumpNum);

		ByteBuffer newLineByteBuffer = newByteBuffer(byteOrder, lineCount * Line.SIZE_OF_LINE);
		for (Maplinedef maplinedef : oldLines) {
			Vertex vertex1 = vertexes.get(maplinedef.v1());
			newLineByteBuffer.putShort(vertex1.x()); // v1.x
			newLineByteBuffer.putShort(vertex1.y()); // v1.y

			Vertex vertex2 = vertexes.get(maplinedef.v2());
			newLineByteBuffer.putShort(vertex2.x()); // v2.x
			newLineByteBuffer.putShort(vertex2.y()); // v2.y

			newLineByteBuffer.putShort(maplinedef.sidenum()[0]); // sidenum[0];
			newLineByteBuffer.putShort(maplinedef.sidenum()[1]); // sidenum[1];

			newLineByteBuffer.put(toByte(maplinedef.flags())); // flags
			newLineByteBuffer.put(toByte(maplinedef.special())); // special
			newLineByteBuffer.put(toByte(maplinedef.tag())); // tag
		}

		Lump newLine = new Lump(lines.name(), newLineByteBuffer);
		wadFile.replaceLump(lineLumpNum, newLine);
	}

	private List<Vertex> getVertexes(int lumpNum) {
		int vtxLumpNum = lumpNum + ML_VERTEXES;
		Lump vxl = wadFile.getLumpByNum(vtxLumpNum);
		List<Vertex> vertexes = new ArrayList<>();
		ByteBuffer vxlByteBuffer = vxl.dataAsByteBuffer();
		for (int i = 0; i < vxl.length() / (2 + 2); i++) {
			vertexes.add(new Vertex(vxlByteBuffer.getShort(), vxlByteBuffer.getShort()));
		}
		return vertexes;
	}

	/**
	 * Change vertexes, offset, angle, sidenum, linenum, frontsectornum and
	 * backsectornum
	 *
	 * @param lumpNum
	 */
	private void processSegs(int lumpNum) {
		int segsLumpNum = lumpNum + ML_SEGS;
		Lump segs = wadFile.getLumpByNum(segsLumpNum);

		int sizeofmapseg = 2 + 2 + 2 + 2 + 2 + 2;

		int segCount = segs.length() / sizeofmapseg;

		List<Mapseg> oldSegs = new ArrayList<>(segCount);
		ByteBuffer oldSegsByteBuffer = segs.dataAsByteBuffer();
		for (int i = 0; i < segCount; i++) {
			short v1 = oldSegsByteBuffer.getShort();
			short v2 = oldSegsByteBuffer.getShort();
			short angle = oldSegsByteBuffer.getShort();
			short linedef = oldSegsByteBuffer.getShort();
			short side = oldSegsByteBuffer.getShort();
			short offset = oldSegsByteBuffer.getShort();
			oldSegs.add(new Mapseg(v1, v2, angle, linedef, side, offset));
		}

		// We need vertexes for this...
		List<Vertex> vertexes = getVertexes(lumpNum);

		// And LineDefs. Must process lines first.
		int linesLumpNum = lumpNum + ML_LINEDEFS;
		Lump lxl = wadFile.getLumpByNum(linesLumpNum);
		List<Line> lines = new ArrayList<>();
		ByteBuffer linesByteBuffer = lxl.dataAsByteBuffer();
		for (int i = 0; i < lxl.length() / Line.SIZE_OF_LINE; i++) {
			Vertex v1 = new Vertex(linesByteBuffer.getShort(), linesByteBuffer.getShort());
			Vertex v2 = new Vertex(linesByteBuffer.getShort(), linesByteBuffer.getShort());
			short[] sidenum = { linesByteBuffer.getShort(), linesByteBuffer.getShort() };
			byte flags = linesByteBuffer.get();
			byte special = linesByteBuffer.get();
			byte tag = linesByteBuffer.get();
			lines.add(new Line(v1, v2, sidenum, flags, special, tag));
		}

		// And SideDefs. Must process sides first.
		int sidesLumpNum = lumpNum + ML_SIDEDEFS;
		Lump oldSidedefs = wadFile.getLumpByNum(sidesLumpNum);
		List<Sidedef> sidedefs = new ArrayList<>();
		ByteBuffer sidesByteBuffer = oldSidedefs.dataAsByteBuffer();
		for (int i = 0; i < oldSidedefs.length() / Sidedef.SIZE_OF_SIDE; i++) {
			short textureoffset = sidesByteBuffer.getShort();
			byte rowoffset = sidesByteBuffer.get();
			byte toptexture = sidesByteBuffer.get();
			byte bottomtexture = sidesByteBuffer.get();
			byte midtexture = sidesByteBuffer.get();
			byte sector = sidesByteBuffer.get();
			sidedefs.add(new Sidedef(textureoffset, rowoffset, toptexture, bottomtexture, midtexture, sector));
		}

		// ****************************

		int sizeofseg = 2 * 2 + 2 * 2 + 2 + 2 + 2 + 2 + 1 + 1;
		ByteBuffer newSegsByteBuffer = newByteBuffer(byteOrder, segCount * sizeofseg);
		for (Mapseg oldSeg : oldSegs) {
			Vertex v1 = vertexes.get(oldSeg.v1());
			newSegsByteBuffer.putShort(v1.x()); // v1.x
			newSegsByteBuffer.putShort(v1.y()); // v1.y

			Vertex v2 = vertexes.get(oldSeg.v2());
			newSegsByteBuffer.putShort(v2.x()); // v2.x
			newSegsByteBuffer.putShort(v2.y()); // v2.y

			newSegsByteBuffer.putShort(oldSeg.offset()); // offset
			newSegsByteBuffer.putShort(toShort(oldSeg.angle() + ANG90_16)); // angle

			short linenum = oldSeg.linedef();
			Line ldef = lines.get(linenum);
			short side = oldSeg.side();
			short sidenum = ldef.sidenum()[side];
			newSegsByteBuffer.putShort(sidenum); // sidenum

			newSegsByteBuffer.putShort(linenum); // linenum

			byte frontsectornum = toByte(sidedefs.get(sidenum).sector());
			newSegsByteBuffer.put(frontsectornum); // frontsectornum

			byte backsectornum = NO_INDEX8;
			if ((ldef.flags() & ML_TWOSIDED) != 0) {
				short backsectorside = ldef.sidenum()[side ^ 1];
				if (backsectorside != NO_INDEX) {
					backsectornum = toByte(sidedefs.get(backsectorside).sector());
				}
			}
			newSegsByteBuffer.put(backsectornum); // backsectornum
		}

		Lump newSeg = new Lump(segs.name(), newSegsByteBuffer);
		wadFile.replaceLump(segsLumpNum, newSeg);
	}

	/**
	 * Change textureoffset, rowoffset, toptexture, bottomtexture, midtexture and
	 * sector
	 *
	 * @param lumpNum
	 */
	private void processSidedefs(int lumpNum) {
		int sidesLumpNum = lumpNum + ML_SIDEDEFS;
		Lump sxl = wadFile.getLumpByNum(sidesLumpNum);
		List<Mapsidedef> oldSidedefs = new ArrayList<>();
		ByteBuffer sidesByteBuffer = sxl.dataAsByteBuffer();
		int sizeofmapsidedef = 2 + 2 + 8 + 8 + 8 + 2;
		for (int i = 0; i < sxl.length() / sizeofmapsidedef; i++) {
			short textureoffset = sidesByteBuffer.getShort();
			short rowoffset = sidesByteBuffer.getShort();
			byte[] toptexture = new byte[8];
			sidesByteBuffer.get(toptexture);
			byte[] bottomtexture = new byte[8];
			sidesByteBuffer.get(bottomtexture);
			byte[] midtexture = new byte[8];
			sidesByteBuffer.get(midtexture);
			short sector = sidesByteBuffer.getShort();
			oldSidedefs.add(new Mapsidedef(textureoffset, rowoffset, toptexture, bottomtexture, midtexture, sector));
		}

		int sideCount = oldSidedefs.size();

		List<String> textureNames = getTextureNames();

		ByteBuffer newSidedefByteBuffer = newByteBuffer(byteOrder, sideCount * Sidedef.SIZE_OF_SIDE);
		for (Mapsidedef oldSidedef : oldSidedefs) {
			newSidedefByteBuffer.putShort(oldSidedef.textureoffset()); // textureoffset
			newSidedefByteBuffer.put(toByte(oldSidedef.rowoffset())); // rowoffset

			newSidedefByteBuffer.put(getTextureNumForName(textureNames, oldSidedef.toptextureAsString())); // toptexture
			newSidedefByteBuffer.put(getTextureNumForName(textureNames, oldSidedef.bottomtextureAsString()));// bottomtexture
			newSidedefByteBuffer.put(getTextureNumForName(textureNames, oldSidedef.midtextureAsString())); // midtexture

			newSidedefByteBuffer.put(toByte(oldSidedef.sector())); // sector
		}

		byte[] sidedefsLumpName = wadFile.getLumpByNum(sidesLumpNum).name();
		Lump newSidedefs = new Lump(sidedefsLumpName, newSidedefByteBuffer);
		wadFile.replaceLump(sidesLumpNum, newSidedefs);
	}

	private List<String> getTextureNames() {
		Lump tex1lump = wadFile.getLumpByName("TEXTURE1");
		ByteBuffer tex1ByteBuffer = tex1lump.dataAsByteBuffer();
		int numtextures1 = tex1ByteBuffer.getInt();
		List<Integer> offsets = new ArrayList<>(numtextures1);
		for (int i = 0; i < numtextures1; i++) {
			offsets.add(tex1ByteBuffer.getInt());
		}

		List<String> textureNames = new ArrayList<>(numtextures1);
		for (int offset : offsets) {
			tex1ByteBuffer.position(offset);
			byte[] name = new byte[8];
			tex1ByteBuffer.get(name);
			textureNames.add(toStringUpperCase(name));
		}
		return textureNames;
	}

	private byte getTextureNumForName(List<String> names, String name) {
		int index = names.indexOf(name);
		return index == -1 ? 0 : toByte(index);
	}

	/**
	 * Only store numsegs as a byte
	 *
	 * @param lumpNum
	 */
	private void processSsectors(int lumpNum) {
		int ssectorsLumpNum = lumpNum + ML_SSECTORS;
		Lump ssectors = wadFile.getLumpByNum(ssectorsLumpNum);
		ByteBuffer byteBuffer = ssectors.dataAsByteBuffer();
		byte[] newnumsegs = new byte[ssectors.length() / (2 + 2)];
		short derivedFirstseg = 0;
		for (int i = 0; i < ssectors.length() / (2 + 2); i++) {
			short numsegs = byteBuffer.getShort();
			newnumsegs[i] = toByte(numsegs);

			short firstseg = byteBuffer.getShort();
			if (firstseg != derivedFirstseg) {
				throw new IllegalStateException();
			}
			derivedFirstseg += numsegs;
		}
		wadFile.replaceLump(ssectorsLumpNum, new Lump(ssectors.name(), newnumsegs, ByteBufferUtils.DONT_CARE));
	}

	/**
	 * Change byte order
	 *
	 * @param lumpNum
	 */
	private void processNodes(int lumpNum) {
		int nodesLumpNum = lumpNum + ML_NODES;
		Lump oldNodes = wadFile.getLumpByNum(nodesLumpNum);
		ByteBuffer oldNodesByteBuffer = oldNodes.dataAsByteBuffer();
		ByteBuffer newNodesByteBuffer = newByteBuffer(byteOrder, oldNodes.length());
		for (int i = 0; i < oldNodes.length() / 2; i++) {
			short s = oldNodesByteBuffer.getShort();
			newNodesByteBuffer.putShort(s);
		}
		Lump newNodes = new Lump(oldNodes.name(), newNodesByteBuffer);
		wadFile.replaceLump(nodesLumpNum, newNodes);
	}

	/**
	 * lightlevel and special fit in a byte, replace flat names by average color of
	 * flat
	 *
	 * @param lumpNum
	 */
	private void processSectors(int lumpNum) {
		int sectorsLumpNum = lumpNum + ML_SECTORS;
		Lump sectors = wadFile.getLumpByNum(sectorsLumpNum);
		ByteBuffer oldbb = sectors.dataAsByteBuffer();
		ByteBuffer newbb = newByteBuffer(byteOrder);
		for (int i = 0; i < sectors.length() / 26; i++) {
			short floorheight = oldbb.getShort();
			short ceilingheight = oldbb.getShort();
			byte[] floorpic = new byte[8];
			oldbb.get(floorpic);
			byte[] ceilingpic = new byte[8];
			oldbb.get(ceilingpic);
			short lightlevel = oldbb.getShort();
			short special = oldbb.getShort();
			short tag = oldbb.getShort();

			newbb.putShort(floorheight);
			newbb.putShort(ceilingheight);

			if (FLAT_SPAN) {
				// floorpic
				short floorpicnum;
				String floorflatname = toStringUpperCase(floorpic);
				if (floorflatname.startsWith("NUKAGE")) {
					floorpicnum = NUKAGE;
				} else {
					floorpicnum = calculateAverageColor(floorflatname);
				}
				newbb.putShort(floorpicnum);

				// ceilingpic
				short ceilingpicnum;
				String ceilingflatname = toStringUpperCase(ceilingpic);
				if (ceilingflatname.startsWith("NUKAGE")) {
					ceilingpicnum = NUKAGE;
				} else if ("F_SKY1".equals(ceilingflatname)) {
					ceilingpicnum = SKY;
				} else {
					ceilingpicnum = calculateAverageColor(ceilingflatname);
				}
				newbb.putShort(ceilingpicnum);
			} else {
				newbb.put(floorpic);
				newbb.put(ceilingpic);
			}
			newbb.put(toByte(lightlevel));
			newbb.put(toByte(special));
			newbb.putShort(tag);
		}

		int size = newbb.position();
		Lump newLump = new Lump(sectors.name(), size, newbb);
		wadFile.replaceLump(sectorsLumpNum, newLump);
	}

	protected short calculateAverageColor(String flatname) {
		if (availableColorsMap.isEmpty()) {
			for (short i = 0; i < 256; i++) {
				this.availableColorsMap.put(i, availableColors.get(i));
			}
		}

		if (!flatToColor.containsKey(flatname)) {
			Lump flat = wadFile.getLumpByName(flatname);

			byte[] source = flat.data();
			int sumr = 0;
			int sumg = 0;
			int sumb = 0;
			for (byte b : source) {
				Color color = vgaColors.get(toInt(b));
				sumr += color.r() * color.r();
				sumg += color.g() * color.g();
				sumb += color.b() * color.b();
			}
			int averager = (int) Math.sqrt(sumr / (64 * 64));
			int averageg = (int) Math.sqrt(sumg / (64 * 64));
			int averageb = (int) Math.sqrt(sumb / (64 * 64));
			Color averageColor = new Color(averager, averageg, averageb);

			short closestAverageColorIndex = -1;
			int minDistance = Integer.MAX_VALUE;
			for (Map.Entry<Short, Color> entry : availableColorsMap.entrySet()) {
				short i = entry.getKey();
				Color color = entry.getValue();

				int distance = averageColor.calculateDistance(color);
				if (distance == 0) {
					closestAverageColorIndex = i;
					break;
				}

				if (distance < minDistance) {
					minDistance = distance;
					closestAverageColorIndex = i;
				}
			}
			availableColorsMap.remove(closestAverageColorIndex);

			flatToColor.put(flatname, closestAverageColorIndex);
			assert flatToColor.size() == new HashSet<>(flatToColor.values()).size();
		}

		return flatToColor.get(flatname);
	}

	/**
	 * Blockmap stacking
	 *
	 * @param lumpNum
	 */
	private void processBlockmap(int lumpNum) {
		int blockmapLumpNum = lumpNum + ML_BLOCKMAP;
		Lump blockmap = wadFile.getLumpByNum(blockmapLumpNum);

		ByteBuffer blockmapByteBuffer = blockmap.dataAsByteBuffer();
		short bmaporgx = blockmapByteBuffer.getShort();
		short bmaporgy = blockmapByteBuffer.getShort();
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

		ByteBuffer newBlockmap = newByteBuffer(byteOrder);
		newBlockmap.putShort(bmaporgx);
		newBlockmap.putShort(bmaporgy);
		newBlockmap.putShort(bmapwidth);
		newBlockmap.putShort(bmapheight);

		// temp offset values
		for (int i = 0; i < bmapwidth * bmapheight; i++) {
			newBlockmap.putShort(toShort(-1));
		}

		List<Map.Entry<Integer, List<Short>>> sorted = mapOfLinenos.entrySet().stream()
				.sorted(Map.Entry.comparingByValue(Comparator.comparing(List::size))).toList().reversed();
		List<Map.Entry<List<Short>, Short>> duplicateDataList = new ArrayList<>();
		for (Map.Entry<Integer, List<Short>> entry : sorted) {
			int index = entry.getKey();
			List<Short> linenos = entry.getValue();

			short offset = -1;
			for (Map.Entry<List<Short>, Short> duplicateEntry : duplicateDataList) {
				List<Short> duplicateLinos = duplicateEntry.getKey();
				if (endsWith(duplicateLinos, linenos)) {
					offset = toShort(duplicateEntry.getValue() + (duplicateLinos.size() - linenos.size()));
					break;
				}
			}

			if (offset == -1) {
				offset = toShort(newBlockmap.position() / 2);

				newBlockmap.putShort(toShort(0));
				for (short lineno : linenos) {
					newBlockmap.putShort(lineno);
				}
				newBlockmap.putShort(toShort(-1));

				duplicateDataList.add(Map.entry(linenos, offset));
			}
			offsets.set(index, offset);
		}

		int newLength = newBlockmap.position();

		newBlockmap.position(8);

		for (short offset : offsets) {
			newBlockmap.putShort(offset);
		}

		Lump newLump = new Lump(blockmap.name(), newLength, newBlockmap);
		wadFile.replaceLump(blockmapLumpNum, newLump);
	}

	/**
	 * Sidedef packing
	 *
	 * @param lumpNum
	 */
	private void packSidedefs(int lumpNum) {
		int lineLumpNum = lumpNum + ML_LINEDEFS;
		Lump oldLinedefs = wadFile.getLumpByNum(lineLumpNum);

		List<Line> lines = new ArrayList<>();
		ByteBuffer linesByteBuffer = oldLinedefs.dataAsByteBuffer();
		for (int i = 0; i < oldLinedefs.length() / Line.SIZE_OF_LINE; i++) {
			Vertex v1 = new Vertex(linesByteBuffer.getShort(), linesByteBuffer.getShort());
			Vertex v2 = new Vertex(linesByteBuffer.getShort(), linesByteBuffer.getShort());
			short[] sidenum = { linesByteBuffer.getShort(), linesByteBuffer.getShort() };
			byte flags = linesByteBuffer.get();
			byte special = linesByteBuffer.get();
			byte tag = linesByteBuffer.get();
			lines.add(new Line(v1, v2, sidenum, flags, special, tag));
		}

		int sidesLumpNum = lumpNum + ML_SIDEDEFS;
		Lump oldSidedefs = wadFile.getLumpByNum(sidesLumpNum);
		List<Sidedef> sides = new ArrayList<>();
		ByteBuffer sidesByteBuffer = oldSidedefs.dataAsByteBuffer();
		for (int i = 0; i < oldSidedefs.length() / Sidedef.SIZE_OF_SIDE; i++) {
			short textureoffset = sidesByteBuffer.getShort();
			byte rowoffset = sidesByteBuffer.get();
			byte toptexture = sidesByteBuffer.get();
			byte bottomtexture = sidesByteBuffer.get();
			byte midtexture = sidesByteBuffer.get();
			byte sector = sidesByteBuffer.get();
			sides.add(new Sidedef(textureoffset, rowoffset, toptexture, bottomtexture, midtexture, sector));
		}

		short newindex = 0;
		List<Line> newLines = new ArrayList<>();
		List<SidedefWithMetadata> sidedefWithMetadataList = new ArrayList<>();
		for (Line oldLine : lines) {
			Sidedef frontside = sides.get(oldLine.sidenum()[0]);
			Sidedef backside = oldLine.sidenum()[1] != NO_INDEX ? sides.get(oldLine.sidenum()[1]) : null;

			short newfrontside;
			short newbackside;
			if (oldLine.special() != 0) {
				newfrontside = newindex;
				newindex++;
				sidedefWithMetadataList.add(new SidedefWithMetadata(frontside, true));

				newbackside = NO_INDEX;
				if (backside != null) {
					newbackside = newindex;
					newindex++;
					sidedefWithMetadataList.add(new SidedefWithMetadata(backside, true));
				}
			} else {
				SidedefWithMetadata frontkey = new SidedefWithMetadata(frontside, false);
				newfrontside = (short) sidedefWithMetadataList.indexOf(frontkey);
				if (newfrontside == -1) {
					newfrontside = newindex;
					newindex++;
					sidedefWithMetadataList.add(frontkey);
				}

				newbackside = NO_INDEX;
				if (backside != null) {
					SidedefWithMetadata backkey = new SidedefWithMetadata(backside, false);
					newbackside = (short) sidedefWithMetadataList.indexOf(backkey);
					if (newbackside == -1) {
						newbackside = newindex;
						newindex++;
						sidedefWithMetadataList.add(backkey);
					}
				}
			}
			newLines.add(new Line(oldLine.v1(), oldLine.v2(), new short[] { newfrontside, newbackside },
					oldLine.flags(), oldLine.special(), oldLine.tag()));
		}

		ByteBuffer newLinesByteBuffer = newByteBuffer(byteOrder, oldLinedefs.length());
		for (Line newLine : newLines) {
			newLinesByteBuffer.putShort(newLine.v1().x());
			newLinesByteBuffer.putShort(newLine.v1().y());
			newLinesByteBuffer.putShort(newLine.v2().x());
			newLinesByteBuffer.putShort(newLine.v2().y());
			newLinesByteBuffer.putShort(newLine.sidenum()[0]);
			newLinesByteBuffer.putShort(newLine.sidenum()[1]);
			newLinesByteBuffer.put(newLine.flags());
			newLinesByteBuffer.put(newLine.special());
			newLinesByteBuffer.put(newLine.tag());
		}
		wadFile.replaceLump(lineLumpNum, new Lump(oldLinedefs.name(), newLinesByteBuffer));

		ByteBuffer newSidesByteBuffer = newByteBuffer(byteOrder, sidedefWithMetadataList.size() * Sidedef.SIZE_OF_SIDE);
		for (SidedefWithMetadata sidedefWithMetadata : sidedefWithMetadataList) {
			Sidedef sidedef = sidedefWithMetadata.sidedef();
			newSidesByteBuffer.putShort(sidedef.textureoffset());
			newSidesByteBuffer.put(sidedef.rowoffset());
			newSidesByteBuffer.put(sidedef.toptexture());
			newSidesByteBuffer.put(sidedef.bottomtexture());
			newSidesByteBuffer.put(sidedef.midtexture());
			newSidesByteBuffer.put(sidedef.sector());
		}
		wadFile.replaceLump(sidesLumpNum, new Lump(oldSidedefs.name(), newSidesByteBuffer));
	}

	private static record Maplinedef(short v1, short v2, short flags, short special, short tag, short[] sidenum) {
	}

	private static record Vertex(short x, short y) {
	}

	private static record Line(Vertex v1, Vertex v2, short[] sidenum, byte flags, byte special, byte tag) {
		public static final int SIZE_OF_LINE = 2 * 2 + 2 * 2 + 2 * 2 + 1 + 1 + 1;
	}

	private static record Mapseg(short v1, short v2, short angle, short linedef, short side, short offset) {
	}

	private static record Mapsidedef(short textureoffset, short rowoffset, byte[] toptexture, byte[] bottomtexture,
			byte[] midtexture, short sector) {
		String toptextureAsString() {
			return toStringUpperCase(toptexture);
		}

		String bottomtextureAsString() {
			return toStringUpperCase(bottomtexture);
		}

		String midtextureAsString() {
			return toStringUpperCase(midtexture);
		}
	}

	private static record Sidedef(short textureoffset, byte rowoffset, byte toptexture, byte bottomtexture,
			byte midtexture, byte sector) {
		public static final int SIZE_OF_SIDE = 2 + 1 + 1 + 1 + 1 + 1;
	}

	private static record SidedefWithMetadata(Sidedef sidedef, boolean special) {
	}
}

package com.sfprod.jwadutil;

import static com.sfprod.utils.NumberUtils.toByte;
import static com.sfprod.utils.NumberUtils.toInt;

import java.awt.image.BufferedImage;
import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.List;

import javax.imageio.ImageIO;

import com.sfprod.utils.NumberUtils;

class WadProcessor4Colors extends WadProcessorLimitedColors {

	private static final List<Color> CGA_COLORS = List.of( //
			new Color(0x00, 0x00, 0x00), // black
			new Color(0x55, 0xFF, 0xFF), // light cyan
			new Color(0xFF, 0x55, 0xFF), // light magenta
			new Color(0xFF, 0xFF, 0xFF) // white
	);

	private static final List<Integer> VGA256_TO_4_LUT = List.of( //
			0, 0, 0, // black
			1, // grey
			3, // white
			0, 0, 0, 0, // black
			0, 0, 0, -1, // dark green
			-1, 0, -1, // brown

			-1, -1, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, //
			2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, // reddish

			-1, -1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, //
			3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, // orangeish

			3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, //
			1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, // gray

			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, -1, // green
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, // brown
			3, 3, 3, 0, 0, 0, 0, 0, // brown
			1, 1, 1, 1, 0, 0, 0, 0, // brown/green
			3, 3, 1, 1, 2, 2, 0, 0, // gold

			-1, -1, -1, 2, 2, 2, 2, 2, //
			2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, // red

			3, 3, 3, -1, -1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, // blue
			-1, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // orange
			3, 3, -1, -1, -1, 3, -1, 3, // yellow
			-1, 0, -1, 0, // dark orange
			-1, -1, 0, 0, // brown
			0, 0, 0, 0, 0, 0, 0, -1, // dark blue
			-1, // orange
			-1, // yellow
			-1, -1, -1, -1, 2, // purple
			-1 // cream-colored
	);

	private static final List<Integer> VGA256_TO_DITHERED_LUT = List.of( //
			0x00, 0x00, 0x00, // black
			0x33, // grey
			0xff, // white
			0x33, 0x33, 0x00, 0x00, // black
			0x00, 0x00, 0x00, 0x00, // dark green
			0x00, 0x00, 0x00, // brown

			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, //
			0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // reddish

			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, //
			0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x00, 0x00, // orangeish

			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, //
			0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, // gray

			0xdd, 0xdd, 0xdd, 0xdd, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x11, 0x11, 0x11, 0x11, // green
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x00, 0x00, 0x00, 0x00, // brown
			0xff, 0xff, 0xff, 0x33, 0x33, 0x33, 0x33, 0x33, // brown
			0x55, 0x55, 0x55, 0x55, 0x00, 0x00, 0x00, 0x00, // brown/green
			0xff, 0xff, 0x55, 0x55, 0xaa, 0xaa, 0x00, 0x00, // gold

			0xff, 0xff, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, //
			0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x00, 0x00, 0x00, 0x00, // red

			0xff, 0xff, 0xdd, 0xdd, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x11, 0x11, 0x11, 0x11, // blue
			0xff, 0xff, 0xee, 0xee, 0xee, 0xee, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, // orange
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // yellow
			0x22, 0x22, 0x22, 0x22, // dark orange
			0x00, 0x00, 0x00, 0x00, // brown
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // dark blue
			0xaa, // orange
			0xff, // yellow
			0xaa, 0xaa, 0xaa, 0xaa, 0x22, // purple
			0x22 // cream-colored
	);

	private static final List<Integer> GRAYSCALE_FROM_DARK_TO_BRIGHT = List.of(0x00, 0x03, 0x30, 0x0c, 0xc0, 0xc3, 0x3c,
			0x33, 0xcc, 0x3f, 0xf3, 0xcf, 0xfc, 0xff);

	WadProcessor4Colors(String title, ByteOrder byteOrder, WadFile wadFile) {
		super(title, byteOrder, wadFile, GRAYSCALE_FROM_DARK_TO_BRIGHT, 3);

		List<Color> colors = new ArrayList<>();
		for (int col0 = 0; col0 < 4; col0++) {
			for (int col1 = 0; col1 < 4; col1++) {
				for (int col2 = 0; col2 < 4; col2++) {
					for (int col3 = 0; col3 < 4; col3++) {
						Color c0 = CGA_COLORS.get(col0);
						Color c1 = CGA_COLORS.get(col1);
						Color c2 = CGA_COLORS.get(col2);
						Color c3 = CGA_COLORS.get(col3);

						int r = (int) Math
								.sqrt((c0.r() * c0.r() + c1.r() * c1.r() + c2.r() * c2.r() + c3.r() * c3.r()) / 4);
						int g = (int) Math
								.sqrt((c0.g() * c0.g() + c1.g() * c1.g() + c2.g() * c2.g() + c3.g() * c3.g()) / 4);
						int b = (int) Math
								.sqrt((c0.b() * c0.b() + c1.b() * c1.b() + c2.b() * c2.b() + c3.b() * c3.b()) / 4);
						Color color = new Color(r, g, b);
						colors.add(color);
					}
				}
			}
		}
		fillAvailableColorsShuffleMap(colors);

		wadFile.replaceLump(createCgaLump("FLOOR4_8"));
	}

	@Override
	protected List<Integer> createVga256ToDitheredLUT(List<Color> vgaCols, List<Color> availableCols) {
		return VGA256_TO_DITHERED_LUT;
	}

	@Override
	protected void changePaletteRaw(Lump lump) {
		wadFile.replaceLump(createCgaLump(lump.nameAsString()));
	}

	private Lump createCgaLump(String lumpname) {
		List<Integer> rgbs = CGA_COLORS.stream().map(Color::getRGB).toList();

		try {
			BufferedImage image = ImageIO
					.read(WadProcessor4Colors.class.getResourceAsStream("/CGA/" + lumpname + ".PNG"));
			byte[] data = new byte[(image.getWidth() / 4) * image.getHeight()];
			int i = 0;
			for (int y = 0; y < image.getHeight(); y++) {
				for (int x = 0; x < image.getWidth() / 4; x++) {
					byte b = 0;
					for (int p = 0; p < 4; p++) {
						int rgb = image.getRGB(x * 4 + p, y);
						b = NumberUtils.toByte((b << 2) | rgbs.indexOf(rgb));
					}
					data[i] = b;
					i++;
				}
			}
			return new Lump(lumpname, data, ByteOrder.LITTLE_ENDIAN);
		} catch (IOException e) {
			throw new UncheckedIOException(e);
		}
	}

	@Override
	protected byte convert256to16(byte b) {
		return toByte(VGA256_TO_4_LUT.get(toInt(b)) << 6);
	}
}

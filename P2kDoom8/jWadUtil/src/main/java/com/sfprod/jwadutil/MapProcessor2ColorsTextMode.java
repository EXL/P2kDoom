package com.sfprod.jwadutil;

import static com.sfprod.jwadutil.WadProcessor2ColorsTextMode.COLORS_FLOORS;
import static com.sfprod.utils.NumberUtils.toInt;
import static com.sfprod.utils.NumberUtils.toShort;

import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;

public class MapProcessor2ColorsTextMode extends MapProcessor {

	private final List<Double> sortedGrays;

	private final Map<String, Short> flatToColor = new HashMap<>();
	private final List<Short> availableColors;

	public MapProcessor2ColorsTextMode(ByteOrder byteOrder, WadFile wadFile) {
		super(byteOrder, wadFile);

		List<Double> grays = vgaColors.stream().map(Color::gray).toList();
		this.sortedGrays = grays.stream().sorted().toList();

		List<Short> colors = new ArrayList<>(COLORS_FLOORS.length);
		for (byte b : COLORS_FLOORS) {
			colors.add(toShort(b));
		}
		this.availableColors = colors;
	}

	@Override
	protected short calculateAverageColor(String flatname) {
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

			double averageGray = averageColor.gray();
			int possibleIndex = Math.clamp(Math.abs(Collections.binarySearch(sortedGrays, averageGray)), 0,
					sortedGrays.size() - 1);

			int indexAvailableColors = possibleIndex * availableColors.size() / sortedGrays.size();
			short c = availableColors.get(indexAvailableColors);
			availableColors.remove(indexAvailableColors);

			flatToColor.put(flatname, c);
			assert flatToColor.size() == new HashSet<>(flatToColor.values()).size();
		}

		return flatToColor.get(flatname);
	}
}

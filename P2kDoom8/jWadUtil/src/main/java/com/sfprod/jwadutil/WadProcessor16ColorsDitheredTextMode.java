package com.sfprod.jwadutil;

import java.nio.ByteOrder;
import java.util.List;

class WadProcessor16ColorsDitheredTextMode extends WadProcessor16ColorsDitheredPC {

	WadProcessor16ColorsDitheredTextMode(String title, ByteOrder byteOrder, WadFile wadFile) {
		super(title, byteOrder, wadFile);
	}

	@Override
	protected void removeUnusedLumps() {
		super.removeUnusedLumps();

		// Menu graphics
		List<Lump> mLumps = wadFile.getLumpsByName("M_");
		mLumps.forEach(wadFile::removeLump);

		// Status bar graphics
		List<Lump> stLumps = wadFile.getLumpsByName("ST");
		stLumps.stream().filter(l -> !("STIMA0".equals(l.nameAsString()) || l.nameAsString().startsWith("STEP")))
				.forEach(wadFile::removeLump);

		// Intermission screen graphics
		List<Lump> wiLumps = wadFile.getLumpsByName("WI");
		wiLumps.stream().filter(l -> !"WIMAP0".equals(l.nameAsString())).forEach(wadFile::removeLump);
	}
}

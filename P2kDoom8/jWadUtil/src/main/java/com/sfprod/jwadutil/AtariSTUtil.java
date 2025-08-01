package com.sfprod.jwadutil;

public interface AtariSTUtil {

	static short[] getDivisors() {
		short[] newDivs = new short[128];
		newDivs[0] = 0;
		for (int i = 1; i < 128; i++) {
			short od = WadProcessor.DIVISORS[i];
			short frequency = (short) (1193181 / od);
			short nd = (short) ((2_000_000 / 16) / frequency);
			newDivs[i] = nd;
		}
		return newDivs;
	}
}

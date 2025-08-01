package com.sfprod.jwadutil;

import java.nio.ByteOrder;

class WadProcessor2ColorsAtariST extends WadProcessor4Colors {

	private final short[] divisors;

	WadProcessor2ColorsAtariST(String title, ByteOrder byteOrder, WadFile wadFile) {
		super(title, byteOrder, wadFile);
		this.divisors = AtariSTUtil.getDivisors();
	}

	@Override
	protected short[] getDivisors() {
		return divisors;
	}
}

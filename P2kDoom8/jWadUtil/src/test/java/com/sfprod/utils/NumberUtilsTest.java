package com.sfprod.utils;

import static org.junit.jupiter.api.Assertions.assertEquals;

import org.junit.jupiter.api.Test;

/**
 * This class tests {@link NumberUtils}
 *
 */
class NumberUtilsTest {

	@Test
	void toInt() {
		assertEquals(0, NumberUtils.toInt((byte) 0));
		assertEquals(1, NumberUtils.toInt((byte) 1));
		assertEquals(127, NumberUtils.toInt((byte) 127));
		assertEquals(129, NumberUtils.toInt((byte) -127));
		assertEquals(128, NumberUtils.toInt((byte) 128));
		assertEquals(128, NumberUtils.toInt((byte) -128));
		assertEquals(255, NumberUtils.toInt((byte) 0xff));
		assertEquals(255, NumberUtils.toInt((byte) -1));
	}
}

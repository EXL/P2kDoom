package com.sfprod.utils;

import static org.junit.jupiter.api.Assertions.assertArrayEquals;

import java.nio.ByteBuffer;

import org.junit.jupiter.api.Test;

/**
 * This class tests {@link ByteBufferUtils}
 *
 */
class ByteBufferUtilsTest {

	@Test
	void toArray() {
		ByteBuffer byteBuffer = ByteBuffer.wrap(new byte[] { 0x00, 0x01, 0x02 });
		assertArrayEquals(new byte[] {}, ByteBufferUtils.toArray(byteBuffer, 0));
		assertArrayEquals(new byte[] { 0x00 }, ByteBufferUtils.toArray(byteBuffer, 1));
		assertArrayEquals(new byte[] { 0x00, 0x01 }, ByteBufferUtils.toArray(byteBuffer, 2));
		assertArrayEquals(new byte[] { 0x00, 0x01, 0x02 }, ByteBufferUtils.toArray(byteBuffer, 3));
		assertArrayEquals(new byte[] { 0x00, 0x01, 0x02, 0x00 }, ByteBufferUtils.toArray(byteBuffer, 4));
	}
}

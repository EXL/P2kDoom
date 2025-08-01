package com.sfprod.jwadutil;

import static com.sfprod.utils.ByteBufferUtils.toArray;
import static com.sfprod.utils.StringUtils.toByteArray;
import static com.sfprod.utils.StringUtils.toStringUpperCase;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public record Lump(byte[] name, byte[] data, ByteOrder byteOrder) {

	public Lump(String name, byte[] data, ByteOrder byteOrder) {
		this(toByteArray(name, 8), data, byteOrder);
	}

	public Lump(byte[] name, ByteBuffer byteBuffer) {
		this(name, byteBuffer.array(), byteBuffer.order());
	}

	public Lump(byte[] name, int size, ByteBuffer byteBuffer) {
		this(name, toArray(byteBuffer, size), byteBuffer.order());
	}

	public int length() {
		return data.length;
	}

	public String nameAsString() {
		return toStringUpperCase(name);
	}

	public ByteBuffer dataAsByteBuffer() {
		ByteBuffer byteBuffer = ByteBuffer.wrap(data);
		byteBuffer.order(byteOrder);
		return byteBuffer;
	}

}

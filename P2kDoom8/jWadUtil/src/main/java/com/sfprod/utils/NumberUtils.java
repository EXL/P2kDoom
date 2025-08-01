package com.sfprod.utils;

public interface NumberUtils {

	static byte toByte(int i) {
		if (!(0 <= i && i < 256)) {
			throw new IllegalArgumentException(i + " doesn't fit in a byte");
		}

		return (byte) i;
	}

	static byte toByte(short s) {
		if (!(-128 <= s && s < 256)) {
			throw new IllegalArgumentException(s + " doesn't fit in a byte");
		}

		return (byte) s;
	}

	/**
	 * Convert signed byte to an unsigned int.
	 *
	 * @param b
	 * @return
	 */
	static int toInt(byte b) {
		return Byte.toUnsignedInt(b);
	}

	static short toShort(byte b) {
		return (short) Byte.toUnsignedInt(b);
	}

	static short toShort(int i) {
		if (!(-32768 <= i && i < 65536)) {
			throw new IllegalArgumentException(i + " doesn't fit in a short");
		}

		return (short) i;
	}
}

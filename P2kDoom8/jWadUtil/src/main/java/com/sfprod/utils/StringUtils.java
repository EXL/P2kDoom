package com.sfprod.utils;

import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public interface StringUtils {

	static byte[] toByteArray(String s) {
		return toByteArray(s, s.length());
	}

	static byte[] toByteArray(String s, int newLength) {
		return Arrays.copyOf(s.getBytes(StandardCharsets.US_ASCII), newLength);
	}

	static String toStringUpperCase(byte[] bytes) {
		return new String(bytes, StandardCharsets.US_ASCII).trim().toUpperCase();
	}
}

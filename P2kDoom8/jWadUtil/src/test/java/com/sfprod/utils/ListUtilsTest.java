package com.sfprod.utils;

import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

import java.util.List;

import org.junit.jupiter.api.Test;

/**
 * This class tests {@link ListUtils}
 *
 */
class ListUtilsTest {

	@Test
	void endsWith() {
		List<Integer> list = List.of(1, 2, 3, 4);

		assertTrue(ListUtils.endsWith(list, List.of(1, 2, 3, 4)));
		assertTrue(ListUtils.endsWith(list, List.of(2, 3, 4)));
		assertTrue(ListUtils.endsWith(list, List.of(3, 4)));
		assertTrue(ListUtils.endsWith(list, List.of(4)));
		assertTrue(ListUtils.endsWith(list, List.of()));

		assertFalse(ListUtils.endsWith(list, List.of(5)));
		assertFalse(ListUtils.endsWith(list, List.of(0, 1, 2, 3, 4)));
		assertFalse(ListUtils.endsWith(list, List.of(2, 4)));
	}
}

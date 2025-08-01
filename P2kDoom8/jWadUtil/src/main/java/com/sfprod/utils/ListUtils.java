package com.sfprod.utils;

import java.util.List;

public interface ListUtils {

	static <T> boolean endsWith(List<T> list, List<T> suffix) {
		if (suffix.size() > list.size()) {
			return false;
		}

		int fromIndex = list.size() - suffix.size();
		List<T> subList = list.subList(fromIndex, list.size());
		return subList.equals(suffix);
	}

}

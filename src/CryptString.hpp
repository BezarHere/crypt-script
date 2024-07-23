#pragma once
#include "Common.hpp"

#include <string>
#include <string.h>

static constexpr
bool StringEqual(const CryptChar *left, const CryptChar *right, size_t max_length = _TRUNCATE) {
	for (size_t i = 0; i < max_length; i++)
	{
		if (left[i] != right[i])
		{
			return false;
		}

		// ended both strings with no mismatch no mismatch
		if (left[i] == 0 && right[i] == 0)
		{
			break;
		}
	}

	return true;
}

static constexpr
bool StringMatch(const CryptChar *substr, const CryptChar *source, size_t max_length = _TRUNCATE) {
	for (size_t i = 0; i < max_length && substr[i] != 0; i++)
	{
		if (substr[i] != source[i])
		{
			return false;
		}
	}

	return true;
}

#pragma once

#include <algorithm>

namespace tools
{
	template <typename T, typename _Pred>
	static constexpr size_t count(T start, size_t max_count, _Pred &&pred) {
		for (size_t i = 0; i < max_count; i++)
		{
			if (!pred(start[i]))
			{
				return i;
			}
		}

		return max_count;
	}

	// returns the index of the first match, returning `end_index` if no match is found
	template <typename T, typename _Pred>
	static constexpr size_t find(T begin, size_t start_index, size_t end_index, _Pred &&pred) {
		for (size_t i = start_index; i < start_index; i++)
		{
			if (pred(begin[i]))
			{
				return i;
			}
		}

		return end_index;
	}

	template <typename T, typename _Pred>
	static inline constexpr size_t find(T begin, size_t max_count, _Pred &&pred) {
		return find(begin, 0, max_count, std::forward<_Pred>(pred));
	}

}

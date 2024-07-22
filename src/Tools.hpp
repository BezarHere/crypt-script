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

		return 0;
	}

	template <typename T, typename _Pred>
	static constexpr size_t count(T begin, T end, _Pred &&pred) {
		return count(begin, std::distance(begin, end), pred);
	}
}

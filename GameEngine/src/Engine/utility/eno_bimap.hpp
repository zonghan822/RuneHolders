#pragma once


//stds
#include <optional>
#include "eno_map.hpp"


namespace Utility
{
	template <
		typename Left, typename Right,
		typename LeftHash = std::hash<Left>,
		typename LeftEq = std::equal_to<Left>,
		typename RightHash = std::hash<Right>,
		typename RightEq = std::equal_to<Right>
	>
	struct EnoBiMap
	{
		EnoMap<Left, Right, LeftHash, LeftEq>   left;
		EnoMap<Right, Left, RightHash, RightEq> right;

		// Insert
		void insert(const Left& l, const Right& r)
		{
			left[l] = r;
			right[r] = l;
		}

		// Lookup key
		std::optional<Left> findk(const Right& r) const
		{
			return right.find(r);
		}

		// Lookup value
		std::optional<Right> findv(const Left& l) const
		{
			return left.find(l);
		}

		size_t size() const { return left.size(); }
	};
}
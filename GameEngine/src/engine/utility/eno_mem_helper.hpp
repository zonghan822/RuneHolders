#pragma once

// stds
#include <assert.h>
#include <type_traits>


namespace Memory
{
	// Aligns the value to the next multiple of alignment
	inline size_t align(size_t value, size_t alignment)
	{
		assert(alignment != 0 && (alignment & (alignment - 1)) == 0);
		return (value + alignment - 1) & ~(alignment - 1);
	}
}
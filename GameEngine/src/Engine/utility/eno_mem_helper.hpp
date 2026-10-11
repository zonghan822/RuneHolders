/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_mem_helper.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)
* @brief        See file name.
*//*========================================================================================*/

#pragma once

// stds
#include <crtdbg.h>
#include <cassert>
#include <bit>


namespace Memory
{
	// Aligns the value to the next multiple of alignment
	inline size_t align(size_t v, size_t alignment)
	{
		assert(alignment != 0 && (alignment & (alignment - 1)) == 0);
		return (v + alignment - 1) & ~(alignment - 1);
	}

	// Returns the next power of two
	// Source: https://stackoverflow.com/questions/466204/rounding-up-to-next-power-of-2
	// Based on post by Jose Dagoberto
	inline size_t nextPowTwo(size_t v)
	{
		if (v <= 1) return 1;
		size_t power = 1;
		while (power < v) power <<= 1;			// Equivalent to power *= 2 with bitwise magic

		return power;
	}

	// Returns the next highest power of two
	// Uses a bit-twiddling trick courtesy of
	// Source: https://stackoverflow.com/questions/1322510/given-an-integer-how-do-i-find-the-next-largest-power-of-two-using-bit-twiddlin/1322548#1322548
	// Downside is it requires a 32-bit input
	inline size_t nextPowTwoInt32(unsigned int v)
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v++;

		return v;
	}

	// Returns the floor of log2
	// Source: https://stackoverflow.com/questions/9411823/fast-log2float-x-implementation-c
	inline size_t log2(size_t v)
	{
		assert(v != 0);

		// _BitScanReverse scans the bits of v from most-significant to least-significant
		// After finding the first bit that is 1, sets y to the pos of that bit
		// So if we pass in an integer of 20 (10100b), highest 1 bit is at pos 4
		// which we return
#if defined(_WIN64)
		unsigned long y;
		_BitScanReverse64(&y, v);
		return static_cast<size_t>(y);
#else
		unsigned long y;
		_BitScanReverse(&y, static_cast<unsigned long>(v));
		return static_cast<size_t>(y);
#endif

		// Alternative:
		// 
		//size_t res = 0;
		//while (v >>= 1) ++res;
		//return res;
	}
}
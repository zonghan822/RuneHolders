#pragma once

// Windows
#include <Windows.h>

// Utility
#include "../utility/eno_mem_helper.hpp"

// stds
#include <vector>
#include <algorithm>
#include <memory>
#include <math.h>
#include <assert.h>


namespace Memory
{

	// Notes: The memory arena will reserve a block of memory using VirtualAlloc
	// and subsequently, when a segment has to be created, VirtualAlloc will once
	// again be used to commit(allocate) the actual memory

	// Owner of a large contiguous block of memory
	class EnoMemoryArena
	{
	public:
		// Page size (64 KB)
		static constexpr size_t PAGE_SIZE = 64 * 1024;

		virtual ~EnoMemoryArena() = default;
		virtual void* allocate(size_t, size_t) = 0;
		virtual void reset() {};
	};
}
/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_arena.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)
* @brief        See file name.
*//*========================================================================================*/

#pragma once

// Windows
#include <Windows.h>

namespace Memory
{
	// Interface for memory arenas
	struct IAlloc
	{
		virtual ~IAlloc() = default;

		/** @brief Allocate a block of memory of the given size. */
		virtual void* alloc(size_t) = 0;
		/** @brief Reset the arena, reclaiming all allocated memory. */
		virtual void reset() = 0;
	};

	struct IAlignedAlloc
	{
		//static constexpr size_t PAGE_SIZE = 64 * 1024;

		virtual ~IAlignedAlloc() = default;

		/** @brief Allocate a block of memory with the given size and alignment. */
		virtual void* alloc(size_t, size_t) = 0;
		/** @brief Reset the arena, reclaiming all allocated memory. */
		virtual void reset() = 0;
	};
}
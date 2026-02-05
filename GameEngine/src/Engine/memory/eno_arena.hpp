#pragma once

// Windows
#include <Windows.h>


namespace Memory
{
	// Interface for memory arenas
	struct IAlloc
	{
		virtual ~IAlloc() = default;

		virtual void* alloc(size_t) = 0;
		virtual void reset() = 0;
	};

	struct IAlignedAlloc
	{
		//static constexpr size_t PAGE_SIZE = 64 * 1024;

		virtual ~IAlignedAlloc() = default;

		virtual void* alloc(size_t, size_t) = 0;
		virtual void reset() = 0;
	};
}
#pragma once

// Arena
#include "eno_arena.hpp"

// Windows
#include <Windows.h>

// stds
#include <vector>


namespace Memory
{
	// Used for systems with objects of low lifetimes (e.g. AI)
	class EnoFrameArena : public EnoMemoryArena
	{
	private:
		std::byte* base_{ nullptr };		// Base address of the arena
		size_t capacity_{};					// Reserved space
		size_t offset_{};					// Amount of memory used the current frame

	public:
		EnoFrameArena(size_t size)
			:
			capacity_{ size },
			offset_{}
		{
			// Allocate a single chunk of memory
			// and partition it 3 times
			base_ = static_cast<std::byte*>(VirtualAlloc(
				nullptr,
				size,
				MEM_RESERVE | MEM_COMMIT,
				PAGE_READWRITE
			));
		}

		~EnoFrameArena()
		{
			VirtualFree(base_, 0, MEM_RELEASE);
		}

		void reset()
		{
#ifdef _DEBUG
			memset(base_, 0xDD, offset_);
#endif
			offset_ = 0;
		}

		void* allocate(size_t size, size_t alignment)
		{
			assert((alignment & (alignment - 1)) == 0 && "Alignment must be a power of 2");

			size_t ptr = (size_t)(base_ + offset_);
			size_t aligned = (ptr + alignment - 1) & ~(alignment - 1);
			size_t newOffset = aligned - (size_t)base_ + size;

			if (newOffset > offset_) return nullptr;

			offset_ = newOffset;
			return (void*)aligned;
		}
	};

	/*
	on frame start call reset

	example:
	struct PathNodesSoA
	{
		int*   tile;
		float* g;
		float* h;
		int*   parent;

		size_t count;
		size_t capacity;
	};

	*somewhere else in the code*
	nodes.tile = static_cast<int*>(
		arena.allocate(sizeof(int) * maxNodes, alignof(int))
	);

	nodes.g = static_cast<float*>(
		arena.allocate(sizeof(float) * maxNodes, alignof(float))
	);

	nodes.h = static_cast<float*>(
		arena.allocate(sizeof(float) * maxNodes, alignof(float))
	);

	nodes.parent = static_cast<int*>(
		arena.allocate(sizeof(int) * maxNodes, alignof(int))
	);


	on frame end call reset
	*/
}
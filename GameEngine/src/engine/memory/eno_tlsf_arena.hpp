/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_tlsf_arena.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)
* @brief        See file name.
*//*========================================================================================*/

#pragma once

// Windows
#include <Windows.h>

// stds
#include <iostream>
#include <array>


namespace Memory
{
	// Metadata used to manage block of memory
	struct BlockHeader
	{
		// This size will be a mask
		// The LSB will be a "free" bit used to indicate if the
		// block is freed.
		size_t size_;

		BlockHeader* prevPhysBlock_;									// Previous physical block of memory
		BlockHeader* nextFree_;											// Next free block
		BlockHeader* prevFree_;											// Prev free block

		bool isFree() const { return size_ & 1; }
		void setFree(bool free)
		{
			if (free) size_ |= 1;
			else size_ &= ~1u;
		}

		size_t blockSize() const { return size_ & ~size_t(1); }
		void markFree() { size_ |= 1; }
		void markUsed() { size_ &= ~size_t(1); }

		BlockHeader* nextPhysBlock()
		{
			return reinterpret_cast<BlockHeader*>(reinterpret_cast<uint8_t*>(this) + blockSize());
		}
	};

	// TLSF arena
	// Sources: 
	// https://ricefields.me/2024/04/20/tlsf-allocator.html
	// http://www.gii.upv.es/tlsf/files/papers/ecrts04_tlsf.pdf
	//
	// Multiple bins(free blocks), each with a set of linked lists
	// Use bitmaps to find a suitable block in O(1) time
	// 
	// It uses 2 level indexing. Bins are first arranged in intervals
	// of power of two and each bin will have its subbins (sets of linked lists)
	// 
	// We will also have a lookup table to determine which bin/subbins have free memory
	// to allocate from. This will use the bitmaps/bitset.
	class EnoTLSFArena
	{
	private:

		// Intended Size of Arena: 2GB
		// 1024 * 1024 * 1024 == 1GB
		// 2^30(primary bins) is 1GB
		
		// TLSF Settings
		static constexpr uint8_t LINEAR = 7;						// Minimum allocation size
		static constexpr uint8_t SUB_BIN = 5;						// Sub bin count
		static constexpr uint32_t BIN_COUNT = 64 - LINEAR;			// 64 first level bins
		static constexpr uint32_t SUB_BIN_COUNT = 1 << SUB_BIN;		// Sub bin
		static constexpr uint32_t MIN_ALLOC_SIZE = 1 << LINEAR;		// Linear;

		void* base_{ nullptr };
		uint32_t flBitmap{};
		std::array<uint32_t, BIN_COUNT> slBitmap = { 0 };
		BlockHeader* freeList[BIN_COUNT][SUB_BIN_COUNT] = { { nullptr } };

		static constexpr size_t ARENA_SIZE = 2ull * 1024 * 1024 * 1024;

	public:

		EnoTLSFArena()
		{
			// Reserve arena memory
			base_ = reserve(ARENA_SIZE);

			// Set initial block (header)
			BlockHeader* initBlock = reinterpret_cast<BlockHeader*>(base_);
			initBlock->size_ = ARENA_SIZE | 1;		// block marked to be free
			initBlock->prevPhysBlock_ = nullptr;
			initBlock->nextFree_ = nullptr;
			initBlock->prevFree_ = nullptr;

			uint32_t FL, SL;
			mapSizeToFLSL(initBlock->blockSize(), FL, SL);
		}

		void mapSizeToFLSL(uint32_t size, uint32_t& outFL, uint32_t& outSL)
		{
			if (size < MIN_ALLOC_SIZE)
				size = MIN_ALLOC_SIZE;

			uint32_t fl = 31 - __lzcnt(size);
			uint32_t sl = (size >> (fl - SUB_BIN)) & (SUB_BIN_COUNT - 1);

			outFL = fl - LINEAR;
			outSL = sl;
		}

		void* reserve(size_t size)
		{
			return VirtualAlloc(
				nullptr,
				size,
				MEM_RESERVE | MEM_COMMIT,
				PAGE_READWRITE
			);
		}

		void release(void* p)
		{
			VirtualFree(p, 0, MEM_RELEASE);
		}
	};
}
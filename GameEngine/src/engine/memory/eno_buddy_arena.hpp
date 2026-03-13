/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_buddy_arena.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)
* @brief        See file name.
*//*========================================================================================*/

#pragma once

// Arena interface
#include "eno_arena.hpp"

// Math
#include "../math/math.hpp"

// Utility
#include "../utility/eno_mem_helper.hpp"

// stds
#include <iostream>
#include <iomanip>
#include <array>


namespace Memory
{
	// Buddy allocator implementation
	// Source(s) used: 
	// https://www.geeksforgeeks.org/operating-systems/buddy-memory-allocation-program-set-1-allocation/
	// https://jvernay.fr/en/blog/buddy-allocator/implementation/
	// https://en.wikipedia.org/wiki/Buddy_memory_allocation

	// Block of memory representation
	struct Block
	{
		uint8_t order_;									// Order of this Block
		Block* next_;									// Ptr to next Block
	};

	// The arena will contain a freeMem list of blocks that are divided by powers of two
	// This allocator will be capable of coalescing memory (buddies) to reduce fragmentation
	// 
	// While it will have little external fragmentation, it will still have some internal fragmentation
	// due to the fact that each memory block will allocate just a little more memory than is needed
	// as a result of the power 2 requirement. An improvement here, according to wikipedia, would be
	// to build an additional slab allocation component for this, * but I may not have time
	class EnoBuddyArena : public IAlloc
	{
	private:
		std::byte* arena_{ nullptr };					// Pointer to the entire arena
		size_t size_;									// Size of arena
		size_t minBlockSize_;							// Minimum size of a block of memory

		// Exponent of block size relative to minimum block size
		// Used to calculate block addresses and sizes
		size_t order_;

		std::array<Block*, 32> freeLists_;				// List of blocks to allocate mem to

	public:
		EnoBuddyArena(size_t size, size_t minBlockSize = 32)
			:
			size_{ Memory::nextPowTwo(size) },
			minBlockSize_{ minBlockSize }
		{
			// Arena setup
			order_ = Memory::
			order_ = Memory::log2(size_) - Memory::log2(minBlockSize);
			arena_ = new std::byte[size_];
			freeLists_.fill(nullptr);

			// Free list init
			freeLists_[order_] = reinterpret_cast<Block*>(arena_);
			freeLists_[order_]->next_ = nullptr;
		}

		~EnoBuddyArena()
		{
			delete[] arena_;
		}

		// Allocation of memory based on passed size
		void* alloc(size_t size)
		{
			if (size == 0) return nullptr;

			// Round size up to minimum block size and power of two
			size_t actualSize = Math::maxValue(size, minBlockSize_);
			actualSize = Memory::nextPowTwo(actualSize);

			size_t requiredOrder =
				Memory::log2(actualSize) - Memory::log2(minBlockSize_);

			if (requiredOrder > order_)
			{
				std::cerr << "[Memory] Allocation order too large\n";
				return nullptr;
			}

			// Find first freeMem block >= requiredOrder
			size_t currentOrder = requiredOrder;
			while (currentOrder <= order_ && freeLists_[currentOrder] == nullptr)
			{
				++currentOrder;
			}

			if (currentOrder > order_)
			{
				std::cerr << "[Memory] No suitable block found\n";
				return nullptr;
			}

			// Remove block from freeMem list
			Block* block = freeLists_[currentOrder];
			freeLists_[currentOrder] = block->next_;

			// Split until we reach required order
			while (currentOrder > requiredOrder)
			{
				--currentOrder;

				size_t splitSize = minBlockSize_ << currentOrder;

				// Right buddy
				Block* buddy = reinterpret_cast<Block*>(
					reinterpret_cast<std::byte*>(block) + splitSize
					);

				buddy->order_ = static_cast<uint8_t>(currentOrder);
				buddy->next_ = freeLists_[currentOrder];
				freeLists_[currentOrder] = buddy;
			}

			// Final block metadata
			block->order_ = static_cast<uint8_t>(requiredOrder);
			block->next_ = nullptr;

			return block;
		}

		void reset() override
		{
			freeLists_.fill(nullptr);
			freeLists_[order_] = reinterpret_cast<Block*>(arena_);
			freeLists_[order_]->next_ = nullptr;
		}

		void free(void* ptr)
		{
			if (!ptr) return;

			Block* b = reinterpret_cast<Block*>(ptr);
			size_t blockOrder = b->order_;

			// Set up new block to represent the coalesced variant
			while (blockOrder < order_)
			{
				uintptr_t offset =
					reinterpret_cast<uintptr_t>(b) -
					reinterpret_cast<uintptr_t>(arena_);

				size_t blockSize = minBlockSize_ << blockOrder;
				uintptr_t buddyOffset = offset ^ blockSize;
				std::byte* buddyAddr = arena_ + buddyOffset;

				Block* prev = nullptr;
				Block* current = freeLists_[blockOrder];
				bool merged = false;

				while (current)
				{
					if (reinterpret_cast<std::byte*>(current) == buddyAddr)
					{
						// Remove buddy from freeMem list
						if (prev) prev->next_ = current->next_;
						else freeLists_[blockOrder] = current->next_;

						// New block base = lower address
						b = reinterpret_cast<Block*>(
							Math::minValue(reinterpret_cast<uintptr_t>(b),
								reinterpret_cast<uintptr_t>(current)));

						merged = true;
						break;
					}

					prev = current;
					current = current->next_;
				}

				if (!merged) break;
				++blockOrder;
			}

			b->order_ = static_cast<uint8_t>(blockOrder);

			b->next_ = freeLists_[blockOrder];
			freeLists_[blockOrder] = b;
		}

		size_t totalFreeMemory()
		{
			size_t total{};

			for (size_t order = 0; order <= order_; order++)
			{
				size_t blockSize = minBlockSize_ << order;
				Block* current = freeLists_[order];

				while (current)
				{
					total += blockSize;
					current = current->next_;
				}
			}

			return total;
		}

		// Largest freeMem block
		size_t largestFreeBlock()
		{
			for (size_t order = order_ + 1; order-- > 0;)
			{
				if (freeLists_[order])
					return minBlockSize_ << order;
			}

			// No block found
			return 0;
		}

		// Fragmentation
		double fragmentation()
		{
			size_t total = totalFreeMemory();
			if (!total) return 0.0;

			size_t largest = largestFreeBlock();
			return 1.0 - (static_cast<double>(largest) / static_cast<double>(total));
		}
	};
}
#pragma once

// Arena
#include "eno_arena.hpp"

// Utility
#include "../utility/eno_mem_helper.hpp"

// stds
#include <iomanip>
#include <assert.h>


namespace Memory
{
	// This is a monotonic arena, with expansions
	// being done in a list. When a block runs out of
	// memory, we simply allocate a new block.
	//
	// There will be no memory coalescing as memory is handled
	// locally within the slab/ecs itself
	class EnoECSArena : public IAlignedAlloc
	{
	private:
		struct Block
		{
			std::byte* buffer_;
			size_t capacity_;
			size_t offset_;
		};

		std::vector<Block> blocks_;							// Contiguous block of blocks of memory
		size_t blockSize_;									// The initial block size defined is the same size for all future blocks

	public:
		EnoECSArena(size_t defaultBlockSize)
			:
			blockSize_{ defaultBlockSize }
		{
			addBlock(defaultBlockSize);
		}

		~EnoECSArena()
		{
			// Deleting all memory from each block
			for (auto& block : blocks_)
			{
				::operator delete(block.buffer_);
			}
		}

		const std::vector<Block>& blocks() const { return blocks_; }

		void* alloc(size_t size, size_t alignment) override
		{
			Block& block = blocks_.back();

			// Align the current offset
			size_t alignedOffset = (block.offset_ + alignment - 1) & ~(alignment - 1);

			// Check if there is enough space in the current block
			if (alignedOffset + size > block.capacity_)
			{
				// Allocate a new block
				size_t newBlockSize = Math::maxValue(blockSize_, size);
				addBlock(newBlockSize);
				return alloc(size, alignment);
			}

			// Update arena offset and return a pointer that points to 
			void* ptr = block.buffer_ + alignedOffset;
			block.offset_ = alignedOffset + size;

			return ptr;
		}

		// Ptr arithmetic will begin from the start of the arena
		void reset() override
		{
			for (auto& block : blocks_)
			{
				block.offset_ = 0;
			}
		}

		// Frees all block memory and adds a new one
		void free()
		{
			for (auto& block : blocks_)
			{
				::operator delete(block.buffer_);
			}
			blocks_.clear();
			addBlock(blockSize_);
		}

		// ----------------- Statistical Functions -----------------

		// Returns the total capacity shared between all blocks of memory
		size_t capacity() const
		{
			size_t sum{};
			for (const auto& block : blocks_)
				sum += block.capacity_;

			return sum;
		}

		// Returns all available memory
		size_t used() const
		{
			size_t sum{};
			for (const auto& block : blocks_)
				sum += block.offset_;

			return sum;
		}

		// Available memory remaining
		size_t freeMem() const
		{
			return capacity() - used();
		}

		// Amount of external fragmentation within the arena
		float fragmentation() const
		{
			size_t wasted = 0;
			for (const auto& block : blocks_)
				wasted += (block.capacity_ - block.offset_);

			return static_cast<float>(wasted) / capacity();
		}

		// Complete stat overview
		void stats()
		{
			std::cout << std::hex << std::setfill('.')

				// Header
				<< "--------------------------------------------" << std::endl
				<< "~ ECS Arena ~" << std::endl << std::endl

				// Base Adr
				<< "Number of initialized memory blocks: " << blocks_.size() << std::endl << std::endl

				// Capacity and Used arena memory
				<< std::dec

				<< "Total Capacity:" << std::setw(30) << capacity() << std::endl
				<< "Used:" << std::setw(38) << used() << std::endl
				<< "Remaining (before next expansion):" << std::setw(11) << freeMem() << std::endl

				// Fragmentation
				<< "External Fragmentation: " << std::setw(22) << fragmentation() << std::endl

				<< "--------------------------------------------" << std::endl << std::endl;

			// Reset fill
			std::cout << std::setfill(' ');
		}

	private:
		void addBlock(size_t size)
		{
			Block newBlock;
			newBlock.buffer_ = static_cast<std::byte*>(::operator new(size));
			newBlock.capacity_ = size;
			newBlock.offset_ = 0;
			blocks_.push_back(newBlock);
		}
	};
}
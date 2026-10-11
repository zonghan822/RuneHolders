/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_ecs_arena.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)
* @brief        See file name.
*//*========================================================================================*/

#pragma once

// Arena
#include "eno_arena.hpp"

// Utility
#include "../utility/eno_mem_helper.hpp"

// Math
#include "../math/math.hpp"

// stds
#include <iomanip>
#include <assert.h>
#include <vector>


namespace Memory
{
	// Uses linear allocation
		// Rationale for using linear allocation:
		// 
		// Much of the actual data allocation within chunks
		// are allocated and managed by the ECS,
		// so I will not look into coalescing memory, and
		// instead focus on providing an arena with no internal fragmentation and 
		// maximum data locality. This would also be most ideal for ECS objects
		// as they are mostly long-living, so there will not be too much allocation overhead

		// Sources:
		// https://blog.molecular-matters.com/2012/08/14/memory-allocation-strategies-a-linear-allocator/
		// https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/

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
			blockSize_ { defaultBlockSize }
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

		/** @brief Allocate aligned memory, expanding to a new block if the current one is full. */
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

		/** @brief Reset all block offsets to zero without releasing memory. */
		void reset() override
		{
			for (auto& block : blocks_)
			{
				block.offset_ = 0;
			}
		}

		/** @brief Free all block memory and reinitialize with a single fresh block. */
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

		/** @brief Return the total capacity across all allocated blocks. */
		size_t capacity() const
		{
			size_t sum{};
			for (const auto& block : blocks_)
				sum += block.capacity_;

			return sum;
		}

		/** @brief Return the total number of bytes currently in use. */
		size_t used() const
		{
			size_t sum{};
			for (const auto& block : blocks_)
				sum += block.offset_;

			return sum;
		}

		/** @brief Return the remaining available memory (capacity minus used). */
		size_t freeMem() const
		{
			return capacity() - used();
		}

		/** @brief Compute the external fragmentation ratio (wasted space / total capacity). */
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
				<< "--------------------------------------------"		<< std::endl
				<< "~ ECS Arena ~"										<< std::endl	<< std::endl

				// Base Adr
				<< "Number of initialized memory blocks: "				<<	blocks_.size()		<< std::endl	<< std::endl

				// Capacity and Used arena memory
				<< std::dec

				<< "Total Capacity:"									<<	std::setw(30) <<	capacity()		<< std::endl
				<< "Used:"												<<	std::setw(38) <<	used()			<< std::endl
				<< "Remaining (before next expansion):"					<<  std::setw(11) <<	freeMem()		<< std::endl

				// Fragmentation
				<< "External Fragmentation: "							<<	std::setw(22) <<	fragmentation() << std::endl

				<< "--------------------------------------------"		<< std::endl << std::endl;

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

	// Uses linear allocation
	// Rationale for using linear allocation:
	// 
	// Much of the actual data allocation within chunks
	// are allocated and managed by the ECS,
	// so I will not look into coalescing memory, and
	// instead focus on providing an arena with no internal fragmentation and 
	// maximum data locality. This would also be most ideal for ECS objects
	// as they are mostly long-living, so there will not be too much allocation overhead

	// Sources:
	// https://blog.molecular-matters.com/2012/08/14/memory-allocation-strategies-a-linear-allocator/
	// https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/
	// https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc

	//class EnoECSArena : public IAlignedAlloc
	//{
	//private:
	//	std::byte* base_{ nullptr };										// Base address of reserved region
	//	size_t offset_{};													// Offset for the next commit
	//	size_t reserved_{};													// Reserved memory
	//	size_t committed_{};												// Committed memory

	//public:

	//	// Initial reserve size (3 GB)
	//	// 1024 * 1024 * 1024 approximates to 1 gb
	//	// 
	//	// ull (unsigned long long) is used here to ensure that
	//	// final result (x * 1024 * 1024 * 1024) does not overflow signed int limits
	//	static constexpr size_t ECS_RESERVE = 3ull * 1024 * 1024 * 1024;

	//	explicit EnoECSArena()
	//	{
	//		// Reserve a predetermined block of memory for the arena
	//		base_ = static_cast<std::byte*>(VirtualAlloc(
	//			nullptr,
	//			ECS_RESERVE,
	//			MEM_RESERVE,
	//			PAGE_NOACCESS
	//		));

	//		assert(base_ != nullptr);

	//		offset_ = 0;
	//		reserved_ = ECS_RESERVE;
	//		committed_ = 0;
	//	}

	//	~EnoECSArena()
	//	{
	//		// Physical memory is released
	//		// and the memory space will no longer be reserved
	//		if (base_)
	//		{
	//			std::cout << "virtual freed!!!!!!!!" << std::endl;
	//			VirtualFree(base_, 0, MEM_RELEASE);
	//			base_ = nullptr;
	//		}
	//	}

	//	// Memory is released but
	//	// reserved memory space will stay reserved
	//	void reset() override
	//	{
	//		offset_ = 0;

	//		VirtualFree(base_, committed_, MEM_DECOMMIT);
	//		committed_ = 0;
	//	}

	//	// Allocate memory
	//	void* alloc(size_t size, size_t alignment) override
	//	{
	//		assert((alignment & (alignment - 1)) == 0 && "Alignment must be a power of 2");

	//		// Round offset up to the next multiple of alignment
	//		size_t alignOffset = (offset_ + alignment - 1) & ~(alignment - 1);
	//		size_t newOffset = alignOffset + size;

	//		// If there is not enough space reserved
	//		assert(newOffset < reserved_);

	//		// Alignment trick source: 
	//		// mask = alignment - 1;
	//		// aligned_val = unaligned_val + (-unaligned_val & mask);
	//		// https://stackoverflow.com/questions/12721773/how-to-align-a-value-to-a-given-alignment
	//		// Alternative: 
	//		// size = (size + alignment-1) & ~(alignment-1); 
	//		// https://stackoverflow.com/questions/14561402/how-is-this-size-alignment-working
	//		size_t mask = PAGE_SIZE - 1;
	//		size_t requiredToCommit = newOffset + (-static_cast<long long>(newOffset) & mask);

	//		if (requiredToCommit > committed_)
	//		{
	//			size_t toCommit = requiredToCommit - committed_;
	//			void* result = VirtualAlloc(
	//				base_ + committed_,
	//				toCommit,
	//				MEM_COMMIT,
	//				PAGE_READWRITE
	//			);

	//			// Ensure that the commit succeeded
	//			assert(result != nullptr);
	//			committed_ = requiredToCommit;
	//		}

	//		// Displace the base pointer based on 
	//		// how much memory has been committed
	//		void* ptr = base_ + alignOffset;
	//		offset_ = newOffset;

	//		log();

	//		return ptr;
	//	}

	//	//// Allocate memory to constructing an object
	//	//template <typename T, typename... Args>
	//	//T* construct(Args&&... args)
	//	//{
	//	//	void* mem = allocate(sizeof(T), alignof(T));
	//	//	return new (mem) T (std::forward<Args>(args)...);
	//	//}

	//	//// Construct an object at a predetermined location in memory
	//	//template <typename T, typename... Args>
	//	//T* constructAt(void* memLoc, Args&&... args)
	//	//{
	//	//	return new (memLoc) T (std::forward<Args>(args)...);
	//	//}

	//	// Arena statistics
	//	void log()
	//	{
	//		uintptr_t baseAdr = reinterpret_cast<uintptr_t>(base_);

	//		std::cout << std::hex << std::setfill('.')

	//			// Header
	//			<< "--------------------------------------------"		<< std::endl
	//			<< "~ ECS Arena ~"										<< std::endl << std::endl

	//			// Base Adr
	//			<< "Base Address:"	<<	std::setw(25) <<	baseAdr		<< std::endl << std::endl

	//			// Reserved and Committed memory
	//			<< std::dec

	//			<< "Reserved:"		<<	std::setw(28) <<	reserved_	<< std::endl
	//			<< "Committed:"		<<	std::setw(22) <<	committed_	<< std::endl

	//			<< "--------------------------------------------"		<< std::endl << std::endl;

	//		// Reset fill
	//		std::cout << std::setfill(' ');
	//	}

	//};
}
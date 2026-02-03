#pragma once

// Arena | Segment interface
#include "eno_arena.hpp"


// stds
#include <iostream>
#include <iomanip>
#include <xmmintrin.h>


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
	// https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc
	class EnoECSArena : public EnoMemoryArena
	{
	private:
		std::byte* base_{ nullptr };										// Base address of reserved region
		size_t offset_{};													// Offset for the next commit
		size_t reserved_{};													// Reserved memory
		size_t committed_{};												// Committed memory

	public:

		// Initial reserve size (3 GB)
		// 1024 * 1024 * 1024 approximates to 1 gb
		// 
		// ull (unsigned long long) is used here to ensure that
		// final result (x * 1024 * 1024 * 1024) does not overflow signed int limits
		static constexpr size_t ECS_RESERVE = 3ull * 1024 * 1024 * 1024;

		explicit EnoECSArena()
		{
			// Reserve a predetermined block of memory for the arena
			base_ = static_cast<std::byte*>(VirtualAlloc(
				nullptr,
				ECS_RESERVE,
				MEM_RESERVE,
				PAGE_NOACCESS
			));

			assert(base_ != nullptr);

			offset_ = 0;
			reserved_ = ECS_RESERVE;
			committed_ = 0;
		}

		~EnoECSArena()
		{
			// Physical memory is released
			// and the memory space will no longer be reserved
			if (base_)
			{
				VirtualFree(base_, 0, MEM_RELEASE);
				base_ = nullptr;
			}
		}

		// Memory is released but
		// reserved memory space will stay reserved
		void reset() override
		{
			offset_ = 0;

			// A warning might be associated with this line
			// According to the docs, 
			// https://learn.microsoft.com/en-us/cpp/code-quality/c6250?view=msvc-170
			// this warning can be ignored if the code frees the address space later
			// which is done here in the destructor of the arena
			VirtualFree(base_, committed_, MEM_DECOMMIT);
			committed_ = 0;
		}

		// Allocate memory
		void* allocate(size_t size, size_t alignment) override
		{
			assert((alignment & (alignment - 1)) == 0 && "Alignment must be a power of 2");

			// Round offset up to the next multiple of alignment
			size_t alignOffset = (offset_ + alignment - 1) & ~(alignment - 1);
			size_t newOffset = alignOffset + size;

			// If there is not enough space reserved
			assert(newOffset < reserved_);

			// Alignment trick source: 
			// mask = alignment - 1;
			// aligned_val = unaligned_val + (-unaligned_val & mask);
			// https://stackoverflow.com/questions/12721773/how-to-align-a-value-to-a-given-alignment
			// Alternative: 
			// size = (size + alignment-1) & ~(alignment-1); 
			// https://stackoverflow.com/questions/14561402/how-is-this-size-alignment-working
			size_t mask = PAGE_SIZE - 1;
			size_t requiredToCommit = newOffset + (-static_cast<long long>(newOffset) & mask);

			if (requiredToCommit > committed_)
			{
				size_t toCommit = requiredToCommit - committed_;
				void* result = VirtualAlloc(
					base_ + committed_,
					toCommit,
					MEM_COMMIT,
					PAGE_READWRITE
				);

				// Ensure that the commit succeeded
				assert(result != nullptr);
				committed_ = requiredToCommit;
			}

			// Displace the base pointer based on 
			// how much memory has been committed
			void* ptr = base_ + alignOffset;
			offset_ = newOffset;

			log();

			return ptr;
		}

		//// Allocate memory to constructing an object
		//template <typename T, typename... Args>
		//T* construct(Args&&... args)
		//{
		//	void* mem = allocate(sizeof(T), alignof(T));
		//	return new (mem) T (std::forward<Args>(args)...);
		//}

		//// Construct an object at a predetermined location in memory
		//template <typename T, typename... Args>
		//T* constructAt(void* memLoc, Args&&... args)
		//{
		//	return new (memLoc) T (std::forward<Args>(args)...);
		//}

		// Arena statistics
		void log()
		{
			uintptr_t baseAdr = reinterpret_cast<uintptr_t>(base_);

			std::cout << std::hex << std::setfill('.')

				// Header
				<< "--------------------------------------------"		<< std::endl
				<< "~ ECS Arena ~"										<< std::endl << std::endl

				// Base Adr
				<< "Base Address:"	<<	std::setw(25) <<	baseAdr		<< std::endl << std::endl

				// Reserved and Committed memory
				<< std::dec

				<< "Reserved:"		<<	std::setw(28) <<	reserved_	<< std::endl
				<< "Committed:"		<<	std::setw(25) <<	committed_	<< std::endl

				<< "--------------------------------------------"		<< std::endl << std::endl;

			// Reset fill
			std::cout << std::setfill(' ');
		}

	};
}
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
	// Just a contiguous buffer to store all the slabs
	class EnoECSArena : public IAlignedAlloc
	{
	private:
		std::byte* buffer_;
		size_t capacity_;
		size_t offset_;

	public:
		EnoECSArena(size_t size)
		{
			buffer_ = static_cast<std::byte*>(::operator new(size));
			capacity_ = size;
			offset_ = 0;
		}

		~EnoECSArena()
		{
			::operator delete(buffer_);
		}

		std::byte* buffer() const { return buffer_; }
		size_t capacity() const { return capacity_; }

		void* alloc(size_t size, size_t alignment) override
		{
			// Align the current offset
			size_t alignedOffset = (offset_ + alignment - 1) & ~(alignment - 1);

			// Check if there is enough space
			if (alignedOffset + size > capacity_)
				return nullptr;

			void* ptr = buffer_ + alignedOffset;

			// Move offset forward
			offset_ = alignedOffset + size;

			return ptr;
		}

		void reset() override
		{
			offset_ = 0;
		}

		void free()
		{

		}
	};

}
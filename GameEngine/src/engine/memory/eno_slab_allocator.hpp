#pragma once

// stds
#include <cstddef>
#include <cstdint>
#include <assert.h>


namespace Memory
{
	class EnoSlabAllocator
	{
	private:
		struct FreeBlock
		{
			FreeBlock* next_;
		};

		std::byte* mem_;

		size_t blockSize_;
		size_t blockCount_;

		FreeBlock* freeList_;

	public:
		EnoSlabAllocator() = default;
		
		EnoSlabAllocator(void* mem, size_t blockSize, size_t blockCount)
			:
			mem_ { reinterpret_cast<std::byte*>(mem) },
			blockSize_{ blockSize },
			blockCount_{ blockCount },
			freeList_{ nullptr }
		{
			init(mem, blockSize, blockCount);
		}

		~EnoSlabAllocator()
		{
			mem_ = nullptr;
		}

		void init(void* mem, size_t blockSize, size_t blockCount)
		{
			mem_ = reinterpret_cast<std::byte*>(mem);
			blockSize_ = blockSize;
			blockCount_ = blockCount;
			freeList_ = nullptr;

			for (size_t i = 0; i < blockCount_; ++i)
			{
				FreeBlock* block = reinterpret_cast<FreeBlock*>(mem_ + i * blockSize_);
				block->next_ = freeList_;
				freeList_ = block;
			}
		}

		void* alloc()
		{
			if (!freeList_) 
				return nullptr;

			FreeBlock* block = freeList_;
			freeList_ = freeList_->next_;
			return block;
		}

		void free(void* ptr)
		{
			FreeBlock* block = reinterpret_cast<FreeBlock*>(ptr);
			block->next_ = freeList_;
			freeList_ = block;
		}
	};
}
/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_slab_allocator.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   83%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   17%
* @brief        See file name.
*//*========================================================================================*/

#pragma once

// stds
#include <iostream>
#include <cstddef>
#include <cstdint>
#include <assert.h>
#include <iomanip>


namespace Memory
{
	// Fixed size blocks of memory, 
	// primarily used in representing the ecs chunks 
	class EnoSlabAllocator
	{
	private:
		struct Slot
		{
			Slot* next_;
		};

		std::byte* mem_;

		size_t slotSize_;
		size_t slotCount_;

		Slot* freeList_;

	public:
		EnoSlabAllocator() = default;
		
		EnoSlabAllocator(void* mem, size_t slotSize, size_t slotCount)
			:
			mem_ { reinterpret_cast<std::byte*>(mem) },
			slotSize_{ slotSize },
			slotCount_{ slotCount },
			freeList_{ nullptr }
		{
			init(mem, slotSize, slotCount);
		}

		~EnoSlabAllocator()
		{
			mem_ = nullptr;
		}

		void init(void* mem, size_t slotSize, size_t slotCount)
		{
			mem_ = reinterpret_cast<std::byte*>(mem);
			slotSize_ = slotSize;
			slotCount_ = slotCount;
			freeList_ = nullptr;

			for (size_t i = 0; i < slotCount_; ++i)
			{
				Slot* block = reinterpret_cast<Slot*>(mem_ + i * slotSize_);
				block->next_ = freeList_;
				freeList_ = block;
			}
		}

		// Once this slab has been allocated,
		// create a new slab and link them
		void* alloc()
		{
			if (!freeList_) 
				return nullptr;

			Slot* block = freeList_;
			freeList_ = freeList_->next_;
			return block;
		}

		void free(void* ptr)
		{
			Slot* block = reinterpret_cast<Slot*>(ptr);
			block->next_ = freeList_;
			freeList_ = block;
		}

		// Append additional slots from a new memory block into the free list
		void extend(void* additionalMem, size_t additionalSlots)
		{
			std::byte* mem = reinterpret_cast<std::byte*>(additionalMem);
			for (size_t i = 0; i < additionalSlots; ++i)
			{
				Slot* block = reinterpret_cast<Slot*>(mem + i * slotSize_);
				block->next_ = freeList_;
				freeList_ = block;
			}
			slotCount_ += additionalSlots;
		}

		// Counts the number of slots in freeList_
		size_t freeSlotCount() const
		{
			size_t count{};

			// Iterate through the freeList_ with
			// a separate pointer to count the number of slots
			Slot* ptr = freeList_;
			while (ptr)
			{
				++count;
				ptr = ptr->next_;
			}

			return count;
		}

		// ----------------- Statistical Functions -----------------

		size_t usedSlots() const { return slotCount_ - freeSlotCount(); }
		size_t freeSlots() const { return freeSlotCount(); }

		// Returns the level of internal fragmentation
		size_t fragBytes() const															// fragmentation by byte count
		{ return freeSlots() * slotSize_; }

		size_t fragPer() const																// fragmentation by percentage
		{ return static_cast<float>(fragBytes()) / (slotSize_ * slotCount_) * 100.f; }

		// Complete stat overview
		void stats()
		{
			std::cout << std::hex << std::setfill('.')

				// Header
				<< "--------------------------------------------" << std::endl
				<< "~ Slab Stats ~" << std::endl << std::endl

				// Slots
				<< std::dec

				<< "Slot Size:"								<< std::setw(31) << slotSize_		<< std::endl
				<< "Total Slots:"							<< std::setw(27) << freeSlotCount()		<< std::endl
				<< "Used Slots:"							<< std::setw(28) << usedSlots()		<< std::endl
				<< "Free Slots:"							<< std::setw(28) << freeSlots()		<< std::endl

				// Fragmentation
				<< "Internal Fragmentation: "				<< std::setw(17) << fragBytes()		<< std::endl
				<< "Internal Fragmentation(%): "			<< std::setw(13) << fragPer()		<< std::endl

				<< "--------------------------------------------" << std::endl << std::endl;

			// Reset fill
			std::cout << std::setfill(' ');
		}
	};
}
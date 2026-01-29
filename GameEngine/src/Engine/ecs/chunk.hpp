#pragma once

// Registry
#include "../component/component_registry.hpp"

// Arena
#include "../memory/eno_ecs_arena.hpp"

// Settings
#include "ecs_settings.hpp"

// stds
#include <vector>
#include <typeindex>
#include <memory>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <limits>
#include <cstdint>


namespace ECS
{
	///
	/// Chunks are memory blocks that contain entities of a single archetype
	/// in a SoA format. 
	///
	struct Chunk
	{
		static constexpr uint32_t INVALID_ENTITY = UINT32_MAX;									// Sentinel for indicating an invalid entity

		std::vector<std::byte*> data_;															// Starting address of each component using 2d indexing
		size_t* componentSizes_;																// Size of each component
		uint32_t* entities_;																	// Used to track global entity id on local chunk

		size_t capacity_;																		// Max number of entities for the chunk to store
		size_t current_;																		// Current number of entities stored

		Memory::EnoECSArena& arena_;															// Arena handle

		Chunk(Memory::EnoECSArena& arena)
			:
			componentSizes_{ nullptr },
			entities_{ nullptr },
			capacity_{ },
			current_{ },
			arena_{ arena }
		{
		}

		Chunk(
			Memory::EnoECSArena& arena,
			uint32_t chunkCapacity,
			const std::vector<std::type_index>& types,
			const std::vector<size_t>& sizes)
			:
			capacity_{ chunkCapacity },
			current_{ 0 },
			arena_{ arena }
		{
			// Allocate space for all entity ids within arena space
			entities_ = static_cast<uint32_t*>(
				arena_.allocate(sizeof(uint32_t) * capacity_, alignof(uint32_t))
				);

			// Resize data to point to a 
			// predetermined number of component blocks
			uint32_t componentCount = static_cast<uint32_t>(sizes.size());
			data_.resize(componentCount);

			componentSizes_ = static_cast<size_t*>(
				arena.allocate(sizeof(size_t) * componentCount, alignof(size_t))
				);

			// Allocate storage for each component
			// within the arena provided
			for (size_t i = 0; i < componentCount; i++)
			{
				size_t bytes = sizes[i] * capacity_;

				data_[i] = static_cast<std::byte*>(
					arena.allocate(
						bytes,
						ECSComponents::EnoComponentRegistry::GetComponentAlign(types[i])
					));

				componentSizes_[i] = sizes[i];
			}
		}

		// Retrieve the value of an entity's component
		// Component Index: Index of the component within the memory layout
		// Entity Index: Index of the entity within the chunk's memory
		template <typename T>
		T& retrieve(const size_t& componentIndex, const size_t& entityIndex)
		{
			// Cast back to a pointer and look for component
			char* p = search(componentIndex, entityIndex);
			return *reinterpret_cast<T*>(p);
		}

		// Default construct a component and its value(s)
		// Component Index: Index of the component within the memory layout
		// Entity Index: Index of the entity within the chunk's memory
		template <typename T>
		void set(
			const size_t& componentIndex,
			const size_t& entityIndex)
		{
#ifdef ARENA_ENABLED

			// Search for the right memory location and default construct
			char* loc = search(componentIndex, entityIndex);
			new (loc) T{};
#endif
		}

		// Set a value in a specified component
		// Component Index: Index of the component within the memory layout
		// Entity Index: Index of the entity within the chunk's memory
		// Value: Value to assign to the component
		template <typename T, typename U>
		void setVal(
			const size_t& componentIndex,
			const size_t& entityIndex,
			U&& value)
		{
#ifdef ARENA_ENABLED

			// Search for the right memory location and construct (with value)
			char* loc = search(componentIndex, entityIndex);
			new (loc) T{ std::forward<U>(value) };
#endif
		}

		// Modify a value in a specified component
		template <typename T, typename U>
		void modify(
			const size_t& componentIndex,
			const size_t& entityIndex,
			U&& value)
		{
#ifdef ARENA_ENABLED

			// Search for the right memory location
			// Before constructing, invoke the destructor of the component
			// and subsequently regenerate the component with the specified value
			char* loc = search(componentIndex, entityIndex);
			reinterpret_cast<T*>(loc)->~T();
			new (loc) T{ std::forward<U>(value) };
#endif
		}

		// Invoke the destructor of all components within the chunk
		void destroyComponents(std::vector<std::type_index>& types)
		{
			for (size_t comp = 0; comp < types.size(); comp++)
			{
				ECSComponents::EnoComponentRegistry::DeduceCompType(types[comp], [&]<typename T>()
				{
					std::byte* base = data_[comp];
					size_t stride = componentSizes_[comp];

					for (size_t row = 0; row < current_; ++row)
					{
						reinterpret_cast<T*>(base + row * stride)->~T();
					}
				});
			}
		}

		// Invoke the destructor of all components belonging to the specified entity
		// Return is the entity index
		uint32_t destroyEntity(size_t entityIndex, const std::vector<std::type_index>& types)
		{
			assert(current_ > 0 && "No entities to destroy in an empty chunk!");
			assert(entityIndex < current_ && "entityIndex out of bounds in destroyEntity!");

			size_t last = current_ - 1;
			uint32_t movedEntityID = INVALID_ENTITY;

			// We want to swap the entity with the 
			// last entity to ensure that memory is well-packed
			if (entityIndex != last)
			{
				std::swap(entities_[entityIndex], entities_[last]);

				movedEntityID = entities_[entityIndex];  // now in the swapped slot

				for (size_t comp = 0; comp < types.size(); ++comp)
				{
					std::byte* base = data_[comp];
					size_t stride = componentSizes_[comp];

					// Swap the memory for each component
					std::vector<std::byte> tmp{ stride };
					std::memcpy(tmp.data(), base + entityIndex * stride, stride);
					std::memcpy(base + entityIndex * stride, base + last * stride, stride);
					std::memcpy(base + last * stride, tmp.data(), stride);
				}
			}

			// Entity to destroy is now the last entity
			for (size_t comp = 0; comp < types.size(); comp++)
			{
				ECSComponents::EnoComponentRegistry::DeduceCompType(types[comp], [&]<typename T>()
				{
					std::byte* base = data_[comp];
					size_t stride = componentSizes_[comp];

					T* component = reinterpret_cast<T*>(base + last * stride);

					component->~T();
				});
			}

			entities_[last] = INVALID_ENTITY;
			--current_;

			return movedEntityID;
		}

		// Identify the location of the component of a given entity
		char* search(const size_t& componentIndex, const size_t& entityIndex)
		{
			std::byte* base = data_[componentIndex];
			size_t stride = componentSizes_[componentIndex];

			return reinterpret_cast<char*>(base + entityIndex * stride);
		}
	};
}
/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         chunk.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   73%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   27%
* @date         10/3/2025
* @brief        Declaration/definition file for chunks
*//*========================================================================================*/



#pragma once

// Arena
#include "../memory/eno_ecs_arena.hpp"

// Registry
#include "../component/component_registry.hpp"

// Settings
#include "eno_ecs_settings.hpp"

// stds
#include <vector>
#include <typeindex>
#include <memory>
#include <cassert>
#include <cstddef>
#include <iostream>


namespace ECS
{
	struct Chunk
	{
		static constexpr uint32_t INVALID_ENTITY = UINT32_MAX;									// Sentinel for indicating an invalid entity

		std::vector<std::byte*> data_;																		// Starting address of each component using 2d indexing
		size_t* componentSizes_;																// Size of each component
		uint32_t* entities_;																	// Used to track global entity id on local chunk
		std::vector<std::type_index> types_;													// All types in the component

		size_t capacity_;																		// Max number of entities for the chunk to store
		size_t current_;																		// Current number of entities stored

		Memory::EnoECSArena& arena_;															// Arena handle

		Chunk(
			Memory::EnoECSArena& arena,
			uint32_t chunkCapacity,
			const std::vector<std::type_index>& types,
			const std::vector<size_t>& sizes)
			:
			capacity_{ chunkCapacity },
			current_{ 0 },
			types_{ types },
			arena_{ arena }
		{
			size_t componentCount = sizes.size();

			// Entity allocations
			entities_ = static_cast<uint32_t*>(arena.alloc(sizeof(uint32_t) * capacity_, alignof(uint32_t)));

			// Component sizes allocations
			componentSizes_ = static_cast<size_t*>(arena.alloc(sizeof(size_t) * componentCount, alignof(size_t)));

			// Data allocations
			data_.resize(componentCount);
			for (size_t i = 0; i < componentCount; i++)
			{
				size_t bytes = sizes[i] * capacity_;
				data_[i] = static_cast<std::byte*>(arena.alloc(bytes, ECSComponents::EnoComponentRegistry::GetComponentAlign(types[i])));
				componentSizes_[i] = sizes[i];
			}
		}

		~Chunk()
		{
			// Invoke the destructor of all components
			for (size_t compIdx = 0; compIdx < types_.size(); ++compIdx)
			{
				ECSComponents::EnoComponentRegistry::DeduceCompType(
					types_[compIdx], [this, compIdx]<typename T>()
				{
					for (size_t entIdx = 0; entIdx < current_; ++entIdx)
					{
						void* ptr = data_[compIdx] + entIdx * sizeof(T);
						reinterpret_cast<T*>(data_[compIdx] + entIdx * sizeof(T))->~T();
					}
				});
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
#if ARENA_ENABLED
			assert(componentIndex < data_.size());
			assert(entityIndex < capacity_);

			T* compArray = reinterpret_cast<T*>(data_[componentIndex]);
			new (&compArray[entityIndex]) T{};
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
#if ARENA_ENABLED
			assert(componentIndex < data_.size());
			assert(entityIndex < capacity_);

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
#if ARENA_ENABLED
			assert(componentIndex < data_.size());
			assert(entityIndex < current_);

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
			assert(current_ > 0 && entityIndex < current_);

			size_t last = current_ - 1;
			uint32_t movedEntityID = INVALID_ENTITY;

			// If not destroying the last entity, move the last entity into this slot
			if (entityIndex != last)
			{
				std::swap(entities_[entityIndex], entities_[last]);
				movedEntityID = entities_[entityIndex];

				// Move components
				for (size_t comp = 0; comp < types.size(); ++comp)
				{
					ECSComponents::EnoComponentRegistry::DeduceCompType(types[comp], [this, comp, entityIndex, last]<typename T>()
					{
						T* dest = reinterpret_cast<T*>(data_[comp] + entityIndex * sizeof(T));
						T* src = reinterpret_cast<T*>(data_[comp] + last * sizeof(T));

						*dest = std::move(*src);
						src->~T();
					});
				}
			}
			else
			{
				// Destroy the last entity directly
				for (size_t comp = 0; comp < types.size(); ++comp)
				{
					ECSComponents::EnoComponentRegistry::DeduceCompType(types[comp], [this, comp, last]<typename T>()
					{
						T* component = reinterpret_cast<T*>(data_[comp] + last * sizeof(T));
						component->~T();
					});
				}
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
/*============================================================================================
* @file         chunk.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration/definition file for chunks
* 
* All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#pragma once

// stds
#include <vector>
#include <typeindex>
#include <memory>
#include <cassert>
#include <cstddef>
#include <iostream>


namespace ECS
{
	/*============================================================================================
		* \brief		memory blocks that contain entities of a single archetype in SoA format
	*//*========================================================================================*/
	struct Chunk
	{
		std::unique_ptr<std::byte[]> data;					// Starting address of the chunk
		std::vector<void*> pComponent;						// Pointer to each component
		std::vector<size_t> componentSizes;					// Size of each component
		std::vector<std::type_index> componentTypes;		// Type of each component (for calling destructors)

		// ****** Old code ******
		//std::unique_ptr<std::byte[]> data_;													// Starting address of the chunk
		//std::vector<void*> pComponent_;														// Pointer to each component
		//std::vector<size_t> componentSizes_;													// Size of each component
		//std::vector<uint32_t> entities_;														// Used to track global entity id on local chunk

		std::vector<std::byte*> data_;															// Starting address of each component using 2d indexing
		size_t* componentSizes_;																// Size of each component
		uint32_t* entities_;																	// Used to track global entity id on local chunk
		std::vector<std::type_index> types_;													// All types in the component

		size_t capacity_;																		// Max number of entities for the chunk to store
		size_t current_;																		// Current number of entities stored

		Memory::EnoECSArena& arena_;															// Arena handle

		Chunk(
			uint32_t chunkCapacity,
			std::vector<std::type_index>& types,
			std::vector<size_t>& sizes)
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

		/*============================================================================================
			* \brief			Default construct a specified component
			* \param[in]		Component index
			* \param[in]		Entity index
		*//*========================================================================================*/
		template <typename T>
		void set(const size_t& componentIndex, const size_t& entityIndex)
		{
			// ****** Old code ******
			//assert(componentIndex < pComponent_.size());

			// Cast back to a pointer and look for component
			//char* p = search(componentIndex, entityIndex);

			// Default construct component into memory
			//new (p) T{};

#ifdef ARENA_ENABLED
			assert(componentIndex < data_.size());
			assert(entityIndex < capacity_);

			// Search for the right memory location and default construct
			//char* loc = search(componentIndex, entityIndex);
			//new (loc) T{};
			T* compArray = reinterpret_cast<T*>(data_[componentIndex]);
			new (&compArray[entityIndex]) T{};
#endif
		}

		/*============================================================================================
			* \brief			Set a value in a specified component
			* \param[in]		Component index
			* \param[in]		Entity index
			* \param[in]		Forwarded value
		*//*========================================================================================*/
		template <typename T, typename U>
		void setVal(const size_t& componentIndex, const size_t& entityIndex, U&& value)
		{
			// ****** Old code ******
			//assert(componentIndex < pComponent_.size());

			// Cast back to a pointer and look for component
			//char* p = search(componentIndex, entityIndex);

			// Construct component into memory
			//new (p) T{ std::forward<U>(value) };

#ifdef ARENA_ENABLED
			assert(componentIndex < data_.size());
			assert(entityIndex < capacity_);

			// Cast back to a pointer and look for component
			char* p = search(componentIndex, entityIndex);

			// Construct component into memory
			new (p) T{ std::forward<U>(value) };
		}

		/*============================================================================================
			* \brief			Modify a value in a specified component
			* \param[in]		Component index
			* \param[in]		Entity index
			* \param[in]		Forwarded value
		*//*========================================================================================*/
		template <typename T, typename U>
		void modify(const size_t& componentIndex, const size_t& entityIndex, U&& value)
		{
			//assert(componentIndex < pComponent_.size());

			// Cast back to a pointer and look for component
			//char* p = search(componentIndex, entityIndex);

			// Invoke the destructor for previous object
			// stored in this location
			//reinterpret_cast<T*>(p)->~T();

			// Construct component into memory

			// ****** Old code ******
			//new (p) T{ std::forward<U>(value) };
			//T* component = arena.construct<T>(std::forward<U>(value));
			//pComponent_[componentIndex][entityIndex] = component;

#ifdef ARENA_ENABLED
			assert(componentIndex < data_.size());
			assert(entityIndex < current_);

			// Cast back to a pointer and look for component
			char* p = search(componentIndex, entityIndex);

			// Invoke the destructor for previous object
			// stored in this location
			reinterpret_cast<T*>(p)->~T();

			// Construct component into memory
			new (p) T{ std::forward<U>(value) };
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
			auto pComp = static_cast<char*>(pComponent[componentIndex]);
			char* loc = pComp + entityIndex * componentSizes[componentIndex];

			return loc;
		}
	};
}
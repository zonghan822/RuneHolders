/*============================================================================================
* @file         chunk.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration/definition file for chunks
* 
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
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

		size_t capacity;									// Max number of entities for the chunk to store
		size_t current;										// Current number of entities stored

		/*============================================================================================
			* \brief			Chunk Constructor
			* \param[in]		Max number of entities to store
			* \param[in]		Types to be stored
			* \param[in]		Sizes of types to store
		*//*========================================================================================*/
		Chunk(
			uint32_t chunkCapacity,
			std::vector<std::type_index>& types,
			std::vector<size_t>& sizes)
			:
			capacity{ chunkCapacity },
			current{ 0 }
		{
			// Allocate the size of the chunk
			// based on components
			size_t totalSize{};
			for (const size_t size : sizes)
			{
				totalSize += size * capacity;
			}

			// Assign new memory to chunk memory
			data = std::make_unique<std::byte[]>(totalSize);

			// Partition each region of memory
			size_t offset{};
			for (const size_t& size : sizes)
			{
				pComponent.push_back(data.get() + offset);
				componentSizes.push_back(size);
				offset += size * capacity;
			}
		}

		/*============================================================================================
			* \brief			Retrieve the value of an entity's component
			* \param[in]		Component index
			* \param[in]		Entity index
			* \return			Value of a component
		*//*========================================================================================*/
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
			assert(componentIndex < pComponent.size());

			// Cast back to a pointer and look for component
			char* p = search(componentIndex, entityIndex);

			// Construct component into memory
			new (p) T{};
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
			assert(componentIndex < pComponent.size());

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
			assert(componentIndex < pComponent.size());

			// Cast back to a pointer and look for component
			char* p = search(componentIndex, entityIndex);

			// Invoke the destructor for previous object
			// stored in this location
			reinterpret_cast<T*>(p)->~T();

			// Construct component into memory
			new (p) T{ std::forward<U>(value) };
		}

		/*============================================================================================
			* \brief			Identify the location of the component of a given entity
			* \param[in]		Component index
			* \param[in]		Entity index
			* \return			Pointer to the start of the component of an entity
		*//*========================================================================================*/
		char* search(const size_t& componentIndex, const size_t& entityIndex)
		{
			auto pComp = static_cast<char*>(pComponent[componentIndex]);
			char* loc = pComp + entityIndex * componentSizes[componentIndex];

			return loc;
		}
	};
}
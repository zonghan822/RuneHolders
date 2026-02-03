/*============================================================================================
* @file         archetype.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration/Implemnentation file for archetypes
* 
* All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/


#pragma once

// Engine Component(s)
#include "chunk.hpp"
#include "../component/component_registry.hpp"

// stds
#include <unordered_map>


namespace ECS
{
	//// Chunk destructor implementation (defined here to avoid circular dependency)
	//inline Chunk::~Chunk()
	//{
	//	// Call destructors for all active entities' components
	//	// This is critical for components with resources like shared_ptr
	//	for (size_t entityIdx = 0; entityIdx < current; ++entityIdx)
	//	{
	//		for (size_t compIdx = 0; compIdx < componentTypes.size(); ++compIdx)
	//		{
	//			char* componentPtr = search(compIdx, entityIdx);

	//			// Call the appropriate destructor based on component type
	//			ECSComponents::EnoComponentRegistry::DeduceCompType(componentTypes[compIdx],
	//				[componentPtr]<typename T>() {
	//					reinterpret_cast<T*>(componentPtr)->~T();
	//				});
	//		}
	//	}
	//}

	/*============================================================================================
		* \brief       Defined by a collection of components created at any point in time
	*//*========================================================================================*/
	struct Archetype
	{
		std::vector<Chunk*> chunks_;									// Chunks storing the component data for the archetype
		std::vector<std::type_index> types_;							// Types of components stored in an archetype instance
		std::vector<size_t> typeSizes_;									// Size of each component stored
		std::unordered_map<uint32_t, size_t> typeIdxMap_;				// Maps a local ID to each type within the archetype

		const uint32_t entityCount_{ 50 };								// Number of entities stored in a chunk

		/*============================================================================================
			* \brief			Constructor for an archetype
			* \param[in]		Component types used to construct the chunk
			* \param[in]		Component sizes determine the size of the chunk
		*//*========================================================================================*/
		Archetype(
			Memory::EnoECSArena& arena,
			std::vector<std::type_index> t,
			std::vector<size_t> sizes)
			: types_(std::move(t)), typeSizes_(std::move(sizes))
		{
			uint32_t localIndex{};
			for (const auto& type : types_)
			{
				uint32_t registryID = ECSComponents::EnoComponentRegistry::ComponentMap()[type].id_;
				typeIdxMap_[registryID] = localIndex;
				++localIndex;
			}

			createChunk(arena, entityCount_, types_, typeSizes_);
		}

		/*============================================================================================
			* \brief			Finds the index of a component within an archetype
			* \return			Component index
		*//*========================================================================================*/
		template <typename Type>
		size_t findComponentIndex()
		{
			auto it = std::find(types_.begin(), types_.end(), std::type_index(typeid(Type)));
			if (it == types_.end()) 
			{
				throw std::runtime_error(
					std::string(typeid(Type).name()) + " component was not able to be found"
				);
			}

			return std::distance(types_.begin(), it);
		}

		/*============================================================================================
			* \brief			Instantiates a new chunk for a particular archetype
			* \param[in]		Number of entities to store in chunk
			* \param[in]		Types to be stored
			* \param[in]		Size of each type to be stored
		*//*========================================================================================*/
		Chunk* createChunk(
			Memory::EnoECSArena& arena,
			const uint32_t& entityCount,
			const std::vector<std::type_index>& types,
			const std::vector<size_t>& typeSizes)
		{
			// Construct the chunk in an arena space
			void* mem = arena.allocate(sizeof(Chunk), alignof(Chunk));
			Chunk* chunk = new (mem) Chunk(arena, entityCount, types, typeSizes);

			chunks_.push_back(chunk);

			return chunk;
		}
	};

	/*============================================================================================
		* \brief				Custom key for archetype manager
	*//*========================================================================================*/
	struct ArchetypeKey
	{
		std::vector<std::type_index> types;
		bool operator==(const ArchetypeKey& other) const
		{
			return types == other.types;
		}
	};

	/*============================================================================================
		* \brief				Custom hash for archetype manager
	*//*========================================================================================*/
	struct ArchetypeKeyHash
	{
		std::size_t operator()(const ArchetypeKey& key) const
		{
			std::size_t hash{};
			for (const auto& type : key.types)
			{
				hash ^= type.hash_code() + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			}

			return hash;
		}
	};

	/*============================================================================================
		* \brief				Maps out all created archetypes
	*//*========================================================================================*/
	class ArchetypeManager
	{
	public:

		// Iterate through a map to discern if an archetype has been created.
		// 
		// If not, create a new archetype.
		// Else retrieve this archetype and insert a new entity
		Archetype* getOrCreateArch(
			Memory::EnoECSArena& arena,
			const std::vector<std::type_index>& types,
			const std::vector<size_t>& sizes)
		{
			// Use types to identify if
			// a suitable archetype exists
			auto key = ArchetypeKey{ types };
			auto it = archetypeMap_.find(key);
			if (it != archetypeMap_.end()) return it->second.get();

			// No existing suitable archetype identified,
			// creating a new archetype
			auto archetype = std::make_unique<Archetype>(arena, types, sizes);
			assert(archetype->types_.size() == types.size());
			for (size_t i = 0; i < types.size(); i++)
			{
				assert(archetype->types_[i] == types[i]);
			}

			Archetype* pArche = archetype.get();
			archetypeMap_[key] = std::move(archetype);

			return pArche;
		}

		/*============================================================================================
			* \brief			archetypeMap_ getter
		*//*========================================================================================*/
		std::unordered_map<ArchetypeKey, std::unique_ptr<Archetype>, ArchetypeKeyHash>& getArchetypes()
		{
			return archetypeMap_;
		}

		/*============================================================================================
			* \brief			Transfer data between 2 archetypes
			* \param[in]		Source archetype
			* \param[in]		Destination archetype
			* \param[in]		Source entity chunk index
			* \param[in]		Destination entity chunk index
			* \param[in]		Source chunk index
			* \param[in]		Destination chunk index
		*//*========================================================================================*/
		void transferData(
			const Archetype& src, const Archetype& dst,						// Archetypes
			const size_t& srcECIdx, const size_t& dstECIdx,					// Entity index in chunk
			const size_t& srcChnkIdx, const size_t& dstChnkIdx)				// Chunk index
		{
			const auto& srcChunk = src.chunks_[srcChnkIdx];
			const auto& dstChunk = dst.chunks_[dstChnkIdx];

			// Copy components that exist in both src and dst
			for (size_t i = 0; i < dst.types_.size(); i++)
			{
				// Use the global type ID from the registry to
				// find the local ID of the component within the chunk
				const auto& type = dst.types_[i];
				uint32_t registryID = ECSComponents::EnoComponentRegistry::ComponentMap()[type].id_;

				// Verify that the destination archetype has the same type
				// If type does not coexist in the source, this is likely
				// a type added to the entity
				auto it = src.typeIdxMap_.find(registryID);
				if (it == src.typeIdxMap_.end())
					continue;

				// Find the start address of the component we are looking for in both locations
				size_t size = dstChunk->componentSizes_[i];
				std::byte* pSrc = srcChunk->data_[it->second] + srcECIdx * size;
				std::byte* pDst = dstChunk->data_[i] + dstECIdx * size;

				std::memcpy(pDst, pSrc, size);
			}
		}

		/*============================================================================================
			* \brief			Deletes an archetype off the archetypeMap_
		*//*========================================================================================*/
		void cleanup(Archetype* archetype)
		{
			ArchetypeKey key{ archetype->types_ };

			auto it = archetypeMap_.find(key);

			if (it != archetypeMap_.end())
			{
				archetypeMap_.erase(it);
			}
		}

		/*============================================================================================
			* \brief			View the archetype map
		*//*========================================================================================*/
		void viewMap()
		{
			for (const auto& [key, archPtr] : archetypeMap_)
			{
				std::cout << "Archetype with " << archPtr->chunks_.size() << " Chunks" << std::endl <<
					"_______________________________" << std::endl;

				for (const auto& type : key.types)
				{
					std::cout << type.name() << std::endl;
				}
			}
		}

	private:
		std::unordered_map<ArchetypeKey, std::unique_ptr<Archetype>, ArchetypeKeyHash> archetypeMap_;
	};
}
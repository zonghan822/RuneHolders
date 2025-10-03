/*============================================================================================
* @file         archetype.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration/Implemnentation file for archetypes
*//*========================================================================================*/

#pragma once

// Engine Component(s)
#include "chunk.hpp"
#include "../component/component_registry.hpp"

// stds
#include <unordered_map>


namespace ECS
{
	/*============================================================================================
		* \brief       Defined by a collection of components created at any point in time
	*//*========================================================================================*/
	struct Archetype
	{
		std::vector<std::unique_ptr<Chunk>> chunks_;					// Storing of the component data for the archetype
		std::vector<std::type_index> types_;							// Types of components stored in an archetype instance
		std::vector<size_t> typeSizes_;									// Size of each component stored
		std::unordered_map<uint32_t, size_t> typeIdxMap_;				// Maps a local ID to each type within the archetype

		const uint32_t entityCount_{ 50 };								// Number of entities stored in a chunk

		/*============================================================================================
			* \brief			Constructor for an archetype
			* \param[in]		Component types used to construct the chunk
			* \param[in]		Component sizes determine the size of the chunk
		*//*========================================================================================*/
		Archetype(std::vector<std::type_index> t, std::vector<size_t> sizes)
			: types_(std::move(t)), typeSizes_(std::move(sizes))
		{
			uint32_t localIndex{};
			for (const auto& type : types_)
			{
				uint32_t registryID = ECSComponents::EnoComponentRegistry::ComponentMap()[type].id;
				typeIdxMap_[registryID] = localIndex;
				++localIndex;
			}

			createChunk(entityCount_, types_, typeSizes_);
		}

		/*============================================================================================
			* \brief			Finds the index of a component within an archetype
			* \return			Component index
		*//*========================================================================================*/
		template <typename Type>
		size_t findComponentIndex()
		{
			auto it = std::find(types_.begin(), types_.end(), std::type_index(typeid(Type)));
			assert(it != types_.end());
			return std::distance(types_.begin(), it);
		}

		/*============================================================================================
			* \brief			Instantiates a new chunk for a particular archetype
			* \param[in]		Number of entities to store in chunk
			* \param[in]		Types to be stored
			* \param[in]		Size of each type to be stored
		*//*========================================================================================*/
		void createChunk(
			const uint32_t& entityCount,
			std::vector<std::type_index> types,
			std::vector<size_t> typeSizes)
		{
			chunks_.push_back(std::make_unique<Chunk>(entityCount, types, typeSizes));
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

		/*============================================================================================
			* \brief			Iterate through a map to discern if an archetype has been created.
			* 
			*					If not, create a new archetype.
			*					Else retrieve this archetype and insert a new entity
			* \param[in]		Types representing the archetype
			* \param[in]		Sizes representing the types of the archetype
			* \return			Pointer to the archetype created/retrieved
		*//*========================================================================================*/
		Archetype* getOrCreateArch(const std::vector<std::type_index>& types, const std::vector<size_t>& sizes)
		{
			// Use types to identify if
			// a suitable archetype exists
			auto key = ArchetypeKey{ types };
			auto it = archetypeMap.find(key);
			if (it != archetypeMap.end()) return it->second.get();

			// No existing suitable archetype identified,
			// creating a new archetype
			auto archetype = std::make_unique<Archetype>(types, sizes);
			Archetype* pArche = archetype.get();
			archetypeMap[key] = std::move(archetype);

			return pArche;
		}

		/*============================================================================================
			* \brief		Archetype map getter
			* \return		Ref to archetype map
		*//*========================================================================================*/
		std::unordered_map<ArchetypeKey, std::unique_ptr<Archetype>, ArchetypeKeyHash>& getArchetypes()
		{
			return archetypeMap;
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

			// Iterate through source types
			for (size_t i = 0; i < dst.types_.size(); i++)
			{
				// Use the global type ID from the registry to
				// find the local ID of the component within the chunk
				const auto& type = dst.types_[i];
				uint32_t registryID = ECSComponents::EnoComponentRegistry::ComponentMap()[type].id;

				// Verify that the destination archetype has the same type
				// If type does not coexist in the source, this is likely
				// a type added to the entity
				auto it = src.typeIdxMap_.find(registryID);
				if (it == src.typeIdxMap_.end())
					continue;

				// Find the start address of the component we are looking for in both locations
				size_t size = dstChunk->componentSizes[i];
				uint8_t* srcCompStart = static_cast<uint8_t*>(srcChunk->pComponent[it->second]);
				uint8_t* dstCompStart = static_cast<uint8_t*>(dstChunk->pComponent[i]);

				std::memcpy(dstCompStart + (dstECIdx * size),
					srcCompStart + (srcECIdx * size),
					size);
			}
		}

		/*============================================================================================
			* \brief			Retrieve raw data of an entity
			* \param[in]		Source archetype
			* \param[in]		Source entity chunk index
			* \param[in]		Source chunk index
			* \return			Data region of the entity's components
		*//*========================================================================================*/
		std::vector<std::byte*> retrieveData(
			const Archetype& src,
			const size_t& srcECIdx,
			const size_t& srcChnkIdx)
		{
			const auto& chunk = src.chunks_[srcChnkIdx];

			std::vector<std::byte*> completeData;
			for (size_t c = 0; c < src.types_.size(); ++c)
			{
				std::byte* component = static_cast<std::byte*>(chunk->pComponent[c]) + srcECIdx * chunk->componentSizes[c];
				completeData.push_back(component);
			}

			return completeData;
		}

		/*============================================================================================
			* \brief			Erase an archetype off the archetype map
			* \param[in]		Pointer to an archetype
		*//*========================================================================================*/
		void cleanup(Archetype* archetype)
		{
			ArchetypeKey key{ archetype->types_ };

			auto it = archetypeMap.find(key);

			if (it != archetypeMap.end())
			{
				archetypeMap.erase(it);
			}
		}

		/*============================================================================================
			* \brief			View Archetype map by printing the types and chunk count
		*//*========================================================================================*/
		void viewMap()
		{
			for (const auto& [key, archPtr] : archetypeMap)
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
		std::unordered_map<ArchetypeKey, std::unique_ptr<Archetype>, ArchetypeKeyHash> archetypeMap;
	};
}
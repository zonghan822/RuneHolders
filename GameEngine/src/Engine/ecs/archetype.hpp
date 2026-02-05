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

// Arena | Slab
#include "../memory/eno_ecs_arena.hpp"
#include "../memory/eno_slab_allocator.hpp"

// stds
#include <unordered_map>


namespace ECS
{
	///
	/// Defined by a collection of components
	/// created at any point in time
	///
	struct Archetype
	{
		std::vector<std::unique_ptr<Chunk>> chunks_;					// Storing of the component data for the archetype
		std::vector<std::type_index> types_;							// Types of components stored in an archetype instance
		std::vector<size_t> typeSizes_;									// Size of each component stored
		std::unordered_map<uint32_t, size_t> typeIdxMap_;				// Maps a local ID to each type within the archetype

#ifdef ARENA_ENABLED
		Memory::EnoECSArena& arena_;
		Memory::EnoSlabAllocator chunkSlab_;
#endif

		const uint32_t entityCount_{ 50 };								// Number of entities stored in a chunk

		Archetype(
			Memory::EnoECSArena& arena,
			std::vector<std::type_index> types,
			std::vector<size_t> sizes,
			size_t initialChunkCount = 4)
			:
			types_(std::move(types)),
			typeSizes_(std::move(sizes)),
			arena_{ arena }
		{
			uint32_t idx{};
			for (const auto& type : types_)
			{
				uint32_t registryID = ECSComponents::EnoComponentRegistry::ComponentMap()[type].id_;
				typeIdxMap_[registryID] = idx;
				++idx;
			}

#ifdef ARENA_ENABLED
			// Allocate mem for chunks
			size_t slabSize = sizeof(Chunk) * initialChunkCount;
			void* slabMem = arena.alloc(slabSize, alignof(Chunk));

			// Slab allocator init
			chunkSlab_.init(slabMem, sizeof(Chunk), initialChunkCount);
#endif

			// Instantiate a new chunk
			createChunk(entityCount_, types_, typeSizes_);
		}

		~Archetype()
		{
#ifdef ARENA_ENABLED
			for (auto* chunk : chunks_)
			{
				std::cout << "Chunk: " << chunk << ", current_: " << chunk->current_
					<< ", data_[0]: " << chunk->data_[0] << std::endl;

				// Free memory from the slab and
				// invoke the destructors of the chunk
				chunk->~Chunk();
				chunkSlab_.free(chunk);
			}
#else
			for (auto* chunk : chunks_)
			{
				chunk->destroyComponents(types_);
				delete chunk;
			}
#endif
			chunks_.clear();
		}

		template <typename Type>
		size_t findComponentIndex()
		{
			auto it = std::find(types_.begin(), types_.end(), std::type_index(typeid(Type)));
			assert(it != types_.end());
			return std::distance(types_.begin(), it);
		}

		// Instantiate a new chunk
		Chunk* createChunk(uint32_t entCount, const std::vector<std::type_index>& types, const std::vector<size_t>& sizes)
		{
#ifdef ARENA_ENABLED
			void* mem = chunkSlab_.alloc();
			if (!mem)
			{
				assert(false && "Chunk slab out of memory!");
				return nullptr;
			}

			Chunk* chunk = new (mem) Chunk(arena_, entCount, types, sizes);
			chunks_.push_back(chunk);
			return chunk;
#endif
		}
	};

	struct ArchetypeKey
	{
		std::vector<std::type_index> types;
		bool operator==(const ArchetypeKey& other) const
		{
			return types == other.types;
		}
	};

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

	class ArchetypeManager
	{
	public:

		// Iterate through a map to discern if an archetype has been created.
		// 
		// If not, create a new archetype.
		// Else retrieve this archetype and insert a new entity
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

		// Get archetype manager map
		std::unordered_map<ArchetypeKey, std::unique_ptr<Archetype>, ArchetypeKeyHash>& getArchetypes()
		{
			return archetypeMap;
		}

		/*============================================================================================
			* \brief		Archetype map getter
			* \return		Ref to archetype map
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

		// Transfer data between 2 archetypes
		void transferData(
			const Archetype& src, const Archetype& dst,						// Archetypes
			const size_t& srcECIdx, const size_t& dstECIdx,					// Entity index in chunk
			const size_t& srcChnkIdx, const size_t& dstChnkIdx)				// Chunk index
		{
			const auto& chunk = src.chunks_[srcChnkIdx];

			std::vector<std::byte*> completeData;
			for (size_t c = 0; c < src.types_.size(); ++c)
			{
				std::byte* component = static_cast<std::byte*>(chunk->pComponent[c]) + srcECIdx * chunk->componentSizes[c];
				completeData.push_back(component);
			}

			// Components in the src which do not 
			// exist in the dst are to be destroyed
			// since they cannot be transferred
			for (size_t i = 0; i < src.types_.size(); i++)
			{
				const auto& type = src.types_[i];
				uint32_t registryID = ECSComponents::EnoComponentRegistry::ComponentMap()[type].id_;

				// Same as before, check that the comopnent exists in dst
				if (dst.typeIdxMap_.find(registryID) != dst.typeIdxMap_.end())
					continue;

				// Retrieve component data
				size_t size = srcChunk->componentSizes_[i];
				std::byte* pComp = srcChunk->data_[i] + srcECIdx * size;

				// Invoke the leftover component's destructor
				ECSComponents::EnoComponentRegistry::DeduceCompType(type, [&]<typename T>()
				{
					T* component = reinterpret_cast<T*>(pComp);
					component->~T();
				});
			}
		}

		// View Archetype map
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

		// Remove an existing archetype
		void cleanup(Archetype* archetype)
		{
			ArchetypeKey key{ archetype->types_ };

			auto it = archetypeMap_.find(key);
			if (it != archetypeMap_.end())
				archetypeMap_.erase(it);
		}

	private:
		std::unordered_map<ArchetypeKey, std::unique_ptr<Archetype>, ArchetypeKeyHash> archetypeMap;
	};
}
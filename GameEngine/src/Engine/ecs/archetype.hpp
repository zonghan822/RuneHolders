/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         archetype.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   71%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   29%
* @date         10/3/2025
* @brief        Declaration/Implemnentation file for archetypes
*//*========================================================================================*/




#pragma once

// Engine Component(s)
#include "chunk.hpp"
#include "../component/component_registry.hpp"

// Settings
#include "eno_ecs_settings.hpp"

// Slab
#include "../memory/eno_slab_allocator.hpp"

// stds
#include <unordered_map>


namespace ECS
{
	/*============================================================================================
		* \brief       Defined by a collection of components created at any point in time
	*//*========================================================================================*/
	struct Archetype
	{
		std::vector<Chunk*> chunks_;									// Chunks storing the component data for the archetype
		std::vector<std::type_index> types_;							// Types of components stored in an archetype instance
		std::vector<size_t> typeSizes_;									// Size of each component stored
		std::unordered_map<uint32_t, size_t> typeIdxMap_;				// Maps a local ID to each type within the archetype

#if ARENA_ENABLED
		Memory::EnoECSArena& arena_;									// Reference to the arena used to store the memory of each chunk
		Memory::EnoSlabAllocator chunkSlab_;							//
		size_t chunkSlabCapacity_{};									// Current total slab slot capacity (for doubling on grow)
#endif

		const uint32_t entityCount_{ 5 };								// Number of entities stored in a chunk

		Archetype(
			Memory::EnoECSArena& arena,
			std::vector<std::type_index> types,
			std::vector<size_t> sizes,
			size_t initialChunkCount = 64)
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

#if ARENA_ENABLED
			// Allocate mem for chunks
			size_t slabSize = sizeof(Chunk) * initialChunkCount;
			void* slabMem = arena.alloc(slabSize, alignof(Chunk));

			// Slab allocator init
			chunkSlab_.init(slabMem, sizeof(Chunk), initialChunkCount);
			chunkSlabCapacity_ = initialChunkCount;
#endif

			// Instantiate a new chunk
			createChunk(entityCount_, types_, typeSizes_);
		}

		~Archetype()
		{
#if ARENA_ENABLED
			for (auto* chunk : chunks_)
			{
				// Free memory from the slab and
				// invoke the destructors of the chunk
				chunk->~Chunk();
				chunkSlab_.free(chunk);
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
#if ARENA_ENABLED
			void* mem = chunkSlab_.alloc();
			if (!mem)
			{
				// Slab exhausted — grow by doubling using the arena
				size_t additionalChunks = chunkSlabCapacity_;
				void* slabMem = arena_.alloc(sizeof(Chunk) * additionalChunks, alignof(Chunk));
				chunkSlab_.extend(slabMem, additionalChunks);
				chunkSlabCapacity_ += additionalChunks;
				mem = chunkSlab_.alloc();
			}
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

		// Get archetype manager map
		std::unordered_map<ArchetypeKey, std::unique_ptr<Archetype>, ArchetypeKeyHash>& getArchetypes()
		{
			return archetypeMap_;
		}

		// Transfer data between 2 archetypes
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

		// Remove an existing archetype
		void cleanup(Archetype* archetype)
		{
			ArchetypeKey key{ archetype->types_ };

			auto it = archetypeMap_.find(key);
			if (it != archetypeMap_.end())
				archetypeMap_.erase(it);
		}

	private:
		std::unordered_map<ArchetypeKey, std::unique_ptr<Archetype>, ArchetypeKeyHash> archetypeMap_;
	};
}
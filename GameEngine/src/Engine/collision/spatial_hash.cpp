/*============================================================================================
* All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         collision.cpp
* @author       Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio@digipen.edu)
* @date         29-01-2026
* @brief
*
*
*//*========================================================================================*/

#include "spatial_hash.hpp"
#include <unordered_set>
#include <cmath>

namespace Collision
{
	/*============================================================================================
	* Constructor
	*//*========================================================================================*/
	SpatialHash::SpatialHash(float cellSize)
		: cellSize_(cellSize)
		, inverseCellSize_(1.0f / cellSize)
	{
	}

	/*============================================================================================
	* Clear - Call at the start of each frame
	*//*========================================================================================*/
	void SpatialHash::Clear() 
	{
		buckets_.clear();
	}

	/*============================================================================================
	* Insert - Add an entity to all cells it overlaps
	*//*========================================================================================*/
	void SpatialHash::Insert(uint32_t entityID,
		const Math::eVec2<float>& position,
		float halfWidth,
		float halfHeight)
	{
		// Calculate the bounding box of the entity
		float minX = position.x - halfWidth;
		float maxX = position.x + halfWidth;
		float minY = position.y - halfHeight;
		float maxY = position.y + halfHeight;

		// Convert bounding box corners to cell coordinates
		int32_t cellMinX = WorldToCell(minX);
		int32_t cellMaxX = WorldToCell(maxX);
		int32_t cellMinY = WorldToCell(minY);
		int32_t cellMaxY = WorldToCell(maxY);

		// Inset entity into every cell it overlaps
		for (int32_t y = cellMinY; y <= cellMaxY; ++y)
		{
			for (int32_t x = cellMinX; x <= cellMaxX; ++x)
			{
				int64_t hash = HashCell(x, y);
				buckets_[hash].push_back(entityID);
			}
		}
	}

	/*============================================================================================
	* GetPotentialPairs - Get all pairs that might be colliding
	*//*========================================================================================*/
	std::vector<std::pair<uint32_t, uint32_t>> SpatialHash::GetPotentialPairs() const
	{
		std::vector<std::pair<uint32_t, uint32_t>> pairs;

		// Use a set to track pairs we've already added (avoid duplicates)
		// We store pairs as: (smaller ID, larger ID) for consistency
		std::unordered_set<int64_t> addedPairs;

		// Helper lambda to create a unique key for a pair of entities
		auto makePairKey = [](uint32_t a, uint32_t b) -> int64_t
		{
			// Ensure consistent ordering: smaller ID first
			if (a > b) std::swap(a, b);

			// Combine two 32-bit IDs into one 4-bit key
			return (static_cast<int64_t>(a) << 32) | static_cast<int64_t>(b);
		};

		// Iterate through all occupied cells
		for (const auto& [hash, entities] : buckets_)
		{
			// Check for all pairs within this cell
			size_t count = entities.size();

			for (size_t i = 0; i < count; ++i) 
			{
				for (size_t j = i + 1; j < count; ++j)
				{
					uint32_t entityA = entities[i];
					uint32_t entityB = entities[j];

					// Create a unique key for this pair
					int64_t pairKey = makePairKey(entityA, entityB);

					// Only add if we haven't seen this pair before
					if (addedPairs.find(pairKey) == addedPairs.end())
					{
						addedPairs.insert(pairKey);
						pairs.emplace_back(entityA, entityB);
					}
				}
			}
		}

		return pairs;
	}

	/*============================================================================================
	* Query - Find all entities near a position
	*//*========================================================================================*/
	std::vector<uint32_t> SpatialHash::Query(const Math::eVec2<float>& position,
		float halfWidth,
		float halfHeight) const
	{
		std::vector<uint32_t> result;

		// Use a set to avoid duplicate entities
		std::unordered_set<uint32_t> found;

		// Calculate the bounding box of the query area
		float minX = position.x - halfWidth;
		float maxX = position.x + halfWidth;
		float minY = position.y - halfHeight;
		float maxY = position.y + halfHeight;

		// Convert to cell coordinates
		int32_t cellMinX = WorldToCell(minX);
		int32_t cellMaxX = WorldToCell(maxX);
		int32_t cellMinY = WorldToCell(minY);
		int32_t cellMaxY = WorldToCell(maxY);

		// Check all cells in the query range
		for (int32_t y = cellMinY; y <= cellMaxY; ++y)
		{
			for (int32_t x = cellMinX; x <= cellMaxX; ++x)
			{
				int64_t hash = HashCell(x, y);

				// Find this cell in the hash table
				auto it = buckets_.find(hash);
				if (it != buckets_.end())
				{
					// Add all entities in this cell
					for (uint32_t entityID : it->second)
					{
						// Avoid duplicates (entity might be in multiple cells)
						if (found.find(entityID) == found.end())
						{
							found.insert(entityID);
							result.push_back(entityID);
						}
					}
				}
			}
		}
		return result;

	}

	/*============================================================================================
	* SetCellSize - Change the cell size
	*//*========================================================================================*/
	void SpatialHash::SetCellSize(float cellSize)
	{
		cellSize_ = cellSize;
		inverseCellSize_ = 1.0f / cellSize;

		// Clear existing data since cell assignments are now invalid
		Clear();
	}

	/*============================================================================================
	* GetCellSize - Get current cell size
	*//*========================================================================================*/
	float SpatialHash::GetCellSize() const
	{
		return cellSize_;
	}

	/*============================================================================================
	* HashCell - Convert cell coordinates to hash key
	*//*========================================================================================*/
	int64_t SpatialHash::HashCell(int32_t cellX, int32_t cellY) const
	{
		// Use prime numbers to reduce hash collisions
		// This formula spreads out the hash values nicely
		const int64_t PRIME1 = 73856093;
		const int64_t PRIME2 = 19349663;

		return (static_cast<int64_t>(cellX) * PRIME1) ^ (static_cast<int64_t>(cellY) * PRIME2);
	}

	/*============================================================================================
	* WorldToCell - Convert world coordinates to cell coordinate
	*//*========================================================================================*/
	int32_t SpatialHash::WorldToCell(float value) const
	{
		// Use floor to handle negative coordinates correctly
		// Example with cellSize = 64;
		// position 50 -> cell 0
		// position 64 -> cell 1
		// position -10 -> cell -1 (not 0)
		return static_cast<int32_t>(std::floor(value * inverseCellSize_));
	}
} // namespace Collision
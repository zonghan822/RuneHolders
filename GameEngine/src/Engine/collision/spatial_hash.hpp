/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         spatial_hash.hpp
* @author       Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio@digipen.edu)
* @date         29-01-2026
* @brief        Declaration for spatial hash grid used in broad phase collision detection
*//*========================================================================================*/

#pragma once

#include <unordered_map>
#include <vector>
#include <cstdint>
#include "../math/math.hpp"

namespace Collision
{
	/*============================================================================================
	* \brief       Spatial hash grid for broad phase collision detection
	*			   
	* Divides the world into cells and uses a hash table to store entities.
	* Only entities in the same or adjacent cells are checks for collision. 
	* (Reduces number of narrow phase checks needed)
	*//*========================================================================================*/
	class SpatialHash
	{
	public:
		/*============================================================================================
		* \brief       Construc a spatial hash with the given cell size
		* \param[in]   cellSize - The size of each cell. Est. 2x your average entity size
		*//*========================================================================================*/
		explicit SpatialHash(float cellSize = 64.0f);

		/*============================================================================================
		* \brief       Rremove all entities from the hash. Call this at start of each frame.
		*//*========================================================================================*/
		void Clear();

		/*============================================================================================
		* \brief       Insert an entity into the spatial hash.
		* \param[in]   entityID - The entity's unique identifier
		* \param[in]   position - The entity's world position (center)
		* \param[in]   halfWidth - Half the width of the entity's bounding box
		* \param[in]   halfHeight - Half the height of the entity's bounding box
		* 
		* The entity will be inserted into all cells it overlaps
		*//*========================================================================================*/
		void Insert(uint32_t entityID,
			const Math::eVec2<float>& position,
			float halfWidth,
			float halfHeight);

		/*============================================================================================
		* \brief       Get all unique pairs of entities that might be colliding
		* \return	   Vector of entity ID pairs to check in narrow phase
		* 
		* Each pair is returned only once (no duplicates like A-B and B-A)
		*//*========================================================================================*/
		std::vector<std::pair<uint32_t, uint32_t>> GetPotentialPairs() const;

		/*============================================================================================
		* \brief       Query for all entities near a given position
		* \param[in]   position - The world position to query
		* \param[in]   halfWidth - Half the width of the query area.
		* \param[in]   halfHeight - Half the height of the query area
		* \return	   Vector of entity IDs in the same or adjacent cells
		*//*========================================================================================*/
		std::vector<uint32_t> Query(const Math::eVec2<float>& position,
			float halfWidth,
			float halfHeight) const;

		/*============================================================================================
		* \brief       Set a new cell size. Requires re-inserting all entities.
		* \return	   cellSize - The new cell size
		*//*========================================================================================*/
		void SetCellSize(float cellSize);

		/*============================================================================================
		* \brief       Get the current cell size
		* \return      The cell size
		*//*========================================================================================*/
		float GetCellSize() const;

	private:
		/*============================================================================================
		* \brief       Hash a cell coordinate pair into a single key
		* \param[in]   cellX - The cell's X coordinate
		* \param[in]   cellY - The cell's Y coordinate
		* \return	   A unique hash key for this cell
		*//*========================================================================================*/
		int64_t HashCell(int32_t cellX, int32_t cellY) const;

		/*============================================================================================
		* \brief       Convert a world position to a cell coordinate
		* \param[in]   value - The world position (x or y)
		* \return	   The cell coordinate
		*//*========================================================================================*/
		int32_t WorldToCell(float value) const;

		// The size of each cell in world units
		float cellSize_;

		// Inverse of cell size (for quicker division)
		float inverseCellSize_;

		// Hash table: cell hash -> list of entity IDs in that cell
		std::unordered_map<int64_t, std::vector<uint32_t>> buckets_;

	};
} // namespace Collision
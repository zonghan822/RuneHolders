/*============================================================================================
* @file         eno_bimap.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief         Declaration/definition file for a bimap
*//*========================================================================================*/

#pragma once

// Map
#include "eno_map.hpp"

#include <optional>


namespace Utility
{

	/*============================================================================================
	 * \brief				Insert key value pair into both maps,
	 *						alternating for each side
	 * \param				Left map
	 * \param				Right map
	*//*========================================================================================*/
	template <
		typename Left, typename Right,
		typename LeftHash = std::hash<Left>,
		typename LeftEq = std::equal_to<Left>,
		typename RightHash = std::hash<Right>,
		typename RightEq = std::equal_to<Right>
	>
	struct EnoBiMap
	{
		/*============================================================================================
		 * \brief				Custom map is used as unordered maps return an iterator,
		 *						but here, I would like the maps to return an optional value when finding
		*//*========================================================================================*/
		EnoMap<Left, Right, LeftHash, LeftEq>   left;
		EnoMap<Right, Left, RightHash, RightEq> right;

		/*============================================================================================
		 * \brief				Insert key value pair into both maps,
		 *						alternating for each side
		 * \param				Left map
		 * \param				Right map
		*//*========================================================================================*/
		void insert(const Left& l, const Right& r);

		/*============================================================================================
		 * \brief				Find via right key
		 * \param				Readonly ref Right map
		 * \return				An optional value from the right map
		*//*========================================================================================*/
		std::optional<Left> findr(const Right& r) const;

		/*============================================================================================
		 * \brief				Find via left key
		 * \param				Readonly ref Left map
		 * \return				An optional value from the left map
		*//*========================================================================================*/
		std::optional<Right> findl(const Left& l) const;

		/*============================================================================================
		 * \brief				Return size of the bimap
		 * \return				Size of left map (equivalent to size of right map)
		*//*========================================================================================*/
		size_t size() const;
	};

	#include "eno_bimap.tpp"
}
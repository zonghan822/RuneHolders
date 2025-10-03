/*============================================================================================
* @file         archetype_presets.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration/definition file for archetype presets
*//*========================================================================================*/

#pragma once

// Engine Component(s)
#include "../component/components/name.hpp"
#include "../component/components/layer.hpp"
#include "../component/components/transform.hpp"
#include "../component/components/visibility.hpp"
#include "../component/components/drawInfo.hpp"

//stds
#include <tuple>


namespace ECSComponents
{
	/*============================================================================================
	 * \brief       Defining type aliases for component groups
	*//*========================================================================================*/
	struct Preset
	{
		using EMPTY = std::tuple<>;

		using DEFAULT = std::tuple<
			NameComponent,
			VisibilityComponent,
			LayerComponent>;
	};
}
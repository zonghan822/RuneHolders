/*============================================================================================
* @file         layer.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration/definition file for the layer component
* 
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#pragma once

//stds
#include <string>
#include <iostream>


namespace ECSComponents
{
	/*============================================================================================
	 * \brief			Definition of the layer component
	*//*========================================================================================*/
	struct LayerComponent
	{
		// Default construction
		LayerComponent() = default;

		// C string construction
		LayerComponent(const char* l)
		{
			strncpy_s(layer, l, sizeof(layer) - 1);
			layer[sizeof(layer) - 1] = '\0';
		}

		char layer[32] = "NULL";
	};

	/*============================================================================================
		 * \brief			Stream insertion operator overload for the name component
		 * \param[in]		reference to the output stream
		 * \param[in]		readonly reference to a LayerComponent to print its members
		 * \return			reference to the output stream
	*//*========================================================================================*/
	inline std::ostream& operator<<(std::ostream& os, const LayerComponent& n)
	{
		std::cout << n.layer << std::endl;
		return os;
	}
}
/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         layer.hpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   57%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   43%
* @date         10/3/2025
* @brief        Declaration/definition file for the layer component
*//*========================================================================================*/



#pragma once

// Signals
#include "../../event/signals.hpp"

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

		// Signals
		std::unordered_map<std::string, Events::EnoSignal> signals_;

		char layer[32] = "NULL";

		// For serialization purposes
		// Generalized way of using reflection to access and iterate
		// over fields!
		// 
		// The idea is to (in a function call) create a lambda that will be a "visitor",
		// and extract all information as parameters of that lambda. Each call over a
		// field of the component is used to extract metadata on 1 field [name, value, type]
		// 
		// Source: https://stackoverflow.com/questions/41453/how-can-i-add-reflection-to-a-c-application
		// Additional learning: https://refactoring.guru/design-patterns/visitor
		template <typename Visitor>
		void reflect(Visitor&& l)
		{
			l("Layer", layer, "char32");
		}

		template <typename Visitor>
		void reflect(Visitor&& l) const
		{
			l("Layer", layer, "char32");
		}
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
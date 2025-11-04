/*============================================================================================
* @file         visibility.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration/definition file for the visibility component
* 
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#pragma once

//stds
#include <iostream>
#include <string>

namespace ECSComponents
{
	/*============================================================================================
	 * \brief       Definition of the visibility component
	*//*========================================================================================*/
	struct VisibilityComponent
	{
		// Rendering order
		uint32_t z_index = 3;

		// Visibility
		bool visible = false;


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
			l("z_index", z_index, "uint32");
			l("visible", visible, "bool");
		}

		template <typename Visitor>
		void reflect(Visitor&& l) const
		{
			l("z_index", z_index, "uint32");
			l("visible", visible, "bool");
		}
	};

	/*============================================================================================
	 * \brief				Equality operator overload for the visibility component
	 * \param[in]			readonly reference to a VisibilityComponent
	 * \param[in]			readonly reference to a VisibilityComponent
	 * \return				Check and return if all members are equivalent
	*//*========================================================================================*/
	inline bool operator==(const VisibilityComponent& lhs, const VisibilityComponent& rhs)
	{
		return (lhs.z_index == rhs.z_index && lhs.visible == rhs.visible);
	}

	/*============================================================================================
	 * \brief				Stream insertion operator overload for the visibility component
	 * \param[in]			reference to the output stream
	 * \param[in]			readonly reference to a VisibilityComponent to print its members
	 * \return				reference to the output stream
	*//*========================================================================================*/
	inline std::ostream& operator<<(std::ostream& os, const VisibilityComponent& i)
	{
		std::cout << "Z Index: " << i.z_index << std::endl <<
			"Visible: " << std::boolalpha << i.visible;
		return os;
	}
}
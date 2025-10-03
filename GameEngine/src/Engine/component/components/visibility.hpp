/*============================================================================================
* @file         visibility.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration/definition file for the visibility component
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
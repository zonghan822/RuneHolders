/*============================================================================================
* @file         name.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration/definition file for the name component
* 
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#pragma once

// stds
#include <string>
#include <iostream>


namespace ECSComponents
{
	/*============================================================================================
	 * \brief       Definition of the name component
	*//*========================================================================================*/
	struct NameComponent
	{
		// Default construction
		NameComponent() = default;

		// C string construction
		NameComponent(const char* n)
		{
			strncpy_s(name, n, sizeof(name) - 1);
			name[sizeof(name) - 1] = '\0';
		}

		char name[32] = "DefaultName";
	};

	/*============================================================================================
	 * \brief				Equality operator overload for the name component
	 * \param[in]			readonly reference to a NameComponent
	 * \param[in]			readonly reference to a NameComponent
	 * \return				Check and return if all members are equivalent
	*//*========================================================================================*/
	inline bool operator==(const NameComponent& lhs, const NameComponent& rhs)
	{
		return strcmp(lhs.name, rhs.name) == 0;
	}

	/*============================================================================================
	 * \brief				Stream insertion operator overload for the name component
	 * \param[in]			reference to the output stream
	 * \param[in]			readonly reference to a NameComponent to print its members
	 * \return				reference to the output stream
	*//*========================================================================================*/
	inline std::ostream& operator<<(std::ostream& os, const NameComponent& n)
	{
		std::cout << n.name << std::endl;
		return os;
	}
}
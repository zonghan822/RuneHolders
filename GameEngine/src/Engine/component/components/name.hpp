/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         name.hpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   67%
* @co-author    Caven Koh (caven.koh@digipen.edu)   17%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   17%
* @date         10/3/2025
* @brief        Declaration/definition file for the name component All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#pragma once

// Signals
#include "../../event/signals.hpp"

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

		// Signals
		std::unordered_map<std::string, Events::EnoSignal> signals_;

		char name[32] = "DefaultName";

		// Getter 
		const char* getName() const { return name; }

		// Setter
		void setName(const char* n)
		{
			strncpy_s(name, n, sizeof(name) - 1);
			name[sizeof(name) - 1] = '\0';
		}

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
			l("Name", name, "char32");
		}

		template <typename Visitor>
		void reflect(Visitor&& l) const
		{
			l("Name", name, "char32");
		}
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
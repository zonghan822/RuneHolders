/*============================================================================================
* @file         tag_events.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Definition file for tag-related events
* 
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#pragma once

// stds
#include <string>


namespace Tag
{
	/*============================================================================================
	 * \brief				A request from game scene -> tag registry to store a new tag
	*//*========================================================================================*/
	struct RecordTag
	{
		std::string tag_;
		bool scope_;
	};
}
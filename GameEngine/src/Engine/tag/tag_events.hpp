/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         tag_events.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   60%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   40%
* @date         10/3/2025
* @brief        Definition file for tag-related events
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
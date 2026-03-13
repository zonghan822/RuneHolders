/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_input_events.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   67%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   33%
* @brief        See file name.
*//*========================================================================================*/

#pragma once

namespace Input
{
	struct InputSent
	{
		int key;
		int scancode;
		int action;
		int mods;
	};
}
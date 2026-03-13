/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         text_component.hpp
* @author       Caven Koh (caven.koh@digipen.edu)
* @brief        Declaration/definition file for the text component All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#pragma once
#include <vector>
#include "../../math/math.hpp"

// Signals
#include "../../event/signals.hpp"

namespace ECSComponents
{
	/*============================================================================================
	 * \brief       Definition of the text component
	*//*========================================================================================*/
	struct TextComponent
	{
		// Default construction
		TextComponent() = default;

		// C string construction
		TextComponent(const char* t)
		{
			strncpy_s(text, t, sizeof(text) - 1);
			text[sizeof(text) - 1] = '\0';
		}

		// Signals
		std::unordered_map<std::string, Events::EnoSignal> signals_;

		char text[256] = "";
		char fontName[64] = "PixelifySans";
		float fontSize = 1.0f;

		// For serialization purposes
		template <typename Visitor>
		void reflect(Visitor&& l)
		{
			l("Text",     text,     "char256");
			l("FontName", fontName, "char64");
			l("FontSize", fontSize, "float");
		}

		template <typename Visitor>
		void reflect(Visitor&& l) const
		{
			l("Text",     text,     "char256");
			l("FontName", fontName, "char64");
			l("FontSize", fontSize, "float");
		}
	};
}

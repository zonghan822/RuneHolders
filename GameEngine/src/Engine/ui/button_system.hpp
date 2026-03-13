/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         button_system.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   50%
* @co-author    Caven Koh (caven.koh@digipen.edu)   25%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   25%
* @brief        UI button system — processes mouse click hit-tests against all active
*//*========================================================================================*/


#pragma once

#include "../math/math.hpp"
#include <utility>

namespace ECS   { class EntityManager;   }
namespace Input { class EnoInputManager; }

namespace UI
{
	/*============================================================================================
	* \class	ButtonSystem
	* \brief	Each frame, converts the mouse screen position to world space and hit-tests every
	*			active ButtonComponent entity. Emit the respective signal for the button for that
	*			frame when the mouse lands inside a button's collision area.
	*//*========================================================================================*/
	class ButtonSystem
	{
	public:
		/*============================================================================================
		* \brief		Process button behavior for the current frame.
		* \param[in]	entityMng    ECS entity manager used to query button entities
		* \param[in]	inputMng     Input manager used to read mouse state and position
		* \param[in]	viewportMin  Screen-space top-left of the rendered game image
		* \param[in]	viewportMax  Screen-space bottom-right of the rendered game image
		*							 Pass {0,0} to fall back to full window bounds
		* \param[in]	windowSize   OS window pixel dimensions (used as fallback viewport)
		*//*========================================================================================*/
		void update(
			ECS::EntityManager&     entityMng,
			Input::EnoInputManager& inputMng,
			Math::eVec2<float>      viewportMin,
			Math::eVec2<float>      viewportMax,
			std::pair<int, int>     windowSize
		);
	};

} // namespace UI

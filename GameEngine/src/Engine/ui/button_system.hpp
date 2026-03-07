/*============================================================================================
* @file         button_system.hpp
* @brief        UI button system — processes mouse click hit-tests against all active
*               ButtonComponent entities and emits their onClick signals.
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
	*			active ButtonComponent entity. Emits the "onClick" signal on the first left-click
	*			frame that lands inside a button's collision area.
	*//*========================================================================================*/
	class ButtonSystem
	{
	public:
		/*============================================================================================
		* \brief		Process button clicks for the current frame.
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

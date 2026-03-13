/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         button_system.cpp
* @author       Caven Koh (caven.koh@digipen.edu)   67%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   33%
* @brief        Implementation of the UI ButtonSystem.
*//*========================================================================================*/


#include "button_system.hpp"

#include "../ecs/entity.hpp"
#include "../input/eno_input.hpp"
#include "../graphics/graphics.hpp"
#include "../component/components/buttons.hpp"
#include "../component/components/camera_component.hpp"
#include "../component/components/transform.hpp"
#include "../component/components/drawInfo.hpp"
#include "../component/components/collision_component.hpp"
#include "../component/components/visibility.hpp"

#include <GLFW/glfw3.h>

namespace UI
{
	void ButtonSystem::update(ECS::EntityManager&		entityMng,
							  Input::EnoInputManager&	inputMng,
							  Math::eVec2<float>		viewportMin,
							  Math::eVec2<float>		viewportMax,
							  std::pair<int, int>		windowSize)
	{

		// Resolve camera top-left corner and viewport size from the active CameraComponent
		float cameraX = 0.f, cameraY = 0.f, vpW = 1920.f, vpH = 1080.f;
		auto cams = entityMng.findEntity<ECSComponents::CameraComponent>();
		if (!cams.empty())
		{
			auto& cam = cams[0].get<ECSComponents::CameraComponent>(entityMng);
			cameraX = cam.position.x - cam.viewportWidth  * 0.5f;
			cameraY = cam.position.y - cam.viewportHeight * 0.5f;
			vpW     = cam.viewportWidth;
			vpH     = cam.viewportHeight;
		}

		// Fall back to full window bounds if no viewport rect was supplied
		Math::eVec2<float> vMax = viewportMax;
		if (vMax.x == 0.f && vMax.y == 0.f)
			vMax = { static_cast<float>(windowSize.first), static_cast<float>(windowSize.second) };

		// Determine mouse position
		auto mouse = inputMng.getMousePosition();
		Math::eVec2<float> worldPos = Graphics::screenToWorld(
			{ static_cast<float>(mouse.first), static_cast<float>(mouse.second) },
			viewportMin, vMax, vpW, vpH, cameraX, cameraY
		);

		// Loop through all buttons
		for (auto& ent : entityMng.findEntity<ECSComponents::ButtonComponent>())
		{
			// Skip if it does not have
			if (!ent.has<ECSComponents::CollisionComponent>(entityMng))  continue;
			if (!ent.has<ECSComponents::TransformComponent>(entityMng)) continue;

			// Button component, transform for pos, collision for hitbox
			auto& b = ent.get<ECSComponents::ButtonComponent>(entityMng);
			auto& t = ent.get<ECSComponents::TransformComponent>(entityMng);
			auto& c = ent.get<ECSComponents::CollisionComponent>(entityMng);

			// Skip if entity is invisible
			if (ent.has<ECSComponents::VisibilityComponent>(entityMng) && !ent.get<ECSComponents::VisibilityComponent>(entityMng).visible) continue;

			// Check if button is active
			if (b.getActive())
			{
				// Check mouse pos within button
				if (b.mouseWithinButton(worldPos, t, c))
				{

					// Mouse over button interactable area, check if hovering or pressing
					// Emit onPressed every frame the mouse button is held
					if (inputMng.isMousePressed(GLFW_MOUSE_BUTTON_LEFT))
					{
						// Pressing button, do not trigger hover
						//std::cout << "[Button_System] Button Pressed!" << std::endl;
						auto it2 = b.signals_.find("onPressed");
						if (it2 != b.signals_.end())
						{
							it2->second.emit();
						}
					} 
					else
					{
						// Mouse within button: emit onHover signal
						//std::cout << "[Button_System] Button Hovered!" << std::endl;
						auto it = b.signals_.find("onHover");
						if (it != b.signals_.end())
						{
							it->second.emit();
						}
					}

					// Mouse click on button, emit onClicked signal
					if (inputMng.isMouseUp(GLFW_MOUSE_BUTTON_LEFT))
					{
						//std::cout << "[Button_System] Button Clicked!" << std::endl;
						auto it2 = b.signals_.find("onClick");
						if (it2 != b.signals_.end())
						{
							it2->second.emit();
						}
					}
				}
			}
		}
	}

} // end of namespace UI

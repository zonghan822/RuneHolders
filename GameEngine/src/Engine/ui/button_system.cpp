/*============================================================================================
* @file         button_system.cpp
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

#include <GLFW/glfw3.h>

namespace UI
{
	void ButtonSystem::update(
		ECS::EntityManager&     entityMng,
		Input::EnoInputManager& inputMng,
		Math::eVec2<float>      viewportMin,
		Math::eVec2<float>      viewportMax,
		std::pair<int, int>     windowSize
	)
	{
		if (!inputMng.isMouseDown(GLFW_MOUSE_BUTTON_LEFT))
			return;

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

		auto mouse = inputMng.getMousePosition();
		Math::eVec2<float> worldPos = Graphics::screenToWorld(
			{ static_cast<float>(mouse.first), static_cast<float>(mouse.second) },
			viewportMin, vMax, vpW, vpH, cameraX, cameraY
		);

		for (auto& ent : entityMng.findEntity<ECSComponents::ButtonComponent>())
		{
			if (!ent.has<ECSComponents::CollisionComponent>(entityMng))  continue;
			if (!ent.has<ECSComponents::TransformComponent>(entityMng)) continue;

			auto& b = ent.get<ECSComponents::ButtonComponent>(entityMng);
			auto& t = ent.get<ECSComponents::TransformComponent>(entityMng);
			auto& c = ent.get<ECSComponents::CollisionComponent>(entityMng);

			ECSComponents::DrawInfoComponent dummyDraw{};
			const auto& d = ent.has<ECSComponents::DrawInfoComponent>(entityMng)
				? ent.get<ECSComponents::DrawInfoComponent>(entityMng) : dummyDraw;

			if (b.getActive() && b.onClick(worldPos, t, d, c))
			{
				auto it = b.signals_.find("onClick");
				if (it != b.signals_.end())
					it->second.emit();
			}
		}
	}

} // namespace UI

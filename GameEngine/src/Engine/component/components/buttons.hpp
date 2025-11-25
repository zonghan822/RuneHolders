#pragma once

// ECS Components
#include "drawInfo.hpp"
#include "transform.hpp"
#include "visibility.hpp"
#include "../../math/math.hpp"
#include "../../input/eno_input.hpp"
#include <string>
// Forward declaration
class GameApplication;

namespace ECSComponents {

	// Base class template for in game buttons
	struct ButtonComponent
	{
	public:

		// Default constructor
		ButtonComponent() = default;

		// Get name
		std::string getName() const
		{
			return std::string(btnText);
		}

		std::string getTargetScene() const
		{
			return std::string(targetScene);
		}

		// Onclick function
		bool onClick(Math::eVec2<float> mousePos, const ECSComponents::VisibilityComponent& v, const ECSComponents::TransformComponent& t, const ECSComponents::DrawInfoComponent& d)
		{
			// Check if button is visible
			if (!v.visible) return false;

			//std::cout << "mouse pos :" << mousePos << std::endl;

			// Check if mousePos is within button 
			if (mousePos.x >= (t.pos.x - (d.width * 0.5f)) &&	// Left limit
				mousePos.x <= (t.pos.x + (d.width * 0.5f)) &&	// Right limit
				mousePos.y >= (t.pos.y - (d.height * 0.5f)) &&	// Upper limit
				mousePos.y <= (t.pos.y + (d.height * 0.5f)))	// Lower limit
			{
				// Button is clicked
				return true;
			}

			return false;

		}

		void onClickAction(void* func())
		{

		}

		template <typename Visitor>
		void reflect(Visitor&& l)
		{
			l("BtnText", btnText, "char32");
			l("TargetScene", targetScene, "char32");
		}

		template <typename Visitor>
		void reflect(Visitor&& l) const
		{
			l("BtnText", btnText, "char32");
			l("TargetScene", targetScene, "char32");
		}


	private:
		char btnText[32] = "Button";
		char targetScene[32];
	};

}
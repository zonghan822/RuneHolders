#include "buttons.hpp"

namespace ECSComponents{

	std::string ButtonComponent::getName() const
	{
		std::string str(btnText);
		return str;
	}

	bool ButtonComponent::onClick(Input::EnoInputManager& input, const ECSComponents::VisibilityComponent& v, const ECSComponents::TransformComponent& t, const ECSComponents::DrawInfoComponent& d)
	{
		// Check if button is visible
		if (!v.visible) return false;

		// Get mouse position
		std::pair<double, double> inputMousePos = input.getMousePosition();
		Math::eVec2<float> mousePos(static_cast<float>(inputMousePos.first), static_cast<float>(inputMousePos.second));
		std::cout << "mouse pos :" << mousePos << std::endl;

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

} // End of namespace ECSComponents
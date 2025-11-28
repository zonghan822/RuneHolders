#pragma once

// Signals
#include "../../event/signals.hpp"

// ECS Components
#include "drawInfo.hpp"
#include "transform.hpp"
#include "visibility.hpp"
#include "../../math/math.hpp"
#include <string>
#include <unordered_map>
#include "../../event/signals.hpp"

// Forward declaration
namespace Engine
{
	class EnoEngine;
}


namespace ECSComponents {

	// Button signal types
	enum signalType
	{
		onClick = 1,
		onHover,
		onPress
	};

	// Function type
	enum functionType
	{
		loadScene = 1
	};

	// Base class template for in game buttons
	struct ButtonComponent
	{
	public:

		// Default constructor
		ButtonComponent()
		{
			signalsMap["onClick"] = Events::EnoSignal();
			signalsMap["onHover"] = Events::EnoSignal();
			signalsMap["onPressed"] = Events::EnoSignal();
		}

		// Get name
		std::string getName() const
		{
			return std::string(btnText);
		}

		std::string getTargetScene() const
		{
			return std::string(targetScene);
		}

		void setTargetScene(const std::string& sceneName)
		{
			size_t copy_length = std::min(sceneName.length(), (size_t)32 - 1);
			std::strncpy(targetScene, sceneName.c_str(), copy_length);
			targetScene[copy_length] = '\0';
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
				// Emit signal

				// Button is clicked
				return true;
			}

			return false;

		}

		void executeSignal(signalType signalType, Engine::EnoEngine& eng);
		void executeFunction(functionType funcType, Engine::EnoEngine& eng);

		// Getters
		size_t signalsSize() const
		{
			return signals.size();
		}

		// Add signal
		void addSignal()
		{
			signals.emplace_back();
		}

		// Delete signal
		void deleteSignal(size_t index)
		{
			if (index < signals.size()) {
				//Replace the element to be deleted (at 'index') 
				//with the element currently at the back.
				//Only do this if the index isn't already the last element.
				if (index != signals.size() - 1) {
					signals[index] = std::move(signals.back());
				}
				signals.pop_back();
			}
		}

		// Signals
		std::unordered_map<std::string, Events::EnoSignal> signals_;

		// Get Signal container
		std::vector<std::pair<signalType, functionType>>& getSignals()
		{
			return signals;
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
		// Button name
		char btnText[32] = "Button";

		// Signal types (To be deleted after bridging)
		std::vector<std::pair<signalType, functionType>> signals;
		
		std::unordered_map<std::string, Events::EnoSignal> signalsMap;

		// Parameter for scene selection (To be deleted after bridging)
		char targetScene[32] = "NIL";
	};

}
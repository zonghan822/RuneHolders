#pragma once

// Signals
#include "../../event/signals.hpp"

// ECS Components
#include "drawInfo.hpp"
#include "transform.hpp"
#include "visibility.hpp"
#include "collision_component.hpp"
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
		void onClick(Math::eVec2<float> mousePos, const ECSComponents::TransformComponent& t, const ECSComponents::DrawInfoComponent& d, const ECSComponents::CollisionComponent& c)
		{
			// Check if button is interactable
			if (!isActive) return;

			//std::cout << "mouse pos :" << mousePos << std::endl;

			// Check if the entity is rotated
			if (c.rotation != 0.0f)
			{
				float halfW = d.width / 2;
				float halfH = d.height / 2;

				Math::eVec2<float> corners[4];
				rotateRectCorners(t.pos, d.width, d.height, c.rotation, corners);
				Math::eVec2<float> newMousePos = { mousePos.x - t.pos.x, mousePos.y - t.pos.y };

				float cos = cosf(c.rotation);
				float sin = sinf(c.rotation);

				Math::eVec2<float> rotMousePos = { (newMousePos.x * cos) + (newMousePos.y * sin), (-newMousePos.x * sin) + (newMousePos.y * cos) };

				if (rotMousePos.x >= -halfW && rotMousePos.x <= halfW &&
					rotMousePos.y >= -halfH && rotMousePos.y <= halfH)
				{
					//return true; // Point is inside
					// The button is clicked, emit a singal
					auto signalIt = signalsMap.find("onClick");
					if (signalIt != signalsMap.end())
					{
						signalIt->second.emit();
					}
				}
				else
				{
					return;
				}
			}
			else
			{
				// Check if mousePos is within button 
				if (mousePos.x >= (t.pos.x - (d.width * 0.5f)) &&	// Left limit
					mousePos.x <= (t.pos.x + (d.width * 0.5f)) &&	// Right limit
					mousePos.y >= (t.pos.y - (d.height * 0.5f)) &&	// Upper limit
					mousePos.y <= (t.pos.y + (d.height * 0.5f)))	// Lower limit
				{
					// The button is clicked, emit a singal
					auto signalIt = signalsMap.find("onClick");
					if (signalIt != signalsMap.end())
					{
						signalIt->second.emit();
					}
					//return true;

				}
				else
				{
					return;
				}

			}
			
		}

		void executeSignal(signalType signalType, Engine::EnoEngine& eng);
		void executeFunction(functionType funcType, Engine::EnoEngine& eng);

		// Getters
		size_t signalsSize() const
		{
			return signals.size();
		}

		bool& getActive()
		{
			return isActive;
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
			l("IsActive", isActive, "bool");
		}

		template <typename Visitor>
		void reflect(Visitor&& l) const
		{
			l("BtnText", btnText, "char32");
			l("TargetScene", targetScene, "char32");
			l("IsActive", isActive, "bool");
		}

		void rotateRectCorners(
			const Math::eVec2<float>& center,
			float width,
			float height,
			float rotation,
			Math::eVec2<float> corners[4])
		{
			// Calculate half extents
			float halfWidth = width * 0.5f;
			float halfHeight = height * 0.5f;

			// Pre-calculate sin and cos
			float cosAngle = std::cos(rotation);
			float sinAngle = std::sin(rotation);

			// Local corner positions (before rotation)
			Math::eVec2<float> localCorners[4] = {
				{ -halfWidth, -halfHeight }, // Bottom-left
				{ halfWidth, -halfHeight }, // Bottom-right
				{ halfWidth, halfHeight },   // Top-right
				{ -halfWidth, halfHeight }, // Top-left
			};

			// Rotate and translate each corner
			for (int i = 0; i < 4; ++i) {
				// Rotate the corner
				float rotatedX = localCorners[i].x * cosAngle - localCorners[i].y * sinAngle;
				float rotatedY = localCorners[i].x * sinAngle + localCorners[i].y * cosAngle;

				// Translate to world position
				corners[i].x = center.x + rotatedX;
				corners[i].y = center.y + rotatedY;
			}
		}


	private:
		// Button name
		char btnText[32] = "Button";

		// Signal types (To be deleted after bridging)
		std::vector<std::pair<signalType, functionType>> signals;
		
		std::unordered_map<std::string, Events::EnoSignal> signalsMap;

		// Parameter for scene selection (To be deleted after bridging)
		char targetScene[32] = "NIL";
		
		// If interactable
		bool isActive = false;
	};

}
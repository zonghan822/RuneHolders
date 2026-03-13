/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         buttons.hpp
* @author       Caven Koh (caven.koh@digipen.edu)   90%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   10%
* @date         27/11/2025
* @brief        This file contains the definition of the button component class for in game buttons
*//*========================================================================================*/


#pragma once

// Signals
#include "../../event/signals.hpp"

// ECS Components
#include "transform.hpp"
#include "visibility.hpp"
#include "collision_component.hpp"
#include "../../math/math.hpp"

// Standard library
#include <string>
#include <unordered_map>


namespace ECSComponents {

	// Base class template for in game buttons
	struct ButtonComponent
	{
	public:

		// Default constructor
		ButtonComponent()
		{
			signals_["onClick"] = Events::EnoSignal();
			signals_["onHover"] = Events::EnoSignal();
			signals_["onPressed"] = Events::EnoSignal();
		}

		// Signals map
		std::unordered_map<std::string, Events::EnoSignal> signals_;

		/*============================================================================================
		* \brief		Get the text of selected signal type
		* \return		std::string
		*//*========================================================================================*/
		std::string getSignalType() const
		{
			return std::string(selectedSignalType);
		}

		/*============================================================================================
		* \brief		Get the text of selected signal type
		* \return		std::string
		*//*========================================================================================*/
		char* getSignalType()
		{
			return selectedSignalType;
		}

		/*============================================================================================
		* \brief		Set the text of selected signal type
		* \return		std::string
		*//*========================================================================================*/
		void setSignalType(const std::string& signalName)
		{
			size_t copy_length = Math::minValue(signalName.length(), (size_t)63);
			std::strncpy(selectedSignalType, signalName.c_str(), copy_length);
			selectedSignalType[copy_length] = '\0';
		}

		/*============================================================================================
		* \brief		Returns the bool data member whether the button is active
		* \return		bool reference to if button isActive
		*//*========================================================================================*/
		bool& getActive()
		{
			return isActive;
		}

		void setTargetObject(const std::string& objName)
		{
			size_t copy_length = Math::minValue(objName.length(), (size_t)63);
			std::strncpy(targetObject, objName.c_str(), copy_length);
			targetObject[copy_length] = '\0';
		}

		std::string getTargetObject() const
		{
			return std::string(targetObject);
		}

		std::unordered_map<std::string, Events::EnoSignal>& getSignalMap()
		{
			return signals_;
		}

		template <typename Visitor>
		void reflect(Visitor&& l)
		{
			l("IsActive", isActive, "bool");
			l("TargetObject", targetObject, "char64");
			l("SelectedSignal", selectedSignalType, "char64");
		}

		template <typename Visitor>
		void reflect(Visitor&& l) const
		{
			l("IsActive", isActive, "bool");
			l("TargetObject", targetObject, "char64");
			l("SelectedSignal", selectedSignalType, "char64");
		}

		/*============================================================================================
		* \brief		Checks if the mouse pos is within the current button interactable area
		* \param[in]	const Math::eVec2<float>& mousePos, const TransformComponent& t, const CollisionComponent& c
		* \return		bool true if in button area, false if outside button area
		*//*========================================================================================*/
		bool mouseWithinButton(const Math::eVec2<float>& mousePos, const ECSComponents::TransformComponent& t, const ECSComponents::CollisionComponent& c) const
		{
			// Check if button is active
			if (!isActive) return false;

			// Check for circle or rectanble collider types
			if (c.type == ECSComponents::ColliderType::CIRCLE)
			{
				float dx = mousePos.x - (t.pos.x + c.offset.x);
				float dy = mousePos.y - (t.pos.y + c.offset.y);
				return (dx * dx + dy * dy) <= (c.data.circle.radius * c.data.circle.radius);
			}
			if (c.type != ECSComponents::ColliderType::RECTANGLE) return false;

			//std::cout << "mouse pos :" << mousePos << std::endl;

			// Check if the entity is rotated
			if (c.rotation != 0.0f)
			{
				float halfW = c.data.rectangle.width / 2;
				float halfH = c.data.rectangle.height / 2;
				
				Math::eVec2<float> newMousePos = { mousePos.x - t.pos.x, mousePos.y - t.pos.y };

				float cos = cosf(c.rotation);
				float sin = sinf(c.rotation);

				Math::eVec2<float> rotMousePos = { (newMousePos.x * cos) + (newMousePos.y * sin), (-newMousePos.x * sin) + (newMousePos.y * cos) };

				if (rotMousePos.x >= -halfW && rotMousePos.x <= halfW &&
					rotMousePos.y >= -halfH && rotMousePos.y <= halfH)
				{
					// Point is inside
					std::cout << "[Button] Mouse in button!" << std::endl;

					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				// Check if mousePos is within button
				if (mousePos.x >= (t.pos.x - (c.data.rectangle.width * 0.5f)) &&	// Left limit
					mousePos.x <= (t.pos.x + (c.data.rectangle.width * 0.5f)) &&	// Right limit
					mousePos.y >= (t.pos.y - (c.data.rectangle.height * 0.5f)) &&	// Upper limit
					mousePos.y <= (t.pos.y + (c.data.rectangle.height * 0.5f)))		// Lower limit
				{
					// The button is clicked
					std::cout << "[Button] Mouse in button!" << std::endl;
					return true;

				}
				else
				{
					return false;
				}

			}
		}

	private:

		// Parameter for linking to selected object (listener node name)
		char targetObject[64] = "";

		// Selected signal type for UI state
		char selectedSignalType[64] = "";

		// If interactable
		bool isActive = true;
	};

}

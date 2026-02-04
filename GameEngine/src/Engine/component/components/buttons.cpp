/*============================================================================================
* @file         buttons.cpp
* @author       Caven Koh (caven.koh@digipen.edu)
* @date         27/11/2025
* @brief        This file defines the execution of the signals
*//*========================================================================================*/
#include "buttons.hpp"
#include "../../engine/eno_engine.hpp"

namespace ECSComponents {

	void ButtonComponent::executeSignal(signalType signalType, Engine::EnoEngine& eng)
	{
		// Loop through all signals and match with input type
		for (auto& signal : signals)
		{
			if (signal.first == signalType)
			{
				executeFunction(signal.second, eng);
			}
		}
	}

	void ButtonComponent::executeFunction(functionType funcType, Engine::EnoEngine& eng)
	{
		// Identify which type of function to execute
		switch (funcType)
		{
		case functionType::loadScene:
		{
			// Load scene
			eng.getEnoEditor().selectScene(targetScene);
			break;
		case functionType::toggleVisibility:
			auto& sceneTree = eng.getEnoEditor().getSceneTree();
			auto& entMng = eng.getEntityManager();
			auto allNodes = sceneTree.getAllNodes();

			// Find the target node by matching the name stored in targetObject
			for (auto* node : allNodes) {
				std::string nodeName;

				// Get the display name
				if (node->entity_.has<ECSComponents::NameComponent>(entMng)) {
					auto& nameComp = node->entity_.get<ECSComponents::NameComponent>(entMng);
					nodeName = std::string(nameComp.name);
				}
				else {
					// Display "node: " + node ID if no name component
					nodeName = "node: " + std::to_string(node->sceneObjectID_);
				}

				// Check if this is the target node
				if (nodeName == std::string(targetObject)) {
					// Check if the entity has a VisibilityComponent
					if (node->entity_.has<ECSComponents::VisibilityComponent>(entMng)) {
						auto& visComp = node->entity_.get<ECSComponents::VisibilityComponent>(entMng);
						visComp.visible = !visComp.visible;  // Flip the visibility
						break;  // Found and toggled, exit loop
					}
					break;  // Found the node but no visibility component
				}
			}
			break;
		}
		default:
			break;
		}
	}

}
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
		}
		default:
			break;
		}
	}

}
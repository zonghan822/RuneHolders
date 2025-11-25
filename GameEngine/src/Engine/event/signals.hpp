/*============================================================================================
* @file         signals.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration/definition file for the signals
*
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#pragma once

// stds
#include <vector>
#include <functional>
#include <any>

namespace Events
{
	// A signal is essentially a container of observers, with each callback stored within the signal
	// allowed to contain a list of params of any type. 
	// Inspired by: https://docs.godotengine.org/en/stable/tutorials/scripting/c_sharp/c_sharp_signals.html
	class EnoSignal
	{
	public:
		using Callback = std::function<void(const std::vector<std::any>&)>;

		// Subcribe
		void connect(Callback cb)
		{
			callbacks_.push_back(cb);
		}

		// Ping
		void emit(const std::vector<std::any>& p = {})
		{
			for (auto& callb : callbacks_) { callb(p); }
		}

	private:
		std::vector<Callback> callbacks_;
	};
}
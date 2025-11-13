#pragma once

// Input
#include "../input/eno_input.hpp"


namespace Bridge
{
	// Bridges functions between Engine end and the Scripting end
	class EnoBridge
	{
	public:

		EnoBridge(Input::EnoInputManager& input)
			:
			input_{ input }
		{}

		// Accessors
		Input::EnoInputManager& input() const { return input_; }

	private:
		Input::EnoInputManager& input_;
	};

	extern EnoBridge* bridge_;
}


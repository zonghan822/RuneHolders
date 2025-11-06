#pragma once

// ECS Components
#include "drawInfo.hpp"
#include "transform.hpp"
#include "visibility.hpp"
#include "../../math/math.hpp"
#include "../../input/eno_input.hpp"

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
		std::string getName() const;

		// Onclick function
		bool onClick(Input::EnoInputManager& input, const ECSComponents::VisibilityComponent& v, const ECSComponents::TransformComponent& t, const ECSComponents::DrawInfoComponent& d);

		template <typename Visitor>
		void reflect(Visitor&& l)
		{
			l("BtnText", btnText, "char32");
		}

		template <typename Visitor>
		void reflect(Visitor&& l) const
		{
			l("BtnText", btnText, "char32");
		}


	private:
		char btnText[32] = "Button";

	};

}
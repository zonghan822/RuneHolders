#pragma once

// Engine Component(s)
#include "../component/components/name.hpp"
#include "../component/components/layer.hpp"
#include "../component/components/transform.hpp"
#include "../component/components/modulation.hpp"
#include "../component/components/visibility.hpp"

//stds
#include <tuple>


namespace ECSComponents
{
	struct Preset
	{
		using EMPTY = std::tuple<>;

		using DEFAULT = std::tuple<
			NameComponent,
			VisibilityComponent,
			LayerComponent>;
	};
}
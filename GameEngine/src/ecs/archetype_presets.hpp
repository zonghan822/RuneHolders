#pragma once

// Engine Component(s)
#include "../component/components/name.hpp"
#include "../component/components/tag.hpp"
#include "../component/components/layer.hpp"
#include "../component/components/transform.hpp"
#include "../component/components/modulation.hpp"
#include "../component/components/visibility.hpp"

//stds
#include <tuple>

namespace ECS
{
	using namespace ECSComponents;

	struct Preset
	{
		using EMPTY = std::tuple<>;

		using DEFAULT = std::tuple<
			NameComponent,
			TagComponent,
			LayerComponent,
			TransformComponent>;
	};
}
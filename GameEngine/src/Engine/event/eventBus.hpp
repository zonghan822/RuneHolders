#pragma once

// stds
#include <unordered_map>
#include <functional>
#include <typeindex>

namespace Event
{
	class EnoEventBus
	{
	public:
		template <typename Event>
		void subscribe(std::function<void(const Event&)> handler)
		{
			// When the handler is inserted into the map
			// wrap it in a lambda that accepts const void* e and store
			//
			// Inside the lambda, cast e back to its original event type
			subscribers_[std::type_index(typeid(Event))].push_back([handler](const void* e)
				{
					handler(*static_cast<const Event*>(e));
				});
		}

		template <typename Event>
		void publish(const Event& event) const
		{
			auto it = subscribers_.find(std::type_index(typeid(Event)));
			if (it != subscribers_.end())
			{
				// each handler is the type-erased wrapper lambda
				// inside these lambdas
				// is 
				// [handler](const void* e) 
				// {
				//	handler(*static_cast<const Event*>(e));
				// }
				// Using the Event type, we can call the original lambda
				for (auto& handler : it->second)
				{
					handler(&event);
				}
			}
		}

	private:
		using Handler = std::function<void(const void*)>;

		// Store type-erased Handler as a common type to wrap
		// across any unique event type
		std::unordered_map<std::type_index, std::vector<Handler>> subscribers_;
	};
}
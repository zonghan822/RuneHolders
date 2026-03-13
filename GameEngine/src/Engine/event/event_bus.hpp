/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         event_bus.hpp
* @author       ZongHan Law (zonghan.law@digipen.edu)
* @date         10/3/2025
* @brief        Declaration/definition file for the event bus All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

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
		/*============================================================================================
			* \brief			Subscribe a handler function to an event type
			* \param[in]		A function (usually a lambda) to invoke when publishing
			* \return			Subscription ID for unsubscribing
		*//*========================================================================================*/
		template <typename Event>
		uint32_t subscribe(std::function<void(const Event&)> handler)
		{
			// When the handler is inserted into the map
			// wrap it in a lambda that accepts const void* e and store
			//
			// Inside the lambda, cast e back to its original event type
			auto id = subscriberCount_++;
			subscribers_[std::type_index(typeid(Event))].push_back({ id, [handler](const void* e)
				{
					handler(*static_cast<const Event*>(e));
				} });

			return id;
		}

		/*============================================================================================
			* \brief			Publish an event to all subscribed handlers
			* \param[in]		Event being published
		*//*========================================================================================*/
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
				for (auto& h : it->second)
				{
					h.handler_(&event);
				}
			}
		}

		/*============================================================================================
			* \brief			Unsubscribe a handler from an event type
			* \param[in]		Subscription ID
		*//*========================================================================================*/
		template <typename Event>
		void unsubscribe(uint32_t id)
		{
			auto it = subscribers_.find(std::type_index(typeid(Event)));
			if (it != subscribers_.end())
			{
				auto& vec = it->second;
				vec.erase(std::remove_if(vec.begin(), vec.end(), [id](const Subscriber& subscriber)
					{
						return subscriber.id_ == id;
					}), vec.end());
			}
		}

	private:
		struct Subscriber
		{
			uint32_t id_;
			std::function<void(const void*)> handler_;
		};

		// Store type-erased Handler as a common type to wrap
		// across any unique event type
		std::unordered_map<std::type_index, std::vector<Subscriber>> subscribers_;

		uint32_t subscriberCount_{ 0 };
	};
}
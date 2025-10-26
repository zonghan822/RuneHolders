#pragma once

// Engine Component(s)
#include "eno_scene_tree.hpp"
#include "../event/event_bus.hpp"

// Events
#include "../tag/tag_events.hpp"


namespace Editor
{
	enum class State
	{
		Playing,
		Paused,
		Stopped
	};

	class EnoScene
	{
	public:
		EnoScene(Event::EnoEventBus bus)
			:
			bus_{ bus },
			state { State::Paused }
		{
			defineTag("Human");
		}

		void assignTag()
		{

		}

		void defineTag(const std::string& tag)
		{
			bus_.publish(Tag::RecordTag{ tag });
		}

		void play()
		{
			state = State::Playing;
		}

		void pause()
		{
			state = State::Paused;
		}

		void stop()
		{
			state = State::Stopped;
		}

	private:
		// Scene state
		State state;

		// Event bus
		Event::EnoEventBus& bus_;
	};
}
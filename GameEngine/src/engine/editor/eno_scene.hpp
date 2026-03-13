/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_scene.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   67%
* @co-author    Caven Koh (caven.koh@digipen.edu)   33%
* @brief        See file name.
*//*========================================================================================*/

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
		NextFrame,
		Stopped
	};

	class EnoScene
	{
	public:
		EnoScene(Event::EnoEventBus bus)
			:
			bus_{ bus },
			state { State::Stopped }
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

		void next()
		{
			state = State::NextFrame;
		}

		State getState() const
		{
			return state;
		}

		State& getState()
		{
			return state;
		}

		// Get current scene name
		std::string getSceneName() const
		{
			return sceneName;
		}

		// Set current scene name
		void setSceneName(const std::string& name)
		{
			if (!name.empty())
			{
				sceneName = name;
			}
			
		}

	private:
		// Scene state
		State state;

		// Scene name
		std::string sceneName;

		// Event bus
		Event::EnoEventBus& bus_;
	};
}
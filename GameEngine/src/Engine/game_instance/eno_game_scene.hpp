/*============================================================================================
* @file         eno_game_scene.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration/definition file for the game scene
*//*========================================================================================*/

#pragma once

// Engine Component(s)
#include "eno_scene_tree.hpp"
#include "../event/event_bus.hpp"

// Events
#include "../tag/tag_events.hpp"


namespace GameInstance
{
	/*============================================================================================
		* \brief			Scene state definition
	*//*========================================================================================*/
	enum class State
	{
		Playing,
		Paused,
		Stopped
	};

	class EnoScene
	{
	public:

		/*============================================================================================
			* \brief			Scene constructor
		*//*========================================================================================*/
		EnoScene(Event::EnoEventBus bus)
			:
			bus_{ bus },
			state { State::Paused }
		{
			defineTag("Human");
		}

		/*============================================================================================
			* \brief			Assign custom tag to an entity
		*//*========================================================================================*/
		void assignTag()
		{

		}

		/*============================================================================================
			* \brief			Define a custom tag for grouping entities
		*//*========================================================================================*/
		void defineTag(const std::string& tag)
		{
			bus_.publish(Tag::RecordTag{ tag });
		}

		/*============================================================================================
			* \brief			Set scene state to Playing
		*//*========================================================================================*/
		void play()
		{
			state = State::Playing;
		}

		/*============================================================================================
			* \brief			Set scene state to Paused
		*//*========================================================================================*/
		void pause()
		{
			state = State::Paused;
		}

		/*============================================================================================
			* \brief			Set scene state to Stopped
		*//*========================================================================================*/
		void stop()
		{
			state = State::Stopped;
		}

	private:
		// Scene tree
		EnoSceneTree tree_;

		// Scene state
		State state;

		// Event bus
		Event::EnoEventBus& bus_;
	};
}
/*============================================================================================
* @file         eno_game_instance.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration/definition file for the game instance class
*//*========================================================================================*/


#pragma once

// Engine Component(s)
#include "eno_game_scene.hpp"

// Events
#include "game_instance_events.hpp"


namespace GameInstance
{
	class EnoGameInstance
	{
	public:

		/*============================================================================================
			* \brief			Game instance constructor
		*//*========================================================================================*/
		EnoGameInstance(Event::EnoEventBus& bus)
			:
			bus_ { bus },
			playingScene_{ false }
		{
			// Creates 1 scene by default if it is not loaded save data
			createScene();

			// First scene will be selected first
			selectedScene_ = scenes_.front().get();
		}

		/*============================================================================================
			* \brief			Create a unique game scene
		*//*========================================================================================*/
		void createScene()
		{
			std::unique_ptr<EnoScene> scene = std::make_unique<EnoScene>(bus_);
			scenes_.push_back(std::move(scene));
		}

		/*============================================================================================
			* \brief			Plays the selected scene via input (F5)
		*//*========================================================================================*/
		void selectScene()
		{
			
		}

		/*============================================================================================
			* \brief			Plays the selected scene via input (F5)
		*//*========================================================================================*/
		void playScene()
		{
			if (!playingScene_)
			{
				playingScene_ = true;

				// Create an additional window
				bus_.publish(CreateGameWindow{});
			}
		}

		/*============================================================================================
			* \brief			Pause a running scene via input (F7)
		*//*========================================================================================*/
		void pauseScene()
		{
			if (playingScene_)
			{
				std::cout << "Pausing scene!" << std::endl;
			}
		}

		/*============================================================================================
			* \brief			Stops playing a running scene via input (F8)
		*//*========================================================================================*/
		void stopScene()
		{
			if (playingScene_)
			{
				playingScene_ = false;

				// Destroy the game window
				bus_.publish(DestroyGameWindow{});
			}
		}

	private:
		std::vector<std::unique_ptr<EnoScene>> scenes_;
		EnoScene* selectedScene_;

		bool playingScene_;

		Event::EnoEventBus& bus_;
	};
}
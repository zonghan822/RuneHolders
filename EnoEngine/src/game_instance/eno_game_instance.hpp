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

		// Create and append a new scene
		void createScene()
		{
			std::unique_ptr<EnoScene> scene = std::make_unique<EnoScene>(bus_);
			scenes_.push_back(std::move(scene));
		}

		void selectScene()
		{
			
		}

		// Plays the selected scene
		void playScene()
		{
			if (!playingScene_)
			{
				playingScene_ = true;

				// Create an additional window
				bus_.publish(CreateGameWindow{});
			}
		}

		// Stops playing a running scene
		void stopScene()
		{
			if (playingScene_)
			{
				playingScene_ = false;

				// Destroy the game window
				bus_.publish(DestroyGameWindow{});
			}
		}

		// Pause a running scene
		void pauseScene()
		{
			if (playingScene_)
			{
				std::cout << "Pausing scene!" << std::endl;
			}
		}

	private:
		std::vector<std::unique_ptr<EnoScene>> scenes_;
		EnoScene* selectedScene_;

		bool playingScene_;

		Event::EnoEventBus& bus_;
	};
}
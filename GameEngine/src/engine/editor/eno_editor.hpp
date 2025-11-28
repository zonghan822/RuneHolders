#pragma once

// Engine Component(s)
#include "../editor/eno_scene.hpp"
#include "../editor/eno_scene_tree.hpp"

#include "../eno_core_host/eno_core_host.hpp"

#include "../scripting/eno_script_manager.hpp"

// Events
#include "../editor/editor_events.hpp"

// Undo System
#include "undo/undo_manager.hpp"


namespace Editor
{
	class EnoEditor
	{
	public:

		EnoEditor() = default;
		EnoEditor(const EnoEditor&) = delete;
		EnoEditor(EnoEditor&&) = delete;

		EnoEditor(Event::EnoEventBus& bus, ECS::EntityManager& entMng, Core::EnoCoreHost& coreHost, Map::TileMapSystem& tileMap)
			:
			bus_ { bus },
			entMng_{ entMng },
			coreHost_{ coreHost },
			undoManager_{ bus, entMng, 20 },
			tree_{ entMng, coreHost, tileMap }
		{

			// Get all available scenes
			// Check that the file is a .txt file
			try {
				for (const auto& fileEntry : std::filesystem::directory_iterator(ASSET_SCENE_PATH))
				{
					if (fileEntry.is_regular_file() && fileEntry.path().extension() == ".txt" && fileEntry.path().string() != (ASSET_SCENE_PATH "LastSavedScene.txt"))
					{
						// Add file to scenes_
						std::unique_ptr<EnoScene> scene = std::make_unique<EnoScene>(bus_);
						// Set scene name
						scene->setSceneName(fileEntry.path().filename().string());
						std::cout << "[Eno_Editor] appending scene: " << scene->getSceneName() << std::endl;
						scenes_.push_back(std::move(scene));
					}
				}
			}
			// In case of invalid file
			catch (const std::filesystem::filesystem_error& e)
			{
				std::cout << "Error: " << e.what();
				return;
			}

			// Find the last loaded scene if possible
			loadLastSavedScene();

			// *** TBD when operation isnt needed! ***
			bus_.subscribe<Editor::TestTree>([&](const Editor::TestTree)
				{
					operation();
				});

			// Subscribe to delete selected entity event
			// NOTE: This is just a placeholder - actual deletion happens in GameApplication
			// because entities are stored in GameApplication's sceneTree_
			bus_.subscribe<Editor::DeleteSelectedEntity>([&](const Editor::DeleteSelectedEntity)
				{
					// Event will be handled by GameApplication's subscriber
				});
		}

		// Testing the scene tree
		void operation()
		{

		}

		// Create and append a new scene
		void createScene()
		{

		}

		void selectScene(const std::string& filename)
		{
			
			// Load selected scene
			tree_.load(ASSET_SCENE_PATH + filename);

			// Point to selected scene
			auto index = std::find_if(scenes_.begin(), scenes_.end(),
				[&](const std::unique_ptr<Editor::EnoScene>& scene) {return filename == scene->getSceneName(); });

			// Update current scene pointer
			if (index != scenes_.end())
			{
				selectedScene_ = index->get();
			}
			else
			{
				selectedScene_ = nullptr;
				std::cout << "[Eno_Editor] Selecting scene: " << filename << " unsuccessful, unable to find scene!" << std::endl;
			}


			// Publish event to notify UI that scene was cleared
			bus_.publish(SceneCleared{});
		}

		// Plays the selected scene
		void playScene()
		{
			std::string sceneName = selectedScene_->getSceneName();
				
			// Save the current state of the scene
			tree_.save(ASSET_SCENE_PATH + sceneName, sceneName);

			// Update last saved scene file
			std::cout << "[Eno_editor] current scene: " << sceneName  << std::endl;
			updateLastSavedScene();

			// Set editor state to playing
			selectedScene_->play();

			// Create an additional window
			//bus_.publish(CreateGameWindow{});
			//bus_.publish(GameInitialized{});
		}

		// Stops playing a running scene
		void stopScene()
		{

			// Load initial setting of the scene
			//std::cout << "[Eno_editor] Loading saved scene tree to restore state..." << std::endl;
			loadLastSavedScene();

			// Set editor state to stop
			selectedScene_->stop();

			// Destroy the game window
			//bus_.publish(DestroyGameWindow{});
			//bus_.publish(GameStopped{});
		}

		// Pause a running scene
		void pauseScene()
		{
			selectedScene_->pause();

			std::cout << "Pausing scene!" << std::endl;
			
		}

		// Create empty scene
		void createEmptyScene(const std::string& filePath)
		{
			// Default name
			std::string defaultSceneName = "New Scene";
			// To count for duplicates
			int index = 0;
			// Track which indices already exist
			std::set<int> existingIndices;
			bool baseNameExists = false;

			// Iterate through the folder, see how many new scenes duplicate there are
			try
			{
				for (const auto& fileName : std::filesystem::directory_iterator(filePath))
				{
					std::string name = fileName.path().filename().string();

					if (name.find("New Scene") != std::string::npos)
					{
						// Check if it's exactly "New Scene.txt"
						if (name == "New Scene.txt")
						{
							baseNameExists = true;
						}
						else
						{
							// Find all other existing duplicates and their indexes
							size_t openParen = name.find('(');
							size_t closeParen = name.find(')');

							if (openParen != std::string::npos && closeParen != std::string::npos)
							{
								std::string numberStr = name.substr(openParen + 1, closeParen - openParen - 1);
								try
								{
									// Convert to int and add to set
									int num = std::stoi(numberStr);
									existingIndices.insert(num);
								}
								catch (...) {} // Not a valid int, ignore
							}
						}
					}
				}
			}
			catch (const std::filesystem::filesystem_error& e)
			{
				std::cerr << "[EnoSceneTree] Error reading directory: " << e.what() << std::endl;
			}

			// Find the next available index
			if (baseNameExists)
			{
				// Start from 1 and find the first gap
				index = 1;
				while (existingIndices.find(index) != existingIndices.end())
				{
					++index;
				}
				// Add suffix
				defaultSceneName += " (" + std::to_string(index) + ')';
			}

			// Else just use base and add .txt extension
			defaultSceneName += ".txt";

			// Create full file path
			std::string fullPath = filePath + defaultSceneName;

			// Create file
			std::ofstream newFile{ fullPath };
			if (!newFile)
			{
				throw std::runtime_error("Failed to create a new scene: " + fullPath);
			}
			newFile << "{\n\n}";
			newFile.close();

			// Load file first (this sets currentScene)
			tree_.load(fullPath);

			// Create and append a new scene
			std::unique_ptr<EnoScene> scene = std::make_unique<EnoScene>(bus_);
			scenes_.push_back(std::move(scene));

			selectedScene_ = scenes_.back().get();

			// Set current scene name
			selectedScene_->setSceneName(defaultSceneName);

			// Save the current scene name to LastSavedScene.txt
			updateLastSavedScene();

			// Publish event to notify UI that scene was cleared
			bus_.publish(SceneCleared{});
		}

		// Load last saved scene
		void loadLastSavedScene()
		{
			// Get path to assets
			std::string currentScenePath = ASSET_SCENE_PATH "LastSavedScene.txt";
			std::ifstream savedSceneFile{ currentScenePath };
			if (!savedSceneFile.is_open())
			{
				// File doesn't exist, create a new scene
				std::cout << "[Eno_Editor] No saved scene found, creating new scene" << std::endl;
				createEmptyScene(ASSET_SCENE_PATH);
			}
			else
			{
				// Saved file name
				std::string savefile;
				std::getline(savedSceneFile, savefile);
				// Close file
				savedSceneFile.close();

				std::cout << "Loading save file: " << savefile << std::endl;

				// If save scene file is empty, create a new scene
				if (savefile.empty())
				{
					std::cout << "[Eno_Editor] LastSavedScene.txt is empty, creating new scene..." << std::endl;
					createEmptyScene(ASSET_SCENE_PATH);
				}
				else
				{
					// Check if the scene file actually exists before loading
					std::string fullScenePath = ASSET_SCENE_PATH + savefile;
					if (std::filesystem::exists(fullScenePath))
					{
						tree_.load(fullScenePath);

						// Find scene index in scenes_ list
						auto index = std::find_if(scenes_.begin(), scenes_.end(), [&](const std::unique_ptr<EnoScene>& scene) {return savefile == scene->getSceneName(); });
						if (index != scenes_.end())
						{
							// Set selected scene pointer to the index in the scenes vector
							selectedScene_ = index->get();
							// Set current scene name
							std::cout << "[Eno_Editor] Loaded scene name: " << selectedScene_->getSceneName() << std::endl;

						}

						updateLastSavedScene();
					}
					else
					{
						std::cerr << "[EnoSceneTree] Scene file not found: " << fullScenePath << ", creating new scene..." << std::endl;
						createEmptyScene(ASSET_SCENE_PATH);
					}
				}
			}
		}

		/*============================================================================================
		* \brief       Updates the text file that is storing the name of the last saved scene 
		* \return      void
		*//*========================================================================================*/
		void updateLastSavedScene()
		{
			std::ofstream lastSceneSave{ ASSET_SCENE_PATH "LastSavedScene.txt" };
			if (!lastSceneSave)
			{
				throw std::runtime_error("[Eno_Editor] Failed to save current active scene name!");
			}
			lastSceneSave << selectedScene_->getSceneName();
			lastSceneSave.close();
		}

		EnoSceneTree& getSceneTree()
		{
			return tree_;
		}

		// Get selected scene
		EnoScene& getCurrentScene()
		{
			return *selectedScene_;
		}

		std::vector<std::unique_ptr<EnoScene>>& getScenes()
		{
			return scenes_;
		}

		Script::EnoScriptManager& getScriptManager() {
			return scriptMng_;
		}

		UndoManager& getUndoManager() {
			return undoManager_;
		}

	private:
		// Scenes
		std::vector<std::unique_ptr<EnoScene>> scenes_;
		EnoScene* selectedScene_;

		// Core Host
		ECS::EntityManager& entMng_;

		// Core Host
		Core::EnoCoreHost& coreHost_;

		// Scripting
		Script::EnoScriptManager scriptMng_{ coreHost_ };

		// Events
		Event::EnoEventBus& bus_;

		// Scene tree
		EnoSceneTree tree_;

		// Undo system
		UndoManager undoManager_;
	};
}
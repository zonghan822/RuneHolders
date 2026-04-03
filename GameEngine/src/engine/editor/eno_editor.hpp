/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_editor.hpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   33%
* @co-author    Caven Koh (caven.koh@digipen.edu)   31%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   23%
* @co-author    Koe Yun Ji (yunji.koe@digipen.edu)   12%
* @brief        See file name.
*//*========================================================================================*/

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

		EnoEditor(Event::EnoEventBus& bus, ECS::EntityManager& entMng, Core::EnoCoreHost& coreHost, Map::TileMapSystem& tileMap, ECSComponents::EnoPrefabManager& prefabMng, Graphics::EnoGraphicsManager& graphicsMng)
			:
			bus_ { bus },
			entMng_{ entMng },
			coreHost_{ coreHost },
			prefabMng_{ prefabMng },
			undoManager_{ bus, entMng, graphicsMng, 30 },
			tree_{ entMng, coreHost, tileMap, prefabMng }
		{
			std::cout << "[EnoEditor] Scanning scenes directory..." << std::endl;

			// Get all available scenes
			try {
				for (const auto& fileEntry : std::filesystem::directory_iterator(FileSystem::GetAssetsDir().string() +  "/saves/scenes"))
				{
					if (fileEntry.is_regular_file() && fileEntry.path().extension() == ".txt" &&
						fileEntry.path().filename().string() != "StartScene.txt")
					{
						std::unique_ptr<EnoScene> scene = std::make_unique<EnoScene>(bus_);
						scene->setSceneName(fileEntry.path().filename().string());
						scenes_.push_back(std::move(scene));
					}
				}
			}
			catch (const std::filesystem::filesystem_error& e)
			{
				std::cout << "[EnoEditor] Scene directory not found: " << e.what() << "\n";
			}

			std::cout << "[EnoEditor] Found " << scenes_.size() << " scene(s). Loading initial scene..." << std::endl;

			// Load the start scene; fall back to first available or create empty
			loadInitialScene();

			std::cout << "[EnoEditor] Scene loaded successfully." << std::endl;

			// Safety net
			if (!selectedScene_)
			{
				scenes_.push_back(std::make_unique<EnoScene>(bus_));
				selectedScene_ = scenes_.back().get();
				selectedScene_->setSceneName("New Scene.txt");
			}

			// Subscribe to delete selected entity event
			// NOTE: This is just a placeholder - actual deletion happens in EditorApplication
			// because entities are stored in EditorApplication's sceneTree_
			bus_.subscribe<Editor::DeleteSelectedEntity>([&](const Editor::DeleteSelectedEntity)
				{
					// Event will be handled by EditorApplication's subscriber
				});
		}

		// Testing the scene tree
		void operation()
		{

		}

		/*============================================================================================
		* \brief		This function deletes the scene from the save folder as well as the scenes_ container
		* \param[in]	std::string sceneName
		*//*========================================================================================*/
		void deleteScene(std::string sceneName)
		{
			std::string path = FileSystem::GetAssetsDir().string() + "/saves/scenes/" + sceneName;
			std::filesystem::remove(path);

			// Also delete the corresponding tilemap JSON file if it exists
			std::string tilemapPath = path;
			if (tilemapPath.size() >= 4 && tilemapPath.substr(tilemapPath.size() - 4) == ".txt")
				tilemapPath = tilemapPath.substr(0, tilemapPath.size() - 4) + "_tilemap.json";
			std::filesystem::remove(tilemapPath);

			// Find scene index in scenes_ list
			auto index = std::find_if(scenes_.begin(), scenes_.end(),
				[&](const std::unique_ptr<Editor::EnoScene>& scene) {return sceneName == scene->getSceneName(); });

			// Delete pointer to scene
			if (index != scenes_.end())
			{
				scenes_.erase(index);
			}
		}

		/*============================================================================================
		* \brief		This function selects the next scene to load
		* \param[in]	std::string fileName
		*//*========================================================================================*/
		void selectScene(const std::string& filename)
		{
			// Check state for current scene
			if (selectedScene_->getState() != State::Stopped)
			{
				// Stop the scene
				bus_.publish(StopScene{});
				selectedScene_->stop();
			}

			// Point to selected scene
			auto index = std::find_if(scenes_.begin(), scenes_.end(),
				[&](const std::unique_ptr<Editor::EnoScene>& scene) {return filename == scene->getSceneName(); });

			// Update current scene pointer
			if (index != scenes_.end())
			{
				selectedScene_ = index->get();
				// Check if the previous scene was in play
				if (wasPlaying)
				{
					selectedScene_->play();
				}
			}
			else
			{
				selectedScene_ = nullptr;
				std::cout << "[Eno_Editor] Selecting scene: " << filename << " unsuccessful, unable to find scene!" << std::endl;
			}

			tree_.load(FileSystem::GetAssetsDir().string() + "/saves/scenes/" + filename);

			// Re-register scripts after scene load (in case scripting system is already initialized)
			tree_.registerAllScripts();

			// Publish event to notify UI that scene was cleared
			bus_.publish(SceneCleared{});
		}

		/*============================================================================================
		* \brief		This function renames the name of the scene file as well as the scene name
		* \param[in]	std::string newName, std::string oldName
		*//*========================================================================================*/
		void renameScene(const std::string& newName, const std::string& oldName)
		{
			try
			{
				// Get current filepath
				std::filesystem::path currentPath = FileSystem::GetAssetsDir().string() + "/saves/scenes/" + oldName;

				// Check for .txt extension in new file
				std::string newSceneName = newName;
				if (newSceneName.find(".txt") == std::string::npos) newSceneName += ".txt";

				std::filesystem::path newPath = FileSystem::GetAssetsDir().string() + "/saves/scenes/" + newSceneName;
				std::filesystem::rename(currentPath, newPath);

				// Check if we're renaming the currently active scene
				bool isCurrentScene = (oldName == selectedScene_->getSceneName());

				if (isCurrentScene)
				{
					selectedScene_->setSceneName(newSceneName);
				}
				else
				{
					auto it = std::find_if(scenes_.begin(), scenes_.end(),
						[&](const std::unique_ptr<EnoScene>& scene) {return oldName == scene->getSceneName(); });

					if (it != scenes_.end())
					{
						it->get()->setSceneName(newSceneName);
					}
				}

				// If the start scene was renamed, update it too
				if (startSceneName_ == oldName)
				{
					setStartScene(newSceneName);
				}
			}
			catch (const std::filesystem::filesystem_error& e)
			{
				std::string str = "Error[Name]: " + std::string(e.what());
				return;
			}
		}

		/*============================================================================================
		* \brief		This function sets current scene state to play
		*//*========================================================================================*/
		void playScene()
		{
			// If editor starts from stop state, save the scene
			if (!wasPlaying)
			{
				std::string sceneName = selectedScene_->getSceneName();

				// Save the current scene so it can be restored on stop
				tree_.save(FileSystem::GetAssetsDir().string() + "/saves/scenes/" + sceneName);

				// Record which scene was active so stopScene() can reload it
				prePlaySceneName_ = sceneName;
				std::cout << "[Eno_editor] current scene: " << sceneName << std::endl;

				// If a start scene is designated and it differs from the current scene, switch to it
				if (!startSceneName_.empty() && startSceneName_ != sceneName)
				{
					auto it = std::find_if(scenes_.begin(), scenes_.end(),
						[&](const std::unique_ptr<EnoScene>& s) { return s->getSceneName() == startSceneName_; });
					if (it != scenes_.end())
					{
						selectedScene_ = it->get();
						tree_.load(FileSystem::GetAssetsDir().string() + "/saves/scenes/" + startSceneName_);
						tree_.registerAllScripts();
						bus_.publish(SceneCleared{});
						std::cout << "[EnoEditor] Starting game from start scene: " << startSceneName_ << std::endl;
					}
				}
				// Flag to check if editor has already been played once
				wasPlaying = true;

			}

			// Set editor state to playing
			selectedScene_->play();

			// Publish PlayScene event to notify systems (e.g., audio)
			bus_.publish(PlayScene{});
		}

		/*============================================================================================
		* \brief		This function sets current scene state to stop
		*//*========================================================================================*/
		void stopScene()
		{
			// Publish StopScene event to notify systems (e.g., audio)
			bus_.publish(StopScene{});

			// Set editor state to stop
			selectedScene_->stop();
			wasPlaying = false;

			// Reload the scene that was active before play to reset any runtime changes
			std::string restoreScene = prePlaySceneName_.empty() ? startSceneName_ : prePlaySceneName_;
			if (!restoreScene.empty())
			{
				std::string fullPath = FileSystem::GetAssetsDir().string() + "/saves/scenes/" + restoreScene;
				if (std::filesystem::exists(fullPath))
				{
					auto it = std::find_if(scenes_.begin(), scenes_.end(),
						[&](const std::unique_ptr<EnoScene>& s) { return s->getSceneName() == restoreScene; });
					if (it != scenes_.end())
						selectedScene_ = it->get();
					tree_.load(fullPath);
					tree_.registerAllScripts();
					bus_.publish(SceneCleared{});
				}
			}
			prePlaySceneName_.clear();
		}

		/*============================================================================================
		* \brief		This function sets current scene state to pause
		*//*========================================================================================*/
		void pauseScene()
		{
			selectedScene_->pause();

			std::cout << "Pausing scene!" << std::endl;

		}

		/*============================================================================================
		* \brief		This function creates a new empty scene
		* \param[in]	const std::string& filePath
		*//*========================================================================================*/
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

			// Ensure directory exists before creating the file
			std::filesystem::create_directories(filePath);

			// Create file
			std::ofstream newFile{ fullPath };
			if (!newFile)
			{
				std::cerr << "[EnoEditor] Failed to create a new scene: " << fullPath << std::endl;
				return;
			}
			newFile << "{\n\n}";
			newFile.close();

			// Load file first (this sets currentScene)
			tree_.load(fullPath);
			tree_.registerAllScripts();

			// Create and append a new scene
			std::unique_ptr<EnoScene> scene = std::make_unique<EnoScene>(bus_);
			scenes_.push_back(std::move(scene));

			selectedScene_ = scenes_.back().get();

			// Set current scene name
			selectedScene_->setSceneName(defaultSceneName);

			// Publish event to notify UI that scene was cleared
			bus_.publish(SceneCleared{});
		}

		/*============================================================================================
		* \brief	Reload the editor for the project now pointed to by FileSystem::GetAssetsDir().
		*           Clears the current scene list, rescans the new project's scenes directory,
		*           and loads the start scene (or the first available, or creates an empty one).
		*           Call after FileSystem::Initialize(newProject).
		*//*========================================================================================*/
		void reloadProject()
		{
			scenes_.clear();

			try
			{
				for (const auto& fileEntry : std::filesystem::directory_iterator(
					FileSystem::GetAssetsDir().string() + "/saves/scenes"))
				{
					if (fileEntry.is_regular_file() &&
						fileEntry.path().extension() == ".txt" &&
						fileEntry.path().filename().string() != "StartScene.txt")
					{
						std::unique_ptr<EnoScene> scene = std::make_unique<EnoScene>(bus_);
						scene->setSceneName(fileEntry.path().filename().string());
						scenes_.push_back(std::move(scene));
					}
				}
			}
			catch (const std::filesystem::filesystem_error& e)
			{
				std::cout << "[EnoEditor] reloadProject: " << e.what() << std::endl;
			}

			loadInitialScene();

			if (!selectedScene_)
			{
				scenes_.push_back(std::make_unique<EnoScene>(bus_));
				selectedScene_ = scenes_.back().get();
				selectedScene_->setSceneName("New Scene.txt");
			}
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

		// Returns true if a scene is loaded and currently in play/paused/next-frame state
		bool isScenePlaying() const
		{
			if (!selectedScene_) return false;
			auto state = selectedScene_->getState();
			return state == State::Playing  ||
			       state == State::Paused   ||
			       state == State::NextFrame;
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

		ECS::EntityManager& getEntityManager() {
			return entMng_;
		}

		// ── Start Scene ─────────────────────────────────────────────────────────
		/*============================================================================================
		* \brief	Returns the name of the designated start scene (empty if not set).
		*//*========================================================================================*/
		const std::string& getStartScene() const { return startSceneName_; }

		/*============================================================================================
		* \brief	Marks sceneName as the start scene and persists it to StartScene.txt.
		*//*========================================================================================*/
		void setStartScene(const std::string& sceneName)
		{
			startSceneName_ = sceneName;
			std::string path = FileSystem::GetAssetsDir().string() + "/saves/scenes/StartScene.txt";
			std::filesystem::create_directories(FileSystem::GetAssetsDir() / "saves/scenes");
			std::ofstream f{ path };
			if (f) f << sceneName;
		}

	private:
		// ── Initial scene loading ────────────────────────────────────────────
		// Reads StartScene.txt, loads that scene into the tree, and sets selectedScene_.
		// Falls back to the first scene in scenes_, or creates an empty scene if none exist.
		void loadInitialScene()
		{
			// Read start scene name from disk
			{
				std::string path = FileSystem::GetAssetsDir().string() + "/saves/scenes/StartScene.txt";
				std::ifstream f{ path };
				if (f) std::getline(f, startSceneName_);
				std::cout << "[EnoEditor] StartScene.txt: " << (startSceneName_.empty() ? "(empty)" : startSceneName_) << std::endl;
			}

			// Try to load the start scene
			if (!startSceneName_.empty())
			{
				std::string fullPath = FileSystem::GetAssetsDir().string() + "/saves/scenes/" + startSceneName_;
				if (std::filesystem::exists(fullPath))
				{
					auto it = std::find_if(scenes_.begin(), scenes_.end(),
						[&](const std::unique_ptr<EnoScene>& s) { return s->getSceneName() == startSceneName_; });
					if (it != scenes_.end())
					{
						selectedScene_ = it->get();
						std::cout << "[EnoEditor] Loading start scene: " << startSceneName_ << std::endl;
						tree_.load(fullPath);
						std::cout << "[EnoEditor] Scene tree loaded, registering scripts..." << std::endl;
						tree_.registerAllScripts();
						std::cout << "[EnoEditor] Scripts registered." << std::endl;
						return;
					}
				}
				// Start scene file missing or not in list — clear stale name
				std::cout << "[EnoEditor] Start scene not found: " << startSceneName_ << std::endl;
				startSceneName_.clear();
			}

			// Fall back to the first scene in the list
			if (!scenes_.empty())
			{
				selectedScene_ = scenes_.front().get();
				std::string fullPath = FileSystem::GetAssetsDir().string() + "/saves/scenes/" + selectedScene_->getSceneName();
				std::cout << "[EnoEditor] Loading fallback scene: " << selectedScene_->getSceneName() << std::endl;
				tree_.load(fullPath);
				tree_.registerAllScripts();
				std::cout << "[EnoEditor] No start scene set, loaded first scene: " << selectedScene_->getSceneName() << std::endl;
				return;
			}

			// No scenes at all — create a blank one (skip if directory is read-only)
			std::cout << "[EnoEditor] No scenes found, creating empty scene..." << std::endl;
			try {
				createEmptyScene(FileSystem::GetAssetsDir().string() + "/saves/scenes/");
			}
			catch (const std::exception& e) {
				std::cerr << "[EnoEditor] Failed to create empty scene: " << e.what() << std::endl;
			}
		}

		// Scenes
		std::vector<std::unique_ptr<EnoScene>> scenes_;
		EnoScene* selectedScene_ = nullptr;
		std::string startSceneName_;
		std::string prePlaySceneName_;
		bool wasPlaying = false;

		// Core Host
		ECS::EntityManager& entMng_;

		// Core Host
		Core::EnoCoreHost& coreHost_;

		// Prefab Manager
		ECSComponents::EnoPrefabManager& prefabMng_;

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

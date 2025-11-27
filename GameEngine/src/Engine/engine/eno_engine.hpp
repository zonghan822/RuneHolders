/*============================================================================================
* @file         eno_engine.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu),
* @author		Muhammad Raziq Bin Mohd Rafig (),
* @author		Law Zong Han (zonghan.law@digipen.edu)
* @date         03-10-2025
* @brief		Main engine class that orchestrates all engine systems and provides the core 
				game loop
*//*========================================================================================*/

#pragma once
// Engine component(s)
#include "../graphics/graphics.hpp"
#include "../settings/eno_config.hpp"
#include "../resources/eno_loader.hpp"
#include "../window/eno_window.hpp"
#include "../input/eno_input.hpp"
#include "../editor/eno_editor.hpp"
#include "../editor/eno_scene_tree.hpp"
#include "../bridge/eno_bridge.hpp"
#include "../ecs/entity.hpp"
#include "../component/component_registry.hpp"
#include "../event/signals.hpp"
#include "../audio/eno_audio_manager.hpp"
#include "../math/math.hpp"
#include "../tag/tag_registry.hpp"
#include "../collision/collision.hpp"
#include "../prefab/prefab.hpp"
#include "../crashlog/crashlogger.hpp"
#include "../graphics/animation/animation.hpp"
#include "../ai/ai_manager.hpp"
#include "../physics/physics_system.hpp"
#include "../tilemap/tilemap.hpp"


// stds
#include <functional>
#include <chrono>
#include <algorithm>
#include <memory>

#if defined(ENGINE_BUILD_DLL)
	#define ENGINE_API __declspec(dllexport)
# elif defined(ENGINE_USE_DLL)
	#define ENGINE_API __declspec(dllimport)
#else
	#define ENGINE_API
#endif

namespace Engine
{
	/*============================================================================================
	* \class	EnoEngine
	* \brief	Core engine class that manages all engine subsystems and provides the main game loop
	*
	*			The EnoEngine serves as the central coordinator for all engine systems including
	*			graphics, audio, input, ECS, and resource management. It provides the main game loop
	*			and exposes interfaces for game code to interact with engine subsystems.
	*//*========================================================================================*/
	class ENGINE_API EnoEngine
	{
	public:
		/*============================================================================================
		* \brief		Initializes all engine systems and sets up the application window
		*//*========================================================================================*/
		void setup();

		/*============================================================================================
		* \brief		Starts the main game loop with the provided update callback
		* \param[in]	gameUpdateCallback 
		*		Function called each frame for game logic updates
		*//*========================================================================================*/
		void run(std::function<void()> gameUpdateCallback);

		/*============================================================================================
		* \brief		Shuts down all engine systems and cleans up resources
		*//*========================================================================================*/
		void shutDown();

		// Provide access to engine systems for game code

		/*============================================================================================
		* \brief		Gets the Entity Manager for ECS operations
		* \return		Reference to the EntityManager instance
		*//*========================================================================================*/
		ECS::EntityManager& getEntityManager() { return entityMng_; }

		/*============================================================================================
		* \brief		Gets the Prefab Manager for preset prefabs
		* \return		Reference to the PrefabManager instance
		*//*========================================================================================*/
		ECSComponents::EnoPrefabManager& getPrefabManager() { return prefabMng_; }

		/*============================================================================================
		* \brief		Gets the Audio Manager for sound operations
		* \return		Reference to the EnoAudioManager instance
		*//*========================================================================================*/
		Audio::EnoAudioManager& getAudioManager() { return audioMng_; }

		/*============================================================================================
		* \brief		Gets the Graphics Manager for rendering operations
		* \return		Reference to the EnoGraphicsManager instance
		*//*========================================================================================*/
		Graphics::EnoGraphicsManager& getGraphicsManager() { return graphicsMng_; }

		/*============================================================================================
		* \brief		Gets the Event Bus for event publishing/subscription
		* \return		Reference to the EnoEventBus instance
		*//*========================================================================================*/
		Event::EnoEventBus& getEventBus() { return bus_; }

		/*============================================================================================
		* \brief		Gets the Resource Loader for asset management
		* \return		Reference to the EnoLoader instance
		*//*========================================================================================*/
		Resources::EnoLoader& getResourceLoader() { return loader_; }

		/*============================================================================================
		* \brief		Gets the Resource Loader for asset management
		* \return		Reference to the EnoLoader instance
		*//*========================================================================================*/
		Animation::AnimationControllerRegistry& getAnimationRegistry() { return animRegistry; }

		/*============================================================================================
		* \brief		Gets the Config Manager for settings management
		* \return		Reference to the EnoConfigManager instance
		*//*========================================================================================*/
		Config::EnoConfigManager& getConfigManager() { return configMng_; }

		/*============================================================================================
		* \brief		Gets the Window Manager for window operations
		* \return		Reference to the EnoWindow instance
		*//*========================================================================================*/
		Window::EnoWindow& getWindowManager() { return window_; }

		/*============================================================================================
		* \brief		Gets the Input Manager for input handling
		* \return		Reference to the EnoInputManager instance
		*//*========================================================================================*/
		Input::EnoInputManager& getInputManager() { return inputMng_; }

		/*============================================================================================
		* \brief		Gets the Core Host for scripting operations
		* \return		Reference to the EnoCoreHost instance
		*//*========================================================================================*/
		Core::EnoCoreHost& getCoreHost() { return coreHost_; }

		/*============================================================================================
		* \brief		Gets the Eno Editor for scripting operations
		* \return		Reference to the EnoCoreHost instance
		*//*========================================================================================*/
		Editor::EnoEditor& getEnoEditor() { return *editor_; }

		/*============================================================================================
		* \brief		Gets the AI Manager for AI operations (State Manager)
		* \return		Reference to the AIManager instance
		*//*========================================================================================*/
		AI::AIManager& getAIManager() { return *aiMng_; }

		/*============================================================================================
		* \brief		Gets the time elapsed since the last frame
		* \return		Delta time in seconds
		*//*========================================================================================*/
		double getDeltaTime() const { return dt_; }

		/*============================================================================================
		* \brief		Gets the current frames per second
		* \return		Current FPS value
		*//*========================================================================================*/
		double getFPS() const { return fps_; }

		/*============================================================================================
		* \brief		Gets the Physics System for physics updates
		* \return		Pointer to PhysicsSystem instance (may be null if not initialized)
		*//*========================================================================================*/
		Physics::PhysicsSystem* getPhysicsSystem() { return physicsSys_.get(); }

		/*============================================================================================
		* \brief		Gets the Collision System for collision detection
		* \return		Pointer to CollisionSystem instance (may be null if not initialized)
		*//*========================================================================================*/
		Collision::CollisionSystem* getCollisionSystem() { return collisionSys_.get(); }

		Map::TileMapSystem& getTileMap() { return *tilemapSystem_; }

		AI::PathfindingSystem& getPathfinder() { return *pathfinder_; }

		/*============================================================================================
		* \brief		Sets whether the game is in playing mode (affects player input processing)
		* \param[in]	isPlaying - True if game is playing, false if in editor mode
		*//*========================================================================================*/
		void setIsPlaying(bool isPlaying) { isPlaying_ = isPlaying; }

		/*============================================================================================
		* \brief		Gets whether the game is in playing mode
		* \return		True if game is playing, false if in editor mode
		*//*========================================================================================*/
		bool getIsPlaying() const { return isPlaying_; }


	private:

		/*============================================================================================
		* \brief		Sets up resource loading for all asset types
		*//*========================================================================================*/
		void setupResourceLoading();

		/*============================================================================================
		* \brief		Updates time calculations including delta time and FPS
		* \param[in] fpsCalcInt 
		*		Interval in seconds for FPS calculation
		*//*========================================================================================*/
		void updateTime(double fpsCalcInt = 1.0);

		// Services & Tools
		Config::EnoConfigManager configMng_;
		Event::EnoEventBus bus_;
		Window::EnoWindow window_{ bus_ };
		Resources::EnoLoader loader_{ bus_ };
		ECS::EntityManager entityMng_{ bus_ };
		ECSComponents::EnoPrefabManager prefabMng_{ bus_ };
		Core::EnoCoreHost coreHost_{ bus_ };
		std::unique_ptr<Map::TileMapSystem> tilemapSystem_;
		std::unique_ptr<AI::PathfindingSystem> pathfinder_;
		std::unique_ptr<Editor::EnoEditor> editor_;
		Animation::AnimationControllerRegistry animRegistry;

		// Systems
		Input::EnoInputManager inputMng_{ bus_ };
		Graphics::EnoGraphicsManager graphicsMng_{ bus_ };
		Audio::EnoAudioManager audioMng_{ bus_ };
		std::unique_ptr<AI::AIManager> aiMng_;
		std::unique_ptr<Physics::PhysicsSystem> physicsSys_;
		std::unique_ptr<Collision::CollisionSystem> collisionSys_;

		// Game state
		bool isPlaying_ = false;  // Tracks if game is in playing mode (vs editor mode)

		// Time
		double dt_ = 0.0;
		double fps_ = 0.0;
	};
}
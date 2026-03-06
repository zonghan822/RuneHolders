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
#include "../file/file_system.hpp"
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
#include "../state_machine/state_machine_system.hpp"
#include "../particle/particle_system.hpp"
#include "../ui/button_system.hpp"


// stds
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
		* \brief		Starts a fully self-contained game loop (no C++ callback needed).
		*               Use this when all game logic lives in registered C# Logic Systems.
		*//*========================================================================================*/
		void run();

		/*============================================================================================
		* \brief		Returns true when the window has been requested to close.
		*               Use in a custom game loop together with beginFrame() / endFrame().
		*//*========================================================================================*/
		bool shouldClose() const { return window_.shouldClose(); }

		/*============================================================================================
		* \brief		Executes the logic half of a frame: time, input polling, script/physics/
		*               collision/particle/button/audio updates. Does NOT render.
		*               Call before updateEditorCamera() and renderFrame().
		*//*========================================================================================*/
		void beginFrame();

		/*============================================================================================
		* \brief		Renders the current frame into whatever framebuffer is currently bound:
		*               updates animations, camera, clears the screen, and issues draw calls.
		*               Call after beginFrame() and any editor-side camera updates.
		*//*========================================================================================*/
		void renderFrame();

		/*============================================================================================
		* \brief		Presents the rendered frame and resets per-frame input state.
		*               Call after renderFrame() and any ImGui rendering.
		*//*========================================================================================*/
		void endFrame();

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
		FSM::StateMachineSystem& getStateMachineSys() { return *stateMachineSys_; }

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

		AI::AIManager& getAIManager() { return *aiMng_; }

		/*============================================================================================
		* \brief		Gets the Particle System for particle effects
		* \return		Pointer to ParticleSystem instance (may be null if not initialized)
		*//*========================================================================================*/
		Particles::ParticleSystem* getParticleSystem() { return particleSys_.get(); }

		/*============================================================================================
		* \brief		Sets whether the game is in playing mode (affects player input processing)
		* \param[in]	isPlaying - True if game is playing, false if in editor mode
		*//*========================================================================================*/
		void setIsPlaying(bool isPlaying) { isPlaying_ = isPlaying; }

		/*============================================================================================
		* \brief		Sets the screen-space bounds of the rendered game viewport (imageMin/Max for
		*               screen-to-world conversion). In full-screen mode this is (0,0)-(w,h); in
		*               editor mode it is the ImGui game-panel bounds.
		*//*========================================================================================*/
		void setViewportBounds(Math::eVec2<float> min, Math::eVec2<float> max) { viewportMin_ = min; viewportMax_ = max; }

		/*============================================================================================
		* \brief		Marks animation controllers as dirty so they are restored on the next frame.
		*               Call this after spawning entities with AnimationControllerComponent,
		*               after hot reload, or any time animation state needs to be re-initialized.
		*//*========================================================================================*/
		void markAnimationsDirty() { graphicsMng_.markAnimationsDirty(); }

		/*============================================================================================
		* \brief		Gets whether the game is in playing mode
		* \return		True if game is playing, false if in editor mode
		*//*========================================================================================*/
		bool getIsPlaying() const { return isPlaying_; }

		/*============================================================================================
		* \brief		Sets the runtime pause state (pauses physics/AI/collision but keeps audio)
		* \param[in]	paused - True to pause game systems, false to resume
		*//*========================================================================================*/
		void setPaused(bool paused) { isPaused_ = paused; }

		/*============================================================================================
		* \brief		Gets whether the game is runtime-paused (dialogue, menus, etc.)
		* \return		True if game systems are paused, false otherwise
		*//*========================================================================================*/
		bool getIsPaused() const { return isPaused_; }

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
		Memory::EnoECSArena ecsArena_{ 8ull * 1024 * 1024 };
		ECS::EntityManager entityMng_{ ecsArena_, bus_ };
		ECSComponents::EnoPrefabManager prefabMng_{ bus_ };
		Core::EnoCoreHost coreHost_{ bus_ };
		std::unique_ptr<Map::TileMapSystem> tilemapSystem_;
		std::unique_ptr<AI::PathfindingSystem> pathfinder_;
		std::unique_ptr<Editor::EnoEditor> editor_;
		std::unique_ptr<AI::AIManager> aiMng_;
		Animation::AnimationControllerRegistry animRegistry;

		// Systems
		Input::EnoInputManager inputMng_{ bus_ };
		Graphics::EnoGraphicsManager graphicsMng_{ bus_ };
		Audio::EnoAudioManager audioMng_{ bus_ };
		std::unique_ptr<FSM::StateMachineSystem> stateMachineSys_;
		std::unique_ptr<Physics::PhysicsSystem> physicsSys_;
		std::unique_ptr<Collision::CollisionSystem> collisionSys_;
		std::unique_ptr<Particles::ParticleSystem> particleSys_;

		// UI Systems
		UI::ButtonSystem buttonSys_;

		// Game state
		bool isPlaying_ = false;  // Tracks if game is in playing mode (vs editor mode)
		bool isPaused_  = false;  // Runtime pause (dialogue, menus) - pauses physics/AI/collision
		Math::eVec2<float> viewportMin_{ 0.f, 0.f };  // Screen-space game viewport bounds (set by editor/app)
		Math::eVec2<float> viewportMax_{ 0.f, 0.f };  // Zero max = fall back to full window size

		// Time
		double dt_ = 0.0;
		double fps_ = 0.0;
	};
}
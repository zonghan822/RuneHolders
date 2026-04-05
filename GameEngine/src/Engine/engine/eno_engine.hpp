/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_engine.hpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   58%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   24%
* @co-author    Koe Yun Ji (yunji.koe@digipen.edu)   13%
* @date         03-10-2025
* @brief        Main engine class that orchestrates all engine systems and provides the core game loop
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
#include <string>
#include <unordered_map>

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
		* \brief		Initializes all engine systems and sets up the application window.
		*               Equivalent to calling setupEarly() then setupLate().
		*               Use for standalone game builds where no project picker is needed.
		*//*========================================================================================*/
		void setup();

		/*============================================================================================
		* \brief		Phase 1 of initialization: config, window, all engine systems, and shaders.
		*               Safe to call before a project is selected. After this returns the window is
		*               open and ImGui can be rendered (e.g. for a project picker).
		*//*========================================================================================*/
		void setupEarly();

		/*============================================================================================
		* \brief		Phase 2 of initialization: editor/scene loading, sounds, images, fonts, and
		*               event subscriptions. Must be called after a project has been selected so
		*               FileSystem::GetAssetsDir() points to the correct project.
		*//*========================================================================================*/
		void setupLate();

		/*============================================================================================
		* \brief		Returns true once setupLate() has been called (editor_ is initialized).
		*//*========================================================================================*/
		bool isEditorReady() const { return editor_ != nullptr; }

		/*============================================================================================
		* \brief		Starts a fully self-contained game loop (no C++ callback needed).
		*               Use this when all game logic lives in registered C# Logic Systems.
		*//*========================================================================================*/
		void run();

		/*============================================================================================
		* \brief		Returns true when the window has been requested to close.
		*               Use in a custom game loop together with beginFrame() / endFrame().
		*//*========================================================================================*/
		bool shouldClose() const { return window_.shouldClose() || closeRequested_; }
		void requestClose() { closeRequested_ = true; }

		// Returns true once initScripting() or setupLate() has created the script host.
		bool isScriptingReady() const { return coreHost_ != nullptr; }

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
		Core::EnoCoreHost& getCoreHost() { return *coreHost_; }

		/*============================================================================================
		* \brief		Initializes (or re-initializes) the scripting system from the specified DLL
		*               directory. If dllDir is empty, uses FileSystem::GetExecutableDir().
		*               For the Editor, call this with the selected project's exe directory
		*               (e.g. GetExecutableDir().parent_path() / projectName) BEFORE setupLate().
		*               setupLate() will skip re-creating the host if this has already been called.
		*//*========================================================================================*/
		void initScripting(const std::filesystem::path& dllDir = {});

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
		* \brief		Gets the time elapsed since the last frame, scaled by timeScale_.
		*               Physics, AI, and all C++ systems receive this value so the entire
		*               simulation slows down uniformly when timeScale_ < 1.
		* \return		Scaled delta time in seconds (dt_ * timeScale_)
		*//*========================================================================================*/
		double getDeltaTime() const { return dt_ * timeScale_; }

		/*============================================================================================
		* \brief		Gets the real (wall-clock) delta time, unaffected by timeScale_.
		*               Used by UI animations and timers that must run at real-world speed.
		*//*========================================================================================*/
		double getUnscaledDeltaTime() const { return dt_; }

		/*============================================================================================
		* \brief		Sets the global time scale applied to GetDeltaTime() in the C# bridge.
		*               1.0 = normal speed; <1.0 = slow motion; 0.0 = frozen scripts.
		*               Does NOT affect C++-side physics/AI dt.
		*//*========================================================================================*/
		void  setTimeScale(float s) { timeScale_ = s < 0.0f ? 0.0f : s; }
		float getTimeScale() const  { return timeScale_; }

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
		* \brief		Re-initializes the C# bridge after a hot-reload. Call this immediately after
		*               EnoCoreHost::hotReload() so Bridge.GetDeltaTime() etc. work in scripts.
		*//*========================================================================================*/
		void reinitBridge();

		/*============================================================================================
		* \brief		Sets the active project/app name so Bridge_Init can initialize the DLL-side
		*               FileSystem to the correct project. Must be called before setup() or setupLate().
		*//*========================================================================================*/
		void setAppName(const std::string& name) { appName_ = name; }
		const std::string& getAppName() const { return appName_; }

		/*============================================================================================
		* \brief		Sets the screen-space bounds of the rendered game viewport (imageMin/Max for
		*               screen-to-world conversion). In full-screen mode this is (0,0)-(w,h); in
		*               editor mode it is the ImGui game-panel bounds.
		*//*========================================================================================*/
		void setViewportBounds(Math::eVec2<float> min, Math::eVec2<float> max) { viewportMin_ = min; viewportMax_ = max; }
		Math::eVec2<float> getViewportMin() const { return viewportMin_; }
		Math::eVec2<float> getViewportMax() const { return viewportMax_; }
		std::pair<int,int>  getCachedWindowSize() const { return { cachedWinW_, cachedWinH_ }; }

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

		/*============================================================================================
		* \brief		Queues a scene change to be executed at the end of the current frame,
		*               after all scripts have finished updating. Safe to call from script Update().
		* \param[in]	name - Scene filename (e.g. "Level.txt")
		*//*========================================================================================*/
		void requestSceneChange(const std::string& name) { pendingScene_ = name; }
		void requestStop() { pendingStop_ = true; }

		/*============================================================================================
		* \brief		Gets the per-subsystem timing data collected during the last frame.
		*               Keys are system names (e.g. "Scripts", "Physics"), values are microseconds.
		*//*========================================================================================*/
		const std::unordered_map<std::string, double>& getTimings() const { return timings_; }

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
		ECS::EntityManager entityMng_{ loader_, ecsArena_, bus_ };
		ECSComponents::EnoPrefabManager prefabMng_{ bus_ };
		std::unique_ptr<Core::EnoCoreHost> coreHost_;
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

		// Project identity
		std::string appName_;  // Set by setAppName() before setup(); used by Bridge_Init for DLL FileSystem

		// Game state
		bool isPlaying_       = false;  // Tracks if game is in playing mode (vs editor mode)
		bool isPaused_        = false;  // Editor-driven pause — blocks everything including canvas/buttons
		bool focusLostPaused_ = false;  // True when paused due to window focus loss
		bool wasFullscreen_   = false;  // Fullscreen state before focus-loss minimize
		bool hasBegun_        = false;  // Guards the one-shot Begin() call at the start of each play session
		bool closeRequested_  = false;  // Set by requestClose(); checked by shouldClose() to avoid GLFW DLL boundary issues
		bool prevWasPlaying_  = false;  // Previous frame's isPlaying_ — used to detect the play→stop transition for video cleanup
		std::string pendingScene_;  // Non-empty when a script has requested a scene change
		bool pendingStop_ = false;  // True when a script requested stop (editor play mode quit)
		Math::eVec2<float> viewportMin_{ 0.f, 0.f };  // Screen-space game viewport bounds (set by editor/app)
		Math::eVec2<float> viewportMax_{ 0.f, 0.f };  // Zero max = fall back to full window size
		int cachedWinW_ = 0;  // Window pixel width  — cached at setupEarly so the bridge DLL can read it
		int cachedWinH_ = 0;  // Window pixel height — without calling GLFW (which is EXE-only in static builds)

		// Time
		double dt_ = 0.0;
		double fps_ = 0.0;
		float  timeScale_ = 1.0f;  // Applied by the C# bridge GetDeltaTime(); C++ systems use raw dt_

		// Per-subsystem timing (microseconds), populated each frame
		std::unordered_map<std::string, double> timings_;
	};
}
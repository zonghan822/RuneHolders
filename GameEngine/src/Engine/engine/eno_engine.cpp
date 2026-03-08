/*============================================================================================
 * @file         eno_engine.cpp
 * @author       Lee Zhe An (zhean.l@digipen.edu),
 * @author		Law Zong Han (zonghan.law@digipen.edu)
 * @date         03-10-2025
 * @brief		Implementation of the core engine systems and main game loop
 *
 *				This file contains the implementation of the EnoEngine class which serves as
 *				the central coordinator for all engine subsystems. It manages the initialization,
 *				main game loop execution, resource loading, time management, and shutdown procedures.
 *				The engine follows a component-based architecture where each system (graphics, audio,
 *				input, etc.) is managed independently but coordinated through the main engine class.
 */
/*========================================================================================*/

#include "eno_engine.hpp"

namespace Engine
{
	void EnoEngine::reinitBridge()
	{
		Bridge::Bridge_Init(this);
	}

	void EnoEngine::setup()
	{
		setupEarly();
		setupLate();
	}

	void EnoEngine::setupEarly()
	{
		configMng_.retrieveSettings();
		auto [w, h] = configMng_.getWindowDim();
		bool fullscreen = configMng_.getFullscreen();
		bool vsync = configMng_.getVSync();

		window_.initialize(configMng_.getWindowTitle(), w, h, fullscreen, vsync);

		auto [tmW, tmH, tmCell] = configMng_.getTilemapConfig();
		tilemapSystem_ = std::make_unique<Map::TileMapSystem>(bus_, tmW, tmH, tmCell);
		pathfinder_ = std::make_unique<AI::PathfindingSystem>(bus_, *tilemapSystem_);
		aiMng_ = std::make_unique<AI::AIManager>(bus_, *pathfinder_, entityMng_);

		// Systems init
		inputMng_.initialize();
		stateMachineSys_ = std::make_unique<FSM::StateMachineSystem>(bus_);
		physicsSys_ = std::make_unique<Physics::PhysicsSystem>(bus_);
		collisionSys_ = std::make_unique<Collision::CollisionSystem>(bus_);
		particleSys_ = std::make_unique<Particles::ParticleSystem>(bus_);
		audioMng_.init();
		graphicsMng_.initialize(window_);

		// Shaders are project-agnostic enough to load immediately
		loader_.loadShaders(loader_.getShaderPath());

		// Give the entity manager a reference to the loader so loadJson can
		// resolve textures by name instead of reconstructing file paths.
		entityMng_.setLoader(loader_);
	}

	void EnoEngine::initScripting(const std::filesystem::path& dllDir)
	{
		auto dir = dllDir.empty() ? FileSystem::GetExecutableDir() : dllDir;
		auto dll = dir / "GameScripts.dll";
		std::cout << "[initScripting] DLL path: " << dll << " - "
		          << (std::filesystem::exists(dll) ? "EXISTS" : "NOT FOUND") << "\n";
		coreHost_ = std::make_unique<Core::EnoCoreHost>(bus_, dir);
	}

	void EnoEngine::setupLate()
	{
		// Load project-specific assets BEFORE the editor so that textures are
		// already in the loader when the scene loads and loadJson resolves them.
		loader_.loadSound(audioMng_.getSystem(), loader_.getSoundExtensions(), loader_.getMusicPath());
		loader_.loadSound(audioMng_.getSystem(), loader_.getSoundExtensions(), loader_.getSFXPath());
		loader_.loadImages(loader_.getImagePath());
		loader_.loadFonts(loader_.getFontPath(), 48);

		// Initialize scripting if not already done by initScripting() (e.g. Editor project picker)
		if (!coreHost_)
			coreHost_ = std::make_unique<Core::EnoCoreHost>(bus_);

		// Initialize editor (loads saves/scenes from the current project)
		editor_ = std::make_unique<Editor::EnoEditor>(bus_, entityMng_, *coreHost_, *tilemapSystem_);

		// Bridge must be initialized before Begin() so that Bridge.* calls inside
		// script Begin() methods resolve correctly.
		Bridge::Bridge_Init(this);

		for (const auto &node : editor_->getSceneTree().getAllNodes())
		{
			if (node->script_)
			{
				for (const auto &org : node->script_->origin())
				{
					coreHost_->begin(org, node->sceneObjectID_);
				}
			}
		}

		// Mark animations dirty whenever a scene is cleared/loaded
		bus_.subscribe<Editor::SceneCleared>([this](const Editor::SceneCleared&)
		{
			graphicsMng_.markAnimationsDirty();
		});

		// Handle audio autoplay when scene enters play mode
		bus_.subscribe<Editor::PlayScene>([this](const Editor::PlayScene&)
		{
			for (const auto& node : editor_->getSceneTree().getAllNodes())
			{
				if (!node->enabled_) continue;
				if (!node->entity_.has<ECSComponents::AudioComponent>(entityMng_)) continue;
				auto& audioComp = node->entity_.get<ECSComponents::AudioComponent>(entityMng_);
				if (audioComp.audioMod_.name_[0] != '\0')
				{
					bus_.publish(Audio::RequestSound{ std::string(audioComp.audioMod_.name_) });
					audioMng_.prepareMod(audioComp);
				}
				if (audioComp.isAutoplay_)
					audioMng_.play(audioComp);
			}
		});
	}

	void EnoEngine::run()
	{
		while (!window_.shouldClose())
		{
			// Script watch (beginFrame also polls events / updates time)
			bool modified = coreHost_->areScriptsModified();

			if (modified)
			{
				std::cout << "------------- Script has been Modified -------------" << std::endl;

				// Re-register all scripts
				for (size_t i = 0; i < editor_->getSceneTree().getAllNodes().size(); i++)
				{
					if (editor_->getSceneTree().getAllNodes()[i]->script_)
					{
						editor_->getSceneTree().getAllNodes()[i]->script_->registerInst(
							editor_->getSceneTree().getAllNodes()[i]->sceneObjectID_);
					}
				}
			}
			beginFrame();
			renderFrame();
			endFrame();
		}
	}

	void EnoEngine::beginFrame()
	{
		updateTime(1.0);
		window_.pollEvents();

		float deltaTime = static_cast<float>(dt_);

		if (isPlaying_ && !isPaused_)
		{
			for (const auto &node : editor_->getSceneTree().getAllNodes())
			{
				if (node->script_)
				{
					for (const auto &org : node->script_->origin())
					{
						coreHost_->update(org, node->sceneObjectID_);
					}
				}
			}
		}

		if (isPlaying_ && !isPaused_) {
			stateMachineSys_->update(editor_->getSceneTree(), entityMng_);
			physicsSys_->Update(entityMng_, deltaTime);
			collisionSys_->Update(entityMng_);
			particleSys_->Update(entityMng_, deltaTime);
			buttonSys_.update(entityMng_, inputMng_, viewportMin_, viewportMax_, window_.getWindowSize());
		}

		// Update audio system (continues during runtime pause)
		if (isPlaying_)
			audioMng_.process(deltaTime);

		// Sync isPlaying_ flag with actual FMOD channel state so the inspector
		// indicator clears automatically when a sound finishes playing naturally.
		for (auto& ent : entityMng_.findEntity<ECSComponents::AudioComponent>())
		{
			auto& ac = ent.get<ECSComponents::AudioComponent>(entityMng_);
			if (ac.isPlaying_ && ac.audioMod_.ch_)
			{
				bool fmodIsPlaying = false;
				ac.audioMod_.ch_->isPlaying(&fmodIsPlaying);
				if (!fmodIsPlaying)
					ac.isPlaying_ = false;
			}
		}
	}

	void EnoEngine::renderFrame()
	{
		// Restore and tick animation controllers
		graphicsMng_.updateAnimations(*this);

		// Update camera projection matrix before rendering
		graphicsMng_.updateCamera(*this);

		// Clear screen and render
		graphicsMng_.beginFrame(Graphics::Color::Black);
		graphicsMng_.render(*this);
	}

	void EnoEngine::endFrame()
	{
		// Present frame and reset input state
		graphicsMng_.endFrame(window_);
		inputMng_.endFrame();
	}

	void EnoEngine::shutDown()
	{
		// Clean up graphics system resources
		graphicsMng_.shutdown();
	}

	// Loader
	void EnoEngine::setupResourceLoading()
	{
		// Loading assets from directories
		loader_.loadSound(audioMng_.getSystem(), loader_.getSoundExtensions(), loader_.getMusicPath());
		loader_.loadSound(audioMng_.getSystem(), loader_.getSoundExtensions(), loader_.getSFXPath());

		// Loading images
		loader_.loadImages(loader_.getImagePath());

		// Loading fonts
		loader_.loadFonts(loader_.getFontPath(), 48);

		// Note: shaders are loaded in setupEarly() and are not reloaded here.
	}

	void EnoEngine::updateTime(double fps_calc_interval)
	{
		// get elapsed time (in seconds) between previous and current frames
		static double prev_time = glfwGetTime();
		double curr_time = glfwGetTime();
		dt_ = curr_time - prev_time;
		prev_time = curr_time;

		// fps calculations
		static double count = 0.0; // number of game loop iterations
		static double start_time = glfwGetTime();
		// get elapsed time since very beginning (in seconds) ...
		double elapsed_time = curr_time - start_time;

		++count;

		// update fps at least every 10 seconds ...
		fps_calc_interval = (fps_calc_interval < 0.0) ? 0.0 : fps_calc_interval;
		fps_calc_interval = (fps_calc_interval > 10.0) ? 10.0 : fps_calc_interval;
		if (elapsed_time > fps_calc_interval)
		{
			fps_ = count / elapsed_time;
			start_time = curr_time;
			count = 0.0;
		}
	}
}
/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_engine.cpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   58%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   25%
* @co-author    Koe Yun Ji (yunji.koe@digipen.edu)   14%
* @date         03-10-2025
* @brief        Implementation of the core engine systems and main game loop
*//*========================================================================================*/


/*========================================================================================*/

#include "eno_engine.hpp"
#include "../graphics/video/video_player.hpp"
#include "../ui/canvas_system.hpp"

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

		// Cache window dimensions so the bridge DLL can read them without calling GLFW
		// (GLFW is statically linked to the EXE only; DLL-side GLFW calls return 0).
		auto [fw, fh] = window_.getFramebufferSize();
		cachedWinW_ = (fw > 0) ? fw : w;
		cachedWinH_ = (fh > 0) ? fh : h;

		auto [tmW, tmH, tmCell] = configMng_.getTilemapConfig();
		tilemapSystem_ = std::make_unique<Map::TileMapSystem>(bus_, tmW, tmH, tmCell);
		pathfinder_ = std::make_unique<AI::PathfindingSystem>(bus_, *tilemapSystem_);
		aiMng_ = std::make_unique<AI::AIManager>(bus_, *pathfinder_, entityMng_);

		// Systems init
		inputMng_.initialize();
		stateMachineSys_ = std::make_unique<FSM::StateMachineSystem>(bus_);
		physicsSys_ = std::make_unique<Physics::PhysicsSystem>(bus_);
		collisionSys_ = std::make_unique<Collision::CollisionSystem>(bus_);
		//particleSys_ = std::make_unique<Particles::ParticleSystem>(bus_);
		audioMng_.init();
		graphicsMng_.initialize(window_);

		// Shaders are project-agnostic enough to load immediately
		loader_.loadShaders(loader_.getShaderPath());
	}

	void EnoEngine::initScripting(const std::filesystem::path& dllDir)
	{
		auto dir = dllDir.empty() ? FileSystem::GetExecutableDir() : dllDir;
		auto dll = dir / "GameScripts.dll";
		coreHost_ = std::make_unique<Core::EnoCoreHost>(bus_, dir);
	}

	void EnoEngine::setupLate()
	{
		// Load project-specific assets BEFORE the editor so that textures are
		// already in the loader when the scene loads and loadJson resolves them.
		setupResourceLoading();

		// Initialize scripting if not already done by initScripting() (e.g. Editor project picker)
		if (!coreHost_)
			coreHost_ = std::make_unique<Core::EnoCoreHost>(bus_);

		// Initialize editor (loads saves/scenes from the current project)
		editor_ = std::make_unique<Editor::EnoEditor>(bus_, entityMng_, *coreHost_, *tilemapSystem_, prefabMng_, graphicsMng_);

		// Bridge must be initialized before Begin() so that Bridge.* calls inside
		// script Begin() methods resolve correctly.
		Bridge::Bridge_Init(this);

		// Build tile colliders for the initial scene
		tilemapSystem_->buildTileColliders(entityMng_);
		collisionSys_->RebuildStaticEntities(entityMng_);

		// Rebuild tile colliders whenever a scene is loaded
		bus_.subscribe<Editor::SceneCleared>([this](const Editor::SceneCleared&)
		{
			tilemapSystem_->buildTileColliders(entityMng_);
			collisionSys_->RebuildStaticEntities(entityMng_);
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

		// Stop all orphaned FMOD channels when the scene stops so they don't
		// carry over into the next play session and cause audio doubling.
		bus_.subscribe<Editor::StopScene>([this](const Editor::StopScene&)
		{
			audioMng_.stopAllImmediate();
			// Reset the isPlaying_ flag on every audio component so the guard
			// in play() doesn't block playback on the next play press.
			for (auto& ent : entityMng_.findEntity<ECSComponents::AudioComponent>())
			{
				auto& ac = ent.get<ECSComponents::AudioComponent>(entityMng_);
				ac.isPlaying_ = false;
				ac.audioMod_.ch_ = nullptr;
			}
		});
	}

	void EnoEngine::run()
	{
		// Standalone game is always in play mode
		setIsPlaying(true);
		bus_.publish(Editor::PlayScene{});  // prepare audio components (load sounds into FMOD)

		while (!shouldClose())
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
		using Clock = std::chrono::high_resolution_clock;

		updateTime(1.0);
		window_.pollEvents();

		float deltaTime = static_cast<float>(getDeltaTime());  // scaled by timeScale_

		// Call Begin() once on the first frame after play starts; reset when play stops
		if (isPlaying_ && !hasBegun_)
		{
			for (const auto& node : editor_->getSceneTree().getAllNodes())
			{
				if (node->script_)
				{
					for (const auto& org : node->script_->origin())
					{
						coreHost_->begin(org, node->sceneObjectID_);
					}
				}
			}
			hasBegun_ = true;
		}
		else if (!isPlaying_)
		{
			hasBegun_ = false;
		}

		if (isPlaying_ && !isPaused_)
		{
			auto t0 = Clock::now();
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
			timings_["Scripts"] = std::chrono::duration<double, std::micro>(Clock::now() - t0).count();
		}
		else
		{
			timings_["Scripts"] = 0.0;
		}

		// Flush nodes queued for deletion by scripts this frame (FreeNode deferred to avoid
		// invalidating the getAllNodes() iterator during the scripts update loop above).
		editor_->getSceneTree().flushKill();

		// Game logic — blocked by editor pause only; in-game pause is handled in C# scripts
		if (isPlaying_ && !isPaused_) {
			auto t0 = Clock::now();
			stateMachineSys_->update(editor_->getSceneTree(), entityMng_);
			timings_["State Machine"] = std::chrono::duration<double, std::micro>(Clock::now() - t0).count();

			t0 = Clock::now();
			physicsSys_->Update(entityMng_, deltaTime);
			timings_["Physics"] = std::chrono::duration<double, std::micro>(Clock::now() - t0).count();

			t0 = Clock::now();
			collisionSys_->Update(entityMng_);
			timings_["Collision"] = std::chrono::duration<double, std::micro>(Clock::now() - t0).count();

			//t0 = Clock::now();
			//particleSys_->Update(entityMng_, deltaTime);
			//timings_["Particles"] = std::chrono::duration<double, std::micro>(Clock::now() - t0).count();
		}
		else
		{
			timings_["State Machine"] = 0.0;
			timings_["Physics"]       = 0.0;
			timings_["Collision"]     = 0.0;
			//timings_["Particles"]     = 0.0;
		}

		// Canvas and buttons: blocked by editor pause, but NOT by in-game pause (UI stays interactive)
		if (isPlaying_ && !isPaused_)
		{
			auto t0 = Clock::now();
			UI::CanvasSystem::update(*this);
			timings_["UI Canvas"] = std::chrono::duration<double, std::micro>(Clock::now() - t0).count();

			t0 = Clock::now();
			buttonSys_.update(entityMng_, inputMng_, viewportMin_, viewportMax_, window_.getWindowSize(), editor_->getSceneTree());
			timings_["UI Buttons"] = std::chrono::duration<double, std::micro>(Clock::now() - t0).count();
		}
		else
		{
			timings_["UI Canvas"]  = 0.0;
			timings_["UI Buttons"] = 0.0;
		}

		// Dynamic spatial audio: update volume/pan for entities with spatialAudio_ enabled
		if (isPlaying_ && !isPaused_)
		{
			// Find the active camera's world position
			Math::eVec2<float> camPos{ 0.0f, 0.0f };
			for (auto& camEnt : entityMng_.findEntity<ECSComponents::CameraComponent>())
			{
				auto& cam = camEnt.get<ECSComponents::CameraComponent>(entityMng_);
				if (cam.isActive)
				{
					camPos = cam.position;
					break;
				}
			}

			// Update each spatial audio entity
			for (auto& ent : entityMng_.findEntity<ECSComponents::AudioComponent>())
			{
				auto& ac = ent.get<ECSComponents::AudioComponent>(entityMng_);
				if (!ac.spatialAudio_ || !ac.isPlaying_) continue;
				if (!ent.has<ECSComponents::TransformComponent>(entityMng_)) continue;
				auto& tc = ent.get<ECSComponents::TransformComponent>(entityMng_);
				audioMng_.updateSpatialAudio(ac, tc.pos, camPos);
			}
		}

		// Update audio system (continues during runtime pause)
		auto t0 = Clock::now();
		if (isPlaying_)
			audioMng_.process(deltaTime);
		timings_["Audio"] = std::chrono::duration<double, std::micro>(Clock::now() - t0).count();
		

		// Advance all video players (always, so the asset browser preview works in editor)
		graphicsMng_.updateVideos(deltaTime);

		// --- Video entity playback: driven by game state ---
		// Entity videos (those with a VideoComponent) follow play/pause/stop mode.
		// Browser-preview videos are unaffected because they are not entity-owned.
		{
			for (auto& ent : entityMng_.findEntity<ECSComponents::VideoComponent>())
			{
				auto& vc = ent.get<ECSComponents::VideoComponent>(entityMng_);
				if (strlen(vc.videoName) == 0) continue;
				auto player = graphicsMng_.getVideo(std::string(vc.videoName));
				if (!player || !player->isLoaded()) continue;

				if (!isPlaying_)
				{
					// Stopped: reset to initial serialised state (IDLE)
					if (player->getState() != Graphics::VideoState::IDLE)
						player->stop();
					vc.pausedByEngine_ = false;
				}
				else if (isPaused_)
				{
					// Game paused: pause any playing entity video and remember it
					if (player->getState() == Graphics::VideoState::PLAYING)
					{
						player->pause();
						vc.pausedByEngine_ = true;
					}
				}
				else
				{
					// Playing: resume engine-paused videos; auto-start on first frame
					if (vc.pausedByEngine_ && player->getState() == Graphics::VideoState::PAUSED)
					{
						player->play();
						vc.pausedByEngine_ = false;
					}
					else if (vc.autoPlay && player->getState() == Graphics::VideoState::IDLE)
					{
						player->setLooping(vc.loop);
						player->play();
					}
				}
			}
		}
		
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

		// Flush any scene change requested by a script this frame.
		// Done after all script/physics updates so no mid-loop iterator invalidation occurs.
		if (!pendingScene_.empty())
		{
			editor_->selectScene(pendingScene_);
			pendingScene_.clear();
			hasBegun_ = false;  // Force Begin() to run for the new scene's scripts
		}
	}

	void EnoEngine::renderFrame()
	{
		using Clock = std::chrono::high_resolution_clock;

		// Restore and tick animation controllers
		auto t0 = Clock::now();
		graphicsMng_.updateAnimations(*this);
		timings_["Animations"] = std::chrono::duration<double, std::micro>(Clock::now() - t0).count();

		// Update camera projection matrix before rendering
		graphicsMng_.updateCamera(*this);

		// Clear screen and render
		t0 = Clock::now();
		graphicsMng_.beginFrame(Graphics::Color::Black);
		graphicsMng_.render(*this);
		timings_["Draw"] = std::chrono::duration<double, std::micro>(Clock::now() - t0).count();
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

		// Load Presets
		prefabMng_.registerPresets();
		ECSComponents::Preset::saveAll(prefabMng_);
		ECSComponents::Preset::loadAll(prefabMng_);
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
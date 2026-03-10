/*============================================================================================
* All content ? 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         game_logic.cpp
* @author       Yun Ji Koe (yunji.koe@digipen.edu)
* @author		Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio@digipen.edu)
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)
* @date         05-11-2025
* @brief        Game application layer that handles game-specific logic, entity management,
*               rendering, and ImGui integration. Systems are now managed by EnoEngine.
*//*========================================================================================*/

#include "editor_logic.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include "imGui.hpp"
#include "math/math.hpp"
//#include "particle/particle_presets.hpp"
//#include "particle/particle_component.hpp"
#include <typeindex>

void EditorApplication::initialize() {
	// Phase 1: window + systems + shaders — enough to render ImGui.
	engine_.setupEarly();
	setupImGui();

	// Project picker: let the user choose a project before any project-specific
	// assets (saves, art, fonts, sounds) are loaded.
	while (!engine_.shouldClose() && isProjectPickerOpen())
	{
		engine_.getWindowManager().pollEvents();
		engine_.getGraphicsManager().beginFrame(Graphics::Color::Black);
		imGuiProjectPicker(engine_);
		engine_.getGraphicsManager().endFrame(engine_.getWindowManager());
		engine_.getInputManager().endFrame();
	}

	if (engine_.shouldClose()) return;

	// Phase 2: editor/scene loading + project-specific assets.
	engine_.setupLate();

	setupGameEvents();

	auto& tileMap = engine_.getTileMap();

	// Set up tile map editor
	mapEditor_ = std::make_unique<MapEditor>(tileMap, engine_.getEntityManager());
	mapEditor_->setHasTilemap(engine_.getEnoEditor().getSceneTree().hasTilemap());
	Math::eVec2<float> worldSize = tileMap.getWorldSize();

	stateMachineEditor_ = std::make_unique<Editor::StateMachineEditor>(engine_.getEnoEditor());

	worldWidth_ = worldSize.x;
	worldHeight_ = worldSize.y;

	editorFramebuffer_ = std::make_unique<Graphics::EnoFramebuffer>();
	editorFramebuffer_->create(1, 1); // Resized to panel size on first frame

	editorCameraX_ = 0.f;
	editorCameraY_ = 0.f;

	// Subscribe to delete selected entity event (using EditorApplication's scene tree)
	engine_.getEventBus().subscribe<Editor::DeleteSelectedEntity>([this](const Editor::DeleteSelectedEntity&)
		{
			extern void deleteSelectedEntity(Editor::EnoSceneTree&, ECS::EntityManager&, Editor::UndoManager&, Graphics::EnoGraphicsManager&);
			deleteSelectedEntity(engine_.getEnoEditor().getSceneTree(), engine_.getEntityManager(), engine_.getEnoEditor().getUndoManager(), engine_.getGraphicsManager());
		});

	// Subscribe to scene cleared event to reset UI state
	engine_.getEventBus().subscribe<Editor::SceneCleared>([this](const Editor::SceneCleared&)
		{
			extern void clearSelectedEntity();
			clearSelectedEntity();
			engine_.markAnimationsDirty();
			// The old tile collider entities are gone after scene reload — clear cache and rebuild
			if (mapEditor_)
			{
				mapEditor_->clearColliderCache();
				mapEditor_->rebuildAllTileColliders();
			}
			std::cout << "[EditorApplication] Scene cleared, reset selected entity" << std::endl;
		});

	std::cout << "Game initialized successfully!" << std::endl;
}

void EditorApplication::run() {
	std::cout << "Starting game..." << std::endl;

	while (!engine_.shouldClose())
	{
		SystemTimer totalFrameTimer("totalTimePerFrame", systemTimes_);

		// 1. Set playing state and run editor state BEFORE game logic so that
		//    isPlaying_ and hot-reload/begin() are correct when beginFrame() runs.
		bool isPlayMode = (engine_.getEnoEditor().getCurrentScene().getState() == Editor::State::Playing ||
		                   engine_.getEnoEditor().getCurrentScene().getState() == Editor::State::NextFrame);
		engine_.setIsPlaying(isPlayMode);
		updateEditorState();

		// 2. Resize editor framebuffer if zoom or camera viewport changed.
		if (editorFramebuffer_) {
			int fbW, fbH;
			if (isPlayMode) {
				// In play mode use the camera's own viewport — unaffected by editor zoom.
				fbW = fbH = 0;
				auto& entityMng = engine_.getEntityManager();
				for (auto& camEnt : entityMng.findEntity<ECSComponents::CameraComponent>()) {
					auto& c = camEnt.get<ECSComponents::CameraComponent>(entityMng);
					if (!c.isActive) continue;
					fbW = static_cast<int>(c.viewportWidth);
					fbH = static_cast<int>(c.viewportHeight);
					break;
				}
				if (fbW <= 0) fbW = editorPanelWidth_;
				if (fbH <= 0) fbH = editorPanelHeight_;
			} else {
				// Edit mode: FBO = actual panel size so the image fills the view without letterboxing.
				// The camera viewport override (getEditorWidth/Height) divides panel size by zoom.
				fbW = getEditorPanelWidth();
				fbH = getEditorPanelHeight();
			}
			if (fbW != lastEditorFbWidth_ || fbH != lastEditorFbHeight_) {
				editorFramebuffer_->create(fbW, fbH);
				lastEditorFbWidth_ = fbW;
				lastEditorFbHeight_ = fbH;
			}
		}

		// 3. Run all game-logic systems (scripts, physics, collision, particles, audio).
		{
			SystemTimer t("Game Logic", systemTimes_);
			engine_.beginFrame();
		}
		// Merge per-subsystem timings from the engine
		for (auto& [k, v] : engine_.getTimings())
			systemTimes_[k] = v;

		// 4. Update editor camera AFTER physics/scripts have moved entities,
		//    so the camera write-back to CameraComponent is current before rendering.
		{
			SystemTimer t("Editor Camera", systemTimes_);
			updateEditorCamera();
		}

		// 5. Render the game scene into the editor framebuffer.
		//    In edit mode the CameraComponent belongs to the game scene and must not
		//    be moved by WASD. Temporarily override it with the editor viewport so
		//    renderFrame() draws from the editor's perspective, then restore it.
		bool isEditMode = (engine_.getEnoEditor().getCurrentScene().getState() == Editor::State::Stopped);

		Math::eVec2<float> savedCamPos{};
		float savedVpW = 0.f, savedVpH = 0.f;
		bool  camOverridden = false;

		if (isEditMode)
		{
			// Always push the editor viewport as a fallback so the graphics
			// system can render entities even when no CameraComponent node exists.
			engine_.getGraphicsManager().setFallbackCamera(
				editorCameraX_ + getEditorWidth()  * 0.5f,
				editorCameraY_ + getEditorHeight() * 0.5f,
				static_cast<float>(getEditorWidth()),
				static_cast<float>(getEditorHeight())
			);

			auto& entityMng = engine_.getEntityManager();
			for (auto& camEnt : entityMng.findEntity<ECSComponents::CameraComponent>())
			{
				auto& c = camEnt.get<ECSComponents::CameraComponent>(entityMng);
				if (!c.isActive) continue;
				savedCamPos = c.position;
				savedVpW    = c.viewportWidth;
				savedVpH    = c.viewportHeight;
				c.position.x    = editorCameraX_ + getEditorWidth()  * 0.5f;
				c.position.y    = editorCameraY_ + getEditorHeight() * 0.5f;
				c.viewportWidth  = static_cast<float>(getEditorWidth());
				c.viewportHeight = static_cast<float>(getEditorHeight());
				camOverridden = true;
				break;
			}
		}

		{
			SystemTimer t("Render Frame", systemTimes_);
			if (editorFramebuffer_) {
				// Tell the graphics manager the pixel size of the FBO so updateCamera()
				// sets glViewport correctly without knowing about the framebuffer.
				engine_.getGraphicsManager().setRenderTargetSize(lastEditorFbWidth_, lastEditorFbHeight_);
				editorFramebuffer_->bind();
			}
			engine_.renderFrame();
			if (editorFramebuffer_) {
				editorFramebuffer_->unbind();
				engine_.getGraphicsManager().setRenderTargetSize(0, 0); // revert to window size
				auto [w, h] = engine_.getWindowManager().getFramebufferSize();
				glViewport(0, 0, w, h);
				engine_.getGraphicsManager().setProjectionMatrix(w, h);
			}
		}
		// Merge render sub-system timings from the engine
		for (auto& [k, v] : engine_.getTimings())
			systemTimes_[k] = v;

		// Restore the CameraComponent to its original scene position.
		if (camOverridden)
		{
			auto& entityMng = engine_.getEntityManager();
			for (auto& camEnt : entityMng.findEntity<ECSComponents::CameraComponent>())
			{
				auto& c = camEnt.get<ECSComponents::CameraComponent>(entityMng);
				if (!c.isActive) continue;
				c.position      = savedCamPos;
				c.viewportWidth  = savedVpW;
				c.viewportHeight = savedVpH;
				break;
			}
		}

		// 6. Draw ImGui panels (game viewport, inspector, etc.), then present.
		updateEditorLogic();
		engine_.endFrame();
	}
}

void EditorApplication::shutdown() {
	std::cout << "Terminating game..." << std::endl;
	imGuiTerminate();
	engine_.shutDown();
}

void EditorApplication::setupGameEvents() {

	auto& inputMng = engine_.getInputManager();

	// Setup Delete key for entity deletion
	inputMng.setupKey("Delete Selected Entity", GLFW_KEY_DELETE, glfwGetKeyScancode(GLFW_KEY_DELETE), GLFW_RELEASE, 0);
	inputMng.setupEvent<Editor::DeleteSelectedEntity>("Delete Selected Entity");

	// Undo/Redo System
	// Ctrl+Z for undo
	inputMng.setupKey("Undo", GLFW_KEY_Z, glfwGetKeyScancode(GLFW_KEY_Z), GLFW_RELEASE, GLFW_MOD_CONTROL);
	inputMng.setupEvent<Editor::UndoAction>("Undo");

	// Ctrl+Shift+Z for redo
	inputMng.setupKey("Redo", GLFW_KEY_Z, glfwGetKeyScancode(GLFW_KEY_Z), GLFW_RELEASE, GLFW_MOD_CONTROL | GLFW_MOD_SHIFT);
	inputMng.setupEvent<Editor::RedoAction>("Redo");
}

void EditorApplication::setupImGui() {
	GLFWwindow* window = engine_.getWindowManager().getWindow();
	imGuiInit(window);
	extern EditorApplication* g_game;
	g_game = this;
}

void EditorApplication::updateEditorLogic() {
	// All game logic and camera updates have already run; just draw ImGui.
	updateImGui();
}

void EditorApplication::updateEditorCamera() {
	auto& tileMap = engine_.getTileMap();
	Math::eVec2<float> worldSize = tileMap.getWorldSize();
	worldWidth_ = worldSize.x;
	worldHeight_ = worldSize.y;

	float deltaTime = static_cast<float>(engine_.getDeltaTime());
	auto  state     = engine_.getEnoEditor().getCurrentScene().getState();

	if (state == Editor::State::Stopped)
	{
		// EDIT MODE ─────────────────────────────────────────────────────────────
		// The editor camera (editorCameraX_/Y_) is a free viewport independent of
		// the CameraComponent. WASD moves it; the CameraComponent stays at the
		// position the user placed it in the scene. The blue overlay in the editor
		// panel shows where the CameraComponent viewport sits in the world.
		// run() handles the temporary CameraComponent override around renderFrame().
		auto& inputManager = engine_.getInputManager();
		float cameraDelta  = editorCameraSpeed_ * deltaTime;
		if (!ImGui::GetIO().WantTextInput) {
			if (inputManager.isKeyPressed(GLFW_KEY_W)) editorCameraY_ -= cameraDelta;
			if (inputManager.isKeyPressed(GLFW_KEY_S)) editorCameraY_ += cameraDelta;
			if (inputManager.isKeyPressed(GLFW_KEY_A)) editorCameraX_ -= cameraDelta;
			if (inputManager.isKeyPressed(GLFW_KEY_D)) editorCameraX_ += cameraDelta;
		}
	}
	else
	{
		// PLAY MODE ─────────────────────────────────────────────────────────────
		// Trigger a room-transition pan with UP/DOWN keys.
		if (state == Editor::State::Playing)
		{
			auto& inputManager = engine_.getInputManager();
			if (inputManager.isKeyDown(GLFW_KEY_UP) && !isPanning_)
				startCameraPan(editorCameraY_ - static_cast<float>(editorPanelHeight_), 1.0f);
			if (inputManager.isKeyDown(GLFW_KEY_DOWN) && !isPanning_)
				startCameraPan(editorCameraY_ + static_cast<float>(editorPanelHeight_), 1.0f);
		}

		if (isPanning_)
		{
			// Animate the editor camera toward the target room.
			panTimer_ += deltaTime;
			float progress = Math::min(panTimer_ / panDuration_, 1.0f);
			float eased    = progress < 0.5f
				? 2.0f * progress * progress
				: 1.0f - Math::pow(-2.0f * progress + 2.0f, 2.0f) / 2.0f;
			editorCameraY_ = panStartY_ + (panTargetY_ - panStartY_) * eased;
			if (progress >= 1.0f) { isPanning_ = false; editorCameraY_ = panTargetY_; }

			// Push the panned position into the CameraComponent so the game
			// camera moves to the new room.  Use the camera's own viewport
			// dimensions to compute the center — do not overwrite them.
			auto& entityMng = engine_.getEntityManager();
			for (auto& camEnt : entityMng.findEntity<ECSComponents::CameraComponent>())
			{
				auto& c = camEnt.get<ECSComponents::CameraComponent>(entityMng);
				if (!c.isActive) continue;
				c.position.x = editorCameraX_ + c.viewportWidth  * 0.5f;
				c.position.y = editorCameraY_ + c.viewportHeight * 0.5f;
				break;
			}
		}
		else
		{
			// Not panning: mirror the game camera position into the editor viewport
			// so the editor panel shows exactly what the game camera sees.
			// Use the camera's own viewport dimensions — do not overwrite them.
			auto& entityMng = engine_.getEntityManager();
			for (auto& camEnt : entityMng.findEntity<ECSComponents::CameraComponent>())
			{
				auto& c = camEnt.get<ECSComponents::CameraComponent>(entityMng);
				if (!c.isActive) continue;
				editorCameraX_ = c.position.x - c.viewportWidth  * 0.5f;
				editorCameraY_ = c.position.y - c.viewportHeight * 0.5f;
				break;
			}
		}
	}
}


void EditorApplication::updateEditorState() {
	static bool keyFrameMode = false;
	static bool playAfterStop = true;
	auto& coreHost = engine_.getCoreHost();

	// Editor state = play game
	switch (engine_.getEnoEditor().getCurrentScene().getState())
	{
	case Editor::State::Playing:
		{
			// Check that the last frame editor state was 'Stopped'
			if (playAfterStop)
			{
				coreHost.hotReload();
				// Re-initialize the bridge after hot reload: the DLL is replaced so
				// gEngine becomes null; every Bridge.* call silently fails until
				// we point it at the engine again.
				engine_.reinitBridge();
				// Re-register all scripts after hot reload to create fresh instances
				// with the newly loaded assembly's types. Without this, the first play
				// would use stale instances from the previous assembly.
				engine_.getEnoEditor().getSceneTree().registerAllScripts();
				// Toggle flag to make it a one time event
				playAfterStop = false;
			}
		}
		case Editor::State::Paused:
		{
			// This toggles keyframe mode
			keyFrameMode = true;
			break;
		}
		case Editor::State::NextFrame:
		{
			// Check if key frame mode is true
			if (keyFrameMode)
			{
				// Reset variables - transition back to Paused for next frame
				// NOTE: Don't set state back to Paused here! It happens too early in the frame.
				// The state will be reset to Paused at the end of this function after rendering.
			}
			engine_.getEnoEditor().getCurrentScene().pause();
			break;
		}
		case Editor::State::Stopped:
		{
			// This condition ensures resetting of pos comes after play
			if (!playAfterStop)
			{
				// Mark entities as dirty to refresh the rendering cache
				markEntitiesDirty();
				// Toggle flag
				playAfterStop = true;
			}
			break;
		}
	}
}


void EditorApplication::updateImGui() {
	SystemTimer ImGuiTimer("ImGui", systemTimes_);
	imGuiDisplay(engine_, engine_.getEnoEditor().getSceneTree(), engine_.getEnoEditor().getUndoManager());
}


// Camera panning for room transitions
void EditorApplication::startCameraPan(float targetY, float duration)
{
	isPanning_ = true;
	panStartY_ = editorCameraY_;
	panTargetY_ = targetY;
	panDuration_ = duration;
	panTimer_ = 0.0f;
}
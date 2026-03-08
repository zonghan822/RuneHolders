/*============================================================================================
* All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         imGui.cpp
* @author       Yun Ji Koe (yunji.koe@digipen.edu)
* @author		Kimberly Charlotte Madlangbayan Ambrosio (k.ambrosio@digipen.edu)
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)
* @date         01-10-2025
* @brief        This file contains the functions that are required for Dear ImGui displays,
*				which includes the init, update and terminate functions.
*     			There are also functions for spawning and counting entities.
*//*========================================================================================*/
#include <algorithm>
#include <filesystem>
#include <random>
#include <unordered_map>
#include "imGui.hpp"


#undef min
#undef max

// Forward declaration for Editor namespace
namespace Editor {
	struct Node;
}

using Editor::Node;

#pragma region Game Variables

bool showColliders_ = true;

EditorApplication* g_game = nullptr;

// Game viewport tracking (for button click detection)
ImVec2 lastGameViewportPos(0, 0);
ImVec2 lastGameViewportSize(0, 0);

// Containers for time display
std::unordered_map<std::string, double> systemTimes_; //actual system time
std::unordered_map<std::string, double> throttledTimes_; //used for displaying throttled timing(more user friendly)

// Hierarchy tree removed - now passed as parameter


// Option Menu
const char* shapes[] = { "Circle", "Rectangle" };
const char* colors[] = { "Green"  ,     "Red" };
const char* colliderShapes[] = { "None", "Circle", "Rectangle" };

// Values to set for entities
static float           eWidth = 10;
static float           eHeight = 10;
static int             eColor = Color::Green;
static int             eShapeIndex = Shapes::Rect;
static int             selectedCollider = 0;         // for the newly spawned entity
static float           colliderWidth = 10.0f;
static float           colliderHeight = 10.0f;
static float           colliderRadius = 5.0f;

// Player Scale Variables
float pScaleX = 1.0f;
float pScaleY = 1.0f;

// Keeping track of animations
static Graphics::TextureData selectedTexData;

// Time
static float shown_fps = 0.0f;
static double next_update = 0.0;         // next time we refresh the text

// Entity counter
size_t entity_count{};
static uint32_t selectedEntityID = UINT32_MAX;  // Use max value to indicate "no selection" since 0 is valid

// Drag-Drop Gizmos
static bool isDragging = false;
static uint32_t draggedEntityID = 0;
static ImVec2 dragOffset;
static ImVec2 lastDragPos;

// Transform Gizmos
enum GizmoMode { GIZMO_TRANSLATE, GIZMO_ROTATE, GIZMO_SCALE };
static GizmoMode currentGizmoMode = GIZMO_TRANSLATE;
static bool isRotating = false;
static bool isScaling = false;
static int activeTransformAxis = 0; // 0=none, 1=X-axis, 2=Y-axis, 3=uniform scale, 4=X-scale, 5=Y-scale
static float initialAngle = 0.0f;
static float initialRotation = 0.0f;
static ImVec2 initialScale = ImVec2(1.0f, 1.0f);
static float initialDrawWidth = 0.0f; // For scaling draw info
static float initialDrawHeight = 0.0f; // For scaling draw info
static ImVec2 lastScale = ImVec2(1.0f, 1.0f);  // Track last frame scale for delta calculation
static ImVec2 scaleStartPos = ImVec2(0.0f, 0.0f);
static ImVec2 dragStartWorldPos = ImVec2(0.0f, 0.0f);

// Undo system - store transform state before gizmo operations
static Math::eVec2<float> transformBeforeDrag;
static Math::eVec2<float> rotationBeforeDrag;
static Math::eVec2<float> scaleBeforeDrag;
static std::vector<uint32_t> affectedEntityIDs;  // Track all entities affected by transformation (parent + children)
static std::unique_ptr<Editor::MultiTransformCommand> pendingTransformCommand;  // Command being built during transformation

// Prefab placement
static std::string g_selectedPrefabForPlacement = "";

// Node preset selection (for Add Root/Child Node)
static int  g_selectedNodePresetIdx  = 0;
static std::string g_selectedNodePreset = "DEFAULT";

// Entity deletion by click
static bool g_waitingForEntityDeletion = false;

// Deferred scene loading to prevent crashes during ImGui rendering
static bool g_pendingNewScene = false;
static bool g_pendingSceneLoad = false;
static std::string g_sceneToLoad = "";
static bool g_refreshSceneList = false;  // Flag to refresh scene list after operations
static bool g_resetEntityCounter = false;  // Flag to reset entity counter when scene changes

// Scene deletion error tracking
static bool g_cannotDeleteActiveScene = false;

// Audio browser tree
struct AudioEntry {
	std::string displayName;         // filename with extension, e.g. "laser.wav"
	std::string stemName;            // filename without extension, used for FMOD lookup
	std::filesystem::path buildPath; // path in the build/runtime assets folder
	std::filesystem::path rootPath;  // path in the source assets folder (for deletion)
};
struct AudioFolder {
	std::string folderName;
	std::vector<AudioFolder> subfolders;
	std::vector<AudioEntry> files;
};
static AudioFolder g_audioTree;

// Audio error tracking - using a queue to store all errors
struct AudioErrorInfo {
	std::string filename;
	std::string extension;
	std::string reason;
};
static std::vector<AudioErrorInfo> g_audioErrorQueue;

#pragma endregion

// ============================================================
// Project Picker — file-scope state and helpers
// ============================================================

static bool                  s_showProjectPicker  = true;
static std::vector<std::string> s_availableProjects;
static int                   s_selectedProjectIdx = 0;
static char                  s_newProjectNameBuf[128] = {};
static bool                  s_creatingNewProject = false;

static void scanAvailableProjects() {
	s_availableProjects.clear();
	std::filesystem::path assetsRoot = FileSystem::GetRootDir() / "assets";
	if (!std::filesystem::exists(assetsRoot)) return;
	for (auto& entry : std::filesystem::directory_iterator(assetsRoot)) {
		if (entry.is_directory() && entry.path().filename().string() != "Editor")
			s_availableProjects.push_back(entry.path().filename().string());
	}
	std::sort(s_availableProjects.begin(), s_availableProjects.end());
}

static void applyProjectSelection(Engine::EnoEngine& engine_, const std::string& projectName) {
	if (projectName == ENO_APP_NAME) {
		std::cout << "[ProjectPicker] Keeping current project: " << projectName << std::endl;
		return;
	}
	std::cout << "[ProjectPicker] Switching to project: " << projectName << std::endl;

	// Ensure scripts directories exist before switching
	std::filesystem::path scriptsBase = FileSystem::GetRootDir() / "scripts" / projectName;
	std::filesystem::create_directories(scriptsBase / "user_scripts");
	{
		std::filesystem::path src = FileSystem::GetRootDir() / "scripts" / "RuneWeavers" / "GameScripts.csproj";
		std::filesystem::path dst = scriptsBase / "GameScripts.csproj";
		if (std::filesystem::exists(src) && !std::filesystem::exists(dst))
			std::filesystem::copy_file(src, dst);
	}

	FileSystem::Initialize(projectName);

	// The project's GameScripts.dll lives in a sibling directory of the Editor exe.
	std::filesystem::path projectDllDir =
		FileSystem::GetExecutableDir().parent_path() / projectName;

	// Build the project scripts only if they haven't been built yet.
	{
		std::filesystem::path csproj        = scriptsBase / "GameScripts.csproj";
		std::filesystem::path gameScriptsDll = projectDllDir / "GameScripts.dll";
		if (std::filesystem::exists(csproj) && !std::filesystem::exists(gameScriptsDll))
		{
			std::filesystem::create_directories(projectDllDir);
			std::cout << "[ProjectPicker] Building scripts for " << projectName << "...\n";
			std::string cmd = std::string(ENO_DOTNET_CMD) + " publish \""
				+ csproj.string() + "\" -f net8.0 --self-contained false --output \""
				+ projectDllDir.string() + "\" --nologo -v minimal";
			int rc = system(cmd.c_str());
			if (rc != 0)
				std::cerr << "[ProjectPicker] Script build failed (exit code " << rc
				          << "). Scripting may be unavailable.\n";
			else
				std::cout << "[ProjectPicker] Scripts built successfully.\n";
		}
	}

	if (!engine_.isEditorReady())
	{
		// Pre-setup: create a fresh EnoCoreHost pointed at the project's DLL.
		// setupLate() will see it's already set and skip recreation.
		engine_.initScripting(projectDllDir);
	}
	else
	{
		// Post-setup: update the existing coreHost in-place so EnoEditor's
		// reference stays valid, then reload delegates for the new project.
		engine_.getCoreHost().reinitializeScripts(projectDllDir);
	}

	engine_.getResourceLoader().reloadPaths();

	// Reload all assets into the loader BEFORE reloading the project so that
	// loadJson can resolve textures by name when scenes are parsed.
	{
		auto& loader = engine_.getResourceLoader();
		auto& audio  = engine_.getAudioManager();
		loader.loadImages(loader.getImagePath());
		loader.loadSound(audio.getSystem(), loader.getSoundExtensions(), loader.getMusicPath());
		loader.loadSound(audio.getSystem(), loader.getSoundExtensions(), loader.getSFXPath());
		loader.loadFonts(loader.getFontPath(), 48);
	}

	if (engine_.isEditorReady())
	{
		engine_.getEnoEditor().reloadProject();
		if (g_game) g_game->markEntitiesDirty();
	}
	std::cout << "[ProjectPicker] Switched to: " << FileSystem::GetAssetsDir() << std::endl;
}

static void createNewProject(Engine::EnoEngine& engine_, const std::string& name) {
	namespace fs = std::filesystem;
	fs::path projectRoot = FileSystem::GetRootDir() / "assets" / name;
	fs::path editorRoot  = FileSystem::GetRootDir() / "assets" / "Editor";

	auto copyTemplate = [&](const fs::path& relPath) {
		fs::path src = editorRoot / relPath;
		fs::path dst = projectRoot / relPath;
		if (fs::exists(src) && !fs::exists(dst))
			fs::copy_file(src, dst);
	};

	auto copyDir = [&](const fs::path& relDir) {
		fs::path src = editorRoot / relDir;
		fs::path dst = projectRoot / relDir;
		fs::create_directories(dst);
		if (!fs::exists(src)) return;
		for (auto& entry : fs::directory_iterator(src)) {
			if (entry.is_regular_file())
				fs::copy_file(entry.path(), dst / entry.path().filename(),
					fs::copy_options::skip_existing);
		}
	};

	fs::create_directories(projectRoot / "saves" / "scenes");
	fs::create_directories(projectRoot / "saves" / "prefabs");
	fs::create_directories(projectRoot / "saves" / "player");
	fs::create_directories(projectRoot / "settings");
	fs::create_directories(projectRoot / "shaders");
	fs::create_directories(projectRoot / "crashlogs");
	fs::create_directories(projectRoot / "fonts");
	fs::create_directories(projectRoot / "art");
	fs::create_directories(projectRoot / "sound" / "music");
	fs::create_directories(projectRoot / "sound" / "sfx");

	fs::path scriptsRoot = FileSystem::GetRootDir() / "scripts" / name;
	fs::create_directories(scriptsRoot / "user_scripts");
	{
		fs::path srcCsproj = FileSystem::GetRootDir() / "scripts" / "RuneWeavers" / "GameScripts.csproj";
		fs::path dstCsproj = scriptsRoot / "GameScripts.csproj";
		if (fs::exists(srcCsproj) && !fs::exists(dstCsproj))
			fs::copy_file(srcCsproj, dstCsproj);
	}

	copyDir("shaders");
	copyTemplate(fs::path("settings") / "config.txt");
	copyTemplate(fs::path("saves") / "player" / "player_config.txt");
	copyDir(fs::path("saves") / "prefabs");

	{
		fs::path logFile = projectRoot / "crashlogs" / "crash_log.txt";
		if (!fs::exists(logFile)) std::ofstream{ logFile };
	}

	scanAvailableProjects();
	auto it = std::find(s_availableProjects.begin(), s_availableProjects.end(), name);
	if (it != s_availableProjects.end())
		s_selectedProjectIdx = static_cast<int>(std::distance(s_availableProjects.begin(), it));

	std::cout << "[ProjectPicker] Created new project: " << name << std::endl;
}

bool isProjectPickerOpen() { return s_showProjectPicker; }

void setProjectPickerOpen(bool open) {
	s_showProjectPicker = open;
	if (open) {
		scanAvailableProjects();
		auto it = std::find(s_availableProjects.begin(), s_availableProjects.end(), std::string(ENO_APP_NAME));
		if (it != s_availableProjects.end())
			s_selectedProjectIdx = static_cast<int>(std::distance(s_availableProjects.begin(), it));
	}
}

void imGuiProjectPicker(Engine::EnoEngine& engine_) {
	// Lazy init on first display
	static bool initialized = false;
	if (!initialized) {
		initialized = true;
		setProjectPickerOpen(true);  // scans projects and pre-selects current
	}

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGuiIO& io = ImGui::GetIO();

	const ImVec2 dlgSize = {460.f, s_creatingNewProject ? 210.f : 240.f};
	ImGui::SetNextWindowPos(
		{(io.DisplaySize.x - dlgSize.x) * 0.5f, (io.DisplaySize.y - dlgSize.y) * 0.5f},
		ImGuiCond_Always);
	ImGui::SetNextWindowSize(dlgSize, ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(1.0f);

	ImGui::Begin(s_creatingNewProject ? "Create New Project" : "Open Project", nullptr,
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoBringToFrontOnFocus);

	const float pad   = ImGui::GetStyle().WindowPadding.x;
	const float itemW = dlgSize.x - pad * 2.f;

	if (!s_creatingNewProject) {
		ImGui::Text("Select a project to open:");
		ImGui::Spacing();

		if (!s_availableProjects.empty()) {
			std::vector<const char*> items;
			items.reserve(s_availableProjects.size());
			for (auto& p : s_availableProjects) items.push_back(p.c_str());
			ImGui::SetNextItemWidth(itemW);
			ImGui::Combo("##projects", &s_selectedProjectIdx, items.data(), static_cast<int>(items.size()));
		} else {
			ImGui::TextDisabled("No projects found in assets/");
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		const float btnW = (itemW - ImGui::GetStyle().ItemSpacing.x * 2.f) / 3.f;
		if (ImGui::Button("Open", {btnW, 0}) && !s_availableProjects.empty()) {
			applyProjectSelection(engine_, s_availableProjects[s_selectedProjectIdx]);
			s_showProjectPicker = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("New Project", {btnW, 0})) {
			s_creatingNewProject = true;
			memset(s_newProjectNameBuf, 0, sizeof(s_newProjectNameBuf));
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", {btnW, 0})) {
			s_showProjectPicker = false;
			engine_.requestClose();
		}
	} else {
		ImGui::Text("Project name:");
		ImGui::SetNextItemWidth(itemW);
		ImGui::InputText("##newname", s_newProjectNameBuf, sizeof(s_newProjectNameBuf));
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		bool nameValid = s_newProjectNameBuf[0] != '\0';
		const float btnW = (itemW - ImGui::GetStyle().ItemSpacing.x) / 2.f;
		if (!nameValid) ImGui::BeginDisabled();
		if (ImGui::Button("Create", {btnW, 0})) {
			createNewProject(engine_, s_newProjectNameBuf);
			applyProjectSelection(engine_, s_newProjectNameBuf);
			s_showProjectPicker = false;
		}
		if (!nameValid) ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("Back", {btnW, 0})) {
			s_creatingNewProject = false;
		}
	}

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/*============================================================================================
* \\brief   Helper function for undo/redo system to mark entities dirty
*
*           This function is called from entity_commands.hpp to update the render cache
*           after entity creation, deletion, or restoration operations.
*//*========================================================================================*/
void markGameEntitiesDirty()
{
	if (g_game)
	{
		g_game->markEntitiesDirty();
	}
}

float randomFloat(float min, float max) {
	std::uniform_real_distribution<float> dist(min, max);
	return dist(rng);
}

double percentage(double target, double max) {
	if (max == 0.0) return 0.0;
	return (target / max) * 100.0;
}

void imGuiInit(GLFWwindow* window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.FontGlobalScale = 1.5f;
	//ImGui::GetStyle().ScaleAllSizes(1.5f);
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigDockingNoSplit = false;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

// ============================================================================
// Raycast 3D View panel
// ============================================================================
void displayRaycastVisualizerTab(Engine::EnoEngine& engine_)
{
    static Raycast::RaycastRenderer renderer;
    static bool initialized   = false;
    static bool syncToPlayer  = false;

    if (!initialized) {
        renderer.initialize(320, 200,
                            engine_.getResourceLoader(),
                            engine_.getTileMap());
        initialized = true;
    }

    // --- Render (GPU: the fragment shader does all the ray casting) ---
    renderer.render();

    // --- Display (flip V: OpenGL y=0 is bottom, ImGui y=0 is top) ---
    ImVec2 avail = ImGui::GetContentRegionAvail();
    avail.y -= 80.0f;   // reserve space for controls below
    float sx    = avail.x / (float)renderer.getWidth();
    float sy    = avail.y / (float)renderer.getHeight();
    float scale = std::min(sx, sy);
    if (scale < 0.1f) scale = 0.1f;

    ImVec2 displaySize((float)renderer.getWidth()  * scale,
                       (float)renderer.getHeight() * scale);
    ImGui::Image(
        (ImTextureID)(intptr_t)renderer.getTextureID(), displaySize,
        ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f)   // flip Y for correct orientation
    );

    // --- Info & settings ---
    ImGui::Separator();
    ImGui::Text("Pos: (%.2f, %.2f)", renderer.playerPos.x, renderer.playerPos.y);
    ImGui::SameLine();
    ImGui::Text("Dir: (%.2f, %.2f)", renderer.playerDir.x, renderer.playerDir.y);
    ImGui::Checkbox("Floor & Ceiling", &renderer.enableFloorCeiling);
    ImGui::SameLine();
    ImGui::Checkbox("Side Shading", &renderer.enableSideShading);

    // Refresh map button — press after editing the tilemap to sync the GPU SSBO
    ImGui::SameLine();
    if (ImGui::Button("Refresh Map"))
        renderer.updateMap(engine_.getTileMap());

    ImGui::TextDisabled("Player position / direction is set by scripts.");
}

void imGuiDisplay(Engine::EnoEngine& engine_, Editor::EnoSceneTree& sceneTree, Editor::UndoManager& undoManager) {
	// Start ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// get engine systems
	auto& entityManager = engine_.getEntityManager();
	auto& loader = engine_.getResourceLoader();

	// Subscribe to audio error events early (only once)
	// This needs to happen before audio tab is opened so we catch errors from initial load
	static bool audioErrorSubscribed = false;
	if (!audioErrorSubscribed)
	{
		Event::EnoEventBus& bus = engine_.getEventBus();
		bus.subscribe<Audio::AudioLoadError>([](const Audio::AudioLoadError& event) {
			// Add error to queue so it persists and can be displayed later
			g_audioErrorQueue.push_back(AudioErrorInfo{
				event.filename_,
				event.extension_,
				event.errorReason_
				});
			std::cout << "[ImGui Audio] Error queued: " << event.filename_ << std::endl;
			});
		audioErrorSubscribed = true;
		std::cout << "[ImGui Audio] Subscribed to audio error events" << std::endl;
	}

	// Main menu bar
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Change Project...")) {
				setProjectPickerOpen(true);
			}
			ImGui::EndMenu();
		}
		ImGui::Text("| %s", FileSystem::GetAssetsDir().filename().string().c_str());
		ImGui::EndMainMenuBar();
	}

	// Project picker modal (opened from "Change Project..." while editor is running)
	if (s_showProjectPicker) {
		ImGui::OpenPopup("Open Project##modal");
	}
	if (ImGui::BeginPopupModal("Open Project##modal", nullptr,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		const float itemW = 400.f;
		const float pad   = ImGui::GetStyle().WindowPadding.x;

		if (!s_creatingNewProject)
		{
			ImGui::Text("Select a project to open:");
			ImGui::Spacing();
			if (!s_availableProjects.empty()) {
				std::vector<const char*> items;
				items.reserve(s_availableProjects.size());
				for (auto& p : s_availableProjects) items.push_back(p.c_str());
				ImGui::SetNextItemWidth(itemW);
				ImGui::Combo("##projects2", &s_selectedProjectIdx, items.data(), static_cast<int>(items.size()));
			} else {
				ImGui::TextDisabled("No projects found in assets/");
			}
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			const float btnW = (itemW - ImGui::GetStyle().ItemSpacing.x * 2.f) / 3.f;
			if (ImGui::Button("Open", {btnW, 0}) && !s_availableProjects.empty()) {
				applyProjectSelection(engine_, s_availableProjects[s_selectedProjectIdx]);
				s_showProjectPicker = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("New Project", {btnW, 0})) {
				s_creatingNewProject = true;
				memset(s_newProjectNameBuf, 0, sizeof(s_newProjectNameBuf));
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", {btnW, 0})) {
				s_showProjectPicker    = false;
				s_creatingNewProject   = false;
				ImGui::CloseCurrentPopup();
			}
		}
		else
		{
			ImGui::Text("Project name:");
			ImGui::SetNextItemWidth(itemW);
			ImGui::InputText("##newname2", s_newProjectNameBuf, sizeof(s_newProjectNameBuf));
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			bool nameValid = s_newProjectNameBuf[0] != '\0';
			const float btnW = (itemW - ImGui::GetStyle().ItemSpacing.x) / 2.f;
			if (!nameValid) ImGui::BeginDisabled();
			if (ImGui::Button("Create", {btnW, 0})) {
				createNewProject(engine_, s_newProjectNameBuf);
				applyProjectSelection(engine_, s_newProjectNameBuf);
				s_showProjectPicker  = false;
				s_creatingNewProject = false;
				ImGui::CloseCurrentPopup();
			}
			if (!nameValid) ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::Button("Back", {btnW, 0})) {
				s_creatingNewProject = false;
			}
		}
		ImGui::EndPopup();
	}

	// BEGIN DOCKSPACE
	imGuiBeginDockspace();

	// Apply default layout once after dockspace is created
	static bool layoutApplied = false;
	static int  focusCountdown = 0;
	if (!layoutApplied) {
		imGuiSetupDefaultLayout();
		layoutApplied = true;
		focusCountdown = 2; // give ImGui 2 frames to register the docked windows
	}
	if (focusCountdown > 0) {
		if (--focusCountdown == 0) {
			ImGui::SetWindowFocus("Editor View");
		}
	}

	// Get ImGui IO for FPS display
	ImGuiIO& io = ImGui::GetIO();

	// FPS display throttling
	double now = ImGui::GetTime();
	if (now >= next_update) {
		shown_fps = io.Framerate;
		throttledTimes_ = systemTimes_;
		next_update = now + 0.5;
	}

	// Hierarchy
	ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_NoCollapse);
	displayHierarchyTab(engine_, sceneTree, undoManager);
	ImGui::End();

	// Scene select
	ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoCollapse);
	displaySceneTab(engine_.getEnoEditor());
	ImGui::End();

	// Audio select
	ImGui::Begin("Audio Browser", nullptr, ImGuiWindowFlags_NoCollapse);
	displayAudioTab(engine_);
	ImGui::End();


	// Edit tab
	ImGui::Begin("Edit", nullptr, ImGuiWindowFlags_NoCollapse);
	displayEditTab(engine_, sceneTree);
	displayAllEntitiesTab(engine_, sceneTree, undoManager);
	ImGui::End();

	// Editor View (zoomed in, center - replaces old Game View behavior)
	ImGui::Begin("Editor View", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
	displayEditorView(engine_, sceneTree, undoManager);
	ImGui::End();

	// Assets Browser
	ImGui::Begin("Assets Browser", nullptr, ImGuiWindowFlags_NoCollapse);
	displayAssetTab(engine_);
	ImGui::End();

	// FPS
	ImGui::Begin("FPS", nullptr, ImGuiWindowFlags_NoCollapse);
	displayFPSTab(engine_);
	ImGui::End();

	// Raycast 3D View
	ImGui::Begin("Raycast 3D View", nullptr,
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
	displayRaycastVisualizerTab(engine_);
	ImGui::End();

	// Tilemap Editor
	g_game->getMapEditor().displayMenu();
	g_game->getMapEditor().displayTilesetMenu(loader);
	
	// END DOCKSPACE
	imGuiEndDockspace();

	// Process deferred scene operations AFTER all ImGui rendering is complete
	// This prevents crashes from destroying entities while ImGui is iterating over them
	if (g_pendingNewScene)
	{
		clearSelectedEntity();
		engine_.getEnoEditor().createEmptyScene(FileSystem::GetAssetsDir().string() + "/saves/scenes/");
		g_game->markEntitiesDirty();  // Force entity cache refresh
		g_pendingNewScene = false;
		g_resetEntityCounter = true;  // Reset counter for new scene
		std::cout << "[ImGui] Deferred: Created new scene" << std::endl;
	}
	else if (g_pendingSceneLoad)
	{
		clearSelectedEntity();
		engine_.getEnoEditor().selectScene(g_sceneToLoad);
		g_game->markEntitiesDirty();  // Force entity cache refresh
		g_pendingSceneLoad = false;
		g_sceneToLoad = "";
		g_resetEntityCounter = true;  // Reset counter for loaded scene
		std::cout << "[ImGui] Deferred: Loaded scene " << g_sceneToLoad << std::endl;
	}

	// Global Audio Load Error Popup Modal (can appear from any tab)
	// Check if there are errors in the queue and show popup
	static bool isPopupOpen = false;

	if (!g_audioErrorQueue.empty() && !isPopupOpen)
	{
		ImGui::OpenPopup("Audio Format Error##GlobalAudioLoadError");
		isPopupOpen = true;
	}

	// Display the error popup modal
	if (ImGui::BeginPopupModal("Audio Format Error##GlobalAudioLoadError", NULL,
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
	{
		if (!g_audioErrorQueue.empty())
		{
			const auto& error = g_audioErrorQueue.front();

			// Header with error icon style
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f)); // Red text
			ImGui::Text("AUDIO FILE ERROR");
			ImGui::PopStyleColor();

			ImGui::Separator();
			ImGui::Spacing();

			// Show how many errors remain if multiple
			if (g_audioErrorQueue.size() > 1)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.7f, 0.0f, 1.0f)); // Orange
				ImGui::Text("(%zu more error%s in queue)",
					g_audioErrorQueue.size() - 1,
					g_audioErrorQueue.size() > 2 ? "s" : "");
				ImGui::PopStyleColor();
				ImGui::Spacing();
			}

			// Error details
			ImGui::Text("Unable to load or play audio file:");
			ImGui::Spacing();

			ImGui::Indent(20.0f);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow text
			ImGui::TextWrapped("%s", error.filename.c_str());
			ImGui::PopStyleColor();
			ImGui::Unindent(20.0f);

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			// Error reason
			ImGui::Text("Reason:");
			ImGui::Indent(20.0f);
			ImGui::TextWrapped("%s", error.reason.c_str());
			ImGui::Unindent(20.0f);

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			// Supported formats
			ImGui::Text("Supported audio formats:");
			ImGui::Indent(20.0f);
			ImGui::BulletText(".wav (recommended)");
			ImGui::BulletText(".mp3");
			ImGui::Unindent(20.0f);

			ImGui::Spacing();
			ImGui::Spacing();

			// Center the OK button
			float buttonWidth = 120.0f;
			float windowWidth = ImGui::GetWindowSize().x;
			ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);

			if (ImGui::Button("OK", ImVec2(buttonWidth, 0)))
			{
				// Remove this error from queue
				g_audioErrorQueue.erase(g_audioErrorQueue.begin());
				isPopupOpen = false;
				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::EndPopup();
	}
	else
	{
		// Popup was closed
		isPopupOpen = false;
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void displayFPSTab(Engine::EnoEngine& engine_) {
	ImGui::Text("Application average %.3f ms", 1000.0f / shown_fps);
	ImGui::Text("Frame (%.1f FPS)", shown_fps);
	ImGui::Text("=== PER SYSTEM TIMING IN MS ===");
	for (auto& time : throttledTimes_) {

		if (time.first != "totalTimePerFrame") {
			ImGui::Text("%s: %.3f ms", time.first.c_str(), time.second / 1000.0);// divide by 1000 to show microseconds as milliseconds
		}

	}
	ImGui::Text("=== PER SYSTEM TIMING IN PERCENTAGE ===");
	for (auto& time : throttledTimes_)
	{
		if (time.first != "totalTimePerFrame") {
			ImGui::Text("%s: %.0f%%", time.first.c_str(), percentage(time.second, throttledTimes_["totalTimePerFrame"]));
		}
	}

	if (ImGui::Button("Show Entity Count")) {
		displayEntityCount(engine_);
	}

	if (ImGui::Button("Show Entity Characteristics")) {
		displayEntityCharacteristics(engine_);
	}

}

void displayEntityCount(Engine::EnoEngine& engine_) {
	auto& entityManager = engine_.getEntityManager();
	size_t counter = 0;

	std::vector<ECS::Entity> EntityList = entityManager.findEntity<>();
	for (ECS::Entity const& e : EntityList) {
		counter++;
		(void)e; // silence unused warning
	}
	std::cout << "Number of entities: " << counter << "\n";
}



void displayEntityCharacteristics(Engine::EnoEngine& engine_) {
	engine_.getEntityManager().viewMap();
}

void displayEditTab(Engine::EnoEngine& engine_, Editor::EnoSceneTree& sceneTree) {

	auto& entityManager = engine_.getEntityManager();
	const char* filters[] = { "Last Spawned", "Mouse Select", "All" };
	static int FilterType = Filters::Select;

	std::vector<ECS::Entity> toDelete;

	ImGui::Combo("Filters", &FilterType, filters, IM_ARRAYSIZE(filters));


	if (FilterType == Filters::Last) {
		if (ImGui::Button("Delete Last Spawned")) {
			if (!ECS::spawnHistory.empty()) {
				uint32_t lastID = ECS::spawnHistory.back();
				ECS::spawnHistory.pop_back();  // remove from history

				bool found = false;
				for (auto& e : entityManager.findEntity<>()) {
					if (e.id() == lastID) {
						toDelete.push_back(e);
						found = true;
						if (g_game) g_game->markEntitiesDirty();
						std::cout << "Deleted last spawned entity (ID: " << lastID << ")\n";
						break;
					}
				}
				if (!found)
					std::cout << "Entity ID " << lastID << " not found in registry.\n";
			}
			else {
				std::cout << "No spawn history left to delete.\n";
			}
		}
	}
	else if (FilterType == Filters::Select) {
		if (!g_waitingForEntityDeletion) {
			if (ImGui::Button("Delete an Entity by Click")) {
				g_waitingForEntityDeletion = true;
			}
		}
		else {
			ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Click in Game View to delete an entity");
			// Cancel option
			if (ImGui::Button("Cancel")) {
				g_waitingForEntityDeletion = false;
			}
		}
	}
	else if (FilterType == Filters::All) {
		if (ImGui::Button("Delete ALL Entities")) {
			for (auto& e : entityManager.findEntity<>()) {
				toDelete.push_back(e);
			}
		}

	}

	if (!toDelete.empty()) {
		for (auto& a : toDelete) {
			// Remove from scene hierarchy and destroy children
			Node* node = sceneTree.getNodeByEntityID(a.id());
			if (node) {
				destroyChildrenRecursive(node, entityManager);
				sceneTree.kill(node);
			}
			entityManager.destroyEntity(a.id());
		}

		// Mark cache as dirty after deletion
		if (g_game) {
			g_game->markEntitiesDirty();
		}
	}

}

std::optional<ECS::Entity> getEntityUnderMouse(Engine::EnoEngine& engine_, Math::eVec2<float> mouse)
{
	auto& entityManager = engine_.getEntityManager();

	// Collect all entities that contain the mouse position
	std::vector<ECS::Entity> matchingEntities;

	for (ECS::Entity& e : engine_.getEntityManager().findEntity<
		ECSComponents::TransformComponent,
		ECSComponents::DrawInfoComponent>())
	{
		auto& tc = e.get<ECSComponents::TransformComponent>(entityManager);
		auto& dc = e.get<ECSComponents::DrawInfoComponent>(entityManager);

		// TransformComponent.pos is the CENTER of the entity
		float halfW = dc.width * 0.5f;
		float halfH = dc.height * 0.5f;
		float left = tc.pos.x - halfW;
		float right = tc.pos.x + halfW;
		float top = tc.pos.y - halfH;
		float bottom = tc.pos.y + halfH;

		if (mouse.x >= left && mouse.x <= right &&
			mouse.y >= top && mouse.y <= bottom)
		{
			matchingEntities.push_back(e);
		}
	}

	// Return the LAST matching entity (most recently created, drawn on top)
	if (!matchingEntities.empty()) {
		return matchingEntities.back();
	}

	return std::nullopt;
}

void moveChildrenRecursive(Editor::Node* node, float deltaX, float deltaY, ECS::EntityManager& entityManager)
{
	if (!node) return;

	// Move all children of this node
	for (Editor::Node* child : node->children_)
	{
		// Get child's transform and move it
		if (child->entity_.has<ECSComponents::TransformComponent>(entityManager))
		{
			auto& tc = child->entity_.get<ECSComponents::TransformComponent>(entityManager);
			tc.pos.x += deltaX;
			tc.pos.y += deltaY;
		}

		// Recursively move this child's children
		moveChildrenRecursive(child, deltaX, deltaY, entityManager);
	}
}

void rotateChildrenRecursive(Editor::Node* node, float deltaRotation, Math::eVec2<float> parentPos, ECS::EntityManager& entityManager)
{
	if (!node) return;

	// Rotate all children of this node around the parent's position
	for (Editor::Node* child : node->children_)
	{
		if (child->entity_.has<ECSComponents::TransformComponent>(entityManager))
		{
			auto& tc = child->entity_.get<ECSComponents::TransformComponent>(entityManager);

			// 1. Rotate the child's own rotation
			tc.rot.x += deltaRotation;

			// 2. Rotate the child's position around the parent position
			// Get relative position from parent
			float relX = tc.pos.x - parentPos.x;
			float relY = tc.pos.y - parentPos.y;

			// Rotate the relative position
			float cosRot = cosf(deltaRotation);
			float sinRot = sinf(deltaRotation);
			float newRelX = relX * cosRot - relY * sinRot;
			float newRelY = relX * sinRot + relY * cosRot;

			// Update child position
			tc.pos.x = parentPos.x + newRelX;
			tc.pos.y = parentPos.y + newRelY;
		}

		// Recursively rotate this child's children
		rotateChildrenRecursive(child, deltaRotation, parentPos, entityManager);
	}
}

void scaleChildrenRecursive(Editor::Node* node, float scaleFactorX, float scaleFactorY, Math::eVec2<float> parentPos, ECS::EntityManager& entityManager)
{
	if (!node) return;

	// Scale all children of this node from the parent's position
	for (Editor::Node* child : node->children_)
	{
		if (child->entity_.has<ECSComponents::TransformComponent>(entityManager))
		{
			auto& tc = child->entity_.get<ECSComponents::TransformComponent>(entityManager);

			// 1. Scale the child's own scale
			tc.scale.x *= scaleFactorX;
			tc.scale.y *= scaleFactorY;

			// 2. Scale the child's position relative to parent
			// Get relative position from parent
			float relX = tc.pos.x - parentPos.x;
			float relY = tc.pos.y - parentPos.y;

			// Scale the relative position
			relX *= scaleFactorX;
			relY *= scaleFactorY;

			// Update child position
			tc.pos.x = parentPos.x + relX;
			tc.pos.y = parentPos.y + relY;
		}

		// Recursively scale this child's children
		scaleChildrenRecursive(child, scaleFactorX, scaleFactorY, parentPos, entityManager);
	}
}

void destroyChildrenRecursive(Editor::Node* node, ECS::EntityManager& entityManager)
{
	if (!node) return;

	// Recursively destroy all children first (depth-first)
	for (Editor::Node* child : node->children_)
	{
		destroyChildrenRecursive(child, entityManager);

		// Destroy the child entity
		entityManager.destroyEntity(child->entity_.id());
		std::cout << "[DEBUG] Destroyed child entity " << child->entity_.id() << "\n";
	}
}

// Helper function to collect entity IDs for a node and all its children
void collectEntityIDsRecursive(Editor::Node* node, std::vector<uint32_t>& entityIDs)
{
	if (!node) return;

	// Add this entity's ID
	entityIDs.push_back(node->entity_.id());

	// Recursively add all children
	for (Editor::Node* child : node->children_)
	{
		collectEntityIDsRecursive(child, entityIDs);
	}
}

uint32_t getSelectedEntityID()
{
	return selectedEntityID;
}

void clearSelectedEntity()
{
	selectedEntityID = UINT32_MAX;
}

// Helper to collect all entity IDs in a node hierarchy
void collectEntityIDs(Editor::Node* node, std::vector<uint32_t>& entityIDs)
{
	if (!node) return;

	// Add this node's entity ID
	entityIDs.push_back(node->entity_.id());

	// Recursively collect children's entity IDs
	for (Editor::Node* child : node->children_)
	{
		collectEntityIDs(child, entityIDs);
	}
}

void deleteSelectedEntity(Editor::EnoSceneTree& sceneTree, ECS::EntityManager& entityManager, Editor::UndoManager& undoManager, Graphics::EnoGraphicsManager& graphicsManager)
{
	if (selectedEntityID == UINT32_MAX) {
		std::cout << "[DEBUG] No entity selected for deletion\n";
		return;
	}

	std::cout << "[DEBUG] Attempting to delete entity with ID: " << selectedEntityID << "\n";

	// Debug: List all nodes in scene tree
	auto allNodes = sceneTree.getAllNodes();
	std::cout << "[DEBUG] Scene tree contains " << allNodes.size() << " nodes:\n";
	for (auto* n : allNodes) {
		std::cout << "  - Node ID: " << n->sceneObjectID_ << ", Entity ID: " << n->entity_.id() << ", Name: " << n->name_ << "\n";
	}

	// Get the node for the selected entity
	Editor::Node* node = sceneTree.getNodeByEntityID(selectedEntityID);

	if (node) {
		std::cout << "[DEBUG] Found node! Deleting node " << selectedEntityID << " (\"" << node->name_ << "\") and all children from hierarchy\n";

		// Get entity to capture snapshot
		ECS::Entity entity = node->entity_;

		// Create delete command (captures snapshot before deletion)
		auto deleteCmd = std::make_unique<Editor::DeleteEntityCommand>(
			sceneTree,
			entityManager,
			entity,
			graphicsManager
		);

		// Execute the deletion (also adds to undo history)
		undoManager.executeCommand(std::move(deleteCmd));

		// Clear selection
		selectedEntityID = UINT32_MAX;

		std::cout << "[DEBUG] Entity deletion completed with undo support\n";
	}
	else {
		std::cout << "[ERROR] Selected entity " << selectedEntityID << " not found in scene tree!\n";
		std::cout << "[ERROR] This entity exists in ECS but has no corresponding node in the scene tree.\n";
	}
}


void displayAllEntitiesTab(Engine::EnoEngine& engine_, Editor::EnoSceneTree& sceneTree, Editor::UndoManager& undoManager)
{
	auto& entityManager = engine_.getEntityManager();

	// Get all entities that have a NameComponent
	std::vector<ECS::Entity> NamedEntityList = entityManager.findEntity<ECSComponents::NameComponent>();

	ImGui::Text("Total Entities: %d", static_cast<int>(NamedEntityList.size()));
	ImGui::Separator();

	// --- Entity selection list ---
	for (ECS::Entity& e : NamedEntityList)
	{
		const auto& nameComp = e.get<ECSComponents::NameComponent>(entityManager);
		std::string name = nameComp.name;
		bool selected = (selectedEntityID == e.id());

		std::string label = std::to_string(e.id()) + ": " + name;
		if (ImGui::Selectable(label.c_str(), selected))
		{
			selectedEntityID = e.id();
		}
	}

	// --- Display inspector if selected entity exists ---
	// Note: Entity ID 0 is valid, so we check if any entity was selected
	if (selectedEntityID != UINT32_MAX)
	{
		// Find and display the selected entity
		bool entityFound = false;
		for (ECS::Entity& e : NamedEntityList)
		{
			if (e.id() == selectedEntityID)
			{
				// Display inspector for all entities (including root nodes)
				displayEntityInspector(engine_, e, undoManager, sceneTree);
				//assetPresetsTab(engine_, e);
				assetPresetsTab(engine_, e);
				entityFound = true;
				break;
			}
		}

		// If entity was not found, clear selection
		if (!entityFound)
		{
			selectedEntityID = UINT32_MAX;
		}
	}
}

void displayHierarchyTab(Engine::EnoEngine& engine_, Editor::EnoSceneTree& sceneTree, Editor::UndoManager& undoManager)
{
	ImGui::Text("=== Scene Hierarchy ===");
	ImGui::Separator();

	// ==================== HIERARCHY CONTROLS ====================
	static int entityCounter = 1;  // Track entity numbering
	static bool entityCounterInitialized = false;  // Flag to ensure we only initialize once per scene load
	static bool attachScript = false; // Checkbox state

	static int selectedScriptIndex = -1;	// Script dropdown
	static std::vector<std::wstring> availableScripts; // List of available scripts
	static bool scriptsLoaded = false;

	Script::EnoScriptManager& scriptMng = engine_.getEnoEditor().getScriptManager();

	// Reset entity counter flag when scene changes
	if (g_resetEntityCounter) {
		entityCounterInitialized = false;
		g_resetEntityCounter = false;
	}

	// Initialize entity counter based on existing entities in the scene
	// This ensures we don't create duplicate names like "Entity 1" when one already exists
	if (!entityCounterInitialized) {
		int maxEntityNumber = 0;
		auto& entityManager = engine_.getEntityManager();

		// Scan all entities with NameComponent to find the highest "Entity N" number
		for (ECS::Entity& e : entityManager.findEntity<ECSComponents::NameComponent>()) {
			auto& nameComp = e.get<ECSComponents::NameComponent>(entityManager);
			std::string name(nameComp.name);

			// Check if name matches pattern "Entity <number>"
			if (name.find("Entity ") == 0) {
				try {
					int num = std::stoi(name.substr(7));  // Extract number after "Entity "
					if (num > maxEntityNumber) {
						maxEntityNumber = num;
					}
				}
				catch (...) {
					// Ignore entities with non-numeric suffixes like "Entity ABC"
				}
			}
		}

		// Set counter to one more than the highest found number
		entityCounter = maxEntityNumber + 1;
		entityCounterInitialized = true;
	}

	ImGui::Checkbox("Attach Script to New Entity", &attachScript);

	// Here i prepare the list for showcase of the scripts
	if (attachScript) {
		// Load scripts only once or when needed
		if (!scriptsLoaded) {
			availableScripts = scriptMng.getAvailableScripts();
			scriptsLoaded = true;
		}

		if (!availableScripts.empty()) {
			std::vector<std::string> scriptNames;
			for (const auto& ws : availableScripts)
			{
				int size_needed = WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(), nullptr, 0, nullptr, nullptr);
				std::string result(size_needed, 0);
				WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(), &result[0], size_needed, nullptr, nullptr);

				scriptNames.push_back(result);
			}

			// Convert vector to array for ImGui Combo
			std::vector<const char*> scriptItems;
			for (auto& name : scriptNames)
				scriptItems.push_back(name.c_str());

			ImGui::Text("Select Script:");
			ImGui::Combo("##ScriptCombo", &selectedScriptIndex, scriptItems.data(), (int)scriptItems.size());
		}
		else {
			ImGui::Text("No scripts found.");
		}
	}

	// Preset selector for node spawning
	{
		auto& prefabMng = engine_.getPrefabManager();
		auto presetNames = prefabMng.getPresetNames();

		// Ensure g_selectedNodePreset is valid
		if (g_selectedNodePreset.empty() && !presetNames.empty())
			g_selectedNodePreset = presetNames[0];

		ImGui::Text("Node Preset:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(160.f);
		if (ImGui::BeginCombo("##NodePreset", g_selectedNodePreset.c_str()))
		{
			for (int i = 0; i < static_cast<int>(presetNames.size()); ++i)
			{
				bool selected = (presetNames[i] == g_selectedNodePreset);
				if (ImGui::Selectable(presetNames[i].c_str(), selected))
				{
					g_selectedNodePreset    = presetNames[i];
					g_selectedNodePresetIdx = i;
				}
				if (selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}

	if (ImGui::Button("Add Root Node")) {
		auto& entityManager = engine_.getEntityManager();
		auto& prefabMng     = engine_.getPrefabManager();

		float nodePosX = g_game->getEditorCameraX() + static_cast<float>(g_game->getEditorWidth()) / 2.f;
		float nodePosY = g_game->getEditorCameraY() + static_cast<float>(g_game->getEditorHeight()) / 2.f;

		// Create entity with NameComponent as base (always required for naming)
		ECS::Entity newEntity = entityManager.create<ECSComponents::NameComponent>();

		// AnimationControllerComponent is always attached and is not part of any preset
		//newEntity.add<ECSComponents::AnimationControllerComponent>(entityManager, engine_.getAnimationRegistry().createController());

		// Add all components from the selected preset (skips any already-present component)
		if (const auto* presetComps = prefabMng.getPresetComponents(g_selectedNodePreset))
		{
			for (const auto& [compTypeIdx, compAny] : *presetComps)
			{
				ECSComponents::EnoComponentRegistry::DeduceCompType(compTypeIdx, [&]<typename T>()
				{
					if (!newEntity.template has<T>(entityManager))
						newEntity.template add<T>(entityManager, std::any_cast<T>(compAny));
				});
			}
		}

		// If the preset included a TransformComponent, place it at screen centre
		if (newEntity.has<ECSComponents::TransformComponent>(entityManager))
		{
			auto& transform = newEntity.get<ECSComponents::TransformComponent>(entityManager);
			transform.pos = { nodePosX, nodePosY };
		}

		// Set entity name
		auto& nameComp = newEntity.get<ECSComponents::NameComponent>(entityManager);
		std::string entityName = "Entity " + std::to_string(entityCounter++);
		strncpy_s(nameComp.name, entityName.c_str(), sizeof(nameComp.name) - 1);
		nameComp.name[sizeof(nameComp.name) - 1] = '\0';

		Node* createdNode = sceneTree.createFromEntity(newEntity, nullptr);

		// Record creation for undo/redo
		auto createCmd = std::make_unique<Editor::CreateEntityCommand>(
			sceneTree,
			entityManager,
			newEntity,
			engine_.getGraphicsManager()
		);
		undoManager.executeCommand(std::move(createCmd));

		std::cout << "[DEBUG] Add Root Node: Entity " << entityName << " created with preset '" << g_selectedNodePreset << "'\n";
		if (attachScript && selectedScriptIndex >= 0) {
			std::wstring selectedScript = availableScripts[selectedScriptIndex];
			std::wcout << L"[DEBUG] Selected Script: " << selectedScript << std::endl;
			auto script = scriptMng.loadScript(selectedScript);
			if (script)
				sceneTree.attachScript(createdNode, std::move(script));
			else
				std::cerr << "No script is attached!" << std::endl;
		}
		if (g_game) g_game->markEntitiesDirty();
	}

	ImGui::SameLine();

	if (ImGui::Button("Add Child Node")) {
		if (selectedEntityID != UINT32_MAX) {
			if (!sceneTree.getNodeByEntityID(selectedEntityID)) {
				std::cout << "[ERROR] Selected entity " << selectedEntityID << " not found in scene tree. Cannot add child.\n";
			}
			else {
				auto& entityManager = engine_.getEntityManager();
				auto& prefabMng     = engine_.getPrefabManager();

				float nodePosX = g_game->getEditorCameraX() + static_cast<float>(g_game->getEditorWidth()) / 2.f;
				float nodePosY = g_game->getEditorCameraY() + static_cast<float>(g_game->getEditorHeight()) / 2.f;

				// Create entity with NameComponent as base (always required for naming)
				ECS::Entity childEntity = entityManager.create<ECSComponents::NameComponent>();

				// AnimationControllerComponent is always attached and is not part of any preset
				childEntity.add<ECSComponents::AnimationControllerComponent>(entityManager, engine_.getAnimationRegistry().createController());

				// Add all components from the selected preset (skips any already-present component)
				if (const auto* presetComps = prefabMng.getPresetComponents(g_selectedNodePreset))
				{
					for (const auto& [compTypeIdx, compAny] : *presetComps)
					{
						ECSComponents::EnoComponentRegistry::DeduceCompType(compTypeIdx, [&]<typename T>()
						{
							if (!childEntity.template has<T>(entityManager))
								childEntity.template add<T>(entityManager, std::any_cast<T>(compAny));
						});
					}
				}

				// If the preset included a TransformComponent, place it at screen centre
				if (childEntity.has<ECSComponents::TransformComponent>(entityManager))
				{
					auto& transform = childEntity.get<ECSComponents::TransformComponent>(entityManager);
					transform.pos = { nodePosX, nodePosY };
				}

				// Set entity name
				auto& nameComp = childEntity.get<ECSComponents::NameComponent>(entityManager);
				std::string entityName = "Entity " + std::to_string(entityCounter++);
				strncpy_s(nameComp.name, entityName.c_str(), sizeof(nameComp.name) - 1);
				nameComp.name[sizeof(nameComp.name) - 1] = '\0';

				// Get parent node and add child entity to scene tree
				Editor::Node* parentNode = sceneTree.getNodeByEntityID(selectedEntityID);
				Editor::Node* childNode = sceneTree.createFromEntity(childEntity, parentNode);

				// Record creation for undo/redo
				auto createCmd = std::make_unique<Editor::CreateEntityCommand>(
					sceneTree,
					entityManager,
					childEntity,
					engine_.getGraphicsManager()
				);
				undoManager.executeCommand(std::move(createCmd));

				if (attachScript && selectedScriptIndex >= 0) {
					std::wstring selectedScript = availableScripts[selectedScriptIndex];
					std::wcout << L"[DEBUG] Selected Script: " << selectedScript << std::endl;
					auto script = scriptMng.loadScript(selectedScript);
					if (script)
						sceneTree.attachScript(childNode, std::move(script));
				}

				if (g_game) g_game->markEntitiesDirty();

				std::cout << "Added child node " << childEntity.id() << " ('" << nameComp.name
					<< "') to parent " << selectedEntityID << " with preset '" << g_selectedNodePreset << "'\n";

				if (childNode) {
					std::cout << "  -> Child's parent: " << (childNode->parent_ ? std::to_string(childNode->parent_->sceneObjectID_) : "none") << "\n";
				}
			}
		}
		else {
			std::cout << "No node selected. Please select a node first.\n";
		}
	}




	ImGui::Separator();

	// --- Delete Node Button ---
	if (ImGui::Button("Delete Selected Node")) {
		auto& em = engine_.getEntityManager();
		Editor::Node* node = sceneTree.getNodeByEntityID(selectedEntityID);
		if (node) {
			std::cout << "[DEBUG] Deleting node " << selectedEntityID << " and all children from hierarchy\n";

			// First, destroy all child entities recursively
			destroyChildrenRecursive(node, em);

			// Then kill the scene tree nodes (removes from tree structure)
			sceneTree.kill(node);

			// Finally, destroy the parent entity
			em.destroyEntity(selectedEntityID);
			selectedEntityID = UINT32_MAX;  // Reset to "no selection"

			if (g_game)
				g_game->markEntitiesDirty();
		}
		else {
			std::cout << "[WARN] Selected node " << selectedEntityID << " not found in scene tree\n";
		}
	}


	ImGui::Separator();

	auto& entityManager = engine_.getEntityManager();
	auto allNodes = sceneTree.getAllNodes();
	bool anyNode = false;
	for (Editor::Node* node : allNodes) {
		if (node->parent_ == nullptr) {  // nullptr means no parent (root node)
			drawNodeRecursive(sceneTree, node->entity_.id(), entityManager);
			anyNode = true;
		}
	}

	if (!anyNode)
		ImGui::TextDisabled("(no nodes to display)");

	ImGui::Separator();

	if (ImGui::Button("Print Tree to Terminal")) {
		auto all = sceneTree.getAllNodes();
		for (Editor::Node* node : all) {
			std::cout << "Entity " << node->entity_.id()
				<< " | Parent: " << (node->parent_ ? std::to_string(node->parent_->entity_.id()) : "none")
				<< " | Children: ";
			for (Editor::Node* c : node->children_) std::cout << c->entity_.id() << " ";
			std::cout << "\n";
		}
	}
}

void displaySceneTab(Editor::EnoEditor& editor_)
{
	// Heading
	ImGui::Text("=== Scene Select===");
	ImGui::Text("Current Scene:");

	// Get current scene
	std::string currScene = editor_.getCurrentScene().getSceneName();

	// Create a container for all scene names inside my scene folder
	std::vector<std::unique_ptr<Editor::EnoScene>>& sceneList = editor_.getScenes();
	std::vector<std::string> sceneNames;
	// Populate the container
	for (auto& scene : sceneList)
	{
		sceneNames.push_back(scene->getSceneName());
	}
	// A toggle flag to refresh the scene list
	static bool refreshSceneName = true;
	// Buffer to change scene name
	static char scene_name[64] = "";

	// Check global flag for scene list refresh
	if (g_refreshSceneList)
	{
		refreshSceneName = true;
		g_refreshSceneList = false;
	}

	if (refreshSceneName)
	{
		// Clear the current container
		//sceneNames.clear();
		// A string path to the assets folder

		// Check if scene list contains at least one scene
		if (!sceneNames.empty())
		{
			sceneNames.clear();
		} 

		// Repopulate the scene list
		for (auto& scene : sceneList)
		{
			sceneNames.push_back(scene->getSceneName());
		}

		// Set refresh to false to prevent reopening
		refreshSceneName = false;

	}

	// If there are no scenes to create
	if (sceneNames.empty()) currScene = "No scenes available.";

	// Display active scene name
	ImGui::Text(currScene.c_str());
	
	ImGui::Separator();

	// Cache start scene name for this frame
	const std::string& startScene = editor_.getStartScene();

	// Display scene names
	for (auto& name : sceneNames)
	{
		std::string sceneName = name;
		bool isStartScene = (sceneName == startScene);

		// Give it an id
		ImGui::PushID(sceneName.c_str());

		// Build display label: prefix [S] for start scene
		std::string displayLabel = isStartScene ? ("[S] " + sceneName) : sceneName;

		// Create an invisible button over the text
		ImVec2 textSize = ImGui::CalcTextSize(displayLabel.c_str());
		float rowWidth = ImGui::GetContentRegionAvail().x;
		ImGui::InvisibleButton("##scene", ImVec2(rowWidth, textSize.y));

		// Left click loads scene
		// IsItemClicked checks the click on the most recent pushed imgui item aka invisible button
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		{
			// Load scene
			std::cout << "current scene: " << editor_.getCurrentScene().getSceneName() << ", filename: " << sceneName << std::endl;
			// Check that scene is not currently loaded
			if (editor_.getCurrentScene().getSceneName() != sceneName)
			{
				std::cout << "Loading file: " << sceneName << std::endl;
				// Defer scene loading until after ImGui rendering completes
				g_pendingSceneLoad = true;
				g_sceneToLoad = sceneName;
			}
			else
			{
				std::cout << "Scene is already loaded!";
			}

		}

		// Right click opens a panel to allow you to change and delete scene name
		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		{
			// Open popup
			ImGui::OpenPopup("scene_menu");
		}

		// Create menu popup
		if (ImGui::BeginPopup("scene_menu"))
		{
			ImGui::Text("%s", sceneName.c_str());
			if (isStartScene)
				ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "(Start Scene)");
			static std::string warningBuf;
			ImGui::Separator();

			// Set as start scene
			if (isStartScene)
			{
				ImGui::TextDisabled("Already the start scene");
			}
			else
			{
				if (ImGui::Button("Set as Start Scene"))
				{
					editor_.setStartScene(sceneName);
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::Separator();

			// Input box to change name
			ImGui::Text("Change scene name");
			ImGui::InputText("##Change_scene_name", scene_name, sizeof(scene_name));

			if (ImGui::Button("Change new name"))
			{
				if (!scene_name || scene_name[0] == '\0')
				{
					warningBuf = "Error[Name]: Empty name not allowed!";
				}
				else
				{
					// Rename scene
					editor_.renameScene(scene_name, sceneName);

					// Close popup after renaming
					ImGui::CloseCurrentPopup();
					// Clear buffer
					warningBuf.clear();
					memset(scene_name, 0, sizeof(scene_name));

					// Refresh scene name
					g_refreshSceneList = true;
					
				}

			}

			// Show error message if trying to delete active scene
			if (g_cannotDeleteActiveScene)
			{
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
					"Cannot delete active scene!");
				ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
					"Switch to another scene first.");
			}

			// Delete scene
			if (ImGui::Button("Delete scene"))
			{
				try
				{
					// Check if we're deleting the currently active scene
					bool isCurrentScene = (sceneName == editor_.getCurrentScene().getSceneName());

					if (isCurrentScene)
					{
						// Set error flag instead of showing popup
						g_cannotDeleteActiveScene = true;
						std::cout << "[ImGui] Cannot delete currently active scene" << std::endl;
					}
					else
					{
						// Safe to delete - scene is not currently loaded
						editor_.deleteScene(sceneName);

						// Reset error flag
						g_cannotDeleteActiveScene = false;

						// Close current popup	
						ImGui::CloseCurrentPopup();

					}
				}
				catch (const std::filesystem::filesystem_error& e)
				{
					warningBuf = "Error[Delete]: " + std::string(e.what());
				}
			}

			// Cancel edits
			if (ImGui::Button("Close menu"))
			{
				// Close popup
				ImGui::CloseCurrentPopup();
				// In case theres some random stuff inside the input box, clear the buffer
				warningBuf.clear();
				memset(scene_name, 0, sizeof(scene_name));
				// Reset error flag
				g_cannotDeleteActiveScene = false;
			}

			if (!warningBuf.empty())
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), warningBuf.c_str());

			ImGui::EndPopup();

		}

		// Aligning text to invisible button
		ImGui::SameLine(0, 0);
		// Go back to the start of the invisible button
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - rowWidth);
		// Display file name as text (green if this is the start scene)
		if (isStartScene)
			ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.2f, 1.0f), "%s", displayLabel.c_str());
		else
			ImGui::Text("%s", displayLabel.c_str());

		if (ImGui::IsItemHovered())
		{
			// Hover feedback
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		}

		// Prevents any duplicate of ID
		ImGui::PopID();

	}

	// Save scene
	if (ImGui::Button("Save scene"))
	{
		std::string sceneName = editor_.getCurrentScene().getSceneName();

		// Save scene
		auto const& assetPath = FileSystem::GetAssetsDir().string();
		editor_.getSceneTree().save(assetPath + "/saves/scenes/" + sceneName);
	}

	if (ImGui::Button("Create new scene"))
	{
		// Defer scene creation until after ImGui rendering completes
		g_pendingNewScene = true;
		//g_refreshSceneList = true;
	}


}

// Recursively scans a directory and populates an AudioFolder tree.
// buildBase / rootBase are the top-level paths so we can derive the matching root path for each file.
static void BuildAudioTree(AudioFolder& folder,
	const std::filesystem::path& buildBase,
	const std::filesystem::path& rootBase,
	const std::filesystem::path& currentPath)
{
	if (!std::filesystem::exists(currentPath)) return;

	for (const auto& entry : std::filesystem::directory_iterator(currentPath))
	{
		if (entry.is_directory())
		{
			AudioFolder sub;
			sub.folderName = entry.path().filename().string();
			BuildAudioTree(sub, buildBase, rootBase, entry.path());
			if (!sub.subfolders.empty() || !sub.files.empty())
				folder.subfolders.push_back(std::move(sub));
		}
		else if (entry.is_regular_file())
		{
			auto ext = entry.path().extension().string();
			if (ext == ".wav" || ext == ".mp3")
			{
				AudioEntry e;
				e.displayName = entry.path().filename().string();
				e.stemName    = entry.path().stem().string();
				e.buildPath   = entry.path();
				// mirror the relative path onto the root (source) folder
				auto rel  = std::filesystem::relative(entry.path(), buildBase);
				e.rootPath = rootBase / rel;
				folder.files.push_back(std::move(e));
			}
		}
	}

	std::sort(folder.subfolders.begin(), folder.subfolders.end(),
		[](const AudioFolder& a, const AudioFolder& b) { return a.folderName < b.folderName; });
	std::sort(folder.files.begin(), folder.files.end(),
		[](const AudioEntry& a, const AudioEntry& b) { return a.displayName < b.displayName; });
}

void LoadAudioAssets(Engine::EnoEngine& engine_)
{
	Resources::EnoLoader& loader = engine_.getResourceLoader();
	g_audioTree = AudioFolder{};
	g_audioTree.folderName = "sound";

	AudioFolder musicFolder;
	musicFolder.folderName = "music";
	BuildAudioTree(musicFolder,
		loader.getMusicPath(), loader.getRootMusicPath(),
		loader.getMusicPath());
	if (!musicFolder.subfolders.empty() || !musicFolder.files.empty())
		g_audioTree.subfolders.push_back(std::move(musicFolder));

	AudioFolder sfxFolder;
	sfxFolder.folderName = "sfx";
	BuildAudioTree(sfxFolder,
		loader.getSFXPath(), loader.getRootSFXPath(),
		loader.getSFXPath());
	if (!sfxFolder.subfolders.empty() || !sfxFolder.files.empty())
		g_audioTree.subfolders.push_back(std::move(sfxFolder));
}

void displayAudioTab(Engine::EnoEngine& engine_)
{
	Resources::EnoLoader& loader = engine_.getResourceLoader();
	Event::EnoEventBus& bus = engine_.getEventBus();

	// Note: Audio error subscription now happens in imGuiDisplay() on first frame

	ImGui::Text("=== AUDIO ASSETS ===");

	// Drag-drop hint
	ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Drag & drop .wav or .mp3 files here to import!");

	// Import destination selector
	static int importDestination = 1; // 0=Music, 1=SFX
	ImGui::Text("Import to:");
	ImGui::SameLine();
	ImGui::RadioButton("Music", &importDestination, 0);
	ImGui::SameLine();
	ImGui::RadioButton("SFX", &importDestination, 1);

	// Button to load/reload audio assets
	if (ImGui::Button("Load Audio Files"))
	{
		// Reload audio files from disk - this will trigger error events for unsupported files
		auto& audioManager = engine_.getAudioManager();
		loader.loadSound(audioManager.getSystem(), loader.getSoundExtensions(), loader.getMusicPath());
		loader.loadSound(audioManager.getSystem(), loader.getSoundExtensions(), loader.getSFXPath());

		// Update the UI list
		LoadAudioAssets(engine_);

		std::cout << "[ImGui Audio] Reloaded audio files" << std::endl;
	}

	ImGui::Separator();

	// --- Handle dropped audio files ---
	auto& g_droppedFiles = Window::EnoWindow::GetDroppedFiles();
	if (!g_droppedFiles.empty()) {
		auto& audioManager = engine_.getAudioManager();

		for (auto& path : g_droppedFiles) {
			std::filesystem::path filePath(path);
			auto ext = filePath.extension().string();

			// Check if this is an audio file
			if (ext == ".wav" || ext == ".mp3" || ext == ".ogg" || ext == ".flac" ||
				ext == ".aac" || ext == ".wma" || ext == ".m4a" || ext == ".aiff") {

				std::cout << "[Audio Drop] Processing dropped file: " << filePath.filename().string() << std::endl;

				// Determine destination folder based on user selection
				std::string destFolder = (importDestination == 0) ? loader.getMusicPath() : loader.getSFXPath();
				std::string folderName = (importDestination == 0) ? "Music" : "SFX";
				std::string filename = filePath.filename().string();
				std::filesystem::path destPath = std::filesystem::path(destFolder) / filename;

				try {
					// Create directory if it doesn't exist
					std::filesystem::create_directories(destFolder);

					// Check if we need to copy the file
					bool needsCopy = !std::filesystem::exists(destPath);
					if (!needsCopy) {
						auto sourceAbsolute = std::filesystem::absolute(filePath);
						auto destAbsolute = std::filesystem::absolute(destPath);
						needsCopy = (sourceAbsolute != destAbsolute);
					}

					if (needsCopy) {
						std::filesystem::copy_file(filePath, destPath,
							std::filesystem::copy_options::overwrite_existing);
						std::cout << "[Audio Drop] Copied " << filename << " to " << folderName << " folder" << std::endl;
					} else {
						std::cout << "[Audio Drop] File " << filename << " already exists in " << folderName << " folder" << std::endl;
					}

					// Now try to load the audio file with FMOD
					// This will trigger error events if the format is unsupported
					FMOD::Sound* sound = nullptr;
					FMOD_RESULT result = audioManager.getSystem()->createSound(
						destPath.string().c_str(), FMOD_DEFAULT, nullptr, &sound);

					if (result == FMOD_OK && (ext == ".wav" || ext == ".mp3")) {
						// Successfully loaded - store it in loader and notify audio manager via event
						std::string soundName = filePath.stem().string();

						// Store in loader (used by UI and serialization)
						loader.addSound(soundName, sound);

						// Publish LoadSound event so audio manager receives it too
						bus.publish(Audio::LoadSound{ soundName, sound });

						std::cout << "[Audio Drop] Successfully loaded: " << filename << " as '" << soundName << "'" << std::endl;

						// Refresh the UI list
						LoadAudioAssets(engine_);
					} else {
						// Failed to load or unsupported format
						if (ext == ".wav" || ext == ".mp3") {
							// Supported format but FMOD failed
							bus.publish(Audio::AudioLoadError{
								filename,
								destPath.string(),
								ext,
								"FMOD failed to load the file. The file may be corrupted or in an unsupported codec."
								});
						} else {
							// Unsupported format
							bus.publish(Audio::AudioLoadError{
								filename,
								destPath.string(),
								ext,
								"File format is not supported. Only .wav and .mp3 files are supported."
								});
						}

						// Clean up the FMOD sound if it was partially created
						if (sound) {
							sound->release();
						}

						std::cout << "[Audio Drop] Failed to load: " << filename << " (unsupported or corrupted)" << std::endl;
					}

				} catch (const std::exception& e) {
					std::cerr << "[Audio Drop] Error processing file: " << e.what() << std::endl;
					bus.publish(Audio::AudioLoadError{
						filename,
						path,
						ext,
						std::string("File operation failed: ") + e.what()
						});
				}
			}
		}
	}

	static std::string selectedName{};
	static FMOD::Sound* selectedSound = nullptr;

	// M4 requirement: Deletion of audio asset files
	static std::filesystem::path selectedFullPath{};
	static std::filesystem::path selectedFullRootPath{};
	static std::string selectedFilenameWithExt{};

	// ============== FILE TREE ==============
	// Build entity->audio attachment map for indicators
	auto& entMgr = engine_.getEntityManager();
	std::vector<ECS::Entity> audioEntities = entMgr.findEntity<ECSComponents::AudioComponent>();
	std::unordered_map<std::string, std::string> attachedAudioMap; // stemName -> entityName
	for (ECS::Entity& ae : audioEntities)
	{
		auto& ac = ae.get<ECSComponents::AudioComponent>(entMgr);
		if (strlen(ac.audioMod_.name_) > 0)
		{
			std::string entityLabel;
			if (ae.has<ECSComponents::NameComponent>(entMgr))
				entityLabel = ae.get<ECSComponents::NameComponent>(entMgr).name;
			else
				entityLabel = std::to_string(ae.id());
			attachedAudioMap[ac.audioMod_.name_] = entityLabel;
		}
	}

	ImGui::BeginChild("AudioAssetTree", ImVec2(0, 200), true);

	bool treeIsEmpty = g_audioTree.subfolders.empty() && g_audioTree.files.empty();
	if (treeIsEmpty)
	{
		ImGui::TextDisabled("(No audio loaded. Press \"Load Audio Files\" above)");
	}
	else
	{
		// Recursive lambda to render each folder node
		std::function<void(const AudioFolder&)> renderFolder = [&](const AudioFolder& folder)
		{
			for (const AudioFolder& sub : folder.subfolders)
			{
				ImGuiTreeNodeFlags folderFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
				if (ImGui::TreeNodeEx(sub.folderName.c_str(), folderFlags))
				{
					renderFolder(sub);
					ImGui::TreePop();
				}
			}

			for (const AudioEntry& entry : folder.files)
			{
				bool isSelected = (selectedFullPath == entry.buildPath);

				std::string label = entry.displayName;
				auto attachIt = attachedAudioMap.find(entry.stemName);
				if (attachIt != attachedAudioMap.end())
					label += "  -> [" + attachIt->second + "]";

				// Grey out files not yet loaded into FMOD
				bool isLoaded = loader.getSounds().count(entry.stemName) > 0;
				if (!isLoaded)
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

				ImGuiSelectableFlags selFlags = ImGuiSelectableFlags_SpanAllColumns;
				if (ImGui::Selectable(label.c_str(), isSelected, selFlags))
				{
					selectedName            = entry.stemName;
					selectedFilenameWithExt = entry.displayName;
					selectedFullPath        = entry.buildPath;
					selectedFullRootPath    = entry.rootPath;

					std::cout << "[Audio] User selected: " << selectedName << "\n";

					bus.publish(Audio::RequestSound{ selectedName });

					const auto& sounds = loader.getSounds();
					auto it = sounds.find(selectedName);
					if (it != sounds.end())
					{
						selectedSound = it->second;
						std::cout << "[Audio] Loaded Sound: " << selectedName << "\n";
					}
					else
					{
						selectedSound = nullptr;
						std::cout << "[Audio] Sound not in FMOD yet (not loaded): " << selectedName << "\n";
					}
				}

				if (!isLoaded)
					ImGui::PopStyleColor();
			}
		};

		renderFolder(g_audioTree);
	}

	ImGui::EndChild();
	ImGui::Separator();

	// ====== Waveform Viewer (Mock) ======
	//ImGui::Text("Waveform Preview");
	//ImGui::BeginChild("WaveformViewer", ImVec2(0, 200), true);
	//ImDrawList* draw = ImGui::GetWindowDrawList();
	//ImVec2 pos = ImGui::GetCursorScreenPos();
	//ImVec2 size = ImGui::GetContentRegionAvail();
	//float midY = pos.y + size.y * 0.5f;
	//float step = size.x / 60.0f;
	//// mock waveform
	//for (int i = 0; i < 60; i++)
	//{
	//	float h = (sinf(i * 0.3f) * 0.5f + 0.5f) * (size.y * 0.9f);
	//	draw->AddLine(
	//		ImVec2(pos.x + i * step, midY - h * 0.5f),
	//		ImVec2(pos.x + i * step, midY + h * 0.5f),
	//		IM_COL32(100, 200, 255, 255), 2.0f
	//	);
	//}
	//ImGui::EndChild();
	//ImGui::Separator();
	//ImGui::Spacing();

	ImGui::Text("=== Test Playback ===");

	if (selectedName.empty())
	{
		ImGui::TextDisabled("Select an audio file to test.");
	}
	else
	{
		ImGui::Text("Selected: %s", selectedName.c_str());

		if (ImGui::Button("Play as SFX"))
		{
			bus.publish(Audio::PlayInstant{ "SFX", selectedName });
			std::cout << "PlayInstant SFX: " << selectedName << "\n";
		}

		if (ImGui::Button("Set as BGM"))
		{
			bus.publish(Audio::SetNewAudio{ selectedName });
			std::cout << "SetNewAudio (BGM): " << selectedName << "\n";
		}

		if (ImGui::Button("Play BGM preview"))
		{
			bus.publish(Audio::SetAudioState{ "BGM" });
		}

		if (ImGui::Button("Stop BGM preview"))
		{
			bus.publish(Audio::StopBGM{});
		}

		// --- Entity target selection for audio attachment ---
		static uint32_t audioTargetEntityID = UINT32_MAX;
		static std::string audioAttachStatusMsg{};
		static float audioAttachStatusTimer = 0.0f;

		auto& entityManager = engine_.getEntityManager();

		// Default to the globally selected entity if no audio target has been picked yet
		if (audioTargetEntityID == UINT32_MAX)
			audioTargetEntityID = selectedEntityID;

		// Build the label for the currently targeted entity
		std::string targetLabel = "(No entity selected)";
		{
			auto& map = entityManager.getMap();
			auto it = map.find(audioTargetEntityID);
			if (it != map.end())
			{
				ECS::Entity targetEnt{ audioTargetEntityID };
				if (targetEnt.has<ECSComponents::NameComponent>(entityManager))
				{
					const auto& nc = targetEnt.get<ECSComponents::NameComponent>(entityManager);
					targetLabel = std::to_string(audioTargetEntityID) + ": " + nc.name;
				}
				else
				{
					targetLabel = std::to_string(audioTargetEntityID) + ": (unnamed)";
				}
			}
		}

		ImGui::Text("Target Entity:");
		ImGui::SameLine();

		// Combo dropdown to pick target entity
		ImGui::SetNextItemWidth(-1);
		if (ImGui::BeginCombo("##AudioTargetEntity", targetLabel.c_str()))
		{
			std::vector<ECS::Entity> namedEntities = entityManager.findEntity<ECSComponents::NameComponent>();
			for (ECS::Entity& e : namedEntities)
			{
				const auto& nc = e.get<ECSComponents::NameComponent>(entityManager);
				std::string label = std::to_string(e.id()) + ": " + nc.name;

				// Show indicator if entity already has audio attached
				if (e.has<ECSComponents::AudioComponent>(entityManager))
				{
					auto& ac = e.get<ECSComponents::AudioComponent>(entityManager);
					if (strlen(ac.audioMod_.name_) > 0)
						label += " [audio: " + std::string(ac.audioMod_.name_) + "]";
				}

				bool isSelected = (audioTargetEntityID == e.id());
				if (ImGui::Selectable(label.c_str(), isSelected))
				{
					audioTargetEntityID = e.id();
				}
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		// Validate target entity
		bool validTarget = false;
		{
			auto& map = entityManager.getMap();
			validTarget = (map.find(audioTargetEntityID) != map.end());
		}

		if (!validTarget)
		{
			ImGui::TextDisabled("Select a valid target entity above.");
		}
		else
		{
			// Channel Group Selection
			static int selectedChannelGroup = 1; // 0=Master, 1=SFX, 2=BGM
			const char* channelGroups[] = { "Master", "SFX", "BGM" };

			ImGui::Text("Channel Group:");
			ImGui::Combo("##ChannelGroup", &selectedChannelGroup, channelGroups, IM_ARRAYSIZE(channelGroups));

			if (ImGui::Button("Attach Audio"))
			{
				ECS::Entity targetEntity{ audioTargetEntityID };

				// Add component if missing
				if (!targetEntity.has<ECSComponents::AudioComponent>(entityManager)) {
					targetEntity.add<ECSComponents::AudioComponent>(entityManager);
					std::cout << "[Audio] Added AudioComponent.\n";
				}

				// Fill it
				ECSComponents::AudioComponent& ac = targetEntity.get<ECSComponents::AudioComponent>(entityManager);

				strncpy(ac.audioMod_.name_, selectedName.c_str(), sizeof(ac.audioMod_.name_) - 1);
				strncpy(ac.audioMod_.chGroup_, channelGroups[selectedChannelGroup], sizeof(ac.audioMod_.chGroup_) - 1);

				ac.audioMod_.sound_ = selectedSound;

				// Build status message
				audioAttachStatusMsg = "Attached '" + selectedName + "' to " + targetLabel;
				audioAttachStatusTimer = 3.0f; // show for ~3 seconds

				std::cout << "[Audio] Attached '" << selectedName
					<< "' to entity " << audioTargetEntityID
					<< " with channel group '" << channelGroups[selectedChannelGroup] << "'\n";
			}
		}

		// Show attach confirmation message (fades out)
		if (audioAttachStatusTimer > 0.0f)
		{
			float alpha = (audioAttachStatusTimer > 1.0f) ? 1.0f : audioAttachStatusTimer;
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.3f, alpha), "%s", audioAttachStatusMsg.c_str());
			audioAttachStatusTimer -= ImGui::GetIO().DeltaTime;
		}

		//Current state of audio deletion takes out the audio file from the build folder only.
		ImGui::Text("-----------DELETION OF AUDIO FILES-------------");

		if (ImGui::Button("Delete selected audio file"))
		{
			ImGui::OpenPopup("ConfirmDeleteAudio");
		}

		if (ImGui::BeginPopupModal("ConfirmDeleteAudio", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::TextWrapped("Delete this file?\n\n%s", selectedFilenameWithExt.c_str());
			ImGui::Spacing();

			if (ImGui::Button("Delete", ImVec2(120, 0)))
			{
				// Stop playback that might be using this sound
				//bus.publish(Audio::StopBGM{});

				// Release FMOD sound before deleting file (important on Windows)
				FMOD::Sound* soundToDelete = selectedSound;
				if (soundToDelete)
				{
					soundToDelete->release();
					selectedSound = nullptr;
				}

				{
					auto& sounds = loader.getSoundsMap();
					auto it2 = sounds.find(selectedName);
					if (it2 != sounds.end())
						sounds.erase(it2);
				}

				// Clear the audio component on any entity that was referencing the deleted sound
				{
					auto& entityMgr = engine_.getEntityManager();
					for (ECS::Entity& ent : entityMgr.findEntity<ECSComponents::AudioComponent>())
					{
						auto& ac = ent.get<ECSComponents::AudioComponent>(entityMgr);
						if (std::string(ac.audioMod_.name_) == selectedName)
						{
							if (ac.isPlaying_)
							{
								engine_.getAudioManager().stopChannelGroup(ac.audioMod_.chGroup_);
								ac.isPlaying_ = false;
							}
							ac.audioMod_.name_[0] = '\0';
							ac.audioMod_.path_[0] = '\0';
							ac.audioMod_.sound_   = nullptr;
							ac.audioMod_.ch_      = nullptr;
						}
					}
				}

				// Delete the file from disk
				bool deleted = false;
				bool deletedRoot = false;
				std::string deleteErr;

				try
				{
					//std::cout << "\n=== AUDIO FILE DELETION ATTEMPT ===\n";
					//std::cout << "[Audio Delete] File to delete: " << selectedFilenameWithExt << "\n";
					//std::cout << "[Audio Delete] BUILD path: " << (selectedFullPath.empty() ? "(empty)" : selectedFullPath.string()) << "\n";
					//std::cout << "[Audio Delete] ROOT path: " << (selectedFullRootPath.empty() ? "(empty)" : selectedFullRootPath.string()) << "\n";

					// Delete from BUILD folder
					if (!selectedFullPath.empty() && std::filesystem::exists(selectedFullPath))
					{
						//std::cout << "[Audio Delete] Attempting to delete from BUILD folder...\n";
						deleted = std::filesystem::remove(selectedFullPath);
						//std::cout << "[Audio Delete] BUILD delete result: " << (deleted ? "SUCCESS" : "FAILED") << "\n";
					}
					else
					{
						//std::cout << "[Audio Delete] BUILD file not found or path empty.\n";
						deleteErr = "File not found in build folder.";
					}

					// Delete from ROOT folder
					if (!selectedFullRootPath.empty() && std::filesystem::exists(selectedFullRootPath))
					{
						//std::cout << "[Audio Delete] Attempting to delete from ROOT folder...\n";
						deletedRoot = std::filesystem::remove(selectedFullRootPath);
						//std::cout << "[Audio Delete] ROOT delete result: " << (deletedRoot ? "SUCCESS" : "FAILED") << "\n";
					}
					else
					{
						//std::cout << "[Audio Delete] ROOT file not found or path empty.\n";
					}

					// If neither deleted anything
					if (!deleted && !deletedRoot)
					{
						deleteErr = "File not found in build or root folders (or delete returned false).";
					}

				}
				catch (const std::exception& e)
				{
					deleteErr = e.what();
				}

				if (!deleted && deleteErr.empty())
					deleteErr = "Delete failed (file may still be in use, or permissions blocked it).";

				if (!deleteErr.empty())
				{
					bus.publish(Audio::AudioLoadError{
						selectedFilenameWithExt,
						selectedFullPath.string(),
						selectedFullPath.extension().string(),
						std::string("Delete failed: ") + deleteErr
						});
				}
				else
				{
					//std::cout << "[Audio] Deleted file: " << selectedFullPath << "\n";
				}

				// Refresh UI list
				LoadAudioAssets(engine_);

				// Clear selection so UI doesn’t reference deleted item
				selectedName.clear();
				selectedFilenameWithExt.clear();
				selectedFullPath.clear();
				selectedFullRootPath.clear();
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

}




void displayEntityInspector(Engine::EnoEngine& engine_, ECS::Entity& entity, Editor::UndoManager& undoManager, Editor::EnoSceneTree& sceneTree)
{
	auto& entMng = engine_.getEntityManager();

	// Note: Entity ID 0 is valid, so don't early return based on ID
	ImGui::Begin("Entity Inspector", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Text("Entity ID: %u", entity.id());

	// Debug output to console when inspector opens
	static uint32_t lastInspectedID = UINT32_MAX;
	if (lastInspectedID != entity.id()) {
		lastInspectedID = entity.id();
	}

	// --- NameComponent ---
	if (entity.has<ECSComponents::NameComponent>(entMng)) {
		auto& nameComp = entity.get<ECSComponents::NameComponent>(entMng);
		ImGui::SeparatorText("Name");
		ImGui::InputText("##Name", nameComp.name, sizeof(nameComp.name));
	}
	else
	{	
		// Add name component
		ImGui::SeparatorText("Name Component");
		ImGui::TextDisabled("No name component attached");

		if (ImGui::Button("Add name Component"))
		{
			ECSComponents::NameComponent nc;

			entity.add<ECSComponents::NameComponent>(entMng, nc);
		}
	}

	// --- Script ---
	auto& smEditor = g_game->getStateMachineEditor();

	Node* node = sceneTree.getNodeByEntityID(entity.id());
	if (node) {
		ImGui::SeparatorText("Script");
		smEditor.renderScriptSection(node);
	}

	// --- StateMachine ---
	if (entity.has<ECSComponents::StateMachineComponent>(entMng)) {
		auto& smc = entity.get<ECSComponents::StateMachineComponent>(entMng);
		ImGui::SeparatorText("State Machine");

		Node* node = sceneTree.getNodeByEntityID(entity.id());
		smEditor.renderStateMachineUI(node, smc);
	}
	else {
		ImGui::SeparatorText("State Machine");
		ImGui::TextDisabled("No State Machine attached");

		// Allow adding state machine component if the entity doesn't have one
		if (ImGui::Button("Add State Machine")) {
			entity.add<ECSComponents::StateMachineComponent>(entMng);

			Node* node = sceneTree.getNodeByEntityID(entity.id());
			if (node && node->script_) {
				std::string scriptPath = node->script_->path();
				auto scriptStates = Script::StateMachineScriptGenerator::extractStatesFromScript(scriptPath);
				auto& smComp = node->entity_.get<ECSComponents::StateMachineComponent>(entMng);
				for (const auto& state : scriptStates) {
					smComp.addState(state.c_str());
					if (smComp.numStates == 1) {
						strncpy(smComp.currentState, state.c_str(), sizeof(smComp.currentState) - 1);
						smComp.currentState[sizeof(smComp.currentState) - 1] = '\0';
					}
				}
			}
		}
	}

	// --- AIComponent ---
	if (entity.has<ECSComponents::AIComponent>(entMng)) {
		auto& ai = entity.get<ECSComponents::AIComponent>(entMng);
		ImGui::SeparatorText("AI");

		// ----------------------------
		// Core AI Behavior Params
		// ----------------------------
		ImGui::DragFloat("Awareness Radius", &ai.awarenessRadius, 1.f, 1.f, 2000.f);
		ImGui::DragFloat("Chase Speed", &ai.chaseSpeed, 1.f, 0.f, 1000.f);
		ImGui::Checkbox("Diagonal Movement", &ai.allowDiagonal);

		ImGui::DragFloat("Attack Range", &ai.attackRange, 1.f, 1.f, 500.f);
		ImGui::DragFloat("Windup Time", &ai.windupTime, 0.01f, 0.f, 5.f);
		ImGui::DragFloat("Attack Duration", &ai.attackDuration, 0.01f, 0.f, 5.f);
		ImGui::DragFloat("Hurt Duration", &ai.hurtDuration, 0.01f, 0.f, 5.f);

		ImGui::DragInt("Health", &ai.health, 1, 0, 9999);
	}
	else {
		// Allow adding AI component if the entity doesn't have one
		ImGui::SeparatorText("AI");
		ImGui::TextDisabled("No AI attached");

		if (ImGui::Button("Add AI")) {
			// Add a default AI component
			entity.add<ECSComponents::AIComponent>(entMng);
		}
	}

	// ---------- Visibility Component ---------
	if (entity.has<ECSComponents::VisibilityComponent>(entMng))
	{
		auto& vi = entity.get<ECSComponents::VisibilityComponent>(entMng);

		ImGui::SeparatorText("Visibility Component");

		// Toggle checkbox for visibility (children inherit at render time via ancestor check)
		if (ImGui::Checkbox("Entity Visibility", &vi.visible)) {
			if (g_game) g_game->markEntitiesDirty();
		}

		// Header with +/- buttons to adjust entity's layer
		ImGui::Text("Z Index Layer");
		ImGui::SameLine();

		int currentLayer = static_cast<int>(vi.getZIndex());

		// Plus button to increase entity's layer
		if (currentLayer < ECSComponents::availableLayers - 1)
		{
			if (ImGui::Button("+"))
			{
				vi.setZIndex(static_cast<uint32_t>(currentLayer + 1));
				if (g_game) g_game->markEntitiesDirty();
			}
		}
		else
		{
			// Disable button if it is at top layer
			ImGui::BeginDisabled();
			ImGui::Button("+");
			ImGui::EndDisabled();
		}

		ImGui::SameLine();

		// Minus button to decrease entity's layer
		if (currentLayer > 0)
		{
			if (ImGui::Button("-"))
			{
				vi.setZIndex(static_cast<uint32_t>(currentLayer - 1));
				if (g_game) g_game->markEntitiesDirty();
			}
		}
		else
		{
			// Disable button if it is at bottom layer
			ImGui::BeginDisabled();
			ImGui::Button("-");
			ImGui::EndDisabled();
		}

		// Build layer names dynamically based on available layers
		std::vector<std::string> layerStrings;
		for (int i = 0; i < ECSComponents::availableLayers; ++i)
		{
			std::string label = std::to_string(i);
			if (i == 0) label += " (Background)";
			else if (i == 3) label += " (Default)";
			else if (i == ECSComponents::availableLayers - 1 && i > 3) label += " (Top)";
			layerStrings.push_back(label);
		}

		// Convert to const char* array for ImGui
		std::vector<const char*> layerNames;
		for (const auto& str : layerStrings)
			layerNames.push_back(str.c_str());

		// Clamp to valid range for display
		int displayLayer = currentLayer;
		if (displayLayer >= ECSComponents::availableLayers) displayLayer = ECSComponents::availableLayers - 1;
		if (displayLayer < 0) displayLayer = 0;

		// Dropdown for layer selection (## hides the label since we have our own header)
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 30.0f);  // Leave room for checkbox
		if (ImGui::Combo("##ZIndexLayer", &displayLayer, layerNames.data(), static_cast<int>(layerNames.size())))
		{
			vi.setZIndex(static_cast<uint32_t>(displayLayer));
			if (g_game) g_game->markEntitiesDirty();
		}

		// Layer visibility checkbox (toggles visibility of the currently selected layer)
		ImGui::SameLine();
		bool layerVisible = ECSComponents::isLayerVisible(displayLayer);
		if (ImGui::Checkbox("##LayerVisible", &layerVisible))
		{
			ECSComponents::setLayerVisible(displayLayer, layerVisible);
			if (g_game) g_game->markEntitiesDirty();
		}
		// Display discription text
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Toggle visibility for layer %d", displayLayer);
		}

		// Check if entity is currently on the top layer (for layer management)
		int currentTopLayer = ECSComponents::availableLayers - 1;
		bool isOnTopLayer = (currentLayer == currentTopLayer);

		// Add New Layer button
		if (ECSComponents::availableLayers < ECSComponents::MAX_LAYERS)
		{
			if (ImGui::Button("Add New Layer"))
			{
				// If entity is on top layer, move it to the new top
				if (isOnTopLayer)
				{
					vi.setZIndex(static_cast<uint32_t>(ECSComponents::availableLayers));
					if (g_game) g_game->markEntitiesDirty();
				}
				ECSComponents::availableLayers++;
			}
		}
		else
		{	// Disable if number of layers reached max
			ImGui::BeginDisabled();
			ImGui::Button("Add New Layer");
			ImGui::EndDisabled();
		}

		ImGui::SameLine();

		// Remove Top Layer button
		if (ECSComponents::availableLayers > 1)
		{
			if (ImGui::Button("Remove Top Layer"))
			{
				ECSComponents::availableLayers--;
				// If entity was on or above the new top layer, clamp it to new top
				if (currentLayer >= ECSComponents::availableLayers)
				{
					vi.setZIndex(static_cast<uint32_t>(ECSComponents::availableLayers - 1));
					if (g_game) g_game->markEntitiesDirty();
				}
			}
		}
		else
		{
			// Disable if number of layers reached minimum
			ImGui::BeginDisabled();
			ImGui::Button("Remove Top Layer");
			ImGui::EndDisabled();
		}

		ImGui::SameLine();
		ImGui::TextDisabled("(%d/%d)", ECSComponents::availableLayers, ECSComponents::MAX_LAYERS);

		// Keyboard shortcuts hint
		ImGui::TextDisabled("Use '[' and ']' to change layer");
	}
	else
	{
		// Allow adding visibility component
		ImGui::SeparatorText("Visibility Component");
		ImGui::TextDisabled("No visibility component attached");

		if (ImGui::Button("Add visibility Component"))
		{
			ECSComponents::VisibilityComponent vi;

			entity.add<ECSComponents::VisibilityComponent>(entMng, vi);
			if (g_game) g_game->markEntitiesDirty();
		}

	}

	// --- TransformComponent ---
	if (entity.has<ECSComponents::TransformComponent>(entMng)) {
		auto& transform = entity.get<ECSComponents::TransformComponent>(entMng);
		ImGui::SeparatorText("Transform");

		// Track position editing for undo
		static Math::eVec2<float> oldPosInspector;
		static bool editingPosInspector = false;

		if (ImGui::IsItemActive() && !editingPosInspector) {
			oldPosInspector = transform.pos;
			editingPosInspector = true;
		}

		ImGui::DragFloat2("Position", (float*)&transform.pos, 0.5f);

		if (editingPosInspector && !ImGui::IsItemActive()) {
			// Record undo when user finishes editing
			if (oldPosInspector.x != transform.pos.x || oldPosInspector.y != transform.pos.y) {
				undoManager.recordMove(entity, oldPosInspector, transform.pos);
			}
			editingPosInspector = false;
		}

		// Scale removed - doesn't do anything currently
	}

	

	// --- DrawInfoComponent ---
	if (entity.has<ECSComponents::DrawInfoComponent>(entMng)) {
		auto& draw = entity.get<ECSComponents::DrawInfoComponent>(entMng);

		ImGui::SeparatorText("Draw Info");

		// Track if width/height changed to auto-update collision
		float oldWidth = draw.width;
		float oldHeight = draw.height;

		
		// Allow independent width/height scaling
		ImGui::DragFloat("Width", &draw.width, 0.5f, 1.0f, 500.0f);
		ImGui::DragFloat("Height", &draw.height, 0.5f, 1.0f, 500.0f);

		// Scale multiplier for both width and height
		static std::unordered_map<uint32_t, float> entityScales;
		static std::unordered_map<uint32_t, std::pair<float, float>> entityBaseSizes;

		uint32_t entityID = entity.id();

		// Initialize base size and scale if not stored
		if (entityBaseSizes.find(entityID) == entityBaseSizes.end()) {
			entityBaseSizes[entityID] = {draw.width, draw.height};
			entityScales[entityID] = 1.0f;
		}

		float& scale = entityScales[entityID];
		auto& baseSize = entityBaseSizes[entityID];

		if (ImGui::DragFloat("Scale", &scale, 0.01f, 0.1f, 10.0f)) {
			// Apply scale to base size
			draw.width = baseSize.first * scale;
			draw.height = baseSize.second * scale;
		}

		// Update base size if width/height changed directly
		if (oldWidth != draw.width || oldHeight != draw.height) {
			baseSize = {draw.width / scale, draw.height / scale};
		}

		// Auto-update collision when width/height changes
		if ((draw.width != oldWidth || draw.height != oldHeight) && entity.has<ECSComponents::CollisionComponent>(entMng)) {

			auto& collision = entity.get<ECSComponents::CollisionComponent>(entMng);

			if (strcmp(draw.shape, "Circle") == 0) {
				// Update circle radius
				collision.data.circle.radius = draw.width * 0.5f;
			}
			else {
				// Update rectangle dimensions
				collision.data.rectangle.width = draw.width;
				collision.data.rectangle.height = draw.height;
			}
		}

		// Rotation field (edits TransformComponent rotation)
		if (entity.has<ECSComponents::TransformComponent>(entMng)) {
			auto& transform = entity.get<ECSComponents::TransformComponent>(entMng);

			// Track rotation editing for undo
			static Math::eVec2<float> oldRotInspector;
			static bool editingRotInspector = false;

			if (ImGui::IsItemActive() && !editingRotInspector) {
				oldRotInspector = transform.rot;
				editingRotInspector = true;
			}

			// Convert radians to degrees for user-friendly editing
			float rotationDegrees = transform.rot.x * (180.0f / 3.14159265f);
			if (ImGui::DragFloat("Rotation", &rotationDegrees, 1.0f, -360.0f, 360.0f, "%.1f deg")) {
				// Convert degrees back to radians
				transform.rot.x = rotationDegrees * (3.14159265f / 180.0f);

				if (g_game) g_game->markEntitiesDirty();
			}

			if (editingRotInspector && !ImGui::IsItemActive()) {
				// Record undo when user finishes editing
				if (oldRotInspector.x != transform.rot.x || oldRotInspector.y != transform.rot.y) {
					undoManager.recordRotation(entity, oldRotInspector, transform.rot);
				}
				editingRotInspector = false;
			}
		}

		// Shape type selector (Circle or Rectangle)
		const char* shapeTypes[] = { "Circle", "Rectangle" };
		int currentShapeIndex = (strcmp(draw.shape, "Circle") == 0) ? 0 : 1;
		if (ImGui::Combo("Shape", &currentShapeIndex, shapeTypes, IM_ARRAYSIZE(shapeTypes))) {
			strncpy_s(draw.shape, shapeTypes[currentShapeIndex], sizeof(draw.shape) - 1);
			draw.shape[sizeof(draw.shape) - 1] = '\0';

			// Auto-update collision component to match new shape
			if (entity.has<ECSComponents::CollisionComponent>(entMng)) {
				auto& collision = entity.get<ECSComponents::CollisionComponent>(entMng);

				if (strcmp(draw.shape, "Circle") == 0) {
					// Changed to circle - auto-calculate circle collider
					collision.type = ECSComponents::ColliderType::CIRCLE;
					collision.data.circle.radius = draw.width * 0.5f;
				}
				else {
					// Changed to Rectangle - auto-calculate rectangle collider
					collision.type = ECSComponents::ColliderType::RECTANGLE;
					collision.data.rectangle.width = draw.width;
					collision.data.rectangle.height = draw.height; 
				}
			}
			if (g_game) g_game->markEntitiesDirty();
		}

		// Color selector (Red or Green)
		const char* colorTypes[] = { "Green", "Red" };
		int currentColorIndex = 0;
		if (draw.color.r == 1.0f && draw.color.g == 0.0f && draw.color.b == 0.0f) {
			currentColorIndex = 1; // Red
		}
		else if (draw.color.r == 0.0f && draw.color.g == 1.0f && draw.color.b == 0.0f) {
			currentColorIndex = 0; // Green
		}

		if (ImGui::Combo("Color", &currentColorIndex, colorTypes, IM_ARRAYSIZE(colorTypes))) {
			if (currentColorIndex == 0) {
				draw.color = Graphics::Color::Green;
			}
			else {
				draw.color = Graphics::Color::Red;
			}
			if (g_game) g_game->markEntitiesDirty();
		}

		// Display color preview
		ImVec4 colorVec(draw.color.r, draw.color.g, draw.color.b, draw.color.a);
		ImGui::ColorButton("##color", colorVec, 0, ImVec2(20, 20));

		// Display texture info if present
		if (draw.texture != nullptr && draw.texture->isValid()) {
			ImGui::Text("Texture ID: %d", draw.texID);
			ImGui::Image((void*)(intptr_t)draw.texture->getID(), ImVec2(64, 64));
		}
		else {
			ImGui::TextDisabled("No texture assigned");
		}
	}

	// --- PhysicsComponent --- (skip for player)
	if (entity.has<ECSComponents::PhysicsComponent>(entMng)) {
		auto& physics = entity.get<ECSComponents::PhysicsComponent>(entMng);
		ImGui::SeparatorText("Physics");
		ImGui::DragFloat2("Velocity", (float*)&physics.velocity, 0.5f);
		ImGui::DragFloat("Mass", &physics.mass, 0.1f, 0.1f, 100.0f);
		ImGui::DragFloat("Drag", &physics.drag, 0.01f, 0.0f, 1.0f);
		ImGui::Checkbox("Use Gravity", &physics.useGravity);
	}

	// --- CollisionComponent ---
	if (entity.has<ECSComponents::CollisionComponent>(entMng)) {
		auto& collider = entity.get<ECSComponents::CollisionComponent>(entMng);
		ImGui::SeparatorText("Collider");

		// Show Collider checkbox
		if (ImGui::Checkbox("Show Collider", &showColliders_)) {
			// Toggle visibility immediately
		}

		// Collider type selector (only Circle and Rectangle)
		const char* colliderTypes[] = { "Circle", "Rectangle" };

		// Map the current type to the combo index (0 = Circle, 1 = Rectangle)
		int comboIndex = (collider.type == ECSComponents::ColliderType::CIRCLE) ? 0 : 1;

		if (ImGui::Combo("Type", &comboIndex, colliderTypes, IM_ARRAYSIZE(colliderTypes))) {
			ECSComponents::ColliderType newType = (comboIndex == 0) ?
				ECSComponents::ColliderType::CIRCLE : ECSComponents::ColliderType::RECTANGLE;

			// Only change if it's a different type
			if (newType != collider.type) {
				collider.type = newType;

				// Initialize default values for the new collider type
				if (newType == ECSComponents::ColliderType::CIRCLE) {
					collider.data.circle.radius = 10.0f;
				}
				else if (newType == ECSComponents::ColliderType::RECTANGLE) {
					collider.data.rectangle.width = 20.0f;
					collider.data.rectangle.height = 20.0f;
				}

				if (g_game) g_game->markEntitiesDirty();
			}
		}

		// Display and edit collider dimensions based on type
		if (collider.type == ECSComponents::ColliderType::CIRCLE) {
			if (ImGui::DragFloat("Radius", &collider.data.circle.radius, 0.5f, 1.0f, 200.0f)) {
				if (g_game) g_game->markEntitiesDirty();
			}
		}
		else if (collider.type == ECSComponents::ColliderType::RECTANGLE) {
			if (ImGui::DragFloat("Width##collider", &collider.data.rectangle.width, 0.5f, 1.0f, 500.0f)) {
				if (g_game) g_game->markEntitiesDirty();
			}
			if (ImGui::DragFloat("Height##collider", &collider.data.rectangle.height, 0.5f, 1.0f, 500.0f)) {
				if (g_game) g_game->markEntitiesDirty();
			}

			// Show auto-calculated dimensions if entity has RenderComponent
			if (entity.has<ECSComponents::RenderComponent>(entMng)) {
				auto& render = entity.get<ECSComponents::RenderComponent>(entMng);
				ImGui::TextDisabled("Auto : %.1f x %.1f", render.width, render.height);

				if (ImGui::Button("Reset to Render Size")) {
					collider.data.rectangle.width = render.width;
					collider.data.rectangle.height = render.height;
					if (g_game) g_game->markEntitiesDirty();
				}
			}
		}

		// Offset controls
		ImGui::Spacing();
		ImGui::Text("Offset:");
		if (ImGui::DragFloat("X##offset", &collider.offset.x, 0.5f, -200.0f, 200.0f)) {
			if (g_game) g_game->markEntitiesDirty();
		}
		if (ImGui::DragFloat("Y##offset", &collider.offset.y, 0.5f, -200.0f, 200.0f)) {
			if (g_game) g_game->markEntitiesDirty();
		}

		// Active and Trigger flags
		ImGui::Spacing();
		if (ImGui::Checkbox("Is Active", &collider.isActive)) {
			if (g_game) g_game->markEntitiesDirty();
		}
		ImGui::SameLine();
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Enable/disable collision detection for this collider");
		}

		if (ImGui::Checkbox("Is Trigger", &collider.isTrigger)) {
			if (g_game) g_game->markEntitiesDirty();
		}
		ImGui::SameLine();
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Trigger detects collisions but doesn't block movement.\nRequires 'Is Active' to be checked.");
		}

		ImGui::Spacing();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - 60.0f);
		if (ImGui::Button("Remove##collider")) {
			entity.remove<ECSComponents::CollisionComponent>(entMng);
			if (g_game) g_game->markEntitiesDirty();
		}
	}
	else {
		// Allow adding a collider if the entity doesn't have one
		ImGui::SeparatorText("Collider");
		ImGui::TextDisabled("No collider attached");

		if (ImGui::Button("Add Collider")) {
			if (entity.has<ECSComponents::RenderComponent>(entMng)) {
				auto& render = entity.get<ECSComponents::RenderComponent>(entMng);
				auto collision = ECSComponents::CreateAABBFromRender(render);
				if (!entity.has<ECSComponents::CollisionComponent>(entMng)) {
					std::cout << "attempting to add collision component\n";
					entity.add<ECSComponents::CollisionComponent>(entMng, collision);
				
				}
			}
			else {
				entity.add<ECSComponents::CollisionComponent>(entMng, ECSComponents::CollisionComponent(10.0f));
			}
			if (g_game) g_game->markEntitiesDirty();
		}

		if (entity.has<ECSComponents::RenderComponent>(entMng)) {
			auto& render = entity.get<ECSComponents::RenderComponent>(entMng);
			ImGui::TextDisabled("Will auto-size: %.1f x %.1f", render.width, render.height);
		}
	}
	if (entity.has<ECSComponents::BoxComponent>(entMng)) {
		auto& pushComp = entity.get<ECSComponents::BoxComponent>(entMng);
		ImGui::SeparatorText("Box (Pushable)");
		if (ImGui::Checkbox("Is Pushable", &pushComp.isPushable)) {
			if (g_game) g_game->markEntitiesDirty();
		}
		ImGui::DragFloat("Push Resistance", &pushComp.pushResistance, 0.01f, 0.0f, 1.0f);
	}
#if 0
	else {
		// Allow adding Box component if the entity doesn't have one
		ImGui::SeparatorText("Box (Pushable)");
		ImGui::TextDisabled("No Box component attached");

		if (ImGui::Button("Add Box Component")) {
			entity.add<ECSComponents::BoxComponent>(entMng);
			if (g_game) g_game->markEntitiesDirty();
		}
	}
#endif
	

	//for dropdown selection of Looping menu.
	const char* loopOptions[] = { "Disabled","Forward","PingPong" };

	if (entity.has<ECSComponents::AudioComponent>(entMng))
	{
		auto& ac = entity.get<ECSComponents::AudioComponent>(entMng);

		// Main Audio Section
		ImGui::SeparatorText("Audio Component");

		// Audio Info with playback indicator
		ImGui::Text("Sound: %s", strlen(ac.audioMod_.name_) > 0 ? ac.audioMod_.name_ : "(No audio attached)");

		// Playback state indicator
		if (ac.isPlaying_) {
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[PLAYING]");
		}
		else if (ac.isAutoplay_) {
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[AUTOPLAY]");
		}

		// Channel Group Selection
		ImGui::Text("Channel Group: %s", ac.audioMod_.chGroup_);
		const char* channelGroupOptions[] = { "Master", "SFX", "BGM" };
		static int currentChannelGroup = 1;

		// Find current channel group index
		for (int i = 0; i < IM_ARRAYSIZE(channelGroupOptions); i++) {
			if (strcmp(ac.audioMod_.chGroup_, channelGroupOptions[i]) == 0) {
				currentChannelGroup = i;
				break;
			}
		}

		if (ImGui::Combo("##ChangeChannelGroup", &currentChannelGroup, channelGroupOptions, IM_ARRAYSIZE(channelGroupOptions))) {
			strncpy(ac.audioMod_.chGroup_, channelGroupOptions[currentChannelGroup], sizeof(ac.audioMod_.chGroup_) - 1);
			ac.audioMod_.chGroup_[sizeof(ac.audioMod_.chGroup_) - 1] = '\0';
		}

		ImGui::Spacing();
		ImGui::Separator();

		// Playback Controls
		if (ImGui::TreeNodeEx("Playback Controls", ImGuiTreeNodeFlags_DefaultOpen))
		{
			// Play/Pause/Stop buttons in a row
			if (ImGui::Button("Play", ImVec2(80, 0))) {
				// If audio is already playing, stop it first to restart from beginning
				if (ac.isPlaying_) {
					engine_.getAudioManager().stopChannelGroup(ac.audioMod_.chGroup_);
					ac.isPlaying_ = false;
				}

				// Use the proper play function that applies volume/pitch/pan from the component
				engine_.getAudioManager().play(ac);
				std::cout << "[Audio] Playing: " << ac.audioMod_.name_
					<< " (Vol:" << ac.volume_ << ", Pitch:" << ac.pitchScale_ << ", Pan:" << ac.panScale_ << ")\n";
			}

			ImGui::SameLine();
			if (ImGui::Button("Stop", ImVec2(80, 0))) {
				// Stop the channel group associated with this entity's audio
				engine_.getAudioManager().stopChannelGroup(ac.audioMod_.chGroup_);
				ac.isPlaying_ = false;
				std::cout << "[Audio] Stopped: " << ac.audioMod_.name_ << "\n";
			}

			ImGui::Checkbox("Autoplay on Start", &ac.isAutoplay_);
			ImGui::TreePop();
		}

		ImGui::Separator();

		// Stream Properties
		if (ImGui::TreeNodeEx("Stream Properties", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Text("Format:");
			ImGui::SameLine(160);
			ImGui::TextDisabled("16-Bit");

			int loopMode = static_cast<int>(ac.audioMod_.loop_);
			ImGui::Text("Loop Mode:");
			ImGui::SameLine(160);
			if (ImGui::Combo("##LoopMode", &loopMode, loopOptions, IM_ARRAYSIZE(loopOptions)))
			{
				ac.audioMod_.loop_ = static_cast<Audio::EnoAudioModule::LoopMode>(loopMode);
			}

			int loopBegin = static_cast<int>(ac.audioMod_.loopStart_);
			ImGui::Text("Loop Begin:");
			ImGui::SameLine(160);
			if (ImGui::InputInt("##LoopBegin", &loopBegin))
			{
				ac.audioMod_.loopStart_ = static_cast<float>(loopBegin);
			}

			int loopEnd = static_cast<int>(ac.audioMod_.loopEnd_);
			ImGui::Text("Loop End:");
			ImGui::SameLine(160);
			if (ImGui::InputInt("##LoopEnd", &loopEnd))
			{
				ac.audioMod_.loopEnd_ = static_cast<float>(loopEnd);
			}

			ImGui::Text("Mix Rate:");
			ImGui::SameLine(160);
			ImGui::TextDisabled("44100");

			ImGui::Text("Stereo:");
			ImGui::SameLine(160);
			static bool stereoToggle = true;
			ImGui::Checkbox("##StereoChk", &stereoToggle);

			ImGui::TreePop();
		}

		ImGui::Separator();

		// Basic Audio Parameters
		if (ImGui::TreeNodeEx("Audio Parameters", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Text("Volume:");
			ImGui::SameLine(160);
			float tmpVol = static_cast<float>(ac.volume_);
			if (ImGui::SliderFloat("##Volume", &tmpVol, 0.f, 100.f, "%.1f")) {
				ac.volume_ = static_cast<double>(tmpVol);
			}

			ImGui::Text("Pitch Scale:");
			ImGui::SameLine(160);
			float tmppitch = static_cast<float>(ac.pitchScale_);
			if (ImGui::SliderFloat("##PitchScale", &tmppitch, 0.1f, 5.f, "%.2f")) {
				ac.pitchScale_ = static_cast<double>(tmppitch);
			}

			ImGui::Text("Pan:");
			ImGui::SameLine(160);
			float pan = static_cast<float>(ac.panScale_);
			if (ImGui::SliderFloat("##Pan", &pan, -1.f, 1.f, "%.2f"))
			{
				ac.panScale_ = static_cast<double>(pan);
			}

			ImGui::TreePop();
		}

		ImGui::Separator();

		// Advanced Effects - Reverb (now per-entity!)
		if (ImGui::TreeNodeEx("Reverb Effects", ImGuiTreeNodeFlags_None))
		{
			ImGui::Checkbox("Enable Reverb", &ac.enableReverb_);

			if (ac.enableReverb_) {
				ImGui::Text("Wet Level:");
				ImGui::SameLine(160);
				ImGui::SliderFloat("##ReverbWet", &ac.reverbWet_, 0.0f, 1.0f, "%.2f");

				ImGui::Text("Dry Level:");
				ImGui::SameLine(160);
				ImGui::SliderFloat("##ReverbDry", &ac.reverbDry_, 0.0f, 1.0f, "%.2f");

				ImGui::Text("Decay Time:");
				ImGui::SameLine(160);
				ImGui::SliderFloat("##ReverbDecay", &ac.reverbDecay_, 100.0f, 5000.0f, "%.0f ms");

				ImGui::TextDisabled("Note: Reverb effects are applied via Audio Manager");
			}

			ImGui::TreePop();
		}

		ImGui::Separator();

		// Advanced Effects - EQ (now per-entity!)
		if (ImGui::TreeNodeEx("EQ Effects", ImGuiTreeNodeFlags_None))
		{
			ImGui::Checkbox("Enable 3-Band EQ", &ac.enableEQ_);

			if (ac.enableEQ_) {
				ImGui::Text("Low Gain:");
				ImGui::SameLine(160);
				ImGui::SliderFloat("##EQLow", &ac.eqLowGain_, -30.0f, 30.0f, "%.1f dB");

				ImGui::Text("Mid Gain:");
				ImGui::SameLine(160);
				ImGui::SliderFloat("##EQMid", &ac.eqMidGain_, -30.0f, 30.0f, "%.1f dB");

				ImGui::Text("High Gain:");
				ImGui::SameLine(160);
				ImGui::SliderFloat("##EQHigh", &ac.eqHighGain_, -30.0f, 30.0f, "%.1f dB");

				ImGui::TextDisabled("Note: EQ effects are applied via Audio Manager");
			}

			ImGui::TreePop();
		}

		ImGui::Separator();

		// Debug Section (collapsed by default)
		if (ImGui::TreeNodeEx("Debug Info", ImGuiTreeNodeFlags_None))
		{
			if (ImGui::Button("Print Audio Component to Console"))
			{
				std::cout << "===== AUDIO COMPONENT DEBUG =====\n";
				std::cout << "Entity:       " << entity.id() << "\n";
				std::cout << "Volume:       " << ac.volume_ << "\n";
				std::cout << "Pitch Scale:  " << ac.pitchScale_ << "\n";
				std::cout << "Pan Scale:    " << ac.panScale_ << "\n";
				std::cout << "Autoplay:     " << (ac.isAutoplay_ ? "true" : "false") << "\n";
				std::cout << "Is Playing:   " << (ac.isPlaying_ ? "true" : "false") << "\n";
				std::cout << "-- MODULE --\n";
				std::cout << "Name:         " << ac.audioMod_.name_ << "\n";
				std::cout << "Path:         " << ac.audioMod_.path_ << "\n";
				std::cout << "ChannelGroup: " << ac.audioMod_.chGroup_ << "\n";
				std::cout << "Loop Mode:    " << static_cast<int>(ac.audioMod_.loop_) << "\n";
				std::cout << "Loop Start:   " << ac.audioMod_.loopStart_ << "\n";
				std::cout << "Loop End:     " << ac.audioMod_.loopEnd_ << "\n";
				std::cout << "Sound Ptr:    " << ac.audioMod_.sound_ << "\n";
				std::cout << "-- REVERB --\n";
				std::cout << "Enabled:      " << (ac.enableReverb_ ? "true" : "false") << "\n";
				std::cout << "Wet:          " << ac.reverbWet_ << "\n";
				std::cout << "Dry:          " << ac.reverbDry_ << "\n";
				std::cout << "Decay:        " << ac.reverbDecay_ << " ms\n";
				std::cout << "-- EQ --\n";
				std::cout << "Enabled:      " << (ac.enableEQ_ ? "true" : "false") << "\n";
				std::cout << "Low Gain:     " << ac.eqLowGain_ << " dB\n";
				std::cout << "Mid Gain:     " << ac.eqMidGain_ << " dB\n";
				std::cout << "High Gain:    " << ac.eqHighGain_ << " dB\n";
				std::cout << "=================================\n";
			}

			ImGui::TreePop();
		}

	}
	else
	{
		ImGui::SeparatorText("Audio");
		ImGui::TextDisabled("No AudioComponent attached.");

		//if (ImGui::Button("Add Audio Component"))
		//{
		//	entity.add<ECSComponents::AudioComponent>(entMng);
		//	std::cout << "[Audio] Added AudioComponent to entity "
		//		<< entity.id() << "\n";
		//}
	}

	// --------- Camera Component ----------
	if (entity.has<ECSComponents::CameraComponent>(entMng)) {
		auto& camera = entity.get<ECSComponents::CameraComponent>(entMng);
		ImGui::SeparatorText("Camera");

		// Active toggle (unique label to avoid conflicts with CollisionComponent)
		ImGui::Checkbox("Is Active##camera", &camera.isActive);

		// Viewport size
		ImGui::DragFloat("Viewport Width", &camera.viewportWidth, 10.0f, 100.0f, 10000.0f);
		ImGui::DragFloat("Viewport Height", &camera.viewportHeight, 10.0f, 100.0f, 10000.0f);

		// Camera position (center of viewport)
		ImGui::DragFloat2("Camera Position", (float*)&camera.position, 1.0f);
	}
	else {
		// Allow adding camera component
		ImGui::SeparatorText("Camera");
		ImGui::TextDisabled("No camera attached");

		if (ImGui::Button("Add Camera")) {
			ECSComponents::CameraComponent camera;
			camera.viewportWidth = 1920.0f;
			camera.viewportHeight = 1080.0f;
			camera.isActive = false; // Don't activate by default

			// Set position to center of world
			auto& tileMap = engine_.getTileMap();
			Math::eVec2<float> worldSize = tileMap.getWorldSize();
			camera.position = Math::eVec2<float>{ worldSize.x * 0.5f, worldSize.y * 0.5f };

			entity.add<ECSComponents::CameraComponent>(entMng, camera);
			if (g_game) g_game->markEntitiesDirty();
		}
	}

	// --- Text Component ---
	if (entity.has<ECSComponents::TextComponent>(entMng)) {
		auto& textComp = entity.get<ECSComponents::TextComponent>(entMng);
		ImGui::SeparatorText("Text");

		// Auto-size textbox height based on line count (minimum 2 lines)
		int lineCount = 1;
		for (int i = 0; textComp.text[i]; ++i) {
			if (textComp.text[i] == '\n') ++lineCount;
		}
		float textHeight = ImGui::GetTextLineHeightWithSpacing() * (lineCount + 1);
		float minHeight  = ImGui::GetTextLineHeightWithSpacing() * 2.0f;
		ImGui::InputTextMultiline("##TextContent", textComp.text, sizeof(textComp.text),
			ImVec2(-1.0f, std::max(textHeight, minHeight)));

		// Font selector dropdown
		auto fontNames = engine_.getResourceLoader().getFontNames();
		std::vector<const char*> fontNamesCStr;
		fontNamesCStr.reserve(fontNames.size());
		for (const auto& name : fontNames) {
			fontNamesCStr.push_back(name.c_str());
		}
		int currentFontIdx = 0;
		for (int i = 0; i < static_cast<int>(fontNames.size()); ++i) {
			if (fontNames[i] == textComp.fontName) {
				currentFontIdx = i;
				break;
			}
		}
		if (ImGui::Combo("Font", &currentFontIdx, fontNamesCStr.data(), static_cast<int>(fontNamesCStr.size()))) {
			strncpy_s(textComp.fontName, fontNames[currentFontIdx].c_str(), sizeof(textComp.fontName) - 1);
			textComp.fontName[sizeof(textComp.fontName) - 1] = '\0';
		}

		// Font size (scale)
		ImGui::DragFloat("Size", &textComp.fontSize, 0.1f, 0.1f, 10.0f);

		ImGui::Spacing();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - 60.0f);
		if (ImGui::Button("Remove##text")) {
			entity.remove<ECSComponents::TextComponent>(entMng);
			if (g_game) g_game->markEntitiesDirty();
		}
	}
	else {
		ImGui::SeparatorText("Text");
		ImGui::TextDisabled("No TextComponent attached");
		if (ImGui::Button("Add Text Component")) {
			entity.add<ECSComponents::TextComponent>(entMng);
			if (g_game) g_game->markEntitiesDirty();
		}
	}

	// --------- Button Component ----------
	bool hasButtonComponent = entity.has<ECSComponents::ButtonComponent>(entMng);
	if (hasButtonComponent)
	{
		ImGui::SeparatorText("Button Component");
		ImGui::Separator();

		auto& button = entity.get<ECSComponents::ButtonComponent>(entMng);

		// Toggle box for button interactibility
		ImGui::Checkbox("Is Active##button", &button.getActive());
		ImGui::Spacing();

		// Get current node
		Editor::Node* currentNode = sceneTree.getNodeByEntityID(entity.id());

		// Section A: Display Active Signal Connections
		if (currentNode && !currentNode->listeners_.empty()) {
			ImGui::Text("Active Signal Connections:");
			ImGui::Indent();

			// Display each listener connection
			int disconnectIndex = -1;
			for (size_t i = 0; i < currentNode->listeners_.size(); ++i) {
				const auto& listener = currentNode->listeners_[i];

				// Find listener node by sceneObjectID
				Editor::Node* listenerNode = sceneTree.getNodeBySceneObjectID(listener.listenerID_);
				if (!listenerNode) continue;

				// Display format: "onClick -> PlayerNode (PlayerScript.cs)"
				std::string displayText = listener.signal_ + " -> " + listenerNode->name_;
				if (listenerNode->script_) {
					displayText += " (" + Conversion::wideToUtf8(listenerNode->script_->getName()) + ")";
				}

				ImGui::Text("%s", displayText.c_str());
				ImGui::SameLine();

				// Disconnect button
				std::string disconnectLabel = "Disconnect##" + std::to_string(i);
				if (ImGui::Button(disconnectLabel.c_str())) {
					disconnectIndex = static_cast<int>(i);
				}
			}

			// Perform disconnect after loop to avoid iterator invalidation
			if (disconnectIndex >= 0) {
				currentNode->listeners_.erase(currentNode->listeners_.begin() + disconnectIndex);
			}

			ImGui::Unindent();
			ImGui::Separator();
		}

		// Section B: Create New Connection UI
		ImGui::Text("Create New Connection:");
		ImGui::Spacing();

		// Dropdown 1: Target Node (filter nodes with scripts)
		auto allNodes = sceneTree.getAllNodes();
		std::vector<std::string> nodeStrings;
		std::vector<const char*> nodeNames;
		std::vector<Editor::Node*> scriptNodes;  // Track nodes with scripts

		nodeNames.push_back("Select Target Node...");

		for (auto* node : allNodes) {
			// Only include nodes with scripts attached
			if (!node->script_) continue;

			std::string displayName;
			if (node->entity_.has<ECSComponents::NameComponent>(entMng)) {
				auto& nameComp = node->entity_.get<ECSComponents::NameComponent>(entMng);
				displayName = std::string(nameComp.name);
			} else {
				displayName = "Node_" + std::to_string(node->sceneObjectID_);
			}

			// Add script name to display
			displayName += " (" + Conversion::wideToUtf8(node->script_->getName()) + ")";

			nodeStrings.push_back(displayName);
			scriptNodes.push_back(node);
		}

		// Populate const char* array for ImGui
		for (const auto& str : nodeStrings) {
			nodeNames.push_back(str.c_str());
		}

		// Find current selection
		int selectedNodeIndex = 0;
		std::string currentTarget = button.getTargetObject();
		for (size_t idx = 0; idx < nodeNames.size(); idx++) {
			if (!currentTarget.empty() && nodeNames[idx] == currentTarget) {
				selectedNodeIndex = static_cast<int>(idx);
				break;
			}
		}

		ImGui::SetNextItemWidth(250);
		if (ImGui::Combo("Target Node##ButtonTarget", &selectedNodeIndex, nodeNames.data(), static_cast<int>(nodeNames.size()))) {
			if (selectedNodeIndex > 0) {
				button.setTargetObject(nodeNames[selectedNodeIndex]);
			} else {
				button.setTargetObject("");
			}
		}

		// Dropdown 2: Signal Type
		const char* signalTypes[] = { "Select Signal Type...", "onClick", "onHover", "onPressed" };
		int selectedSignalIndex = 0;

		// Find current selection based on selectedSignalType
		std::string currentSignal(button.getSignalType());
		if (currentSignal == "onClick") selectedSignalIndex = 1;
		else if (currentSignal == "onHover") selectedSignalIndex = 2;
		else if (currentSignal == "onPressed") selectedSignalIndex = 3;

		ImGui::SetNextItemWidth(200);
		if (ImGui::Combo("Signal Type##ButtonSignal", &selectedSignalIndex, signalTypes, IM_ARRAYSIZE(signalTypes))) {
			if (selectedSignalIndex > 0) {
				std::strncpy(button.getSignalType(), signalTypes[selectedSignalIndex], 15);
				button.getSignalType()[15] = '\0';
			} else {
				button.getSignalType()[0] = '\0';
			}
		}

		// Connect button (only enabled if both selections valid)
		bool canConnect = (selectedNodeIndex > 0) && (selectedSignalIndex > 0);
		if (!canConnect) {
			ImGui::BeginDisabled();
		}

		if (ImGui::Button("Connect", ImVec2(100, 0))) {
			if (currentNode && selectedNodeIndex > 0) {
				Editor::Node* listenerNode = scriptNodes[selectedNodeIndex - 1];  // -1 for "Select..." offset
				std::string signalName = signalTypes[selectedSignalIndex];

				// Call SceneTree::connect to establish connection
				sceneTree.connect<ECSComponents::ButtonComponent>(
					currentNode,     // emitter (button)
					listenerNode,    // listener (script node)
					signalName       // signal name
				);

				// Clear UI selections after successful connection
				button.setTargetObject("");
				button.getSignalType()[0] = '\0';
			}
		}

		if (!canConnect) {
			ImGui::EndDisabled();
		}

		// Section C: Remove Button
		ImGui::Separator();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - 60.0f);
		if (ImGui::Button("Remove##button")) {
			entity.remove<ECSComponents::ButtonComponent>(entMng);
			if (g_game) g_game->markEntitiesDirty();
		}
	}
	else
	{
		// Allow adding button component
		ImGui::SeparatorText("Button Component");
		ImGui::TextDisabled("No button component attached");

		if (ImGui::Button("Add Button Component"))
		{
			ECSComponents::ButtonComponent button;

			// Check for collider component
			if (!entity.has<ECSComponents::CollisionComponent>(entMng))
			{
				ECSComponents::CollisionComponent collider;
				// Make it non-collidable
				collider.isTrigger = true;
				entity.add<ECSComponents::CollisionComponent>(entMng, collider);
			}

			entity.add<ECSComponents::ButtonComponent>(entMng, button);
			if (g_game) g_game->markEntitiesDirty();
		}
	}

	ImGui::End();
}


// Read states from entity's StateMachineComponent
void assetPresetsTab(Engine::EnoEngine& engine_, ECS::Entity& entity) {
	//Node* entNode = engine_.getEnoEditor().getSceneTree().getNodeByEntityID(entity.id());
	auto& entMng = engine_.getEntityManager();

	// Read states exclusively from StateMachineComponent; fall back to "Default" if empty
	std::vector<std::string> states;
	if (entity.has<ECSComponents::StateMachineComponent>(entMng)) {
		auto& smc = entity.get<ECSComponents::StateMachineComponent>(entMng);
		for (int i = 0; i < smc.numStates; ++i) {
			if (smc.states[i].name[0] != '\0') {
				states.push_back(smc.states[i].name);
			}
		}
		if (states.empty()) {
			smc.addState("Default");
			states.push_back("Default");
		}
	} else {
		states.push_back("Default");
	}
	Resources::EnoLoader& loader = engine_.getResourceLoader();
	const auto& textures = loader.getAllTextures();
	static std::string selected = "";  //currently selected
	//auto& entMng = engine_.getEntityManager();
	//auto& entityScript = engine_.getEnoEditor().getSceneTree().getNodeByEntityID(entity.id())->script_;  //This can be useful later on to find the behaviors/entity states available

	//Dynamically allow player to choose the state that they want to edit.
	static int selectedStateIdx = 0;
	selectedStateIdx = std::clamp(selectedStateIdx, 0, (int)states.size() - 1);
	const std::string& selectedState = states[selectedStateIdx];


	ImGui::Begin("Assets Presets", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	// Check if the selected entity has an AnimationControllerComponent
	if (!entity.has<ECSComponents::AnimationControllerComponent>(entMng))
	{
		ImGui::TextWrapped("The currently selected entity does not have an Animation Controller component.");
		ImGui::End();
		return;
	}


	// New UI: pick which state this "slot" applies to
	ImGui::Text("State");
	if (ImGui::BeginCombo("##StateSelect", selectedState.c_str()))
	{
		for (int i = 0; i < (int)states.size(); ++i)
		{
			bool isSelected = (i == selectedStateIdx);
			if (ImGui::Selectable(states[i].c_str(), isSelected))
				selectedStateIdx = i;
			if (isSelected) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}



	//================================= SELECTED STATE ANIMATION CONTROLLER ===================================
	ImGui::Text("%s", selectedState.c_str());

	static Graphics::TextureData selectedTextureData;
	static int selectedMode = 1;              // 0 = rows/cols, 1 = pixel size
	static int selectedRows = 1;
	static int selectedCols = 1;
	static int selectedFrameW = 96;
	static int selectedFrameH = 96;


	// LEFT SIDE PREVIEW
	if (!selected.empty())
	{
		auto it = textures.find(selected);
		if (it != textures.end())
		{
			auto tex = it->second;   // shared_ptr<EnoTexture>
			ImTextureID texID = (ImTextureID)(intptr_t)tex->getID();

			ImGui::Image(texID, ImVec2(180, 150));
		}
		else
		{
			ImGui::Button("Missing Texture", ImVec2(180, 150));
		}
	}
	else
	{
		ImGui::Button("##SelectedPreview", ImVec2(180, 150));
	}

	//Drag and Drop target
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_NAME"))
		{
			selected = (const char*)payload->Data;

			auto it = textures.find(selected);
			if (it != textures.end())
			{
				selectedTexData.texture = it->second; // shared_ptr<EnoTexture>
			}
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::Text("Assigned: %s", selected.empty() ? "(none)" : selected.c_str());


	// RIGHT SIDE SETTINGS
	if (!selected.empty())
	{
		ImGui::Separator();
		ImGui::Text("Animation Settings");

		const char* modes[] = { "Use Rows / Columns", "Use Pixel Width / Height" };
		ImGui::Combo("Input Mode##Idle", &selectedMode, modes, IM_ARRAYSIZE(modes));

		auto it = textures.find(selected);
		if (it != textures.end())
		{
			auto tex = it->second;


			if (selectedMode == 0)
			{
				// ROWS / COLUMNS MODE
				ImGui::InputInt("Rows##selected", &selectedRows);
				ImGui::InputInt("Columns##selected", &selectedCols);

				selectedRows = std::max(1, selectedRows);
				selectedCols = std::max(1, selectedCols);

				ImGui::Text("Frame Size: %d × %d", selectedFrameW, selectedFrameH);
			}
			else if (selectedMode == 1)
			{
				// PIXEL MODE
				ImGui::InputInt("Frame Width##selected", &selectedFrameW);
				ImGui::InputInt("Frame Height##selected", &selectedFrameH);

				selectedFrameW = std::max(1, selectedFrameW);
				selectedFrameH = std::max(1, selectedFrameH);

				ImGui::Text("Grid: %d rows × %d cols", selectedRows, selectedCols);
			}

			selectedTexData.frameWidth = selectedFrameW;
			selectedTexData.frameHeight = selectedFrameH;
			selectedTexData.rows = selectedRows;
			selectedTexData.cols = selectedCols;
		}
	}

	ImGui::Spacing();

	// ================== APPLY BUTTON FOR IDLE ANIMATION ==================
	// Button is disabled if no texture is selected
	bool selectedHasTexture = selectedTexData.texture != nullptr && !selected.empty();
	if (!selectedHasTexture)
	{
		ImGui::BeginDisabled();
	}

	if (ImGui::Button("APPLY Selected State"))
	{
		ECS::Entity e{ selectedEntityID };

		// Comprehensive validation checks
		try {
			if (!e.has<ECSComponents::AnimationControllerComponent>(entMng)) {
				std::cerr << "[APPLY IDLE] Entity missing AnimationControllerComponent" << std::endl;
				if (!selectedHasTexture) ImGui::EndDisabled();
				return;
			}
			if (!e.has<ECSComponents::DrawInfoComponent>(entMng)) {
				std::cerr << "[APPLY IDLE] Entity missing DrawInfoComponent" << std::endl;
				if (!selectedHasTexture) ImGui::EndDisabled();
				return;
			}
		}
		catch (const std::exception& ex) {
			std::cerr << "[APPLY IDLE] Exception during validation: " << ex.what() << std::endl;
			if (!selectedHasTexture) ImGui::EndDisabled();
			return;
		}
		catch (...) {
			std::cerr << "[APPLY IDLE] Unknown exception during validation" << std::endl;
			if (!selectedHasTexture) ImGui::EndDisabled();
			return;
		}

		auto& animComp = e.get<ECSComponents::AnimationControllerComponent>(entMng);
		auto& drawComp = e.get<ECSComponents::DrawInfoComponent>(entMng);

		// Get or create animation controller
		Animation::AnimationController* ctrl = engine_.getAnimationRegistry().get(animComp.controllerID);

		if (!ctrl || animComp.controllerID == 0) {
			// Controller doesn't exist, create a new one
			std::cerr << "[APPLY IDLE] Animation controller is null or invalid (ID: " << animComp.controllerID << "), creating new one" << std::endl;
			animComp.controllerID = engine_.getAnimationRegistry().createController();
			ctrl = engine_.getAnimationRegistry().get(animComp.controllerID);

			if (!ctrl) {
				std::cerr << "[APPLY IDLE] Failed to create animation controller" << std::endl;
				if (!selectedHasTexture) ImGui::EndDisabled();
				return;
			}
			std::cout << "[APPLY IDLE] Created new animation controller with ID: " << animComp.controllerID << std::endl;
		}

		// Safety check: Ensure texture is valid before proceeding
		if (!selectedTexData.texture || !selectedTexData.texture->isValid())
		{
			std::cerr << "[APPLY IDLE] Invalid texture data" << std::endl;
			ImGui::OpenPopup("Error##selectedNoTexture");
			if (!selectedHasTexture) ImGui::EndDisabled();
			return;
		}

		//drawComp.texture = selectedTexData.texture;
		drawComp.setTexture(selectedTexData.texture, selected);
		drawComp.color = Graphics::Color::White;
		drawComp.texID = 0;

		//checkpoint
		if (selectedMode == 0)   // rows/cols mode
		{
			selectedTexData.rows = std::max(1, selectedRows);
			selectedTexData.cols = std::max(1, selectedCols);
			selectedTexData.computeFrameSize();
		}
		else                 // pixel mode
		{
			selectedTexData.rows = selectedTexData.texture->getHeight() / std::max(1, selectedFrameH);
			selectedTexData.cols = selectedTexData.texture->getWidth() / std::max(1, selectedFrameW);
			selectedTexData.frameWidth = selectedFrameW;
			selectedTexData.frameHeight = selectedFrameH;
		}


		// Create animation with explicit frame calculation
		Animation::EnoAnimation selectedAnim = Animation::createSpriteSheetAnimation(
			selectedTexData.frameWidth,
			selectedTexData.frameHeight,
			selectedTexData.texture->getWidth(),
			selectedTexData.texture->getHeight(),
			0,                                  // start frame
			selectedTexData.rows * selectedTexData.cols,// total frames
			0.1f                                // duration per frame
		);

		selectedAnim.setLooping(true);
		ctrl->addAnimation(selectedState, selectedAnim, selectedTexData.texture); // Pass the texture
		ctrl->setAnimation(selectedState);

		// Save animation configuration to component for serialization
		std::strncpy(animComp.currentAnimationName, selectedState.c_str(), sizeof(animComp.currentAnimationName) - 1); 

		animComp.setAnimationData(																		
			selectedState.c_str(),
			selected.c_str(),
			selectedTexData.frameWidth,
			selectedTexData.frameHeight,
			selectedTexData.rows,
			selectedTexData.cols,
			0.1f,
			true
		);
	}

	if (!selectedHasTexture)
	{
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("Please select a texture first");
		}
	}

	// Error popup for missing texture: Error message that pops up without crashing the engine
	if (ImGui::BeginPopupModal("Error##selectedNoTexture", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Error: No texture selected!");
		ImGui::Text("Please drag and drop a texture onto the preview box first.");
		if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}

	ImGui::Separator();
	ImGui::Text("Animation Test");

	if (entity.has<ECSComponents::AnimationControllerComponent>(entMng))
	{
		auto& animComp = entity.get<ECSComponents::AnimationControllerComponent>(entMng);
		Animation::AnimationController* ctrl =
			engine_.getAnimationRegistry().get(animComp.controllerID);

		if (ctrl)
		{
			for (const std::string& state : states)
			{
				std::string label = "Play " + state;

				if (ImGui::Button(label.c_str()))
				{
					if (ctrl->animations_.count(state))
					{
						// Drive via currentState so updateAnimations() renders this
						// animation every frame instead of reverting to whatever state
						// the state machine was in before the button was clicked.
						if (entity.has<ECSComponents::StateMachineComponent>(entMng))
						{
							auto& sm = entity.get<ECSComponents::StateMachineComponent>(entMng);
							std::strncpy(sm.currentState, state.c_str(), sizeof(sm.currentState) - 1);
							sm.currentState[sizeof(sm.currentState) - 1] = '\0';
						}

						ctrl->setAnimation(state);
						std::strncpy(animComp.currentAnimationName, state.c_str(),
							sizeof(animComp.currentAnimationName) - 1);
						animComp.currentAnimationName[sizeof(animComp.currentAnimationName) - 1] = '\0';
					}
					else
					{
						std::cout << "[ImGui] " << state << " animation not found!\n";
					}
				}
			}

			ImGui::NewLine(); // clean line break after loop
		}
	}
	else
	{
		ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1),
			"This entity has no AnimationController component!");
	}


	ImGui::End();
}

void drawNodeRecursive(Editor::EnoSceneTree& sceneTree, uint32_t entityID, ECS::EntityManager& entityManager)
{
	using namespace Editor;
	Editor::Node* node = sceneTree.getNodeByEntityID(entityID);
	if (!node) return;

	// Get entity name from NameComponent if it exists
	std::string label = "Entity " + std::to_string(node->entity_.id());
#if 1
	// Try to get the actual name from the entity
	try {
		for (ECS::Entity& e : entityManager.findEntity<ECSComponents::NameComponent>()) {
			if (e.id() == node->entity_.id()) {
				auto& nameComp = e.get<ECSComponents::NameComponent>(entityManager);
				label = nameComp.name;
				break;
			}
		}
	}
	catch (...) {
		// If we can't get the name, use the default
	}
#else
	try {
		for (Node* node : sceneTree.getAllNodes()) {
			label = node->name_;
			break;
		}
	}
	catch (...) {
		// If we can't get the name, use the default
	}


#endif
	// If node has children -> use TreeNodeEx
	if (node->children_.empty()) {
		bool selected = (selectedEntityID == node->entity_.id());
		// Use ## to separate visible label from unique ID
		std::string uniqueLabel = label + "##" + std::to_string(node->entity_.id());
		if (ImGui::Selectable(uniqueLabel.c_str(), selected))
			selectedEntityID = node->entity_.id();
	}
	else {
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
		if (selectedEntityID == node->entity_.id())
			flags |= ImGuiTreeNodeFlags_Selected;

		// Use ## to separate visible label from unique ID
		std::string uniqueLabel = label + "##" + std::to_string(node->entity_.id());
		bool open = ImGui::TreeNodeEx(uniqueLabel.c_str(), flags);
		if (ImGui::IsItemClicked())
			selectedEntityID = node->entity_.id();

		if (open) {
			for (Editor::Node* child : node->children_)
				drawNodeRecursive(sceneTree, child->entity_.id(), entityManager);
			ImGui::TreePop();
		}
	}
}

void displayAssetTab(Engine::EnoEngine& engine_) {
	Resources::EnoLoader& loader = engine_.getResourceLoader();
	const auto& textures = loader.getAllTextures();
	auto& g_droppedFiles = Window::EnoWindow::GetDroppedFiles();

	ImGui::Text("=== Loaded Textures ===");
	ImGui::Separator();

	static std::string selectedTexture;
	static Graphics::TextureData textureData;
	static int inputRows = 1, inputCols = 1;
	static int inputFrameW = 96, inputFrameH = 96;
	static int mode = 0; // 0 = Rows/Cols, 1 = Pixels

	// --- Handle dropped files ---
	if (!g_droppedFiles.empty()) {
		for (auto& path : g_droppedFiles) {
			std::filesystem::path filePath(path);
			auto ext = filePath.extension().string();

			if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
				// Construct destination path in assets folder
				std::string assetsImagePath = FileSystem::GetAssetsDir().string() + "/art/";
				std::string filename = filePath.filename().string();
				std::filesystem::path destPath = std::filesystem::path(assetsImagePath) / filename;

				// Copy file to assets folder if it doesn't exist or is different
				try {
					// Create assets/art directory if it doesn't exist
					std::filesystem::create_directories(assetsImagePath);

					// Check if we need to copy the file
					bool needsCopy = !std::filesystem::exists(destPath);
					if (!needsCopy) {
						// File exists, check if it's the same file (by comparing paths)
						auto sourceAbsolute = std::filesystem::absolute(filePath);
						auto destAbsolute = std::filesystem::absolute(destPath);
						needsCopy = (sourceAbsolute != destAbsolute);
					}

					if (needsCopy) {
						std::filesystem::copy_file(filePath, destPath, std::filesystem::copy_options::overwrite_existing);
						std::cout << "[Asset Import] Copied " << filename << " to assets folder\n";
					} else {
						std::cout << "[Asset Import] File " << filename << " already exists in assets folder\n";
					}

					// Load texture from the assets folder location
					auto tex = std::make_shared<Graphics::EnoTexture>();
					if (tex->loadFromFile(destPath.string())) {
						std::string name = filePath.stem().string();
						loader.addTexture(name, tex);
						std::cout << "[DEBUG] Imported texture: " << name << std::endl;
					}
				}
				catch (const std::filesystem::filesystem_error& e) {
					std::cerr << "[Asset Import] Failed to copy file: " << e.what() << std::endl;
				}
			}
		}
	}

	// Texture list
	for (auto& [name, texture] : textures) {
		if (!texture || !texture->isValid())
			continue;

		ImGui::Image((void*)(intptr_t)texture->getID(), ImVec2(48, 48));
		ImGui::SameLine();
		bool isSelected = (selectedTexture == name);
		if (ImGui::Selectable(name.c_str(), isSelected, 0, ImVec2(200, 48))) {
			selectedTexture = name;
			textureData.texture = texture;
		}
		if (ImGui::BeginDragDropSource())
		{
			// Store C-string name inside drag payload
			ImGui::SetDragDropPayload("TEXTURE_NAME", name.c_str(), name.size() + 1);

			ImGui::Text("%s", name.c_str());

			ImGui::EndDragDropSource();
		}

		ImGui::Spacing();
		ImGui::Separator();
	}

	ImGui::Separator();

	// --- Texture Deletion ---
	ImGui::Text("-----------DELETION OF TEXTURE FILES-------------");

	if (ImGui::Button("Delete selected texture"))
	{
		if (!selectedTexture.empty())
		{
			ImGui::OpenPopup("ConfirmDeleteTexture");
		}
	}

	if (ImGui::BeginPopupModal("ConfirmDeleteTexture", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextWrapped("Delete this texture?\n\n%s", selectedTexture.c_str());
		ImGui::Spacing();

		if (ImGui::Button("Delete", ImVec2(120, 0)))
		{
			// Get the texture to find its file path
			auto& texturesMap = loader.getTexturesMap();
			auto it = texturesMap.find(selectedTexture);

			if (it != texturesMap.end())
			{
				// Reset the shared_ptr (set texture pointer to null)
				it->second.reset();

				// Remove from the textures map
				texturesMap.erase(it);
			}

			// Delete the file from disk (both build and root folders)
			bool deleted = false;
			bool deletedRoot = false;
			std::string deleteErr;

			try
			{
				// Build folder path
				std::filesystem::path buildPath = std::filesystem::path(loader.getImagePath()) / (selectedTexture + ".png");
				std::filesystem::path buildPathJpg = std::filesystem::path(loader.getImagePath()) / (selectedTexture + ".jpg");

				// Root folder path
				std::filesystem::path rootPath = FileSystem::GetRootDir() / "assets/art" / (selectedTexture + ".png");
				std::filesystem::path rootPathJpg = FileSystem::GetRootDir() / "assets/art" / (selectedTexture + ".jpg");

				// Delete from BUILD folder (try PNG first, then JPG)
				if (std::filesystem::exists(buildPath))
				{
					deleted = std::filesystem::remove(buildPath);
				}
				else if (std::filesystem::exists(buildPathJpg))
				{
					deleted = std::filesystem::remove(buildPathJpg);
				}

				// Delete from ROOT folder (try PNG first, then JPG)
				if (std::filesystem::exists(rootPath))
				{
					deletedRoot = std::filesystem::remove(rootPath);
				}
				else if (std::filesystem::exists(rootPathJpg))
				{
					deletedRoot = std::filesystem::remove(rootPathJpg);
				}

				if (!deleted && !deletedRoot)
				{
					deleteErr = "File not found in build or root folders.";
				}
			}
			catch (const std::exception& e)
			{
				deleteErr = e.what();
			}

			if (!deleteErr.empty())
			{
				std::cerr << "[Texture Delete] Error: " << deleteErr << std::endl;
			}
			else
			{
				std::cout << "[Texture Delete] Deleted texture: " << selectedTexture << std::endl;
			}

			// Clear selection
			selectedTexture.clear();
			textureData.texture.reset();

			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	ImGui::Separator();

	////YunQi Bookmark  (i believe this below section will have to be moved to Asset Prefabs)
	//// If texture selected
	//if (!selectedTexture.empty()) {
	//	ImGui::Text("Selected: %s", selectedTexture.c_str());

	//	// Mode selector
	//	const char* modes[] = { "Use Rows / Columns", "Use Pixel Width / Height" };
	//	ImGui::Combo("Input Mode", &mode, modes, IM_ARRAYSIZE(modes));

	//	if (mode == 0) {
	//		// ROWS/COLS MODE
	//		ImGui::InputInt("Rows", &inputRows);
	//		ImGui::InputInt("Columns", &inputCols);

	//		if (ImGui::Button("Apply Texture (Rows/Cols)")) {
	//			textureData.rows = std::max(1, inputRows);
	//			textureData.cols = std::max(1, inputCols);
	//			textureData.computeFrameSize();

	//			if (g_game)
	//				g_game->applyTextureToPlayer(textureData);
	//		}

	//		ImGui::Text("Frame Size: %d x %d",
	//			textureData.frameWidth, textureData.frameHeight);
	//	}
	//	else {
	//		// PIXEL INPUT MODE
	//		ImGui::InputInt("Frame Width", &inputFrameW);
	//		ImGui::InputInt("Frame Height", &inputFrameH);

	//		if (ImGui::Button("Apply Texture (Pixels)")) {
	//			textureData.rows = textureData.texture->getHeight() / std::max(1, inputFrameH);
	//			textureData.cols = textureData.texture->getWidth() / std::max(1, inputFrameW);
	//			textureData.frameWidth = inputFrameW;
	//			textureData.frameHeight = inputFrameH;

	//			if (g_game)
	//				g_game->applyTextureToPlayer(textureData);
	//		}

	//		ImGui::Text("Grid: %d rows × %d cols",
	//			textureData.rows, textureData.cols);
	//	}
	//}


	// Loading audio
	const auto& sounds = loader.getSounds();
	if (!g_droppedFiles.empty())
	{
		for (auto& path : g_droppedFiles)
		{
			std::filesystem::path filePath(path);
			auto name = filePath.stem().string();
			auto ext = filePath.extension().string();

			std::cout << "path: " << path << std::endl;

			if (ext == ".wav" || ext == ".mp3")
			{
				auto it = sounds.find(filePath.stem().string());
				if (it == sounds.end())
				{
					throw std::runtime_error("Failed to find dropped in sound!");
				}

				std::cout << "Sound obtained: " << name << std::endl;

				// Construct Audio module
				auto audioMod = std::make_unique<Audio::EnoAudioModule>(name, ext, it->second);
				loader.storeAudioMod(std::move(audioMod));
				std::cout << "Construction done" << std::endl;
			}
		}
	}

	// Display audio on asset list



	static std::string selectedAudio;

	// Selected audio
	if (!selectedAudio.empty())
	{
		ImGui::Text("Selected: %s", selectedAudio.c_str());
	}

	g_droppedFiles.clear();
}

// Docking shi

void imGuiBeginDockspace() {
	// Get the main viewport
	ImGuiViewport* viewport = ImGui::GetMainViewport();

	// Set the dockspace to cover the entire viewport
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	// Configure the dockspace window flags
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	window_flags |= ImGuiWindowFlags_NoBackground;

	// Push style vars to remove padding and borders
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	// Begin the dockspace window
	ImGui::Begin("EnoEngine Dockspace", nullptr, window_flags);
	ImGui::PopStyleVar(3);

	// Create the dockspace
	ImGuiID dockspace_id = ImGui::GetID("MainDockspace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
}

// Draw gizmos for selected entity
void drawGizmos(ImDrawList* drawList, ImVec2 entityCenter, float gizmoSize, GizmoMode mode) {
	const float handleSize = 8.0f;
	const float rotateRadius = gizmoSize * 0.6f;

	// Translate gizmo handles
	if (mode == GIZMO_TRANSLATE) {
		// Draw X axis handle (blue square to the right)
		ImVec2 xHandle = ImVec2(entityCenter.x + gizmoSize, entityCenter.y);
		drawList->AddRectFilled(
			ImVec2(xHandle.x - handleSize / 2, xHandle.y - handleSize / 2),
			ImVec2(xHandle.x + handleSize / 2, xHandle.y + handleSize / 2),
			IM_COL32(0, 150, 255, 255)
		);
		drawList->AddLine(entityCenter, xHandle, IM_COL32(0, 150, 255, 255), 2.0f);

		// Draw Y axis handle (blue square above)
		ImVec2 yHandle = ImVec2(entityCenter.x, entityCenter.y - gizmoSize);
		drawList->AddRectFilled(
			ImVec2(yHandle.x - handleSize / 2, yHandle.y - handleSize / 2),
			ImVec2(yHandle.x + handleSize / 2, yHandle.y + handleSize / 2),
			IM_COL32(0, 150, 255, 255)
		);
		drawList->AddLine(entityCenter, yHandle, IM_COL32(0, 150, 255, 255), 2.0f);
	}
	// Rotate gizmo
	else if (mode == GIZMO_ROTATE) {
		// Draw rotation circle
		drawList->AddCircle(entityCenter, rotateRadius, IM_COL32(0, 150, 255, 255), 32, 2.0f);

		// Draw rotation handle at top of circle
		ImVec2 rotHandle = ImVec2(entityCenter.x, entityCenter.y - rotateRadius);
		drawList->AddCircleFilled(rotHandle, handleSize / 2, IM_COL32(0, 150, 255, 255));
	}
	// Scale gizmo
	else if (mode == GIZMO_SCALE) {
		// Draw uniform scale handle (diagonal bottom-right corner - blue)
		ImVec2 uniformScaleHandle = ImVec2(entityCenter.x + gizmoSize * 0.7f, entityCenter.y + gizmoSize * 0.7f);
		drawList->AddRectFilled(
			ImVec2(uniformScaleHandle.x - handleSize / 2, uniformScaleHandle.y - handleSize / 2),
			ImVec2(uniformScaleHandle.x + handleSize / 2, uniformScaleHandle.y + handleSize / 2),
			IM_COL32(0, 150, 255, 255)
		);
		drawList->AddLine(entityCenter, uniformScaleHandle, IM_COL32(0, 150, 255, 255), 2.0f);

		// Draw horizontal scale handle (right side - blue)
		ImVec2 xScaleHandle = ImVec2(entityCenter.x + gizmoSize, entityCenter.y);
		drawList->AddRectFilled(
			ImVec2(xScaleHandle.x - handleSize / 2, xScaleHandle.y - handleSize / 2),
			ImVec2(xScaleHandle.x + handleSize / 2, xScaleHandle.y + handleSize / 2),
			IM_COL32(0, 150, 255, 255)
		);
		drawList->AddLine(entityCenter, xScaleHandle, IM_COL32(0, 150, 255, 255), 2.0f);

		// Draw vertical scale handle (top - blue)
		ImVec2 yScaleHandle = ImVec2(entityCenter.x, entityCenter.y - gizmoSize);
		drawList->AddRectFilled(
			ImVec2(yScaleHandle.x - handleSize / 2, yScaleHandle.y - handleSize / 2),
			ImVec2(yScaleHandle.x + handleSize / 2, yScaleHandle.y + handleSize / 2),
			IM_COL32(0, 150, 255, 255)
		);
		drawList->AddLine(entityCenter, yScaleHandle, IM_COL32(0, 150, 255, 255), 2.0f);
	}
}

// Check if mouse is over a gizmo handle
bool isMouseOverGizmoHandle(ImVec2 mousePos, ImVec2 entityCenter, float gizmoSize, GizmoMode mode, int& handleType) {
	const float handleSize = 12.0f;
	const float rotateRadius = gizmoSize * 0.6f;

	if (mode == GIZMO_TRANSLATE) {
		// Check X handle (right)
		ImVec2 xHandle = ImVec2(entityCenter.x + gizmoSize, entityCenter.y);
		if (fabs(mousePos.x - xHandle.x) < handleSize && fabs(mousePos.y - xHandle.y) < handleSize) {
			handleType = 1;
			return true;
		}
		// Check Y handle (up)
		ImVec2 yHandle = ImVec2(entityCenter.x, entityCenter.y - gizmoSize);
		if (fabs(mousePos.x - yHandle.x) < handleSize && fabs(mousePos.y - yHandle.y) < handleSize) {
			handleType = 2;
			return true;
		}
	}
	else if (mode == GIZMO_ROTATE) {
		// Check rotation handle
		ImVec2 rotHandle = ImVec2(entityCenter.x, entityCenter.y - rotateRadius);
		float dist = sqrtf((mousePos.x - rotHandle.x) * (mousePos.x - rotHandle.x) +
			(mousePos.y - rotHandle.y) * (mousePos.y - rotHandle.y));
		if (dist < handleSize) {
			handleType = 3;
			return true;
		}
	}
	else if (mode == GIZMO_SCALE) {
		// Check horizontal scale handle (right side - handleType 4)
		ImVec2 xScaleHandle = ImVec2(entityCenter.x + gizmoSize, entityCenter.y);
		if (fabs(mousePos.x - xScaleHandle.x) < handleSize && fabs(mousePos.y - xScaleHandle.y) < handleSize) {
			handleType = 4;
			return true;
		}
		// Check vertical scale handle (top - handleType 5)
		ImVec2 yScaleHandle = ImVec2(entityCenter.x, entityCenter.y - gizmoSize);
		if (fabs(mousePos.x - yScaleHandle.x) < handleSize && fabs(mousePos.y - yScaleHandle.y) < handleSize) {
			handleType = 5;
			return true;
		}
		// Check uniform scale handle (diagonal - handleType 3)
		ImVec2 uniformScaleHandle = ImVec2(entityCenter.x + gizmoSize * 0.7f, entityCenter.y + gizmoSize * 0.7f);
		if (fabs(mousePos.x - uniformScaleHandle.x) < handleSize && fabs(mousePos.y - uniformScaleHandle.y) < handleSize) {
			handleType = 3;
			return true;
		}
	}

	handleType = 0;
	return false;
}

void displayEditorView(Engine::EnoEngine& engine_, Editor::EnoSceneTree& sceneTree, Editor::UndoManager& undoManager) {
	auto& entityManager = engine_.getEntityManager();
	auto& sceneEditor = engine_.getEnoEditor();

	// Display toolbar for PLAY/PAUSE/STOP
	static bool isPlaying = false;
	isPlaying = (sceneEditor.getCurrentScene().getState() != Editor::State::Stopped) ? true : false;

	if (ImGui::Button(isPlaying ? "Stop" : "Play")) {
		isPlaying = !isPlaying;
		if (isPlaying) {
			sceneEditor.playScene();
		} else {
			sceneEditor.stopScene();
			// Immediately invalidate entity cache after stopping scene
			if (g_game) g_game->markEntitiesDirty();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button(sceneEditor.getCurrentScene().getState() != Editor::State::Paused ? "Pause" : "Unpause")) {
		if (sceneEditor.getCurrentScene().getState() != Editor::State::Stopped) {
			(sceneEditor.getCurrentScene().getState() == Editor::State::Playing ? sceneEditor.pauseScene() : sceneEditor.playScene());
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Next")) {
		if (sceneEditor.getCurrentScene().getState() == Editor::State::Paused) {
			sceneEditor.getCurrentScene().next();
		}
	}

	ImGui::SameLine();
	ImGui::Text("|");
	ImGui::SameLine();

	// Gizmo mode selector buttons
	if (ImGui::Button("Move (Q)")) {
		currentGizmoMode = GIZMO_TRANSLATE;
	}
	ImGui::SameLine();
	if (ImGui::Button("Rotate (E)")) {
		currentGizmoMode = GIZMO_ROTATE;
	}
	ImGui::SameLine();
	if (ImGui::Button("Scale (R)")) {
		currentGizmoMode = GIZMO_SCALE;
	}

	// Keyboard shortcuts for gizmo modes
	if (!ImGui::GetIO().WantTextInput) {
		if (ImGui::IsKeyPressed(ImGuiKey_Q)) currentGizmoMode = GIZMO_TRANSLATE;
		if (ImGui::IsKeyPressed(ImGuiKey_E)) currentGizmoMode = GIZMO_ROTATE;
		if (ImGui::IsKeyPressed(ImGuiKey_R)) currentGizmoMode = GIZMO_SCALE;
	}

	// Keyboard shortcuts for zoom (+ and - keys) — edit mode only
	if (g_game && !ImGui::GetIO().WantTextInput && !engine_.getIsPlaying()) {
		if (ImGui::IsKeyPressed(ImGuiKey_Equal) || ImGui::IsKeyPressed(ImGuiKey_KeypadAdd)) {
			g_game->adjustEditorZoom(0.1f);  // Zoom in
		}
		if (ImGui::IsKeyPressed(ImGuiKey_Minus) || ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract)) {
			g_game->adjustEditorZoom(-0.1f);  // Zoom out
		}
	}

	// Keyboard shortcuts for layer change ([ and ] keys)
	if (selectedEntityID != UINT32_MAX && !ImGui::GetIO().WantTextInput) {
		for (auto& e : entityManager.findEntity<>()) {
			if (e.id() == selectedEntityID && e.has<ECSComponents::VisibilityComponent>(entityManager)) {
				auto& vi = e.get<ECSComponents::VisibilityComponent>(entityManager);
				int currentLayer = static_cast<int>(vi.getZIndex());

				// '[' key decreases layer
				if (ImGui::IsKeyPressed(ImGuiKey_LeftBracket)) {
					if (currentLayer > 0) {
						vi.setZIndex(static_cast<uint32_t>(currentLayer - 1));
						if (g_game) g_game->markEntitiesDirty();
					}
				}
				// ']' key increases layer
				if (ImGui::IsKeyPressed(ImGuiKey_RightBracket)) {
					if (currentLayer < ECSComponents::availableLayers - 1) {
						vi.setZIndex(static_cast<uint32_t>(currentLayer + 1));
						if (g_game) g_game->markEntitiesDirty();
					}
				}
				break;
			}
		}
	}

	ImGui::SameLine();
	ImGui::Text("|");
	ImGui::SameLine();

	auto& mapEditor = g_game->getMapEditor();
	bool wasEdit = mapEditor.isEditMode();
	if (wasEdit) {
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f)); // green
	}

	if (ImGui::Button("Map Editor")) {
		mapEditor.setEditMode(!wasEdit);
	}

	if (wasEdit) {
		ImGui::PopStyleColor();
	}

	ImGui::Separator();

	// Informational text with zoom level
	if (g_game) {
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f),
			"Editor View - WASD: Move | Mouse Wheel/+/-: Zoom (%.2fx)",
			g_game->getEditorZoom());
	} else {
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Editor View - Use WASD to move camera");
	}
	ImGui::Separator();

	// Remaining space below the toolbar
	ImVec2 viewportSize = ImGui::GetContentRegionAvail();

	// Feed panel size back so the FBO is resized to match next frame
	if (g_game && viewportSize.x > 0 && viewportSize.y > 0)
		g_game->setEditorPanelSize(static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y));

	// World-unit viewport dimensions (panel size ÷ zoom), used by screen↔world conversions
	float editorWidth  = g_game ? static_cast<float>(g_game->getEditorWidth())  : viewportSize.x;
	float editorHeight = g_game ? static_cast<float>(g_game->getEditorHeight()) : viewportSize.y;

	// Child window fills remaining space; NoScrollWithMouse keeps the toolbar from scrolling away
	ImGui::BeginChild("##editorViewport", viewportSize, false,
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	if (g_game && g_game->getEditorFramebuffer()) {
		auto* fb = g_game->getEditorFramebuffer();

		ImVec2 childSize = ImGui::GetContentRegionAvail();
		// Display the framebuffer stretched to fill the viewport child window.
		ImGui::Image(
			(void*)(intptr_t)fb->getTextureID(),
			childSize,
			ImVec2(0, 1),  // UV coordinates (flipped Y for OpenGL)
			ImVec2(1, 0)
		);

		// Capture the editor view min/max coordinates and store them in EditorApplication
		ImVec2 imGuiImageMin = ImGui::GetItemRectMin();
		ImVec2 imGuiImageMax = ImGui::GetItemRectMax();
		g_game->setEditorViewRect({imGuiImageMin.x, imGuiImageMin.y}, {imGuiImageMax.x, imGuiImageMax.y});

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		// Clip all overlay drawing to the viewport image area
		drawList->PushClipRect(imGuiImageMin, imGuiImageMax, true);

			// ========== MOUSE WHEEL ZOOM (edit mode only) ==========
			if (ImGui::IsItemHovered() && !engine_.getIsPlaying()) {
				float wheel = ImGui::GetIO().MouseWheel;
				if (wheel != 0.0f) {
					// Get mouse position in viewport before zoom
					ImVec2 mousePos = ImGui::GetMousePos();
					Math::eVec2<float> worldPosBeforeZoom = Graphics::screenToWorld(
						{ mousePos.x, mousePos.y },
						g_game->getEditorViewMin(),
						g_game->getEditorViewMax(),
						editorWidth,
						editorHeight,
						g_game->getEditorCameraX(),
						g_game->getEditorCameraY()
					);

					// Adjust zoom (0.1 per wheel notch)
					float zoomDelta = wheel * 0.1f;
					g_game->adjustEditorZoom(zoomDelta);
				}
			}

			auto& g_droppedFiles_editor = Window::EnoWindow::GetDroppedFiles();
			if (!g_droppedFiles_editor.empty() && ImGui::IsItemHovered())
			{
				Resources::EnoLoader& loader = engine_.getResourceLoader();

				for (auto& path : g_droppedFiles_editor)
				{
					std::filesystem::path filePath(path);
					auto ext = filePath.extension().string();

					// Only process image files
					if (ext == ".png" || ext == ".jpg" || ext == ".jpeg")
					{
						// Construct destination path in assets folder
						std::string assetsImagePath = FileSystem::GetAssetsDir().string() + "/art/";
						std::string filename = filePath.filename().string();
						std::string textureName = filePath.stem().string();
						std::filesystem::path destPath = std::filesystem::path(assetsImagePath) / filename;

						try {
							// Create assets/art directory if it doesn't exist
							std::filesystem::create_directories(assetsImagePath);

							// Check if we need to copy the file
							bool needsCopy = !std::filesystem::exists(destPath);
							if (!needsCopy) {
								auto sourceAbsolute = std::filesystem::absolute(filePath);
								auto destAbsolute = std::filesystem::absolute(destPath);
								needsCopy = (sourceAbsolute != destAbsolute);
							}

							if (needsCopy) {
								std::filesystem::copy_file(filePath, destPath, std::filesystem::copy_options::overwrite_existing);
								std::cout << "[Asset Import] Copied " << filename << " to assets folder\n";
							}

							// Load or get the texture
							auto texture = loader.getTexture(textureName);
							if (!texture || !texture->isValid()) {
								// Need to load it
								texture = std::make_shared<Graphics::EnoTexture>();
								if (texture->loadFromFile(destPath.string())) {
									loader.addTexture(textureName, texture);
									std::cout << "[Asset Import] Loaded texture: " << textureName << std::endl;
								} else {
									std::cerr << "[Asset Import] Failed to load texture from: " << destPath << std::endl;
									continue;
								}
							}

							// Get mouse position and convert to world coordinates
							ImVec2 mousePos = ImGui::GetMousePos();
							Math::eVec2<float> worldPos = Graphics::screenToWorld(
								{ mousePos.x, mousePos.y },
								g_game->getEditorViewMin(),
								g_game->getEditorViewMax(),
								editorWidth,
								editorHeight,
								g_game->getEditorCameraX(),
								g_game->getEditorCameraY()
							);

							// Create entity at drop location
							ECSComponents::TransformComponent transform;
							transform.pos = worldPos;
							transform.rot = Math::eVec2<float>(0.0f, 0.0f);
							transform.scale = Math::eVec2<float>(1.0f, 1.0f);

							ECS::Entity newEntity = entityManager.create<ECSComponents::TransformComponent>(transform);

							// Add render component with texture
							ECSComponents::RenderComponent render;
							render.texture = texture;
							render.width = static_cast<float>(texture->getWidth());
							render.height = static_cast<float>(texture->getHeight());
							strncpy_s(render.texturePath, textureName.c_str(), sizeof(render.texturePath) - 1);
							render.texturePath[sizeof(render.texturePath) - 1] = '\0';
							render.color = Graphics::Color::White;
							newEntity.add<ECSComponents::RenderComponent>(entityManager, render);

							// Add name component
							ECSComponents::NameComponent nameComp;
							strncpy_s(nameComp.name, textureName.c_str(), sizeof(nameComp.name) - 1);
							nameComp.name[sizeof(nameComp.name) - 1] = '\0';
							newEntity.add<ECSComponents::NameComponent>(entityManager, nameComp);

							// Create animation controller
							uint32_t controllerID = engine_.getAnimationRegistry().createController();
							newEntity.add<ECSComponents::AnimationControllerComponent>(entityManager, controllerID);

							// Add to scene tree
							Node* node = sceneTree.createFromEntity(newEntity, nullptr);
							node->name_ = textureName;

							// Record creation for undo/redo
							auto createCmd = std::make_unique<Editor::CreateEntityCommand>(
								sceneTree,
								entityManager,
								newEntity,
								engine_.getGraphicsManager()
							);
							undoManager.executeCommand(std::move(createCmd));

							std::cout << "[Drag-Drop] Created entity '" << textureName
									  << "' at world position (" << worldPos.x << ", " << worldPos.y << ")\n";

							if (g_game) g_game->markEntitiesDirty();
						}
						catch (const std::filesystem::filesystem_error& e) {
							std::cerr << "[Asset Import] Filesystem error: " << e.what() << std::endl;
						}
						catch (const std::exception& e) {
							std::cerr << "[Asset Import] Error creating entity: " << e.what() << std::endl;
						}
					}
				}
			}

			// ========== DRAG-AND-DROP TEXTURE FROM ASSET BROWSER ==========
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_NAME"))
				{
					std::string textureName = (const char*)payload->Data;
					Resources::EnoLoader& loader = engine_.getResourceLoader();

					// Get the texture from the loader
					auto texture = loader.getTexture(textureName);
					if (texture && texture->isValid())
					{
						// Get mouse position and convert to world coordinates
						ImVec2 mousePos = ImGui::GetMousePos();
						Math::eVec2<float> worldPos = Graphics::screenToWorld(
							{ mousePos.x, mousePos.y },
							g_game->getEditorViewMin(),
							g_game->getEditorViewMax(),
							editorWidth,
							editorHeight,
							g_game->getEditorCameraX(),
							g_game->getEditorCameraY()
						);

						// Create entity at drop location
						ECSComponents::TransformComponent transform;
						transform.pos = worldPos;
						transform.rot = Math::eVec2<float>(0.0f, 0.0f);
						transform.scale = Math::eVec2<float>(1.0f, 1.0f);

						ECS::Entity newEntity = entityManager.create<ECSComponents::TransformComponent>(transform);

						// Add render component with texture
						ECSComponents::RenderComponent render;
						render.texture = texture;
						render.width = static_cast<float>(texture->getWidth());
						render.height = static_cast<float>(texture->getHeight());
						strncpy_s(render.texturePath, textureName.c_str(), sizeof(render.texturePath) - 1);
						render.texturePath[sizeof(render.texturePath) - 1] = '\0';
						render.color = Graphics::Color::White;
						newEntity.add<ECSComponents::RenderComponent>(entityManager, render);

						// Add name component
						ECSComponents::NameComponent nameComp;
						strncpy_s(nameComp.name, textureName.c_str(), sizeof(nameComp.name) - 1);
						nameComp.name[sizeof(nameComp.name) - 1] = '\0';
						newEntity.add<ECSComponents::NameComponent>(entityManager, nameComp);

						// Create animation controller
						uint32_t controllerID = engine_.getAnimationRegistry().createController();
						newEntity.add<ECSComponents::AnimationControllerComponent>(entityManager, controllerID);

						// Add to scene tree
						Node* node = sceneTree.createFromEntity(newEntity, nullptr);
						node->name_ = textureName;

						// Record creation for undo/redo
						auto createCmd = std::make_unique<Editor::CreateEntityCommand>(
							sceneTree,
							entityManager,
							newEntity,
							engine_.getGraphicsManager()
						);
						undoManager.executeCommand(std::move(createCmd));

						std::cout << "[Drag-Drop] Created entity '" << textureName
								  << "' at world position (" << worldPos.x << ", " << worldPos.y << ")\n";

						if (g_game) g_game->markEntitiesDirty();
					}
				}
				ImGui::EndDragDropTarget();
			}
			// ========== ALL EDITOR INTERACTIONS (moved from Game View) ==========


			// --- Handle entity deletion by click ---                                        

			if (g_waitingForEntityDeletion && ImGui::IsItemHovered() && ImGui::IsMouseClicked(0) && sceneEditor.getCurrentScene().getState() == Editor::State::Stopped) {                                                                                                     

				Math::eVec2<float> worldPos = Graphics::screenToWorld(
											{ ImGui::GetMousePos().x, ImGui::GetMousePos().y },
											g_game->getEditorViewMin(),
											g_game->getEditorViewMax(),
											editorWidth,
											editorHeight,
											g_game->getEditorCameraX(),
											g_game->getEditorCameraY());

				auto clicked = getEntityUnderMouse(engine_, worldPos);
				if (clicked.has_value()) {
					std::cout << "Deleting entity: " << clicked->id()
						<< " and all children at world coords (" << worldPos.x << ", " << worldPos.y << ")\n";

					// Get the node first before destroying
					Editor::Node* node = sceneTree.getNodeByEntityID(clicked->id());
					if (node) {
						// Create delete command (captures snapshot before deletion)
						auto deleteCmd = std::make_unique<Editor::DeleteEntityCommand>(
							sceneTree,
							entityManager,
							clicked.value(),
							engine_.getGraphicsManager()
						);

						// Execute the deletion (also adds to undo history)
						undoManager.executeCommand(std::move(deleteCmd));

						std::cout << "[DEBUG] Entity deletion completed with undo support\n";
					}

					if (g_game) g_game->markEntitiesDirty();
				}
				else {
					std::cout << "No entity found at world coords (" << worldPos.x << ", " << worldPos.y << ")\n";
				}

				g_waitingForEntityDeletion = false;
			}

			// --- Gizmo-based entity transformation logic ---
			// Draw gizmos for selected entity
			if (selectedEntityID != UINT32_MAX && sceneEditor.getCurrentScene().getState() == Editor::State::Stopped) {
				auto allEntities = entityManager.findEntity<ECSComponents::TransformComponent, ECSComponents::DrawInfoComponent>();
				for (auto& e : allEntities) {
					if (e.id() == selectedEntityID) {
						auto& tc = e.get<ECSComponents::TransformComponent>(entityManager);

						// Convert entity position to screen coordinates          
						Math::eVec2<float> entityWorldPos = {tc.pos.x, tc.pos.y}; 
						Math::eVec2<float> screenPos = Graphics::worldToScreen(
														entityWorldPos,
														g_game->getEditorViewMin(),
														g_game->getEditorViewMax(),
														editorWidth,
														editorHeight,
														g_game->getEditorCameraX(),
														g_game->getEditorCameraY());

						// Convert to ImVec2 for easier logic comparison
						ImVec2 entityScreenPos = { screenPos.x, screenPos.y };

						// Draw gizmo based on current mode
						float gizmoSize = 50.0f;
						drawGizmos(drawList, entityScreenPos, gizmoSize, currentGizmoMode);
						break;
					}
				}
			}

			// Draw camera preview rectangle for all active cameras (edit mode only)
			if (sceneEditor.getCurrentScene().getState() == Editor::State::Stopped)
			for (auto& camEntity : entityManager.findEntity<ECSComponents::CameraComponent>()) {
				auto& cam = camEntity.get<ECSComponents::CameraComponent>(entityManager);

				if (cam.isActive) {
					// Calculate camera bounds in world space
					// Camera position is CENTER, so calculate edges from center
					float camLeft = cam.position.x - cam.viewportWidth * 0.5f;
					float camRight = cam.position.x + cam.viewportWidth * 0.5f;
					float camTop = cam.position.y - cam.viewportHeight * 0.5f;
					float camBottom = cam.position.y + cam.viewportHeight * 0.5f;

					// Convert camera corners to screen space
					Math::eVec2<float> topLeft = Graphics::worldToScreen(
						{ camLeft, camTop },
						g_game->getEditorViewMin(),
						g_game->getEditorViewMax(),
						editorWidth,
						editorHeight,
						g_game->getEditorCameraX(),
						g_game->getEditorCameraY()
					);

					Math::eVec2<float> bottomRight = Graphics::worldToScreen(
						{ camRight, camBottom },
						g_game->getEditorViewMin(),
						g_game->getEditorViewMax(),
						editorWidth,
						editorHeight,
						g_game->getEditorCameraX(),
						g_game->getEditorCameraY()
					);

					// Draw camera viewport rectangle (cyan border, semi-transparent fill)
					drawList->AddRectFilled(
						ImVec2(topLeft.x, topLeft.y),
						ImVec2(bottomRight.x, bottomRight.y),
						IM_COL32(0, 255, 255, 30)  // Cyan fill with low alpha
					);
					drawList->AddRect(
						ImVec2(topLeft.x, topLeft.y),
						ImVec2(bottomRight.x, bottomRight.y),
						IM_COL32(0, 255, 255, 255),  // Solid cyan border
						0.0f, 0, 2.0f  // 2px thick border
					);

					// Draw camera center crosshair
					Math::eVec2<float> camCenterScreen = Graphics::worldToScreen(
						cam.position,
						g_game->getEditorViewMin(),
						g_game->getEditorViewMax(),
						editorWidth,
						editorHeight,
						g_game->getEditorCameraX(),
						g_game->getEditorCameraY()
					);
					float crossSize = 10.0f;
					drawList->AddLine(
						ImVec2(camCenterScreen.x - crossSize, camCenterScreen.y),
						ImVec2(camCenterScreen.x + crossSize, camCenterScreen.y),
						IM_COL32(0, 255, 255, 255), 2.0f
					);
					drawList->AddLine(
						ImVec2(camCenterScreen.x, camCenterScreen.y - crossSize),
						ImVec2(camCenterScreen.x, camCenterScreen.y + crossSize),
						IM_COL32(0, 255, 255, 255), 2.0f
					);

					// Draw label
					char label[128];
					snprintf(label, sizeof(label), "Camera %.0fx%.0f",
						cam.viewportWidth, cam.viewportHeight);
					drawList->AddText(
						ImVec2(topLeft.x + 5, topLeft.y + 5),
						IM_COL32(0, 255, 255, 255),
						label
					);
				}
			}

			// Begin gizmo interaction                                                    
			if (!g_waitingForEntityDeletion && g_selectedPrefabForPlacement.empty() &&        
				ImGui::IsItemHovered() && ImGui::IsMouseClicked(0) && !isDragging && !isRotating && !isScaling 
				&& sceneEditor.getCurrentScene().getState() == Editor::State::Stopped) 
			{             
				                                                                                                       
				ImVec2 mouseScreenPos = ImGui::GetMousePos();                             
				Math::eVec2<float> worldPos = 
					Graphics::screenToWorld(
					{ mouseScreenPos.x, mouseScreenPos.y },
					g_game->getEditorViewMin(),
					g_game->getEditorViewMax(),
					editorWidth,
					editorHeight,
					g_game->getEditorCameraX(),
					g_game->getEditorCameraY());

				// Check if we have a selected entity
				if (selectedEntityID != UINT32_MAX) {
					auto allEntities = entityManager.findEntity<ECSComponents::TransformComponent, ECSComponents::DrawInfoComponent>();
					for (auto& e : allEntities) {
						if (e.id() == selectedEntityID) {
							auto& tc = e.get<ECSComponents::TransformComponent>(entityManager);
							Math::eVec2<float> currentEntityWorldPos = {tc.pos.x, tc.pos.y};
							Math::eVec2<float> screenPos = Graphics::worldToScreen(
								currentEntityWorldPos,
								g_game->getEditorViewMin(),
								g_game->getEditorViewMax(),
								editorWidth,
								editorHeight,
								g_game->getEditorCameraX(),
								g_game->getEditorCameraY());
							ImVec2 entityScreenPos = {screenPos.x,screenPos.y};

							int handleType = 0;
							float gizmoSize = 50.0f;

							// Check if clicking on gizmo handle
							if (isMouseOverGizmoHandle(mouseScreenPos, entityScreenPos, gizmoSize, currentGizmoMode, handleType)) {
								// Collect all affected entity IDs (parent + children) for undo
								affectedEntityIDs.clear();
								Editor::Node* node = sceneTree.getNodeByEntityID(selectedEntityID);
								if (node) {
									collectEntityIDsRecursive(node, affectedEntityIDs);
								}

								// Create command and capture old state before transformation
								if (!affectedEntityIDs.empty()) {
									pendingTransformCommand = std::make_unique<Editor::MultiTransformCommand>(
										undoManager.getEntityManager(),
										currentGizmoMode == GIZMO_TRANSLATE ? "Move" :
										(currentGizmoMode == GIZMO_ROTATE ? "Rotate" : "Scale")
									);
									pendingTransformCommand->captureOldState(affectedEntityIDs);
								}

								if (currentGizmoMode == GIZMO_ROTATE) {   

									// Start rotation (handleType 3 for rotation)                                                                                               
									isRotating = true;                
									draggedEntityID = selectedEntityID;                                                                                                         
									activeTransformAxis = 3;          
									initialAngle = atan2f(mouseScreenPos.y - entityScreenPos.y, mouseScreenPos.x - entityScreenPos.x);                                          
									initialRotation = tc.rot.x;       
									rotationBeforeDrag = tc.rot;  // Store for undo

									std::cout << "Started rotating entity " << selectedEntityID << " (affecting " << affectedEntityIDs.size() << " entities)\n";                

								}
								else if (currentGizmoMode == GIZMO_SCALE) {
									// Start scaling
									isScaling = true;
									draggedEntityID = selectedEntityID;
									activeTransformAxis = handleType; // 3=uniform, 4=X-scale, 5=Y-scale
									scaleStartPos = mouseScreenPos;
									dragStartWorldPos = { worldPos.x, worldPos.y };
									initialScale = ImVec2(tc.scale.x, tc.scale.y);
									lastScale = ImVec2(tc.scale.x, tc.scale.y);  // Initialize last scale
									scaleBeforeDrag = tc.scale;  // Store for undo
									// Store initial draw info size
									if (e.has<ECSComponents::DrawInfoComponent>(entityManager)) {
										auto& dc = e.get<ECSComponents::DrawInfoComponent>(entityManager);
										initialDrawWidth = dc.width;
										initialDrawHeight = dc.height;
									}
									if (handleType == 3) {
										std::cout << "Started uniform scaling entity " << selectedEntityID << " (affecting " << affectedEntityIDs.size() << " entities)\n";
									}
									else if (handleType == 4) {
										std::cout << "Started horizontal scaling entity " << selectedEntityID << " (affecting " << affectedEntityIDs.size() << " entities)\n";
									}
									else if (handleType == 5) {
										std::cout << "Started vertical scaling entity " << selectedEntityID << " (affecting " << affectedEntityIDs.size() << " entities)\n";
									}
								}
								else if (currentGizmoMode == GIZMO_TRANSLATE) {
									// Start dragging with axis constraint
									draggedEntityID = selectedEntityID;
									isDragging = true;
									activeTransformAxis = handleType; // 1=X-axis, 2=Y-axis
									dragOffset.x = worldPos.x - tc.pos.x;
									dragOffset.y = worldPos.y - tc.pos.y;
									dragStartWorldPos = { worldPos.x, worldPos.y };
									lastDragPos = ImVec2(tc.pos.x, tc.pos.y);
									transformBeforeDrag = tc.pos;  // Store for undo
									if (handleType == 1) {
										std::cout << "Started dragging entity " << selectedEntityID << " on X-axis (affecting " << affectedEntityIDs.size() << " entities)\n";
									}
									else if (handleType == 2) {
										std::cout << "Started dragging entity " << selectedEntityID << " on Y-axis (affecting " << affectedEntityIDs.size() << " entities)\n";
									}
								}
								break;
							}
						}
					}
				}
				
				// If not clicking gizmo, check if clicking on entity
				if (!isDragging && !isRotating && !isScaling) {
					auto clicked = getEntityUnderMouse(engine_, worldPos);
					if (clicked) {
						// Select entity in hierarchy when clicked in editor window
						selectedEntityID = clicked->id();
						std::cout << "Selected entity " << selectedEntityID << "\n";
					}
				}
			}

			// --- TILEMAP PAINTING ---
			g_game->getMapEditor().paint(g_game);

			// --- Draw tilemap overlay ---
			g_game->getMapEditor().draw(g_game, drawList);

			// While transforming (drag, rotate, or scale)
			if ((isDragging || isRotating || isScaling) && ImGui::IsMouseDown(0)) {
				ImVec2 mouseScreenPos = ImGui::GetMousePos();
				Math::eVec2<float> eWorldPos = Graphics::screenToWorld(
					{ mouseScreenPos.x, mouseScreenPos.y },
					g_game->getEditorViewMin(),
					g_game->getEditorViewMax(),
					editorWidth,
					editorHeight,
					g_game->getEditorCameraX(),
					g_game->getEditorCameraY());
				ImVec2 worldPos = { eWorldPos.x, eWorldPos.y };

				auto all = entityManager.findEntity<ECSComponents::TransformComponent, ECSComponents::DrawInfoComponent>();
				for (auto& e : all) {
					if (e.id() == draggedEntityID) {
						auto& tc = e.get<ECSComponents::TransformComponent>(entityManager);

						if (isDragging) {
							// Calculate delta movement with axis constraints
							float newX = worldPos.x - dragOffset.x;
							float newY = worldPos.y - dragOffset.y;

							// Apply axis constraint based on which handle was grabbed
							if (activeTransformAxis == 1) {
								// X-axis only movement
								newY = lastDragPos.y;
							}
							else if (activeTransformAxis == 2) {
								// Y-axis only movement
								newX = lastDragPos.x;
							}

							float deltaX = newX - lastDragPos.x;
							float deltaY = newY - lastDragPos.y;

							// Move parent entity
							tc.pos.x = newX;
							tc.pos.y = newY;

							// Move all children recursively
							Editor::Node* draggedNode = sceneTree.getNodeByEntityID(draggedEntityID);
							if (draggedNode) {
								moveChildrenRecursive(draggedNode, deltaX, deltaY, entityManager);
							}

							// Update last position
							lastDragPos = ImVec2(newX, newY);
						}
						else if (isRotating) {
							// Calculate rotation based on mouse angle
							Math::eVec2<float> eEntityScreenPos =
								Graphics::worldToScreen(
									tc.pos,
									g_game->getEditorViewMin(),
									g_game->getEditorViewMax(),
									editorWidth,
									editorHeight,
									g_game->getEditorCameraX(),
									g_game->getEditorCameraY()
								);

							ImVec2 entityScreenPos = {eEntityScreenPos.x, eEntityScreenPos.y};
							float currentAngle = atan2f(mouseScreenPos.y - entityScreenPos.y, mouseScreenPos.x - entityScreenPos.x);
							float newRotation = initialRotation + (currentAngle - initialAngle);
							float deltaAngle = newRotation - tc.rot.x;
							tc.rot.x = newRotation;

							// Rotate all children recursively around parent position
							Editor::Node* rotatedNode = sceneTree.getNodeByEntityID(draggedEntityID);
							if (rotatedNode) {
								rotateChildrenRecursive(rotatedNode, deltaAngle, { tc.pos.x, tc.pos.y }, entityManager);
							}
						}
						else if (isScaling) {
							// Calculate scale based on which handle was grabbed
							float newScaleX = tc.scale.x;
							float newScaleY = tc.scale.y;

							if (activeTransformAxis == 3) {
								// Uniform scaling using diagonal handle
								Math::eVec2<float> entityWorldPosForScaling = {tc.pos.x, tc.pos.y};
								Math::eVec2<float> entityScreenPosForScaling = {
									Graphics::worldToScreen(
									entityWorldPosForScaling,
									g_game->getEditorViewMin(),
									g_game->getEditorViewMax(),
									editorWidth,
									editorHeight,
									g_game->getEditorCameraX(),
									g_game->getEditorCameraY()) };

								float initialDist = sqrtf((scaleStartPos.x - entityScreenPosForScaling.x) * (scaleStartPos.x - entityScreenPosForScaling.x)
														+ (scaleStartPos.y - entityScreenPosForScaling.y) * (scaleStartPos.y - entityScreenPosForScaling.y));

								float currentDist = sqrtf((mouseScreenPos.x - entityScreenPosForScaling.x) * (mouseScreenPos.x - entityScreenPosForScaling.x)
														+ (mouseScreenPos.y - entityScreenPosForScaling.y) * (mouseScreenPos.y - entityScreenPosForScaling.y));

								float scaleFactor = currentDist / std::max(initialDist, 1.0f);

								newScaleX = initialScale.x * scaleFactor;
								newScaleY = initialScale.y * scaleFactor;
							}
							else if (activeTransformAxis == 4) {
								// Horizontal scaling only (X-axis)
								float deltaX = worldPos.x - dragStartWorldPos.x;
								auto& dc = e.get<ECSComponents::DrawInfoComponent>(entityManager);
								float scaleChange = deltaX / std::max(dc.width, 1.0f);
								newScaleX = std::max(0.1f, initialScale.x + scaleChange);
								newScaleY = tc.scale.y;  // Keep Y unchanged
							}
							else if (activeTransformAxis == 5) {
								// Vertical scaling only (Y-axis)
								float deltaY = worldPos.y - dragStartWorldPos.y;
								auto& dc = e.get<ECSComponents::DrawInfoComponent>(entityManager);
								float scaleChange = -deltaY / std::max(dc.height, 1.0f);
								newScaleX = tc.scale.x;  // Keep X unchanged
								newScaleY = std::max(0.1f, initialScale.y + scaleChange);
							}

							// Calculate scale factors (delta)
							float scaleFactorX = newScaleX / std::max(lastScale.x, 0.01f);
							float scaleFactorY = newScaleY / std::max(lastScale.y, 0.01f);

							// Update parent scale
							tc.scale.x = newScaleX;
							tc.scale.y = newScaleY;
							lastScale = ImVec2(newScaleX, newScaleY);

							// Update DrawInfo width and height to reflect the scale
							if (e.has<ECSComponents::DrawInfoComponent>(entityManager)) {
								auto& dc = e.get<ECSComponents::DrawInfoComponent>(entityManager);

								// scale DrawInfo
								dc.width = dc.width * scaleFactorX;
								dc.height = dc.height * scaleFactorY;
								

								// Auto-update collision component if it exists
								if (e.has<ECSComponents::CollisionComponent>(entityManager)) {
									auto& collision = e.get<ECSComponents::CollisionComponent>(entityManager);

									// update collision from DrawInfo
									if (collision.type == ECSComponents::ColliderType::CIRCLE) {
										collision.data.circle.radius = dc.width * 0.5f;
									} else if (collision.type == ECSComponents::ColliderType::RECTANGLE) {
										collision.data.rectangle.width = dc.width;
										collision.data.rectangle.height = dc.height;
									}
									
								}
							}

							// Scale all children recursively from parent position
							Editor::Node* scaledNode = sceneTree.getNodeByEntityID(draggedEntityID);
							if (scaledNode) {
								scaleChildrenRecursive(scaledNode, scaleFactorX, scaleFactorY, { tc.pos.x, tc.pos.y }, entityManager);
							}
						}

						if (g_game)
							g_game->markEntitiesDirty();
						break;
					}
				}
			}

			// End transformation
			if ((isDragging || isRotating || isScaling) && ImGui::IsMouseReleased(0)) {
				std::cout << "Finished transformation on entity " << draggedEntityID << "\n";

				// Use MultiTransformCommand if we have multiple entities affected
				if (pendingTransformCommand && !affectedEntityIDs.empty()) {
					// Capture the new state after transformation
					pendingTransformCommand->captureNewState(affectedEntityIDs);

					// Execute the command through undo manager
					undoManager.executeCommand(std::move(pendingTransformCommand));
					std::cout << "Recorded multi-entity transform for " << affectedEntityIDs.size() << " entities\n";

					affectedEntityIDs.clear();
				}
				else {
					// Fallback to single entity undo (shouldn't normally happen)
					auto allEntities = entityManager.findEntity<ECSComponents::TransformComponent>();
					for (auto& e : allEntities) {
						if (e.id() == draggedEntityID) {
							auto& tc = e.get<ECSComponents::TransformComponent>(entityManager);

							if (isDragging) {
								undoManager.recordMove(e, transformBeforeDrag, tc.pos);
							}
							else if (isRotating) {
								undoManager.recordRotation(e, rotationBeforeDrag, tc.rot);
							}
							else if (isScaling) {
								undoManager.recordScale(e, scaleBeforeDrag, tc.scale);
							}
							break;
						}
					}
				}

				// Clear transformation state
				isDragging = false;
				isRotating = false;
				isScaling = false;
				activeTransformAxis = 0;
				draggedEntityID = 0;
			}

			// Tooltips for different interaction modes
			if (ImGui::IsItemHovered()) {
				if (g_waitingForEntityDeletion) {
					ImGui::BeginTooltip();
					ImGui::Text("Click on an entity to delete it");
					ImGui::EndTooltip();
				}
				else if (!g_selectedPrefabForPlacement.empty()) {
					ImGui::BeginTooltip();
					ImGui::Text("Click to place: %s", g_selectedPrefabForPlacement.c_str());
					ImGui::EndTooltip();
				}
			}
		drawList->PopClipRect();
	}
	ImGui::EndChild();
}

void imGuiSetupDefaultLayout() { // Change this if you wanna edit the amount of space the docking takes
	static bool firstRun = true;
	if (!firstRun) return;
	firstRun = false;

	ImGuiID dockspace_id = ImGui::GetID("MainDockspace");

	// Clear any existing layout
	ImGui::DockBuilderRemoveNode(dockspace_id);
	ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

	// Split the dockspace into sections
	ImGuiID dock_main_id = dockspace_id;
	ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, nullptr, &dock_main_id);
	ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, nullptr, &dock_main_id);
	ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.32f, nullptr, &dock_main_id);

	// Split right side vertically to have Assets on top and Game View on bottom
	ImGuiID dock_id_right_top = ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Up, 0.5f, nullptr, &dock_id_right);
	ImGuiID dock_id_right_bottom = ImGui::DockBuilderSplitNode(dock_id_bottom, ImGuiDir_Right, 0.5f, nullptr, &dock_id_bottom);

	// Dock windows to their respective nodes
	// Left side - all three windows docked to same node will appear as tabs
	ImGui::DockBuilderDockWindow("Hierarchy", dock_id_left);
	ImGui::DockBuilderDockWindow("Prefabs", dock_id_left);
	ImGui::DockBuilderDockWindow("Edit", dock_id_left);
	ImGui::DockBuilderDockWindow("Scene", dock_id_left);

	// Center - Editor View and Raycast 3D View (tabbed)
	ImGui::DockBuilderDockWindow("Editor View",    dock_main_id);
	ImGui::DockBuilderDockWindow("Raycast 3D View", dock_main_id);

	// Right side top - Assets Browser
	ImGui::DockBuilderDockWindow("Entity Inspector", dock_id_right_top);
	ImGui::DockBuilderDockWindow("Assets Presets", dock_id_right_top);
	ImGui::DockBuilderDockWindow("Tilemap Editor", dock_id_right_top);
	ImGui::DockBuilderDockWindow("Tileset", dock_id_right);


	// Right side bottom - Mini Map
	ImGui::DockBuilderDockWindow("Mini Map", dock_id_right_bottom);

	// Bottom - FPS
	ImGui::DockBuilderDockWindow("FPS", dock_id_bottom);
	ImGui::DockBuilderDockWindow("Assets Browser", dock_id_bottom);
	ImGui::DockBuilderDockWindow("Audio Browser", dock_id_bottom);

	ImGui::DockBuilderFinish(dockspace_id);

	std::cout << "Default dockspace layout applied" << std::endl;
}

void imGuiEndDockspace() {
	ImGui::End();
}

void imGuiTerminate() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

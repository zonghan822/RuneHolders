/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         project_picker.cpp
* @author       ZongHan Law (zonghan.law@digipen.edu)
* @brief        Project picker dialog — open, create, and switch between projects.
*//*========================================================================================*/

#include "project_picker.hpp"
#include "../editor_logic.hpp"
#include "file/file_system.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

// g_game is defined in imGui.cpp — needed to mark entities dirty after project switch.
extern EditorApplication* g_game;

// ============================================================
// Project Picker — file-scope state
// ============================================================

static bool                     s_showProjectPicker  = true;
static std::vector<std::string> s_availableProjects;
static int                      s_selectedProjectIdx = 0;
static char                     s_newProjectNameBuf[128] = {};
static bool                     s_creatingNewProject = false;

// ============================================================
// Internal helpers
// ============================================================

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

static void writeGameScriptsCsproj(const std::filesystem::path& dest) {
	std::ofstream out(dest);
	out <<
		"<Project Sdk=\"Microsoft.NET.Sdk\">\n"
		"  <PropertyGroup>\n"
		"    <AssemblyName>GameScripts</AssemblyName>\n"
		"    <OutputType>Library</OutputType>\n"
		"    <RuntimeIdentifier>win-x64</RuntimeIdentifier>\n"
		"    <TargetFramework>net8.0</TargetFramework>\n"
		"    <ImplicitUsings>enable</ImplicitUsings>\n"
		"    <Nullable>enable</Nullable>\n"
		"    <SelfContained>false</SelfContained>\n"
		"    <AllowUnsafeBlocks>true</AllowUnsafeBlocks>\n"
		"    <GenerateRuntimeConfigurationFiles>true</GenerateRuntimeConfigurationFiles>\n"
		"  </PropertyGroup>\n"
		"  <ItemGroup>\n"
		"    <PackageReference Include=\"Microsoft.CodeAnalysis.CSharp\" Version=\"4.14.0\" />\n"
		"    <PackageReference Include=\"Microsoft.CodeAnalysis.CSharp.Scripting\" Version=\"4.14.0\" />\n"
		"  </ItemGroup>\n"
		"  <ItemGroup>\n"
		"    <ProjectReference Include=\"../Core/Core.csproj\" />\n"
		"  </ItemGroup>\n"
		"</Project>\n";
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
	std::filesystem::path dst = scriptsBase / "GameScripts.csproj";
	if (!std::filesystem::exists(dst))
		writeGameScriptsCsproj(dst);

	engine_.setAppName(projectName);
	FileSystem::Initialize(projectName);

	// The project's GameScripts.dll lives in a sibling directory of the Editor exe.
	std::filesystem::path projectDllDir =
		FileSystem::GetExecutableDir().parent_path() / projectName;

	// Build the project scripts only if they haven't been built yet.
	{
		std::filesystem::path csproj         = scriptsBase / "GameScripts.csproj";
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

static void setWindowTitle(const std::filesystem::path& configPath, const std::string& title) {
	std::ifstream in(configPath);
	if (!in.is_open()) return;
	std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	in.close();

	const std::string key = "Window Title: ";
	auto pos = content.find(key);
	if (pos != std::string::npos) {
		auto lineEnd = content.find('\n', pos + key.size());
		content.replace(pos + key.size(),
			(lineEnd == std::string::npos ? content.size() : lineEnd) - (pos + key.size()),
			title);
	}

	std::ofstream out(configPath);
	out << content;
}

static void createNewProject(const std::string& name) {
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
		fs::path dstCsproj = scriptsRoot / "GameScripts.csproj";
		if (!fs::exists(dstCsproj))
			writeGameScriptsCsproj(dstCsproj);
	}

	copyDir("shaders");
	copyTemplate(fs::path("settings") / "config.txt");
	setWindowTitle(projectRoot / "settings" / "config.txt", name);
	copyDir(fs::path("saves") / "prefabs");

	{
		fs::path logFile = projectRoot / "crashlogs" / "crash_log.txt";
		if (!fs::exists(logFile)) std::ofstream{ logFile };
	}

	// Create src/App/<Name>/main.cpp
	{
		fs::path appSrcDir = FileSystem::GetRootDir() / "src" / "App" / name;
		fs::path mainCpp   = appSrcDir / "main.cpp";
		if (!fs::exists(mainCpp)) {
			fs::create_directories(appSrcDir);
			std::ofstream out(mainCpp);
			out <<
				"#include \"engine/eno_engine.hpp\"\n"
				"\n"
				"void EnableMemoryLeakChecking(int breakAlloc = -1)\n"
				"{\n"
				"    int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);\n"
				"    tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;\n"
				"    _CrtSetDbgFlag(tmpDbgFlag);\n"
				"    if (breakAlloc != -1) _CrtSetBreakAlloc(breakAlloc);\n"
				"}\n"
				"\n"
				"#ifdef _DEBUG\n"
				"int main() {\n"
				"#else\n"
				"int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {\n"
				"#endif\n"
				"    try {\n"
				"#ifdef _DEBUG\n"
				"        EnableMemoryLeakChecking();\n"
				"#endif\n"
				"\n"
				"        FileSystem::Initialize(ENO_APP_NAME);\n"
				"\n"
				"        Engine::EnoEngine engine_;\n"
				"        engine_.setup();\n"
				"        engine_.run();\n"
				"        engine_.shutDown();\n"
				"\n"
				"    }\n"
				"    catch (const std::exception& e) {\n"
				"        CrashLogger::logError(e.what());\n"
				"        return -1;\n"
				"    }\n"
				"    catch (...) {\n"
				"        CrashLogger::logError(\"Unknown Error\");\n"
				"    }\n"
				"    return 0;\n"
				"}\n";
		}
	}

	scanAvailableProjects();
	auto it = std::find(s_availableProjects.begin(), s_availableProjects.end(), name);
	if (it != s_availableProjects.end())
		s_selectedProjectIdx = static_cast<int>(std::distance(s_availableProjects.begin(), it));

	std::cout << "[ProjectPicker] Created new project: " << name << std::endl;
}

// ============================================================
// Public API
// ============================================================

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
			createNewProject(s_newProjectNameBuf);
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

void imGuiProjectPickerPopup(Engine::EnoEngine& engine_) {
	if (s_showProjectPicker) {
		ImGui::OpenPopup("Open Project##modal");
	}
	if (ImGui::BeginPopupModal("Open Project##modal", nullptr,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		const float itemW = 400.f;

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
				s_showProjectPicker  = false;
				s_creatingNewProject = false;
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
				createNewProject(s_newProjectNameBuf);
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
}

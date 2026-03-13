/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_script_manager.cpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   50%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   25%
* @co-author    Koe Yun Ji (yunji.koe@digipen.edu)   17%
* @brief        See file name.
*//*========================================================================================*/

#include "eno_script_manager.hpp"
#include "../file/file_system.hpp"

namespace Script
{
	EnoScriptManager::EnoScriptManager(Core::EnoCoreHost& core) :
		coreHost_{ core }, scriptPath{ FileSystem::GetScriptsDir().string() }
	{}

	// Select a script from a file dialog and 
	// create a unique_ptr out of the script to be returned
	std::unique_ptr<EnoScript> EnoScriptManager::selectScript()
	{
		std::unique_ptr<EnoScript> script;
		
		HRESULT result = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

		if (FAILED(result))
		{
			CoUninitialize();
			throw std::runtime_error("COM Init failed!");
		}

		// Init file diag
		// 
		// Source: https://learn.microsoft.com/en-us/windows/win32/learnwin32/example--the-open-dialog-box
		IFileDialog* diag{ nullptr };
		result = CoCreateInstance(CLSID_FileOpenDialog,
			NULL, CLSCTX_ALL, IID_IFileDialog, reinterpret_cast<void**>(&diag));

		if (FAILED(result))
		{
			CoUninitialize();
			throw std::runtime_error("COM Instance creation failed!");
		}

		// Filter for .cs files, but allow the user to view all files as well!
		// 
		// Source: https://cplusplus.com/forum/windows/275617/
		COMDLG_FILTERSPEC filteredFiles[] = { { L"C# Files", L"*.cs"}, { L"All Files", L"*.*"} };
		diag->SetFileTypes(ARRAYSIZE(filteredFiles), filteredFiles);

		// Loop will run until the user exits out of the diag OR
		// when the user picks a .cs file!!!
		while (true)
		{
			result = diag->Show(NULL);

			// User cancels the dialog, so we exit
			if (result == HRESULT_FROM_WIN32(ERROR_CANCELLED))
				break;

			if (SUCCEEDED(result))
			{
				IShellItem* file{ nullptr };
				result = diag->GetResult(&file);

				if (SUCCEEDED(result))
				{
					PWSTR filePath{ nullptr };
					PWSTR fileName{ nullptr };

					result = file->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
					HRESULT result2 = file->GetDisplayName(SIGDN_NORMALDISPLAY, &fileName);
					if (SUCCEEDED(result) && SUCCEEDED(result2))
					{
						std::filesystem::path path{ filePath };

						// Ensure that this is a C# file.
						if (path.extension() == L".cs")
						{
							script = std::make_unique<EnoScript>(coreHost_, fileName, filePath);
							std::cout << "[scriptmng] path : " << script->path() << std::endl;
							// Cleanup
							CoTaskMemFree(filePath);
							CoTaskMemFree(fileName);
							
							file->Release();
							diag->Release();
							
							CoUninitialize();

							return script;
						}
						else
						{
							MessageBoxW(
								NULL,
								L"Invalid file type selected! Please select a C# script(.cs)",
								L"Invalid selection",
								MB_OK | MB_ICONWARNING);
						}
					}

					if (filePath) CoTaskMemFree(filePath);
					if (fileName) CoTaskMemFree(fileName);
					file->Release();
				}
			}
		}
		
		diag->Release();
		CoUninitialize();

		return script;
	}

	// Get available scripts for the user
	std::vector<std::wstring> EnoScriptManager::getAvailableScripts()
	{
		std::vector<std::wstring> result;

		for (auto& entry : std::filesystem::directory_iterator(FileSystem::GetScriptsDir())) {
			if (!entry.is_regular_file() || entry.path().extension() != ".cs")
				continue;

			int count{};
			//cname for classname
			void* names = (coreHost_.cname())(entry.path().wstring().c_str(), &count);
			if (!names || count == 0)
				continue;

			wchar_t** classNames = reinterpret_cast<wchar_t**>(names);
			for (size_t i = 0; i < count; ++i) {
				std::wstring fullName{ classNames[i] };
				size_t dpos = fullName.find('.');
				std::wstring className = (dpos != std::wstring::npos) ? fullName.substr(dpos + 1) : fullName;
				result.push_back(className);
				CoTaskMemFree(classNames[i]);
			}
			CoTaskMemFree(classNames);
		}

		return result;
	}

	std::unique_ptr<EnoScript> EnoScriptManager::loadScript(const std::wstring& className)
	{
		std::wstring fullTypeWithAsm = L"Eno." + className + L", GameScripts"; 
		//std::optional<Core::EnoCoreHost::DelegateInfo> del = coreHost_.loadGameStateDelegate(fullTypeWithAsm.c_str(), L"Begin");
		//if (del) coreHost_.callScript<Core::EnoCoreHost::DelegateInfo>(fullTypeWithAsm.c_str(), method.c_str(), del);

		std::wcout << L"[DEBUG] loadScript() called for: " << className << std::endl;
		// Construct and return your script object as usual

		std::filesystem::path path = FileSystem::GetScriptsDir();
		path /= className + L".cs";
		return std::make_unique<EnoScript>(coreHost_, className, path.wstring());
	}

	std::wstring EnoScriptManager::StringToWString(const std::string& str)
	{
		if (str.empty())
			return L"";

		// 1. Calculate the required buffer size
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), nullptr, 0);

		// 2. Create the wstring buffer
		std::wstring wstrTo(size_needed, 0);

		// 3. Perform the conversion
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &wstrTo[0], size_needed);

		return wstrTo;
	}

	// Create a CS script in the user_scripts folder
	std::unique_ptr<EnoScript> EnoScriptManager::createScript(const std::string& name)
	{
		// Ensure that the name passed will have the .cs extension appended
		std::string filename = appendExtension(name, ".cs");

		// Create the script in the specified folder path
		std::filesystem::path folder{ scriptPath };
		std::filesystem::path absPath = std::filesystem::absolute(folder / filename);
		std::ofstream file(absPath);

		if (!file)
		{
			throw std::runtime_error("Failed to open a file in " + absPath.string());
		}

		// Adapt the default script template
		// with the class name
		file << prepareScript(name);

		// Very very important....waste 2 hours fixing this
		file.close();

		// Create and return the unique EnoScript
		std::wstring wname{ name.begin(), name.end() };
		auto script = std::make_unique<EnoScript>(coreHost_, wname, absPath.make_preferred().wstring());

		return script;
	}

	// Create and open the CS script in the user_scripts folder
	std::unique_ptr<EnoScript> EnoScriptManager::createAndOpenScript(const std::string& name)
	{
		std::string filename = appendExtension(name, ".cs");

		std::filesystem::path folder{ scriptPath };
		std::filesystem::path absPath = std::filesystem::absolute(folder / filename);

		// Create the script if it does not exist
		if (!scriptExists(absPath.string()))
			createScript(name);

		// Create and return the unique EnoScript
		std::wstring wname{ name.begin(), name.end() };

		std::wcout << "name: " << wname << std::endl <<
			"path: " << absPath.wstring() << std::endl;

		auto script = std::make_unique<EnoScript>(coreHost_, wname, absPath.make_preferred().wstring());
		
		openScript(absPath.string());

		return script;
	}

	// Open an existing CS script
	void EnoScriptManager::openScript(const std::string& path)
	{
		// Convert std::string (UTF-8 or ANSI) to UTF-16
		int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, nullptr, 0);
		std::wstring wpath(sizeNeeded, 0);
		MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, &wpath[0], sizeNeeded);

		// Surround with quotes (important if the path contains spaces)
		std::wstring quotedPath = L"\"" + wpath + L"\"";

		std::wcout << L"Opening: " << quotedPath << std::endl;

		HINSTANCE result = ShellExecuteW(nullptr, L"open", L"code", quotedPath.c_str(), nullptr, SW_SHOWNORMAL);
		if ((INT_PTR)result <= 32) 
		{
			std::cerr << "ShellExecute failed with code " << (INT_PTR)result << std::endl;
		}
	}

	// Replace/add necessary script contents
	std::string EnoScriptManager::prepareScript(const std::string& className)
	{
		std::string scriptContents{ scriptTemplate };

		// Look for ${}
		size_t pos = scriptContents.find("${}");
		if (pos != std::string::npos)
		{
			scriptContents.replace(pos, 3, className);
		}

		return scriptContents;
	}

	// Check if a script file exists in the specified location
	bool EnoScriptManager::scriptExists(const std::string& loc)
	{
		std::filesystem::path p = std::filesystem::absolute(loc);

		return std::filesystem::exists(p);
	}

	// Append an extension to a name
	std::string EnoScriptManager::appendExtension(const std::string& str, const std::string& extension)
	{
		return (str.find(extension) == std::string::npos) ? str + extension : str;
	}

	std::string EnoScriptManager::getScriptsDirectory() {
		return FileSystem::GetScriptsDir().string();
	}

	// Helper to check if a script has state machine methods
	bool EnoScriptManager::hasStateMachineMethods(const std::string& scriptPath) {
		std::ifstream file(scriptPath);
		if (!file) return false;

		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string content = buffer.str();

		// Check for pattern: *_enter(), *_update(), *_exit()
		return (content.find("_enter()") != std::string::npos &&
			content.find("_update()") != std::string::npos &&
			content.find("_exit()") != std::string::npos);
	}

	// Get all states defined in a script
	std::vector<std::string> EnoScriptManager::extractStatesFromScript(const std::string& scriptPath) {
		std::vector<std::string> states;
		std::ifstream file(scriptPath);

		if (!file) return states;

		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string content = buffer.str();

		// Find all *_enter() methods
		size_t pos = 0;
		std::string pattern = "_enter()";

		while ((pos = content.find(pattern, pos)) != std::string::npos) {
			// Walk backwards to find the method name
			size_t methodStart = pos;
			while (methodStart > 0 &&
				(std::isalnum(content[methodStart - 1]) ||
					content[methodStart - 1] == '_')) {
				--methodStart;
			}

			std::string methodName = content.substr(methodStart, pos - methodStart);

			// Remove the trailing part to get state name
			if (methodName.length() > 0) {
				// State name is everything before "_enter"
				size_t underscorePos = methodName.rfind('_');
				if (underscorePos != std::string::npos) {
					std::string stateName = methodName.substr(0, underscorePos);
					states.push_back(stateName);
				}
			}

			pos += pattern.length();
		}

		return states;
	}

	// Validate that a script has all required methods for a state
	bool EnoScriptManager::validateStateMethods(const std::string& scriptPath,
		const std::string& stateName) {
		std::ifstream file(scriptPath);
		if (!file) return false;

		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string content = buffer.str();

		std::string lowerState = stateName;
		std::transform(lowerState.begin(), lowerState.end(),
			lowerState.begin(), ::tolower);

		bool hasEnter = content.find(lowerState + "_enter()") != std::string::npos;
		bool hasUpdate = content.find(lowerState + "_update()") != std::string::npos;
		bool hasExit = content.find(lowerState + "_exit()") != std::string::npos;

		return hasEnter && hasUpdate && hasExit;
	}
}
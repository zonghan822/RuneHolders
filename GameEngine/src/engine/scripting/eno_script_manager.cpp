#include "eno_script_manager.hpp"

namespace Script
{
	EnoScriptManager::EnoScriptManager(Core::EnoCoreHost& core) :
		coreHost_{ core }
	{}

	std::optional<std::unique_ptr<EnoScript>> EnoScriptManager::selectScript()
	{
		// Using COM for windows
		HRESULT result = CoInitializeEx(NULL,
			COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

		if (FAILED(result))
		{
			throw std::runtime_error("COM Init failed!");
		}
		else if (SUCCEEDED(result))
		{
			IFileOpenDialog* diag{ nullptr };

			// Create FileOpenDialog
			result = CoCreateInstance(
				CLSID_FileOpenDialog,
				NULL,
				CLSCTX_ALL,
				IID_IFileOpenDialog,
				reinterpret_cast<void**>(&diag));

			if (SUCCEEDED(result))
			{
				// Directory to open
				IShellItem* folder{ nullptr };
				std::filesystem::path scriptFolder = std::filesystem::absolute(scriptPath);
				result = SHCreateItemFromParsingName(scriptFolder.wstring().c_str(), NULL, IID_PPV_ARGS(&folder));
				if (SUCCEEDED(result))
				{
					diag->SetFolder(folder);
					folder->Release();
				}

				// Show dialog
				result = diag->Show(NULL);
				if (SUCCEEDED(result))
				{
					IShellItem* file;
					result = diag->GetResult(&file);

					// Succeeded in picking a file
					// Extract information e.g filename
					if (SUCCEEDED(result))
					{
						PWSTR filePath{ nullptr };
						PWSTR fileName{ nullptr };

						result = file->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
						HRESULT result2 = file->GetDisplayName(SIGDN_NORMALDISPLAY, &fileName);
						if (SUCCEEDED(result) && SUCCEEDED(result2))
						{
							auto script = std::make_unique<EnoScript>(coreHost_, fileName, filePath);

							CoTaskMemFree(filePath);
							CoTaskMemFree(fileName);

							file->Release();
							diag->Release();

							CoUninitialize();

							return script;
						}

						file->Release();
					}
				}
			}

			diag->Release();
		}

		CoUninitialize();
		return std::nullopt;
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

		std::wstring wname{ name.begin(), name.end() };
		auto script = std::make_unique<EnoScript>(coreHost_, wname, absPath.wstring());

		return script;
	}

	// Create and open the CS script in the user_scripts folder
	std::unique_ptr<EnoScript> EnoScriptManager::createAndOpenScript(const std::string& name)
	{
		std::string filename = appendExtension(name, ".cs");

		std::filesystem::path folder{ scriptPath };
		std::filesystem::path absPath = std::filesystem::absolute(folder / filename);

		// Create the script if it does not exist
		if (!scriptExists(name, absPath.string()))
			createScript(name);

		// Create and return EnoScript
		std::wstring wname{ name.begin(), name.end() };
		auto script = std::make_unique<EnoScript>(coreHost_, wname, absPath.wstring());
		
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
		if ((INT_PTR)result <= 32) {
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
	bool EnoScriptManager::scriptExists(const std::string& name, const std::string& loc)
	{
		std::filesystem::path p = std::filesystem::absolute(loc);

		return std::filesystem::exists(p);
	}

	// Append an extension to a name
	std::string EnoScriptManager::appendExtension(const std::string& str, const std::string& extension)
	{
		return (str.find(extension) == std::string::npos) ? str + extension : str;
	}
}
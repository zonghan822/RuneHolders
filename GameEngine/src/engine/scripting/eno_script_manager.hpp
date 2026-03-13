/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         eno_script_manager.hpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   57%
* @co-author    Koe Yun Ji (yunji.koe@digipen.edu)   29%
* @co-author    Caven Koh (caven.koh@digipen.edu)   14%
* @brief        See file name.
*//*========================================================================================*/

#pragma once

// Engine Component(s)
#include "../eno_core_host/eno_core_host.hpp"
#include "eno_script.hpp"
#include "eno_field.hpp"

// Windows
#include <Windows.h>
#include <ShObjIdl.h>

// stds
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <variant>
#include <optional>

namespace Script
{
	class EnoScriptManager
	{
	private:
		std::string scriptPath;

	public:
		EnoScriptManager(Core::EnoCoreHost&);

		// Open scripts folder and browse for a script
		std::unique_ptr<EnoScript> selectScript();
		
		// Create a CS script
		std::unique_ptr<EnoScript> createScript(const std::string&);

		// Create and open a CS script
		std::unique_ptr<EnoScript> createAndOpenScript(const std::string&);

		// Open an existing CS script
		void openScript(const std::string&);

		// Curate script
		std::string prepareScript(const std::string&);

		// Verify that a script exists
		bool scriptExists(const std::string&);

		// All available scripts for user to choose from in Imgui
		std::vector<std::wstring> getAvailableScripts();

		// Addition to selectScript so that it only loads
		std::unique_ptr<EnoScript> loadScript(const std::wstring& className);

        static std::string getScriptsDirectory();

        // Helper to check if a script has state machine methods
        static bool hasStateMachineMethods(const std::string& scriptPath);

        // Get all states defined in a script
		static std::vector<std::string> extractStatesFromScript(const std::string& scriptPath);

        // Validate that a script has all required methods for a state
		static bool validateStateMethods(const std::string& scriptPath, const std::string& stateName);
		
		// Helper function to convert std::string to wstring from script management
		std::wstring StringToWString(const std::string& str);


	private:
		Core::EnoCoreHost& coreHost_;

		static constexpr std::string_view scriptTemplate =
			R"(using System;
using System.Collections.Generic;

namespace Eno
{
	public class ${} : ScriptBase
    {
        public void Begin()
        {

		}

        public void Update()
        {

		}
	}
}
			)";


		// Helper for appending an extension
		std::string appendExtension(const std::string&, const std::string&);
	};
}
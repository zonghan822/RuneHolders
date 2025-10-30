#pragma once

// Engine Component(s)
#include "../coreclr/eno_core_host.hpp"
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
		static constexpr std::string_view scriptPath = "./scripts/user_scripts";

	public:
		EnoScriptManager(Core::EnoCoreHost&);

		// Open scripts folder and browse for a script
		std::optional<std::unique_ptr<EnoScript>> selectScript();
		
		// Create a CS script
		std::unique_ptr<EnoScript> createScript(const std::string&);

		// Create and open a CS script
		std::unique_ptr<EnoScript> createAndOpenScript(const std::string&);

		// Open an existing CS script
		void openScript(const std::string&);

		// Curate script
		std::string prepareScript(const std::string&);

		// Verify that a script exists
		bool scriptExists(const std::string&, const std::string&);

	private:
		Core::EnoCoreHost& coreHost_;

		static constexpr std::string_view scriptTemplate =
			R"(using System;
using System.Collections.Generic;

namespace Eno
{
	public class ${} : ScriptBase
    {
        public static void Begin()
        {

		}

        public static void Update()
        {

		}
	}
}
			)";


		// Helper for appending an extension
		std::string appendExtension(const std::string&, const std::string&);
	};
}
#pragma once

// Engine Component(s)
#include "../event/event_bus.hpp"

#include "eno_script.hpp"

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

	private:

		static constexpr std::string_view scriptTemplate =
			R"(using System;
using System.Runtime.InteropServices;

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
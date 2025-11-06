#pragma once

// Engine Component(s)
#include "../event/event_bus.hpp"

// Field
#include "../scripting/eno_field.hpp"

// Events
#include "../input/eno_input_events.hpp"

// Windows | CoreClr
#include <Windows.h>
#include <coreclr_delegates.h>
#include <nethost.h>
#include <hostfxr.h>

// stds
#include <string>
#include <iostream>
#include <filesystem>
#include <optional>
#include <variant>
#include <cstdlib>
#include <filesystem>

namespace Core
{
	// Source: https://learn.microsoft.com/en-us/dotnet/core/tutorials/netcore-hosting
	class EnoCoreHost
	{
	private:
		const std::string dllPath = SCRIPT_BIN_PATH "/GameScripts.dll";
		const std::string runtConfigJson = SCRIPT_BIN_PATH "/GameScripts.runtimeconfig.json";
	public:

		// Define it as a pointer to a delegate type
		using load_assembly_and_get_function_pointer_fn =
			int32_t(CORECLR_DELEGATE_CALLTYPE*)(
				const char_t* assemblyPath,
				const char_t* typeName,
				const char_t* methodName,
				const char_t* delegateTypeName,
				void* reserved,
				void** delegate);

		struct Del
		{
			load_assembly_and_get_function_pointer_fn fn_;
			std::wstring assemblyPath_;
			std::wstring type_;
			std::wstring method_;
		};

		struct KeyDel
		{
			load_assembly_and_get_function_pointer_fn fn_;
			std::wstring assemblyPath_;
			std::wstring type_;
			std::wstring method_;

			int key_;
			int scancode_;
			int action_;
			int mods_;
		};

		// Types of methods to retrieve from C#
		// ------------------------------------
		// Method with no params to pass in and out
		// Input method with the 4 in params
		// Method to scrape all fields, getting them from a specific script
		// Method for retrieving class names
		//
		// __stdcall is needed to inform that the callee (CoreCLR) will clean the stack (of the CPU thread)
		// so the VS compiler will not clean it
		using Method = void(*)();
		using InputMethod = void(*)(int, int, int, int);
		using GetScriptFieldsMethod = Script::PortedField* (__stdcall*)(const wchar_t* scriptName, int* count);
		using GetFileClassNamesMethod = void* (*)(const wchar_t* pAssemblyPath, int* count);

		EnoCoreHost(Event::EnoEventBus&);
		~EnoCoreHost();

		EnoCoreHost(const EnoCoreHost&) = delete;
		EnoCoreHost(EnoCoreHost&&) = delete;

		GetScriptFieldsMethod fields() const;
		GetFileClassNamesMethod cname() const;

		// Loading a delegate without parameters
		std::optional<Del> loadDelegate(const char_t*, const char_t*, void*);

		// Loading an input delegate
		std::optional<KeyDel> loadDelegate(
			const char_t*, const char_t*, void*,
			const Input::InputSent&);

		// Calling a delegate without any parameters
		void callDelegate(Del&);

		// Calling a delegate with input parameters
		void callDelegate(KeyDel&);

		template <typename T>
		std::optional<T> retrieveDelegate(Del& params);

		// Handling engine state
		void begin();
		void update();
		void frameEnd();

		// ----------------------- Events -----------------------
		void onInputReceived(const Input::InputSent&);

	private:

		// .Net lib
		HMODULE hfLib_;

		// Event bus
		Event::EnoEventBus bus_;

		// Handles
		hostfxr_initialize_for_runtime_config_fn initfp_;	// Handle for initialization of hosting components
		hostfxr_get_runtime_delegate_fn delegatefp_;		// Handle for retrieving a runtime delegate
		hostfxr_close_fn closefp_;							// Handle for closing an initialized host context

		hostfxr_handle hfHandle_;							// For interfacing CoreCLR through hostfxr
		void* stateDelegate;								// Delegate for calling methods responsible for different game/engine states (e.g begin, update)
		void* scriptDelegate_;								// Delegate for retrieving script fields
		void* inputDelegate_;								// Delegate for communnicating input to C# end
		void* cnameDelegate_;								// Delegate for getting 

		GetScriptFieldsMethod fieldsHandle_;
		GetFileClassNamesMethod cnameHandle_;

		std::vector<Del> beginDelegates_;
		std::vector<Del> updateDelegates_;
		std::vector<std::wstring> scriptNames_;


		// Initialize hostfxr library
		void init();

		// Get handles for interfacing with hostfxr
		void getHFHandles();

		// Initialize CoreCLR
		void initCoreCLR();

		// Set CoreCLR delegate
		void setDelegate(void**) const;
	};
}
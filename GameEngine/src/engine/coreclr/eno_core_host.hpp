#pragma once

// Engine Component(s)
#include "../event/event_bus.hpp"

// Events
#include "../input/eno_input_events.hpp"

// Fields
#include "../scripting/eno_field.hpp"

// CoreClr
#include "Windows.h"
#include "hostfxr.h"
#include "nethost.h"
#include "coreclr_delegates.h"

// stds
#include <string>
#include <iostream>
#include <filesystem>
#include <optional>


namespace Core
{
	class EnoCoreHost
	{
	private:
		const std::string dllPath{ "working directory to GameScripts.dll in publish" };
		const wchar_t* runtConfigJson{ L"working directory to GameScripts.runtimeconfig.json in publish" };
		const wchar_t* hfLibPath{ L"working directory to hostfxr.dll" };

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

		// Ensure this matches FieldInfoData
		// under Expose.cs!!!
		//
		// Using wchar_t* here as the 
		// classic char is UTF-8
		// while wchar_t is UTF-16, which matches C# string
		struct Field
		{
			const wchar_t* name_;
			const wchar_t* type_;
			const wchar_t* value_;
			float min_;
			float max_;
			bool hasRange_;
		};
		
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
		// Method with no params to pass in and out								|
		// Input method with the 4 in params									|
		// Method to scrape all fields, getting them from a specific script
		//
		// __stdcall is needed to inform that the callee (CoreCLR) will clean the stack (of the CPU thread)
		// so the VS compiler will not clean it
		using Method = void(*)();
		using InputMethod = void(*)(int, int, int, int);
		using GetScriptFieldsMethod = Script::PortedField* (__stdcall*)(const wchar_t* scriptName, int* count);

		EnoCoreHost(Event::EnoEventBus&);
		~EnoCoreHost();

		EnoCoreHost(const EnoCoreHost&) = delete;
		EnoCoreHost(EnoCoreHost&&) = delete;

		GetScriptFieldsMethod fields() const;

		// Set up events for the cost host
		void eventSetup();

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

		std::optional<GetScriptFieldsMethod> retrieveDelegate(Del&);

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
		void* delegate_;									// Delegate
		void* scriptDelegate_;								// Delegate
		void* inputDelegate_;								// Delegate
		GetScriptFieldsMethod fieldsHandle_;


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
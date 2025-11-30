/*============================================================================================
* @file         eno_core_host.hpp
* @author       Zhe An Lee (zhean.l@digipen.edu)
* @date         30-11-2025
* @brief        Scripting set up and update
*//*========================================================================================*/

#pragma once

// Engine Component(s)
#include "../event/event_bus.hpp"

// Field
#include "../scripting/eno_field.hpp"

// Events
#include "../input/eno_input_events.hpp"
#include "../event/signals.hpp"

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


namespace Core
{
	// Source: https://learn.microsoft.com/en-us/dotnet/core/tutorials/netcore-hosting
	class EnoCoreHost
	{
	private:
		const std::string asmDllPath = SCRIPT_BIN_PATH "GameScripts.dll";
		const wchar_t* runtConfigJson = SCRIPT_BIN_PATH L"GameScripts.runtimeconfig.json";

		//const wchar_t* userScriptPath{ SCRIPT_SRC_PATH };
		//const wchar_t* asmDllAbsPath{ L"C:/Users/Lee Zhe An/source/repos/EnoEngine/scripting/GameScripts/Publish/GameScripts.dll" };

		bool hasBegun_ = false;
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

		// To call methods with no params
		struct DelegateInfo
		{
			load_assembly_and_get_function_pointer_fn fn_;
			std::wstring assemblyPath_;
			std::wstring type_;
			std::wstring method_;
		};

		// For backend methods (such as compiling or reloading)
		struct ScriptingDelegateInfo
		{
			load_assembly_and_get_function_pointer_fn fn_;
			std::wstring assemblyPath_;
			std::wstring type_;
			std::wstring method_;

			const wchar_t* userScriptsPath_;
			const wchar_t* asmDLLPath_;
		};

		// For proxy calling (calling from the proxy class)
		struct ProxyDelegateInfo
		{
			load_assembly_and_get_function_pointer_fn fn_;
			std::wstring assemblyPath_;
			std::wstring type_;
			std::wstring method_;

			const wchar_t* className_;
			const wchar_t* methodName_;
		};

		// For the InputBoarding method (streaming in input from engine end)
		struct KeyDelegateInfo
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

		struct SigDelegateInfo
		{
			load_assembly_and_get_function_pointer_fn fn_;
			std::wstring assemblyPath_;
			std::wstring type_;
			std::wstring method_;

			const wchar_t* key_;
			const wchar_t* scancode_;
			const wchar_t* action_;
			const wchar_t* mods_;
		};

		// For registering instances
		struct RegisterInstProxyDLGInfo
		{
			load_assembly_and_get_function_pointer_fn fn_;
			std::wstring assemblyPath_;
			std::wstring type_;
			std::wstring method_;

			const wchar_t* className_;
			uint32_t entityID_;
			uint8_t op_;
		};

		// For instanced calling
		struct InstancedProxyDLGInfo
		{
			load_assembly_and_get_function_pointer_fn fn_;
			std::wstring assemblyPath_;
			std::wstring type_;
			std::wstring method_;

			const wchar_t* className_;
			const wchar_t* methodName_;
			uint32_t entityID_;
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
		using AddSignalMethod = void(*)(const wchar_t*, const wchar_t*, const wchar_t*, const wchar_t*);
		using ProxyMethod = void(*)(const wchar_t* className_, const wchar_t* methodName_);
		using InstancedProxyMethod = void(*)(const wchar_t* className_, const wchar_t* methodName_, uint32_t entityID);
		using RegisterInstanceProxyMethod = void(*)(const wchar_t* className_, uint32_t entityID, uint8_t operation);
		using GetScriptFieldsMethod = Script::PortedField* (__stdcall*)(const wchar_t* scriptName, int* count);
		using GetFileClassNamesMethod = void* (*)(const wchar_t* pAssemblyPath, int* count);

		EnoCoreHost(Event::EnoEventBus&);
		~EnoCoreHost();

		EnoCoreHost(const EnoCoreHost&) = delete;
		EnoCoreHost(EnoCoreHost&&) = delete;

		std::unordered_map<std::string, Events::EnoSignal>& signalMethods();

		GetScriptFieldsMethod fields() const;
		GetFileClassNamesMethod cname() const;

		// Loading a delegate without parameters
		std::optional<DelegateInfo> loadDelegate(const char_t*, const char_t*, void*);

		// Loading a delegate with two additional path param
		std::optional<EnoCoreHost::ScriptingDelegateInfo> loadDelegate(
			const char_t*, const char_t*, void*,
			const wchar_t*, const wchar_t*);

		// Loading an input delegate
		std::optional<KeyDelegateInfo> loadDelegate(
			const char_t*, const char_t*, void*,
			const Input::InputSent&);

		// Calling a delegate to retrieve script vars
		template <typename T>
		std::optional<T> retrieveDelegate(DelegateInfo& params);

		// Calling a script
		template <typename DelegateInfoType>
		void callScript(const wchar_t*, const wchar_t*, void*);

		template <typename DelegateInfoType>
		void callScript(const wchar_t*, const wchar_t*, void*, const wchar_t*, const wchar_t*);

		// Helper for using the proxy call
		ProxyMethod proxyCall(const wchar_t*, const wchar_t*);
		RegisterInstanceProxyMethod proxyCall(const wchar_t*, uint32_t, uint8_t);
		InstancedProxyMethod proxyCall(const wchar_t*, const wchar_t*, uint32_t);

		// Define and call any method within the assembly via proxy (a method in C#)
		ProxyMethod defineProxyDelegate(
			const char_t*, const char_t*, void*,
			const wchar_t*, const wchar_t*);

		RegisterInstanceProxyMethod defineProxyDelegate(
			const char_t*, const char_t*, void*,
			const wchar_t*, uint32_t, uint8_t);

		InstancedProxyMethod defineProxyDelegate(
			const char_t*, const char_t*, void*,
			const wchar_t*, const wchar_t*, uint32_t);

		// ---------  Handling script state based on engine state ---------
		void HRSetup();
		void hotReload();

		// Script backend (compilation and reloading)

		// Scripts frontend (user scripts)
		void begin(const std::wstring&, uint32_t);
		void update(const std::wstring&, uint32_t);
		void frameEnd();

		void setMetaData(const std::wstring&, uint32_t, uint8_t);

		//need to call this before getting into the gameplay loop
		void defineBegin();
		void defineUpdate();
		void defineSMD();


		void areScriptsModified();

		void runScriptMethod(const std::wstring& className, const wchar_t* method);



		// ----------------------- Events -----------------------
		//void onInputReceived(const Input::InputSent&);

		// =================== Signals ===================

		// Receive a signal emit
		void signalEmitted(uint32_t, const std::string&, const std::string&);

	private:

		// .Net lib
		HMODULE hfLib_;

		// Event bus
		Event::EnoEventBus bus_;

		// Handles
		hostfxr_initialize_for_runtime_config_fn initfp_ = nullptr;	// Handle for initialization of hosting components
		hostfxr_get_runtime_delegate_fn delegatefp_ = nullptr;		// Handle for retrieving a runtime delegate
		hostfxr_close_fn closefp_ = nullptr;							// Handle for closing an initialized host context

		hostfxr_handle hfHandle_;							 // For interfacing CoreCLR through hostfxr
		void* stateDelegate_;                                // Delegate for calling methods responsible for different game/engine states (e.g begin, update)
		void* scriptDelegate_;                               // Delegate for retrieving script fields
		void* proxyDelegate_;                                // Delegate for retrieving script fields
		void* inputDelegate_;                                // Delegate for communnicating input to C# end
		void* cnameDelegate_;                                // Delegate for getting class names
		void* reloadDelegate_;                               // Delegate for handling backend affairs such as script recompilation and reloading							// Delegate for handling backend affairs such as script recompilation and reloading
		void* writeDelegate_;								 // Delegate for writing signal methods using roslyn

		GetScriptFieldsMethod fieldsHandle_ = nullptr;
		GetFileClassNamesMethod cnameHandle_ = nullptr;

		// =================== Signals ===================

		// Signal methods
		std::unordered_map<std::string, Events::EnoSignal> signalMethods_;

		// Initialize hostfxr library
		void init();

		// Get handles for interfacing with hostfxr
		void getHFHandles();

		// Initialize CoreCLR
		void hostRTInit();

		// Set CoreCLR delegate
		void setDelegate(void**) const;

		// Calling a delegate without any parameters
		void callDelegate(DelegateInfo&);

		// Calling a delegate with a script path parameter
		void callDelegate(ScriptingDelegateInfo& params);

		// Calling a delegate with input parameters
		void callDelegate(KeyDelegateInfo&);

		InstancedProxyMethod begin_;
		InstancedProxyMethod update_;
		ProxyMethod synchronize_;
		InstancedProxyMethod signal_;

		RegisterInstanceProxyMethod rpm_;
		uint32_t* lastID{ nullptr };
	};
}
#include "eno_core_host.hpp"

namespace Core
{
	EnoCoreHost::EnoCoreHost(Event::EnoEventBus& bus) : bus_{ bus }
	{
		eventSetup();

		init();

		getHFHandles();

		initCoreCLR();

		setDelegate(&delegate_);
		setDelegate(&scriptDelegate_);

		// Test Player Controller script
		// Namespace.Type(Function), Assembly(The .dll)
		std::optional<Del> dele = loadDelegate(L"Eno.PlayerController, GameScripts", L"Begin", delegate_);

		if (dele)
			callDelegate(dele.value());

		// Test getting the variables from the Player Controller script
		std::optional<Del> dele2 = loadDelegate(L"Eno.ScriptBase, GameScripts", L"GetScriptFields", scriptDelegate_);

		// Setting up handle for retrieving fields
		std::optional<GetScriptFieldsMethod> result;
		if (dele2)
		{
			result = retrieveDelegate(dele2.value());
			if (result)
				fieldsHandle_ = result.value();
		}
	}

	EnoCoreHost::~EnoCoreHost()
	{
		closefp_(hfHandle_);
		FreeLibrary(hfLib_);
	}

	EnoCoreHost::GetScriptFieldsMethod EnoCoreHost::fields() const 
	{ return fieldsHandle_; }

	// Set up events for the cost host
	void EnoCoreHost::eventSetup()
	{
		bus_.subscribe<Input::InputSent>([this](const Input::InputSent& e)
			{
				onInputReceived(e);
			});
	}

	// Initialize hostfxr library
	void EnoCoreHost::init()
	{
		// Load hostfxr module
		hfLib_ = LoadLibraryW(hfLibPath);
		if (!hfLib_)
		{
			std::cerr << "Failed to load hostfxr.dll" << std::endl;
		}
	}

	// Get handles for interfacing with hostfxr
	void EnoCoreHost::getHFHandles()
	{
		initfp_ = (hostfxr_initialize_for_runtime_config_fn)
			GetProcAddress(hfLib_, "hostfxr_initialize_for_runtime_config");
		delegatefp_ = (hostfxr_get_runtime_delegate_fn)
			GetProcAddress(hfLib_, "hostfxr_get_runtime_delegate");
		closefp_ = (hostfxr_close_fn)
			GetProcAddress(hfLib_, "hostfxr_close");

		if (!initfp_ || !delegatefp_ || !closefp_)
		{
			FreeLibrary(hfLib_);
			throw std::runtime_error("Failed to get hostfxr functions");
		}
	}

	// Initialize CoreCLR
	void EnoCoreHost::initCoreCLR()
	{
		// Get runtime context handle for CoreCLR
		int returnCode = initfp_(runtConfigJson, nullptr, &hfHandle_);

		// If returnCode != 0,
		// runtime context was not initialized properly
		if (returnCode != 0 || !hfHandle_)
		{
			closefp_(hfHandle_);
			FreeLibrary(hfLib_);
			throw std::runtime_error("Failed to initialize CoreCLR");
		}
	}

	// Set CoreCLR delegate
	void EnoCoreHost::setDelegate(void** d) const
	{
		int returnCode = delegatefp_(hfHandle_, hdt_load_assembly_and_get_function_pointer, &*d);

		if (returnCode != 0 || !d)
		{
			closefp_(hfHandle_);
			FreeLibrary(hfLib_);
			throw std::runtime_error("Failed to get load_assembly_and_get_function_pointer delegate");
		}
	}

	// Loading a delegate without parameters
	std::optional<EnoCoreHost::Del> EnoCoreHost::loadDelegate(
		const char_t* typeName, const char_t* methodName, 
		void* d)
	{
		auto load = reinterpret_cast<load_assembly_and_get_function_pointer_fn>(d);

		// Get path to managed assembly
		std::filesystem::path asmPath = std::filesystem::absolute(dllPath);
		if (!std::filesystem::exists(asmPath))
		{
			std::cerr << "DLL not found!\n";
			return std::nullopt;
		}

		// Set assembly path, 
		//		type (namespace.class, assemblyName),
		//		method to call
		std::wstring assemblyPath = asmPath.wstring();

		return Del({ 
			load, std::wstring(assemblyPath), std::wstring(typeName), std::wstring(methodName) });
	}

	// Loading an input delegate
	std::optional<EnoCoreHost::KeyDel> EnoCoreHost::loadDelegate(
		const char_t* typeName, const char_t* methodName, void* d,
		const Input::InputSent& input)
	{
		auto load = reinterpret_cast<load_assembly_and_get_function_pointer_fn>(d);

		// Get path to managed assembly
		std::filesystem::path asmPath = std::filesystem::absolute(dllPath);
		if (!std::filesystem::exists(asmPath))
		{
			std::cerr << "DLL not found!\n";
			return std::nullopt;
		}

		// Set assembly path, 
		//		type (namespace.class, assemblyName),
		//		method to call
		std::wstring assemblyPath = asmPath.wstring();

		return KeyDel({ 
			load, std::wstring(assemblyPath), std::wstring(typeName), std::wstring(methodName),
			input.key, input.scancode, input.action, input.mods });
	}

	// Calling a delegate without any parameters
	void EnoCoreHost::callDelegate(Del& params)
	{
		Method call{ nullptr };
		int returnCode = params.fn_(
			params.assemblyPath_.c_str(),
			params.type_.c_str(),
			params.method_.c_str(),
			UNMANAGEDCALLERSONLY_METHOD,
			nullptr,
			reinterpret_cast<void**>(&call)
		);

		if (returnCode || !call)
		{
			std::cerr << "Failed to get function pointer" << std::endl;
			return;
		}
		
		call();
	}

	// Calling a delegate with input parameters
	void EnoCoreHost::callDelegate(KeyDel& params)
	{
		InputMethod call = nullptr;
		int returnCode = params.fn_(
			params.assemblyPath_.c_str(),
			params.type_.c_str(),
			params.method_.c_str(),
			UNMANAGEDCALLERSONLY_METHOD,
			nullptr,
			reinterpret_cast<void**>(&call)
		);

		if (returnCode || !call)
		{
			std::cerr << "Failed to get function pointer" << std::endl;
			return;
		}
		else
		{
			call(params.key_, params.scancode_, params.action_, params.mods_);
		}
	}

	// Retrieve script vars
	std::optional<EnoCoreHost::GetScriptFieldsMethod> EnoCoreHost::retrieveDelegate(Del& params)
	{
		GetScriptFieldsMethod call{ nullptr };
		int returnCode = params.fn_(
			params.assemblyPath_.c_str(),
			params.type_.c_str(),
			params.method_.c_str(),
			UNMANAGEDCALLERSONLY_METHOD,
			nullptr,
			reinterpret_cast<void**>(&call)
		);

		if (returnCode || !call)
		{
			std::cerr << "Failed to get function pointer" << std::endl;
			return std::nullopt;
		}
		
		return call;
	}

	// ----------------------- Events -----------------------

	void EnoCoreHost::onInputReceived(const Input::InputSent& event)
	{
		loadDelegate(L"Eno.Input, GameScripts", L"InputBoarding", inputDelegate_, event);
	}
}
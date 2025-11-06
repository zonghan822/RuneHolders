#include "eno_core_host.hpp"

namespace Core
{
	EnoCoreHost::EnoCoreHost(Event::EnoEventBus& bus) : bus_{ bus }
	{
		init();

		getHFHandles();

		hostRTInit();

		setDelegate(&stateDelegate);
		setDelegate(&inputDelegate_);
		setDelegate(&scriptDelegate_);
		setDelegate(&cnameDelegate_);

		// Test Player Controller script
		// Namespace.Type(Type => Function to be called), Assembly(.dll)

		// Test getting the variables from the Player Controller script
		std::optional<DelegateInfo> dele2 = loadDelegate(L"Eno.ScriptBase, GameScripts", L"GetScriptFields", scriptDelegate_);

		// Setting up handle for retrieving fields
		std::optional<GetScriptFieldsMethod> result;
		if (dele2)
		{
			result = retrieveDelegate<GetScriptFieldsMethod>(dele2.value());
			if (result)
				fieldsHandle_ = result.value();
		}

		std::optional<DelegateInfo> dele3 = loadDelegate(L"Eno.ScriptBase, GameScripts", L"GetFileClassNames", cnameDelegate_);
		std::optional<GetFileClassNamesMethod> result2;
		if (dele3)
		{
			result2 = retrieveDelegate<GetFileClassNamesMethod>(dele3.value());
			if (result2)
				cnameHandle_ = result2.value();
		}
	}

	EnoCoreHost::~EnoCoreHost()
	{
		closefp_(hfHandle_);
		FreeLibrary(hfLib_);
	}

	EnoCoreHost::GetScriptFieldsMethod EnoCoreHost::fields() const 
	{ return fieldsHandle_; }

	EnoCoreHost::GetFileClassNamesMethod EnoCoreHost::cname() const
	{ return cnameHandle_; }

	// Initialize hostfxr library
	void EnoCoreHost::init()
	{
		_putenv_s("DOTNET_ROOT", DOTNET_ROOT);
		
		// Load hostfxr module
		char_t hfLibPath[MAX_PATH];
		size_t hfLibPath_size = sizeof(hfLibPath) / sizeof(char_t);

		int rc = get_hostfxr_path(hfLibPath, &hfLibPath_size, nullptr);
		if (rc != 0)
		{
			throw std::runtime_error("Failed to locate hostfxr");
		}

		hfLib_ = LoadLibraryW(hfLibPath);
		if (!hfLib_)
		{
			throw std::runtime_error("Failed to load hostfxr.dll");
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
	void EnoCoreHost::hostRTInit()
	{
		// Create and initilaize hostfxr runtime context
		// 
		// Prepares runtime environment for hostfxr to interface
		// and load the .NET runtime (CoreCLR)
		
		

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

	// Loading an input delegate
	std::optional<EnoCoreHost::ScriptingDelegateInfo> EnoCoreHost::loadDelegate(
		const char_t* typeName, const char_t* methodName, void* d,
		const wchar_t* usPath, const wchar_t* dllPath)
	{
		auto load = reinterpret_cast<load_assembly_and_get_function_pointer_fn>(d);

		// Get path to managed assembly
		std::filesystem::path asmPath = std::filesystem::absolute(asmDllPath);
		if (!std::filesystem::exists(asmPath))
		{
			std::cerr << "DLL not found!\n";
			return std::nullopt;
		}

		// Set assembly path, 
		//		type (namespace.class, assemblyName),
		//		method to call
		std::wstring assemblyPath = asmPath.wstring();

		return ScriptingDelegateInfo({
			load, std::wstring(assemblyPath), std::wstring(typeName), std::wstring(methodName),
			usPath, dllPath });
	}

	// Loading a delegate without parameters
	std::optional<EnoCoreHost::DelegateInfo> EnoCoreHost::loadDelegate(
		const char_t* typeName, const char_t* methodName, 
		void* d)
	{
		auto load = reinterpret_cast<load_assembly_and_get_function_pointer_fn>(d);

		// Get path to managed assembly
		std::filesystem::path asmPath = std::filesystem::absolute(asmDllPath);
		if (!std::filesystem::exists(asmPath))
		{
			std::cerr << "DLL not found!\n";
			return std::nullopt;
		}

		// Set assembly path, 
		//		type (namespace.class, assemblyName),
		//		method to call
		std::wstring assemblyPath = asmPath.wstring();

		return DelegateInfo({ 
			load, std::wstring(assemblyPath), std::wstring(typeName), std::wstring(methodName) });
	}

	// Loading an input delegate
	std::optional<EnoCoreHost::KeyDelegateInfo> EnoCoreHost::loadDelegate(
		const char_t* typeName, const char_t* methodName, void* d,
		const Input::InputSent& input)
	{
		auto load = reinterpret_cast<load_assembly_and_get_function_pointer_fn>(d);

		// Get path to managed assembly
		std::filesystem::path asmPath = std::filesystem::absolute(asmDllPath);
		if (!std::filesystem::exists(asmPath))
		{
			std::cerr << "DLL not found!\n";
			return std::nullopt;
		}

		// Set assembly path, 
		//		type (namespace.class, assemblyName),
		//		method to call
		std::wstring assemblyPath = asmPath.wstring();

		return KeyDelegateInfo({ 
			load, std::wstring(assemblyPath), std::wstring(typeName), std::wstring(methodName),
			input.key, input.scancode, input.action, input.mods });
	}

	// Calling a delegate without any parameters
	void EnoCoreHost::callDelegate(DelegateInfo& params)
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
	void EnoCoreHost::callDelegate(ScriptingDelegateInfo& params)
	{
		ScriptingMethod call{ nullptr };
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
			call(params.userScriptsPath_, params.asmDLLPath_);
		}
	}

	// Calling a delegate with input parameters
	void EnoCoreHost::callDelegate(KeyDelegateInfo& params)
	{
		InputMethod call{ nullptr };
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
	template <typename T>
	std::optional<T> EnoCoreHost::retrieveDelegate(DelegateInfo& params)
	{
		T call{ nullptr };
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

	void EnoCoreHost::scriptingSetup()
	{
		// Load and start watching for changes made

		// Detect if changes have been made to the assembly
		// e.g modifying a .cs script
		callScript<ScriptingDelegateInfo>(
			L"Eno.Initiator, GameScripts", L"InitSequence", reloadDelegate_,
			userScriptPath, asmDllAbsPath);
	}

	// Hot reload
	void EnoCoreHost::hotReload()
	{
		std::cout << "Hot reloading is occurring!" << std::endl;
		callScript<ScriptingDelegateInfo>(
			L"Eno.Initiator, GameScripts", L"ReloadSequence", reloadDelegate_,
			userScriptPath, asmDllAbsPath);
	}

	void EnoCoreHost::begin()
	{
		// Calls Begin method on scripts
		callScript<DelegateInfo>(L"Eno.PlayerController, GameScripts", L"Begin", stateDelegate);
	}

	// Handling Update on scripts
	void EnoCoreHost::update()
	{
		// Calls Update method on scripts
		callScript<DelegateInfo>(L"Eno.PlayerController, GameScripts", L"Update", stateDelegate);
	}

	// Handling state when frame has ended on the engine side
	void EnoCoreHost::frameEnd()
	{
		// Resets input when no input is received
		callScript<DelegateInfo>(L"Eno.Input, GameScripts", L"noInputReceived", stateDelegate);
	}

	// Detect change in script
	void EnoCoreHost::areScriptsModified()
	{
		// Modify this with a map next time to map out scripts and times
		static std::filesystem::file_time_type previousWriteTime{};

		auto lastWrite = std::filesystem::last_write_time(asmDllPath);
		if (lastWrite != previousWriteTime)
		{
			previousWriteTime = lastWrite;
			hotReload();
		}
	}

	// Call a method from a specified type (namespace.class, dll)
	template <typename DelegateInfoType>
	void EnoCoreHost::callScript(const wchar_t* type, const wchar_t* method, void* deletype)
	{
		std::optional<DelegateInfoType> dele = loadDelegate(type, method, deletype);

		if (dele.has_value())
			callDelegate(dele.value());
		else
			std::cerr << "Failed to retrieve delegate info upon loading!" << std::endl;
	}

	// Overload for calling scripting-related methods with 1 path argument
	template <typename DelegateInfoType>
	void EnoCoreHost::callScript(
		const wchar_t* type, const wchar_t* method, void* deletype,
		const wchar_t* usPath, const wchar_t* dllPath)
	{
		std::optional<ScriptingDelegateInfo> dele = loadDelegate(type, method, deletype, usPath, dllPath);
		callDelegate(dele.value());
	}

	// ----------------------- Events -----------------------

	void EnoCoreHost::onInputReceived(const Input::InputSent& event)
	{
		KeyDelegateInfo kd = loadDelegate(L"Eno.Input, GameScripts", L"InputBoarding", inputDelegate_, event).value();

		callDelegate(kd);
	}
}
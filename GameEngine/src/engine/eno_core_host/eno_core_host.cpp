#include "eno_core_host.hpp"

namespace Core
{
	EnoCoreHost::EnoCoreHost(Event::EnoEventBus& bus) : bus_{ bus }
	{
		init();

		getHFHandles();

		hostRTInit();

		setDelegate(&stateDelegate_);
		setDelegate(&inputDelegate_);
		setDelegate(&proxyDelegate_);
		setDelegate(&scriptDelegate_);
		setDelegate(&cnameDelegate_);
		setDelegate(&reloadDelegate_);

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
		// Safely close the runtime context
		if (closefp_ && hfHandle_)
		{
			closefp_(hfHandle_);
			hfHandle_ = nullptr;
		}

		// Unload hostfxr.dll
		if (hfLib_)
		{
			FreeLibrary(hfLib_);
			hfLib_ = nullptr;
		}

		// Invalidate managed delegates
		fieldsHandle_ = nullptr;
		cnameHandle_ = nullptr;
		stateDelegate_ = nullptr;
		scriptDelegate_ = nullptr;
		inputDelegate_ = nullptr;
		cnameDelegate_ = nullptr;
	}

	EnoCoreHost::GetScriptFieldsMethod EnoCoreHost::fields() const
	{
		return fieldsHandle_;
	}

	EnoCoreHost::GetFileClassNamesMethod EnoCoreHost::cname() const
	{
		return cnameHandle_;
	}

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
			if (closefp_ && hfHandle_) closefp_(hfHandle_);
			if (hfLib_) FreeLibrary(hfLib_);
			hfHandle_ = nullptr;
			hfLib_ = nullptr;
			throw std::runtime_error("Failed to initialize CoreCLR");
		}
	}

	// Set CoreCLR delegate
	void EnoCoreHost::setDelegate(void** d) const
	{
		int returnCode = delegatefp_(hfHandle_, hdt_load_assembly_and_get_function_pointer, d);

		if (returnCode != 0 || *d == nullptr)
		{
			if (closefp_ && hfHandle_) closefp_(hfHandle_);
			if (hfLib_) FreeLibrary(hfLib_);
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


		//std::wcout << "params asm: " << params.assemblyPath_.c_str() << '\n';
		//std::wcout << "params type: " << params.type_.c_str() << '\n';
		//std::wcout << "params method: " << params.method_.c_str() << '\n';


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

	void EnoCoreHost::HRSetup()
	{
		// Load and start watching for changes made

		// Detect if changes have been made to the assembly
		// e.g modifying a .cs script

	// Build the correct absolute or relative paths




		// Use the std::wstring variables here, NOT the macros directly
		callScript<ScriptingDelegateInfo>(
			L"Eno.Initiator, GameScripts",
			L"InitSequence",
			reloadDelegate_, 
			WSCRIPT_SRC_PATH,
			WSCRIPT_BIN_PATH);
	}

	// Hot reload
	void EnoCoreHost::hotReload()
	{



		callScript<ScriptingDelegateInfo>(
			L"Eno.Initiator, GameScripts",
			L"ReloadSequence", 
			reloadDelegate_,
			WSCRIPT_SRC_PATH,
			WSCRIPT_BIN_PATH);
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

	// Can change this later but its 4.50 am method can be either begin or update
	void EnoCoreHost::runScriptMethod(const std::wstring& className, const wchar_t* method)
	{
		std::wstring fullTypeWithAsm = L"Eno." + className + L", GameScripts";
		std::optional<DelegateInfo> del = loadDelegate(fullTypeWithAsm.c_str(), method, stateDelegate);



		if (del)
		{
			std::wcout << L"[DEBUG] Running " << method << L"() for " << fullTypeWithAsm << L"\n";
			callDelegate(del.value());
		}
		else
		{
			std::wcerr << L"[WARN] Could not load delegate for " << fullTypeWithAsm << L" (" << method << L")\n";
		}
	}

	void EnoCoreHost::defineBegin() {


		for (auto& entry : std::filesystem::directory_iterator(SCRIPT_SRC_PATH)) {
			if (!entry.is_regular_file() || entry.path().extension() != ".cs")
				continue;

			// stores number of class names were returned.
			int count{};
			void* names = (cname())(entry.path().wstring().c_str(), &count);

			//continue if there is no class names available
			if (!names || count == 0)
				continue;

			// Cast and loop over each class name using double ptr 
			wchar_t** classNames = reinterpret_cast<wchar_t**>(names);


			for (size_t i = 0; i < count; ++i) {

				std::wstring fullName{ classNames[i] };
				size_t dpos = fullName.find('.');

				std::wstring className = (dpos != std::wstring::npos) ? fullName.substr(dpos + 1) : fullName;
				std::wstring fullTypeWithAsm = L"Eno." + className;


				begin_.push_back(std::make_pair(proxyCall(fullTypeWithAsm.c_str(), L"Begin"), std::make_pair(fullTypeWithAsm, L"Begin")));


				CoTaskMemFree(classNames[i]);
			}
			CoTaskMemFree(classNames);
		}

	}


	void EnoCoreHost::defineUpdate() {

		for (auto& entry : std::filesystem::directory_iterator(SCRIPT_SRC_PATH)) {
			if (!entry.is_regular_file() || entry.path().extension() != ".cs")
				continue;

			// stores number of class names were returned.
			int count{};
			void* names = (cname())(entry.path().wstring().c_str(), &count);

			//continue if there is no class names available
			if (!names || count == 0)
				continue;

			// Cast and loop over each class name using double ptr 
			wchar_t** classNames = reinterpret_cast<wchar_t**>(names);


			for (size_t i = 0; i < count; ++i) {

				std::wstring fullName{ classNames[i] };
				size_t dpos = fullName.find('.');

				std::wstring className = (dpos != std::wstring::npos) ? fullName.substr(dpos + 1) : fullName;
				std::wstring fullTypeWithAsm = L"Eno." + className;

				//std::vector<std::pair<ProxyMethod, std::pair<const wchar_t*, const wchar_t*>>> begin_;


				update_.push_back(std::make_pair(proxyCall(fullTypeWithAsm.c_str(), L"Update"), std::make_pair(fullTypeWithAsm, L"Update")));


				CoTaskMemFree(classNames[i]);
			}
			CoTaskMemFree(classNames);
		}
	}


	void EnoCoreHost::begin()
	{
		// Calls Begin method on scripts
#if 0
        std::optional<DelegateInfo> dele = loadDelegate(L"Eno.PlayerController, GameScripts", L"Begin", stateDelegate);
		callDelegate(dele.value());
        std::optional<DelegateInfo> dele1 = loadDelegate(L"Eno.EnemyController, GameScripts", L"Begin", stateDelegate);
		callDelegate(dele1.value());
		//std::optional<Del> dele1 = loadDelegate(L"Eno.EnemyController, GameScripts", L"Begin", stateDelegate);
		//callDelegate(dele1.value());

		// seperator

		//This calls every begin that exists as of current
		for (auto& entry : std::filesystem::directory_iterator("../../../scripts/GameScripts/user_scripts/")) {
			if (!entry.is_regular_file() || entry.path().extension() != ".cs")
				continue;

			// stores number of class names were returned.
			int count{};
			void* names = (cname())(entry.path().wstring().c_str(), &count);

			//continue if there is no class names available
			if (!names || count == 0)
				continue;

			// Cast and loop over each class name using double ptr 
			wchar_t** classNames = reinterpret_cast<wchar_t**>(names);


			for (size_t i = 0; i < count; ++i) {

				std::wstring fullName{ classNames[i]};
				size_t dpos = fullName.find('.');

				std::wstring className = (dpos != std::wstring::npos) ? fullName.substr(dpos + 1) : fullName;
				std::wstring fullTypeWithAsm = L"Eno." + className + L", GameScripts";

				std::optional<DelegateInfo> beginDel = loadDelegate(fullTypeWithAsm.c_str(), L"Begin", stateDelegate);
				if (beginDel) callDelegate(beginDel.value());


				CoTaskMemFree(classNames[i]);
			}
			CoTaskMemFree(classNames);
		}
#else
		std::cout << "corehost begin run now\n" << "begin size: " << begin_.size();
		for (const auto& Pairs : begin_) {
			if (Pairs.first) {
				Pairs.first(Pairs.second.first.c_str(), Pairs.second.second.c_str());
			}
		}

#endif
	}

	// Handling Update on scripts
	void EnoCoreHost::update()
	{
#if 0
		// Calls Update method on scripts
		std::optional<DelegateInfo> dele = loadDelegate(L"Eno.PlayerController, GameScripts", L"Update", stateDelegate);
		callDelegate(dele.value());

		std::optional<DelegateInfo> dele1 = loadDelegate(L"Eno.EnemyController, GameScripts", L"Update", stateDelegate);
		callDelegate(dele1.value());

		//seperator
		for (auto& entry : std::filesystem::directory_iterator("../../../scripts/GameScripts/user_scripts/")) {
			if (!entry.is_regular_file() || entry.path().extension() != ".cs")
				continue;

			// stores number of class names were returned.
			int count{};
			void* names = (cname())(entry.path().wstring().c_str(), &count);

			//continue if there is no class names available
			if (!names || count == 0)
				continue;

			// Cast and loop over each class name using double ptr 
			wchar_t** classNames = reinterpret_cast<wchar_t**>(names);


			for (size_t i = 0; i < count; ++i) {

				std::wstring fullName{ classNames[i] };
				size_t dpos = fullName.find('.');

				std::wstring className = (dpos != std::wstring::npos) ? fullName.substr(dpos + 1) : fullName;
				std::wstring fullTypeWithAsm = L"Eno." + className + L", GameScripts";

				std::optional<DelegateInfo> beginDel = loadDelegate(fullTypeWithAsm.c_str(), L"Update", stateDelegate);
				if (beginDel) callDelegate(beginDel.value());


				CoTaskMemFree(classNames[i]);
			}
			CoTaskMemFree(classNames);
		}
#else
		for (const auto& Pairs : update_) {
			if (Pairs.first) {
				Pairs.first(Pairs.second.first.c_str(), Pairs.second.second.c_str());
			}
		}
#endif
	}
	//All within cs
// L Eno ->Namespace     //L is an "alternate form" of char
// PlayerController -> Class
// GameScripts -> dll
// L widestring"Begin" -> Begin is the function to call
// stateDelegate is a void pointer (Delegate is a function to call from cs) // see as if its (*)(void)

// todo 1: 
// Looping through filesystem to look for scripts.cs and run all begin^ and updatev functions
// Call them appropriately

// todo 2: (optional for now...)
// To have a queue system for the begin
//



// Handling state when frame has ended on the engine side
	void EnoCoreHost::frameEnd()
	{
		// Resets input when no input is received
		//std::optional<Del> dele = loadDelegate(L"Eno.Input, GameScripts", L"noInputReceived", stateDelegate_);
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

	// Helper for using the proxy call
	EnoCoreHost::ProxyMethod EnoCoreHost::proxyCall(const wchar_t* className, const wchar_t* methodName)
	{
		return defineProxyDelegate(
			L"Eno.ProxyEntry, GameScripts",
			L"CallProxyMethod",
			proxyDelegate_,
			className,
			methodName);
	}

	// Creates a special proxy delegate used to call other methods in C#
	// Combines both the load and get delegate operations
	EnoCoreHost::ProxyMethod EnoCoreHost::defineProxyDelegate(
		const char_t* typeName, const char_t* methodName, void* d,
		const wchar_t* className, const wchar_t* callMethodName)
	{
		auto load = reinterpret_cast<load_assembly_and_get_function_pointer_fn>(d);

		// Get path to managed assembly
		std::filesystem::path asmPath = std::filesystem::absolute(asmDllPath);
		if (!std::filesystem::exists(asmPath))
		{
			throw std::runtime_error("Managed assembly could not be found!");
		}

		// Set assembly path, 
		//		type (namespace.class, assemblyName),
		//		method to call
		std::wstring assemblyPath = asmPath.wstring();

		// Create Delegate Info
		ProxyDelegateInfo params{
			load,
			std::wstring(assemblyPath),
			std::wstring(typeName),
			std::wstring(methodName),
			className, callMethodName };

		// Create the actual method delegate
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
			throw std::runtime_error("Could not define the proxy method to be used!");
		}

		return call;
	}

	// ----------------------- Events -----------------------

	void EnoCoreHost::onInputReceived(const Input::InputSent& event)
	{
		KeyDelegateInfo kd = loadDelegate(L"Eno.Input, GameScripts", L"InputBoarding", inputDelegate_, event).value();

		callDelegate(kd);
	}
}
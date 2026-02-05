#include "eno_script.hpp"


namespace Script
{
#if 1
	EnoScript::EnoScript(Core::EnoCoreHost& core, const std::wstring& name, const std::wstring& path) :
		coreHost_{ core },
		name_{ name },
		path_{ path }
	{
		// All fields imported from C# will be "unmanaged memory" so
		// I am making an extra operation to port them in as PortedField
		// into Field data structs so that I can convert all wchar_t*
		// fields into std::wstring fields instead. A little extra overhead
		// but I dont have to worry about memory management now!
		// 
		// MUST use CoTaskMemFree! wchar_t* is memory allocated by the COM task memory allocator
		// NOT the CRT heap!!! Likely implicitly with CoTaskMemAlloc. Hence, use the COM heap to deallocate!
		// Source: https://stackoverflow.com/questions/10055607/when-using-cotaskmemalloc-should-i-always-call-cotaskmemfree
		

		// Get all script variables
		int count{};
		void* names = (coreHost_.cname())(path.c_str(), &count);
		
		// Map script variables
		wchar_t** classNames = reinterpret_cast<wchar_t**>(names);
		for (size_t i = 0; i < count; i++)
		{
			std::wstring org = classNames[i];
			std::wstring cname = extrClassName(classNames[i]);

			map(cname);
			origin_.insert(org);

			// Cleanup names after use
			CoTaskMemFree(classNames[i]);
		}
	}
#else
	EnoScript::EnoScript(Core::EnoCoreHost& core, const std::wstring& name, const std::wstring& path)
		: coreHost_{ core }, name_{ name }, path_{ path }
	{
		std::wcout << L"\n[DEBUG] ===== EnoScript Constructor =====" << std::endl;
		std::wcout << L"[DEBUG] Name: " << name_ << std::endl;
		std::wcout << L"[DEBUG] Path: " << path_ << std::endl;

		if (!std::filesystem::exists(path_))
		{
			std::wcerr << L"[ERROR] File does not exist at path: " << path_ << std::endl;
			return;
		}

		if (!coreHost_.cname())
		{
			std::wcerr << L"[ERROR] coreHost_.cname() delegate is null! Did you initialize CoreCLR and set delegates?" << std::endl;
			return;
		}

		int count{};
		std::wcout << L"[DEBUG] Calling cname()..." << std::endl;
		void* names = nullptr;

		try
		{
			names = (coreHost_.cname())(path_.c_str(), &count);
			std::wcout << L"[DEBUG] cname() call succeeded. Count = " << count << std::endl;
		}
		catch (...)
		{
			std::wcerr << L"[EXCEPTION] cname() threw an exception. Possibly invalid delegate or path encoding." << std::endl;
			return;
		}

		if (!names)
		{
			std::wcerr << L"[WARN] cname() returned null pointer (no classes found or C# side failed)." << std::endl;
			return;
		}

		if (count <= 0)
		{
			std::wcerr << L"[WARN] cname() returned count = " << count << L". Nothing to map." << std::endl;
			CoTaskMemFree(names);
			return;
		}

		wchar_t** classNames = reinterpret_cast<wchar_t**>(names);
		std::wcout << L"[DEBUG] Mapping class names..." << std::endl;

		for (size_t i = 0; i < static_cast<size_t>(count); ++i)
		{
			if (!classNames[i])
			{
				std::wcerr << L"[WARN] classNames[" << i << L"] is null. Skipping." << std::endl;
				continue;
			}

			std::wstring cname = extrClassName(classNames[i]);
			std::wcout << L"[DEBUG]   [" << i << L"] Extracted class: " << cname << std::endl;

			try
			{
				map(cname);
			}
			catch (const std::exception& e)
			{
				std::wcerr << L"[EXCEPTION] During map(): " << e.what() << std::endl;
			}

			CoTaskMemFree(classNames[i]);
		}

		std::wcout << L"[DEBUG] Freeing names array..." << std::endl;
		CoTaskMemFree(classNames);

		std::wcout << L"[DEBUG] ===== EnoScript Constructor End =====\n" << std::endl;
	}
#endif



	// Extract the class name
	std::wstring EnoScript::extrClassName(wchar_t* name)
	{
		std::wstring fullName{ name };
		size_t dpos = fullName.find('.');

		// Keep the class name which comes after the namespace name
		// if it is present
		std::wstring className = (dpos != std::wstring::npos) ? fullName.substr(dpos + 1) : fullName;

		return className;
	}

	// Map revealed variables from all classes
	// within a script
	void EnoScript::map(const std::wstring& cname)
	{
		const wchar_t* className = cname.c_str();

		// Call delegate to retrieve revealed variables from script
		int count{};
		PortedField* data = (coreHost_.fields())(className, &count);

		// Mapping
		for (size_t i = 0; i < count; i++)
		{
			std::wstring varname{ data[i].name_ };
			vars_.emplace(std::move(varname), mimicField(data[i]));
		}

		cleanup(data);
	}

	// Free all fields allocated memory with IntPtr from C#
	void EnoScript::cleanup(PortedField* data)
	{
		if (data) {
			if (data->name_) CoTaskMemFree((LPVOID)data->name_);
			if (data->type_) CoTaskMemFree((LPVOID)data->type_);
			if (data->value_) CoTaskMemFree((LPVOID)data->value_);
			if (data->origin_) CoTaskMemFree((LPVOID)data->origin_);
		}
	}

	// Display revealed variables
	void EnoScript::displayVars()
	{
		std::wcout << L"All Revealed Script Variables" << std::endl 
				   << L"____________________________" << std::endl;
		for (const auto& [key, value] : vars_)
		{
			std::wcout << std::left << std::setw(25) << 
						  L"Namespace.Class: " << value.origin_ << std::endl <<
						  std::setw(25) << 
						  L"Variable: " << key << std::endl <<
						  std::setw(25) << 
						  L"Value: " << value.value_ << std::endl <<
						  L"-------------------------------------------" << std::endl << std::endl;
		}
		std::cout << std::endl;
	}

	// Create a Field out of a PortedField
	Field EnoScript::mimicField(const PortedField& pField)
	{
		return Field{ pField.name_, pField.type_, deduceType(pField.type_, pField.value_),
					  pField.min_, pField.max_, pField.hasRange_,
					  pField.origin_ };
	}

	// Deduce value type of field
	Value EnoScript::deduceType(const wchar_t* type, const wchar_t* val)
	{
		auto it = convertf.find(type);
		return it != convertf.end() ? it->second(val) : std::wstring(val);
	}

	// Modify the value of a script field
	void EnoScript::modValue(const std::wstring& var, const Value& newValue)
	{
		// Signal to a delegate that value has changed
		auto it = vars_.find(var);
		if (it != vars_.end())
		{
			it->second.value_ = newValue;
		}
	}

	std::wstring EnoScript::getName() const { return name_; }
	std::string EnoScript::path() { return wideToUtf8(path_); }
	std::unordered_multimap<std::wstring, Field>& EnoScript::vars() { return vars_; }
	std::unordered_set<std::wstring>& EnoScript::origin() { return origin_; }

	// Helper for converting wide strings to standard utf-8
	std::string EnoScript::wideToUtf8(const std::wstring& wstr)
	{
		if (wstr.empty()) return {};

		// Get number of bytes needed for UTF-8 conversion
		// Allocate a string of suitable size and convert
		int size = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), NULL, 0, NULL, NULL);
		std::string str(size, 0);
		WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), str.data(), size, NULL, NULL);

		return str;
	}

	// ================== Signal ==================
	std::pair<std::string, std::string> EnoScript::signalWrite(std::string nodeName, std::string signalName)
	{
		std::fstream file{ path_, std::ios::in | std::ios::out };
		if (!file)
			throw std::runtime_error("Failed to find the file to write to!");

		// Lowercase all
		Conversion::strToLower(nodeName);
		Conversion::strToLower(signalName);

		// Form the signal function signature
		std::string funcName =
			std::string{ signalSuffix } +
			nodeName +
			'_' +
			signalName;

		std::string funcSignature =
			std::string{ signalKeyWords } +
			std::string{ signalSuffix } +
			nodeName + '_' +
			signalName + "()\n"
			"        {\n\n"

			"        }\n";

		// Write the signal func on the bottom of the script
		std::stringstream stream{};
		stream << file.rdbuf();
		std::string data = stream.str();

		// Verify that this signal func has not already been written
		std::string className = classToNest(data);

		size_t fs = data.find(funcName);
		if (fs != std::string::npos)
		{
			std::cout << funcName << " has already been defined in the script!" << std::endl;
			return { funcName, className };
		}

		// Write and close file
		writeMethod(data, funcSignature);

		file.close();
		std::ofstream out{ path_, std::ios::trunc };
		out << data;

		return { funcName, className };
	}

	void EnoScript::writeMethod(std::string& data, const std::string& method)
	{
		// Find top level class
		size_t cpos = data.find("class ");
		if (cpos == std::string::npos)
		{
			std::cerr << "No class found! Will not continue writing the signal method!" << std::endl;
			return;
		}

		// Find start of the line containing "class "
		size_t lineStart = data.rfind('\n', cpos);
		if (lineStart == std::string::npos)
			lineStart = 0;
		else
			++lineStart;

		// Get indentation before "class"
		size_t indentEnd = lineStart;
		while (indentEnd < data.size() && (data[indentEnd] == ' ' || data[indentEnd] == '\t'))
			++indentEnd;

		std::string classIndent = data.substr(lineStart, indentEnd - lineStart);

		// Find the opening brace
		size_t openBrace = data.find("{", cpos);
		if (openBrace == std::string::npos)
		{
			std::cerr << "Class has no opening brace!" << std::endl;
			return;
		}

		// Find closing brace
		int depth{ 1 };
		size_t pos{ openBrace + 1 };

		while (pos < data.size() && depth > 0)
		{
			if (data[pos] == '{') ++depth;
			else if (data[pos] == '}') --depth;

			++pos;
		}

		if (depth != 0)
		{
			std::cerr << "Braces not indented properly!" << std::endl;
			return;
		}

		// Write
		size_t writePos = pos - 1;
		data.insert(writePos, method + classIndent);
	}

	std::string EnoScript::classToNest(const std::string& data)
	{
		// Find top level class
		size_t cpos = data.find("class ");
		if (cpos == std::string::npos)
		{
			std::cerr << "No class found!" << std::endl;
			return "";
		}

		// Start position of class name (right after "class ")
		size_t nameStart = cpos + 6;

		// Skip whitespace
		while (nameStart < data.size() && (data[nameStart] == ' ' || data[nameStart] == '\t'))
			++nameStart;

		// End of class name: either whitespace, {, or :
		size_t nameEnd = nameStart;
		while (nameEnd < data.size() &&
			!(data[nameEnd] == ' ' || data[nameEnd] == '\t' ||
				data[nameEnd] == '{' || data[nameEnd] == ':'))
		{
			++nameEnd;
		}

		return data.substr(nameStart, nameEnd - nameStart);
	}

	std::wostream& operator<<(std::wostream& os, const Value& value)
	{
		std::visit([&os](const auto& val)
			{
				os << val;
			}, value);

		return os;
	}
}
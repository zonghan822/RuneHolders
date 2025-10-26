#include "eno_script.hpp"


namespace Script
{
	EnoScript::EnoScript(const std::wstring& name, const std::wstring& path) :
		name_{ name },
		path_{ path }
	{}

	//std::string EnoScript::path() { return wideToUtf8(path_); }

	// Helper for converting wide strings to standard utf-8
	//std::string EnoScript::wideToUtf8(const std::wstring& wstr)
	//{
	//	if (wstr.empty()) return {};

	//	// Get number of bytes needed for UTF-8 conversion
	//	// Allocate a string of suitable size and convert
	//	int size = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), NULL, 0, NULL, NULL);
	//	std::string str(size, 0);
	//	WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), str.data(), size, NULL, NULL);

	//	return str;
	//}
}
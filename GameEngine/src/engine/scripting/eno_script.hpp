#pragma once

// Windows | Corehost
#include <Windows.h>
#include <ShObjIdl.h>
#include "../eno_core_host/eno_core_host.hpp"

// Field Struct
#include "eno_field.hpp"

// Value
#include "eno_field_value.hpp"

// Utility
#include "../utility/eno_conversion.hpp"

// stds
#include <variant>
#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <fstream>

namespace Script
{
	struct ScriptEvent {
		uint32_t entityID;
		const char* methodName;

	};

	class EnoScript
	{
	public:
		EnoScript(Core::EnoCoreHost&, const std::wstring&, const std::wstring&);

		void registerInst(uint32_t id);
		std::wstring extrClassName(wchar_t*);
		void map(const std::wstring&);
		void displayVars();
		void cleanup(PortedField*);
		Field mimicField(const PortedField&);
		Value deduceType(const wchar_t*, const wchar_t*);
		std::wstring getName() const;

		// Accessor(s)
		std::string path();
		std::unordered_multimap<std::wstring, Field>& vars();
		std::unordered_set<std::wstring>& origin();

		// Modifier(s)
		void modValue(const std::wstring&, const Value&);

		//YunQi moved this out
		std::wstring name_;

		// ================== Signal ==================

		std::pair<std::string, std::string> signalWrite(std::string, std::string);
		void writeMethod(std::string&, const std::string&);
		std::string classToNest(const std::string&);

		//Yun Qi's dummy set of states. Will use to populate and serialise for the IMGUI later on
		//std::unordered_map<std::string, std::string> dummystates; //first holds state, second holds texture?

	private:
		Core::EnoCoreHost& coreHost_;

		std::wstring path_;
		std::unordered_multimap<std::wstring, Field> vars_;		// namespace.class | (varname, value)  //Yunqi's Note: Store [port]-ed vars here.
		std::unordered_set<std::wstring> origin_;				// unique namespace.class entries

		// Lambdas for convert raw wchar_t* to native types
		// (Got the idea from crafting interpreters!)
		std::unordered_map<std::wstring, std::function<Value(const std::wstring&)>> convertf =
		{
			{ L"Int32", [](const std::wstring& s) { return std::stoi(s); }},
			{ L"Single", [](const std::wstring& s) { return std::stof(s); }},
			{ L"Double", [](const std::wstring& s) { return std::stod(s); }},
			{ L"Boolean", [](const std::wstring& s) { return (s == L"true" || s == L"1"); }},
			{ L"String", [](const std::wstring& s) { return s; }}
		};

		// Signal function name template
		static constexpr std::string_view signalKeyWords = R"(
		public void )";
		static constexpr std::string_view signalSuffix = R"(_when_)";
		static constexpr std::string_view last = R"(()
	{

	}
)";

		// Helper for converting wide strings to standard utf-8
		std::string wideToUtf8(const std::wstring&);
	};

	std::wostream& operator<<(std::wostream&, const Value&);
}
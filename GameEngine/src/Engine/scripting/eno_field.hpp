#pragma once

#include "eno_field_value.hpp"

namespace Script
{
	// Ensure this matches FieldInfoData
	// under Reveal.cs!!!
	//
	// Using wchar_t* here as the 
	// classic char is UTF-8
	// while wchar_t is UTF-16, which matches C# string
	struct PortedField
	{
		const wchar_t* name_;
		const wchar_t* type_;
		const wchar_t* value_;

		float min_;
		float max_;
		bool hasRange_;

		const wchar_t* origin_;
	};

	// Looks similar to PortedField, but string fields
	// are redefined as wstrings as wchar_t* requires manual freeing
	struct Field
	{
		std::wstring name_;
		std::wstring type_;
		Value value_;

		float min_;
		float max_;
		bool hasRange_;

		std::wstring origin_;
	};
}
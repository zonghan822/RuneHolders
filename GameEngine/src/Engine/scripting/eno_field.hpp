#pragma once

#include "eno_field_value.hpp"

// stds
#include <vector>


namespace Script
{
	struct PortedEnum
	{
		int count_;
		const wchar_t** names_;
		void* values_;
		int valueSize_;
	};

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

		PortedEnum* enum_;
	};

	struct EnumEntry
	{
		std::wstring name_;
		uint64_t value_{};
	};

	// Looks similar to PortedField, but string fields
	// are redefined as wstrings as wchar_t* requires manual freeing
	struct Field
	{
		std::wstring name_;
		std::wstring type_;
		Value value_;

		float min_{};
		float max_{};
		bool hasRange_;

		std::wstring origin_;

		std::vector<EnumEntry> enumEntries_;
	};
}
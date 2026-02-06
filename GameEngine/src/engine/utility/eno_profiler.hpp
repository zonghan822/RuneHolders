#pragma once

// stds
#include <iostream>
#include <chrono>
#include <unordered_map>
#include <mutex>
#include <thread>


namespace Profiler
{
	// https://stackoverflow.com/questions/43276695/stdchronosystem-clock-vs-stdchronohigh-resolution-clock-behavior
	// https://stackoverflow.com/questions/64080301/what-are-the-pros-cons-of-the-different-c-clocks-for-logging-time-stamps
	// so apparently high_resolution_clock is a typedef for
	// steady_clock and its implementation may be too unspecified
	// so I will avoid it

	// Define time unit
#ifdef TIME_NS
	// Nanoseconds
	using Duration = std::chrono::nanoseconds;
	constexpr const char* unit = "ns";

#elif defined(TIME_MC)
	// Microseconds
	using Duration = std::chrono::microseconds;
	constexpr const char* unit = "µs";

#elif defined(TIME_ML)
	// Milliseconds
	using Duration = std::chrono::milliseconds;
	constexpr const char* unit = "ms";

#elif defined(TIME_S)
	// Seconds
	using Duration = std::chrono::seconds;
	constexpr const char* unit = "s";

#else
	// Default
	using Duration = std::chrono::nanoseconds;
	constexpr const char* unit = "ns";
#endif

	using Clock = std::chrono::steady_clock;

	// When this timer dies in scope, it will report on
	// time taken for said scope to end when its 
	class ScopeTimer
	{
	public:
		ScopeTimer(const std::string& name)
			:
			name_{ name },
			start_{ record() }
		{}

		~ScopeTimer()
		{
			uint64_t end = record();
			report(end);
		}

		// Record current time based on epoch
		uint64_t record()
		{
			return std::chrono::duration_cast<Duration>(
				Clock::now().time_since_epoch()).count();
		}

		// Report the total amount of time spent executing a piece of code
		void report(uint64_t end)
		{
			std::cout
				<< "--- Timer " << name_ << " ---" << std::endl
				<< "Time taken: " << end - start_ << unit << std::endl << std::endl;
		}

	private:
		std::string name_;			// Name of time scope
		uint64_t start_;			// Start time
	};
}
#pragma once

// stds
#include <vector>
#include <functional>


namespace Events
{
	template <typename... Type>
	class Signal
	{
	public:
		using Slot = std::function<void(Type...)>;

		// Connects a listener to a signal
		void connect(Slot slot)
		{
			slots_.push_back(std::move(slot));
		}
		
		// Value to broadcast when signal has been triggered
		void emit(Type... values)
		{
			for (auto& slot : slots_)
			{
				slot(values...);
			}
		}

	private:
		std::vector <Slot> slots_;
	};
}
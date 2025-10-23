/*============================================================================================
* @file         signals.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration/definition file for the signals
*
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/


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

		/*============================================================================================
			* \brief			Connects a listener to a signal
			* \param[in]		A callable to invoke when signal is emitted
		*//*========================================================================================*/
		void connect(Slot slot)
		{
			slots_.push_back(std::move(slot));
		}

		/*============================================================================================
			* \brief			Emit signal and invoke listeners
			* \param[in]		Arguments to forward to each slot
		*//*========================================================================================*/
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
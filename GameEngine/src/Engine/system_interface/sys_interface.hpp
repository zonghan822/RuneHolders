#pragma once

namespace Sys
{
	class ISystem
	{
	public:
		virtual ~ISystem() {}
		virtual void Update() = 0;
	};
}
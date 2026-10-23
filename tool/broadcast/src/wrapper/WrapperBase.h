#pragma once

#include <any>

namespace suz
{
	class DelegateWrapperBase
	{
	public:
		virtual void ExecutePack(const std::any& AnyTuple) = 0;
		virtual void Remove() = 0;
		virtual ~DelegateWrapperBase() = default;
	};
}
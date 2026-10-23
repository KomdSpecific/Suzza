#pragma once

#include "WrapperBase.h"

#include <functional>

namespace suz
{
	template<typename ...Args>
	class LambdaWrapper : public DelegateWrapperBase
	{
	public:
		template<typename Lambda>
		LambdaWrapper(Lambda&& Func) : m_callbackFunction(std::forward<Lambda>(Func)) {}

		template<typename Lambda>
		void Bind(Lambda&& Func)
		{
			m_callbackFunction = std::forward<Lambda>(Func);
		}

		void Execute(const Args& ...args)
		{
			m_callbackFunction(args...);
		}

		virtual void ExecutePack(const std::any& AnyTuple) override
		{
			const auto& tuple = std::any_cast<std::tuple<Args...>>(AnyTuple);
			std::apply(
				[this](auto&&...args)
				{
					this->Execute(std::forward<decltype(args)>(args)...);
				},
				tuple
			);
		}

		virtual void Remove() override
		{
			m_callbackFunction = nullptr;
		}

	private:
		std::function<void(Args...)> m_callbackFunction;
	};
}
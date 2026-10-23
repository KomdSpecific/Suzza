#pragma once

#include "WrapperBase.h"

#include <memory>
#include <tuple>

namespace suz
{
	template<typename Class, typename... Args>
	class DelegateWrapper : public DelegateWrapperBase
	{
		using MemberFuncPtr = void (Class::*)(Args...);
	public:
		void Bind(Class* TargetObj, MemberFuncPtr CallbackFunc)
		{
			m_objPtr = TargetObj;
			m_isObjAlive = TargetObj->GetValidPtr();
			m_callbackFunction = CallbackFunc;
		}

		void Execute(Args... arg) const
		{
			if (m_objPtr && m_isObjAlive && *m_isObjAlive)
			{
				(m_objPtr->*m_callbackFunction)(std::forward<Args>(arg)...);
			}
		}

		virtual void ExecutePack(const std::any& AnyTuple) override
		{
			if (!m_objPtr || !m_isObjAlive || !(*m_isObjAlive)) return;

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
			m_objPtr = nullptr;
			m_isObjAlive.reset();
			m_callbackFunction = nullptr;
		}

		bool IsSame(Class* TargetObj, MemberFuncPtr CallbackFunc) const
		{
			return m_objPtr == TargetObj && m_callbackFunction == CallbackFunc;
		}

	private:
		Class* m_objPtr;
		std::shared_ptr<bool> m_isObjAlive;
		MemberFuncPtr m_callbackFunction;
	};
}
#pragma once

#include <memory>
#include <any>
#include <tuple>

namespace suz
{
	class DelegateWrapperBase
	{
	public:
		virtual void ExecutePack(const std::any& AnyTuple) = 0;
		virtual void Remove() = 0;
		virtual ~DelegateWrapperBase() = default;
	};

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


	class SingleDelegate
	{
	public:
		template<typename Class>
		void Bind(Class* TargetObj, void(Class::* CallbackFunc)());

		void Execute() const;

		void Remove();

		template<typename Class>
		bool IsSame(Class* TargetObj, void(Class::* CallbackFunc)()) const
		{
			if (!m_delegate) return false;
			
			auto* realDelegate = dynamic_cast<DelegateWrapper<Class>*>(m_delegate.get());
			if (realDelegate)
			{
				return realDelegate->IsSame(TargetObj, CallbackFunc);
			}

			return false;
		}

	private:
		std::unique_ptr<DelegateWrapperBase> m_delegate = {};
	};

	template<typename Class>
	inline void SingleDelegate::Bind(Class* TargetObj, void(Class::* CallbackFunc)())
	{
		auto delegateWrapper = std::make_unique<DelegateWrapper<Class>>();
		delegateWrapper->Bind(TargetObj, CallbackFunc);
		m_delegate = std::move(delegateWrapper);
	}

	inline void SingleDelegate::Execute() const
	{
		if (!m_delegate) return;

		auto pack = std::make_any<std::tuple<>>();
		m_delegate->ExecutePack(pack);
	}

	inline void SingleDelegate::Remove()
	{
		if (m_delegate)
		{
			m_delegate->Remove();
		}
	}


	template<typename TArg1>
	class SingleDelegateOneParam
	{
	public:
		template<typename Class>
		void Bind(Class* TargetObj, void(Class::* CallbackFunc)(TArg1));

		void Execute(TArg1 Value1) const;

		void Remove();

		template<typename Class>
		bool IsSame(Class* TargetObj, void(Class::* CallbackFunc)(TArg1)) const
		{
			if (!m_delegate) return false;

			auto* realDelegate = dynamic_cast<DelegateWrapper<Class, TArg1>*>(m_delegate.get());
			if (realDelegate)
			{
				return realDelegate->IsSame(TargetObj, CallbackFunc);
			}

			return false;
		}

	private:
		std::unique_ptr<DelegateWrapperBase> m_delegate = {};
	};

	template<typename TArg1>
	template<typename Class>
	inline void SingleDelegateOneParam<TArg1>::Bind(Class* TargetObj, void(Class::* CallbackFunc)(TArg1))
	{
		auto delegateWrapper = std::make_unique<DelegateWrapper<Class, TArg1>>();
		delegateWrapper->Bind(TargetObj, CallbackFunc);
		m_delegate = std::move(delegateWrapper);
	}

	template<typename TArg1>
	inline void SingleDelegateOneParam<TArg1>::Execute(TArg1 Value1) const
	{
		if (!m_delegate) return;

		auto pack = std::make_any<std::tuple<TArg1>>(std::move(Value1));
		m_delegate->ExecutePack(pack);
	}

	template<typename TArg1>
	inline void SingleDelegateOneParam<TArg1>::Remove()
	{
		if (m_delegate)
		{
			m_delegate->Remove();
		}
	}


	template<typename TArg1, typename TArg2>
	class SingleDelegateTwoParams
	{
	public:
		template<typename Class>
		void Bind(Class* TargetObj, void(Class::* CallbackFunc)(TArg1, TArg2));

		void Execute(TArg1 Value1, TArg2 Value2) const;

		void Remove();

		template<typename Class>
		bool IsSame(Class* TargetObj, void(Class::* CallbackFunc)(TArg1, TArg2)) const
		{
			if (!m_delegate) return false;

			auto* realDelegate = dynamic_cast<DelegateWrapper<Class, TArg1, TArg2>*>(m_delegate.get());
			if (realDelegate)
			{
				return realDelegate->IsSame(TargetObj, CallbackFunc);
			}

			return false;
		}

	private:
		std::unique_ptr<DelegateWrapperBase> m_delegate = {};
	};

	template<typename TArg1, typename TArg2>
	template<typename Class>
	inline void SingleDelegateTwoParams<TArg1, TArg2>::Bind(Class* TargetObj, void(Class::* CallbackFunc)(TArg1, TArg2))
	{
		auto delegateWrapper = std::make_unique<DelegateWrapper<Class, TArg1, TArg2>>();
		delegateWrapper->Bind(TargetObj, CallbackFunc);
		m_delegate = std::move(delegateWrapper);
	}

	template<typename TArg1, typename TArg2>
	inline void SingleDelegateTwoParams<TArg1, TArg2>::Execute(TArg1 Value1, TArg2 Value2) const
	{
		if (!m_delegate) return;

		auto pack = std::make_any<std::tuple<TArg1, TArg2>>(std::move(Value1), std::move(Value2));
		m_delegate->ExecutePack(pack);
	}

	template<typename TArg1, typename TArg2>
	inline void SingleDelegateTwoParams<TArg1, TArg2>::Remove()
	{
		if (m_delegate)
		{
			m_delegate->Remove();
		}
	}


	template<typename TArg1, typename TArg2, typename TArg3>
	class SingleDelegateThreeParam
	{
	public:
		template<typename Class>
		void Bind(Class* TargetObj, void (Class::* CallbackFunc)(TArg1, TArg2, TArg3));
		
		void Execute(TArg1 Value1, TArg2 Value2, TArg3 Value3) const;

		void Remove();

		template<typename Class>
		bool IsSame(Class* TargetObj, void(Class::* CallbackFunc)(TArg1, TArg2, TArg3)) const
		{
			if (!m_delegate) return false;

			auto* realDelegate = dynamic_cast<DelegateWrapper<Class, TArg1, TArg2, TArg3>*>(m_delegate.get());
			if (realDelegate)
			{
				return realDelegate->IsSame(TargetObj, CallbackFunc);
			}

			return false;
		}

	private:
		std::unique_ptr<DelegateWrapperBase> m_delegate = {};
	};

	template<typename TArg1, typename TArg2, typename TArg3>
	template<typename Class>
	inline void SingleDelegateThreeParam<TArg1, TArg2, TArg3>::Bind(Class* TargetObj, void(Class::* CallbackFunc)(TArg1, TArg2, TArg3))
	{
		auto delegateWrapper = std::make_unique<DelegateWrapper<Class, TArg1, TArg2, TArg3>>();
		delegateWrapper->Bind(TargetObj, CallbackFunc);
		m_delegate = std::move(delegateWrapper);
	}

	template<typename TArg1, typename TArg2, typename TArg3>
	inline void SingleDelegateThreeParam<TArg1, TArg2, TArg3>::Execute(TArg1 Value1, TArg2 Value2, TArg3 Value3) const
	{
		if (!m_delegate) return;

		auto pack = std::make_any<std::tuple<TArg1, TArg2, TArg3>>(std::move(Value1), std::move(Value2), std::move(Value3));
		m_delegate->ExecutePack(pack);
	}

	template<typename TArg1, typename TArg2, typename TArg3>
	inline void SingleDelegateThreeParam<TArg1, TArg2, TArg3>::Remove()
	{
		if (m_delegate)
		{
			m_delegate->Remove();
		}
	}
}
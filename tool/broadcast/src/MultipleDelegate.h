#pragma once

#include "SingleDelegate.h"

#include <vector>

namespace suz
{
	class MultipleDelegate
	{
	public:
		template<typename Class>
		void Bind(Class* TargetObj, void (Class::* CallbackFunc)());

		void Broadcast() const;

		template<typename Class>
		void Remove(Class* TargetObj, void (Class::* CallbackFunc)());

	private:
		std::vector<SingleDelegate> m_delegates = {};
	};

	template<typename Class>
	inline void MultipleDelegate::Bind(Class* TargetObj, void (Class::* CallbackFunc)())
	{
		SingleDelegate delegate;
		delegate.Bind<Class>(TargetObj, CallbackFunc);

		m_delegates.push_back(std::move(delegate));
	}

	inline void MultipleDelegate::Broadcast() const
	{
		for (const SingleDelegate& delegate : m_delegates)
		{
			delegate.Execute();
		}
	}

	template<typename Class>
	inline void MultipleDelegate::Remove(Class* TargetObj, void (Class::* CallbackFunc)())
	{
		for (auto it = m_delegates.begin(); it != m_delegates.end();)
		{
			if (it->IsSame<Class>(TargetObj, CallbackFunc))
			{
				it = m_delegates.erase(it);
			}
			else
			{
				it++;
			}
		}
	}


	template<typename TArg1>
	class MultipleDelegateOneParam
	{
	public:
		template<typename Class>
		void Bind(Class* TargetObj, void(Class::* CallbackFunc)(TArg1));

		void Broadcast(TArg1 Value) const;

		template<typename Class>
		void Remove(Class* TargetObj, void (Class::* CallbackFunc)(TArg1));

	private:
		std::vector<SingleDelegateOneParam<TArg1>> m_delegates = {};
	};

	template<typename TArg1>
	template<typename Class>
	inline void MultipleDelegateOneParam<TArg1>::Bind(Class* TargetObj, void (Class::* CallbackFunc)(TArg1))
	{
		SingleDelegateOneParam<TArg1> delegate;
		delegate.Bind<Class>(TargetObj, CallbackFunc);

		m_delegates.push_back(std::move(delegate));
	}

	template<typename TArg1>
	inline void MultipleDelegateOneParam<TArg1>::Broadcast(TArg1 Value) const
	{
		for (const SingleDelegateOneParam<TArg1>& delegate : m_delegates)
		{
			delegate.Execute(Value);
		}
	}

	template<typename TArg1>
	template<typename Class>
	inline void MultipleDelegateOneParam<TArg1>::Remove(Class* TargetObj, void (Class::* CallbackFunc)(TArg1))
	{
		for (auto it = m_delegates.begin(); it != m_delegates.end();)
		{
			if (it->IsSame<Class>(TargetObj, CallbackFunc))
			{
				it = m_delegates.erase(it);
			}
			else
			{
				it++;
			}
		}
	}


	template<typename TArg1, typename TArg2>
	class MultipleDelegateTwoParams
	{
	public:
		template<typename Class>
		void Bind(Class* TargetObj, void(Class::* CallbackFunc)(TArg1, TArg2));

		void Broadcast(TArg1 Value1, TArg2 Value2) const;

		template<typename Class>
		void Remove(Class* TargetObj, void (Class::* CallbackFunc)(TArg1, TArg2));

	private:
		std::vector<SingleDelegateTwoParams<TArg1, TArg2>> m_delegates = {};
	};

	template<typename TArg1, typename TArg2>
	template<typename Class>
	inline void MultipleDelegateTwoParams<TArg1, TArg2>::Bind(Class* TargetObj, void (Class::* CallbackFunc)(TArg1, TArg2))
	{
		SingleDelegateTwoParams<TArg1, TArg2> delegate;
		delegate.Bind<Class>(TargetObj, CallbackFunc);

		m_delegates.push_back(std::move(delegate));
	}

	template<typename TArg1, typename TArg2>
	inline void MultipleDelegateTwoParams<TArg1, TArg2>::Broadcast(TArg1 Value1, TArg2 Value2) const
	{
		for (const SingleDelegateTwoParams<TArg1, TArg2>& delegate : m_delegates)
		{
			delegate.Execute(Value1, Value2);
		}
	}

	template<typename TArg1, typename TArg2>
	template<typename Class>
	inline void MultipleDelegateTwoParams<TArg1, TArg2>::Remove(Class* TargetObj, void (Class::* CallbackFunc)(TArg1, TArg2))
	{
		for (auto it = m_delegates.begin(); it != m_delegates.end();)
		{
			if (it->IsSame<Class>(TargetObj, CallbackFunc))
			{
				it = m_delegates.erase(it);
			}
			else
			{
				it++;
			}
		}
	}
}
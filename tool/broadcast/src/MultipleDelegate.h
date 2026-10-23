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
		template<typename Lambda>
		void Bind(Lambda&& CallbackFunc);

		void Broadcast() const;

		template<typename Class>
		void Remove(Class* TargetObj, void (Class::* CallbackFunc)());

		void RemoveAll();

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

	template<typename Lambda>
	inline void MultipleDelegate::Bind(Lambda&& CallbackFunc)
	{
		SingleDelegate delegate;
		delegate.Bind<Lambda>(CallbackFunc);

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
				it->Remove();
				it = m_delegates.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	inline void MultipleDelegate::RemoveAll()
	{
		for (SingleDelegate& delegate : m_delegates)
		{
			delegate.Remove();
		}

		m_delegates.clear();
	}


	template<typename TArg1>
	class MultipleDelegateOneParam
	{
	public:
		template<typename Class>
		void Bind(Class* TargetObj, void(Class::* CallbackFunc)(TArg1));
		template<typename Lambda>
		void Bind(Lambda&& CallbackFunc);

		void Broadcast(TArg1 Value) const;

		template<typename Class>
		void Remove(Class* TargetObj, void (Class::* CallbackFunc)(TArg1));

		void RemoveAll();

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
	template<typename Lambda>
	inline void MultipleDelegateOneParam<TArg1>::Bind(Lambda&& CallbackFunc)
	{
		SingleDelegateOneParam<TArg1> delegate;
		delegate.Bind<Lambda>(CallbackFunc);

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
				it->Remove();
				it = m_delegates.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	template<typename TArg1>
	inline void MultipleDelegateOneParam<TArg1>::RemoveAll()
	{
		for (auto& delegate : m_delegates)
		{
			delegate.Remove();
		}

		m_delegates.clear();
	}
	

	template<typename TArg1, typename TArg2>
	class MultipleDelegateTwoParams
	{
	public:
		template<typename Class>
		void Bind(Class* TargetObj, void(Class::* CallbackFunc)(TArg1, TArg2));
		template<typename Lambda>
		void Bind(Lambda&& CallbackFunc);

		void Broadcast(TArg1 Value1, TArg2 Value2) const;

		template<typename Class>
		void Remove(Class* TargetObj, void (Class::* CallbackFunc)(TArg1, TArg2));

		void RemoveAll();

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
	template<typename Lambda>
	inline void MultipleDelegateTwoParams<TArg1, TArg2>::Bind(Lambda&& CallbackFunc)
	{
		SingleDelegateTwoParams<TArg1, TArg2> delegate;
		delegate.Bind<Lambda>(CallbackFunc);

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
				it->Remove();
				it = m_delegates.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	template<typename TArg1, typename TArg2>
	inline void MultipleDelegateTwoParams<TArg1, TArg2>::RemoveAll()
	{
		for (auto& delegate : m_delegates)
		{
			delegate.Remove();
		}

		m_delegates.clear();
	}
}
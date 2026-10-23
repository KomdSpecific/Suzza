#pragma once

#include "MultipleDelegate.h"

#include <vector>

#define DECLARE_MULTIPLE_DELEGATE(DELEGATE_CLASS_NAME) \
namespace suz \
{ \
	class DELEGATE_CLASS_NAME \
	{ \
	public: \
		template<typename Class> \
		void Bind(Class* TargetObj, void (Class::* CallbackFunc)()); \
		template<typename Lambda> \
		void Bind(Lambda&& CallbackFunc); \
	\
		void Broadcast() const; \
	\
		template<typename Class> \
		void Remove(Class* TargetObj, void (Class::* CallbackFunc)()); \
	\
		void RemoveAll(); \
	\
	private: \
		std::vector<SingleDelegate> m_delegates = {}; \
	}; \
	\
	template<typename Class> \
	inline void DELEGATE_CLASS_NAME::Bind(Class* TargetObj, void (Class::* CallbackFunc)()) \
	{ \
		SingleDelegate delegate; \
		delegate.Bind<Class>(TargetObj, CallbackFunc); \
	\
		m_delegates.push_back(std::move(delegate)); \
	} \
	\
	template<typename Lambda> \
	inline void DELEGATE_CLASS_NAME::Bind(Lambda&& CallbackFunc) \
	{ \
		SingleDelegate delegate; \
		delegate.Bind<Lambda>(CallbackFunc); \
		\
		m_delegates.push_back(std::move(delegate)); \
	} \
	\
	inline void DELEGATE_CLASS_NAME::Broadcast() const \
	{ \
		for (const SingleDelegate& delegate : m_delegates) \
		{ \
			delegate.Execute(); \
		} \
	} \
	\
	template<typename Class> \
	inline void DELEGATE_CLASS_NAME::Remove(Class* TargetObj, void (Class::* CallbackFunc)()) \
	{ \
		for (auto it = m_delegates.begin(); it != m_delegates.end();) \
		{ \
			if (it->IsSame<Class>(TargetObj, CallbackFunc)) \
			{ \
				it->Remove(); \
				it = m_delegates.erase(it); \
			} \
			else \
			{ \
				it++; \
			} \
		} \
	} \
	\
	inline void DELEGATE_CLASS_NAME::RemoveAll() \
	{ \
		for (SingleDelegate& delegate : m_delegates) \
		{ \
			delegate.Remove(); \
		} \
		\
		m_delegates.clear(); \
	} \
}

#define DECLARE_MULTIPLE_DELEGATE_ONE_PARAM(DELEGATE_CLASS_NAME, PARAM1) \
namespace suz \
{ \
	class DELEGATE_CLASS_NAME \
	{ \
	public: \
		template<typename Class> \
		void Bind(Class* TargetObj, void(Class::* CallbackFunc)(PARAM1)); \
		template<typename Lambda> \
		void Bind(Lambda&& CallbackFunc); \
	\
		void Broadcast(PARAM1 Value) const; \
	\
		template<typename Class> \
		void Remove(Class* TargetObj, void (Class::* CallbackFunc)(PARAM1)); \
	\
		void RemoveAll(); \
	\
	private: \
		std::vector<SingleDelegateOneParam<PARAM1>> m_delegates = {}; \
	}; \
	\
	template<typename Class> \
	inline void DELEGATE_CLASS_NAME::Bind(Class* TargetObj, void (Class::* CallbackFunc)(PARAM1)) \
	{ \
		SingleDelegateOneParam<PARAM1> delegate; \
		delegate.Bind<Class>(TargetObj, CallbackFunc); \
	\
		m_delegates.push_back(std::move(delegate)); \
	} \
	\
	template<typename Lambda> \
	inline void DELEGATE_CLASS_NAME::Bind(Lambda&& CallbackFunc) \
	{ \
		SingleDelegateOneParam<PARAM1> delegate; \
		delegate.Bind<Lambda>(CallbackFunc); \
		\
		m_delegates.push_back(std::move(delegate)); \
	} \
	\
	inline void DELEGATE_CLASS_NAME::Broadcast(PARAM1 Value) const \
	{ \
		for (const SingleDelegateOneParam<PARAM1>& delegate : m_delegates) \
		{ \
			delegate.Execute(Value); \
		} \
	} \
	\
	template<typename Class> \
	inline void DELEGATE_CLASS_NAME::Remove(Class* TargetObj, void (Class::* CallbackFunc)(PARAM1)) \
	{ \
		for (auto it = m_delegates.begin(); it != m_delegates.end();) \
		{ \
			if (it->IsSame<Class>(TargetObj, CallbackFunc)) \
			{ \
				it->Remove(); \
				it = m_delegates.erase(it); \
			} \
			else \
			{ \
				it++; \
			} \
		} \
	} \
	\
	inline void DELEGATE_CLASS_NAME::RemoveAll() \
	{ \
		for (SingleDelegateOneParam<PARAM1>& delegate : m_delegates) \
		{ \
			delegate.Remove(); \
		} \
		\
		m_delegates.clear(); \
	} \
}

#define DECLARE_MULTIPLE_DELEGATE_TWO_PARAMS(DELEGATE_CLASS_NAME, PARAM1, PARAM2) \
namespace suz \
{ \
	class DELEGATE_CLASS_NAME \
	{ \
	public: \
		template<typename Class> \
		void Bind(Class* TargetObj, void(Class::* CallbackFunc)(PARAM1, PARAM2)); \
		template<typename Lambda> \
		void Bind(Lambda&& CallbackFunc); \
	\
		void Broadcast(PARAM1 Value1, PARAM2 Value2) const; \
	\
		template<typename Class> \
		void Remove(Class* TargetObj, void (Class::* CallbackFunc)(PARAM1, PARAM2)); \
	\
		void RemoveAll(); \
	\
	private: \
		std::vector<SingleDelegateTwoParams<PARAM1, PARAM2>> m_delegates = {}; \
	}; \
	\
	template<typename Class> \
	inline void DELEGATE_CLASS_NAME::Bind(Class* TargetObj, void (Class::* CallbackFunc)(PARAM1, PARAM2)) \
	{ \
		SingleDelegateTwoParams<PARAM1, PARAM2> delegate; \
		delegate.Bind<Class>(TargetObj, CallbackFunc); \
		\
		m_delegates.push_back(std::move(delegate)); \
	} \
	\
	template<typename Lambda> \
	inline void DELEGATE_CLASS_NAME::Bind(Lambda&& CallbackFunc) \
	{ \
		SingleDelegateTwoParams<PARAM1, PARAM2> delegate; \
		delegate.Bind<Lambda>(CallbackFunc); \
		\
		m_delegates.push_back(std::move(delegate)); \
	} \
	\
	inline void DELEGATE_CLASS_NAME::Broadcast(PARAM1 Value1, PARAM2 Value2) const \
	{ \
		for (const SingleDelegateTwoParams<PARAM1, PARAM2>& delegate : m_delegates) \
		{ \
			delegate.Execute(Value1, Value2); \
		} \
	} \
	\
	template<typename Class> \
	inline void DELEGATE_CLASS_NAME::Remove(Class* TargetObj, void (Class::* CallbackFunc)(PARAM1, PARAM2)) \
	{ \
		for (auto it = m_delegates.begin(); it != m_delegates.end();) \
		{ \
			if (it->IsSame<Class>(TargetObj, CallbackFunc)) \
			{ \
				it->Remove(); \
				it = m_delegates.erase(it); \
			} \
			else \
			{ \
				it++; \
			} \
		} \
	} \
	\
	inline void DELEGATE_CLASS_NAME::RemoveAll() \
	{ \
		for (SingleDelegateTwoParams<PARAM1, PARAM2>& delegate : m_delegates) \
		{ \
			delegate.Remove(); \
		} \
		\
		m_delegates.clear(); \
	} \
}
#pragma once

#include "SingleDelegate.h"

#define DECLARE_SINGLE_DELEGATE(DELEGATE_CLASS_NAME) \
namespace suz \
{ \
	class DELEGATE_CLASS_NAME \
	{ \
	public: \
		template<typename Class> \
		void Bind(Class* TargetObj, void(Class::* CallbackFunc)()); \
		template<typename Lambda> \
		void Bind(Lambda&& CallbackFunc); \
	\
		void Execute() const; \
	\
		void Remove(); \
	\
		template<typename Class> \
		bool IsSame(Class* TargetObj, void(Class::* CallbackFunc)()) const \
		{ \
			if (!m_delegate) return false; \
			\
			auto* realDelegate = dynamic_cast<DelegateWrapper<Class>*>(m_delegate.get()); \
			if (realDelegate) \
			{ \
				return realDelegate->IsSame(TargetObj, CallbackFunc); \
			} \
			\
			return false; \
		} \
	\
	private: \
		std::unique_ptr<DelegateWrapperBase> m_delegate = {}; \
	}; \
	\
	template<typename Class> \
	inline void DELEGATE_CLASS_NAME::Bind(Class* TargetObj, void(Class::* CallbackFunc)()) \
	{ \
		auto delegateWrapper = std::make_unique<DelegateWrapper<Class>>(); \
		delegateWrapper->Bind(TargetObj, CallbackFunc); \
		m_delegate = std::move(delegateWrapper); \
	} \
	\
	template<typename Lambda> \
	inline void DELEGATE_CLASS_NAME::Bind(Lambda&& CallbackFunc) \
	{ \
		m_delegate = std::make_unique<LambdaWrapper>(std::forward<Lambda>(CallbackFunc)); \
	} \
	\
	inline void DELEGATE_CLASS_NAME::Execute() const\
	{ \
		if (!m_delegate) return; \
		\
		auto pack = std::make_any<std::tuple<>>(); \
		m_delegate->ExecutePack(pack); \
	} \
	\
	inline void DELEGATE_CLASS_NAME::Remove() \
	{ \
		if (m_delegate) \
		{ \
			m_delegate->Remove(); \
			m_delegate = nullptr; \
		} \
	} \
}

#define DECLARE_SINGLE_DELEGATE_ONE_PARAM(DELEGATE_CLASS_NAME, PARAM1) \
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
		void Execute(PARAM1 Value1) const; \
	\
		void Remove(); \
	\
		template<typename Class> \
		bool IsSame(Class* TargetObj, void(Class::* CallbackFunc)(PARAM1)) const \
		{	\
			if (!m_delegate) return false; \
			\
			auto* realDelegate = dynamic_cast<DelegateWrapper<Class, PARAM1>*>(m_delegate.get()); \
			if (realDelegate) \
			{ \
				return realDelegate->IsSame(TargetObj, CallbackFunc); \
			} \
			\
			return false;	\
		} \
	\
	private:	\
		std::unique_ptr<DelegateWrapperBase> m_delegate = {}; \
	}; \
	\
	template<typename Class> \
	inline void DELEGATE_CLASS_NAME::Bind(Class* TargetObj, void(Class::* CallbackFunc)(PARAM1)) \
	{ \
		auto delegateWrapper = std::make_unique<DelegateWrapper<Class, PARAM1>>(); \
		delegateWrapper->Bind(TargetObj, CallbackFunc); \
		m_delegate = std::move(delegateWrapper); \
	} \
	\
	template<typename Lambda> \
	inline void DELEGATE_CLASS_NAME::Bind(Lambda&& CallbackFunc) \
	{ \
		m_delegate = std::make_unique<LambdaWrapper<PARAM1>>(std::forward<Lambda>(CallbackFunc)); \
	} \
	\
	inline void DELEGATE_CLASS_NAME::Execute(PARAM1 Value1) const \
	{ \
		if (!m_delegate) return; \
		\
		auto pack = std::make_any<std::tuple<PARAM1>>(std::move(Value1)); \
		m_delegate->ExecutePack(pack); \
	} \
	\
	inline void DELEGATE_CLASS_NAME::Remove() \
	{ \
		if (m_delegate) \
		{ \
			m_delegate->Remove(); \
			m_delegate = nullptr; \
		} \
	} \
}
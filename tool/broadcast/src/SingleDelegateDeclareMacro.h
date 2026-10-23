#pragma once

#define DECLARE_SINGLE_DELEGATE(DELEGATE_CLASS_NAME) \
namespace suz \
{ \
	template<typename Class> \
	class DELEGATE_CLASS_NAME \
	{ \
	public: \
		void Bind(Class* TargetObj, void(Class::* CallbackFunc)()); \
	\
		void Execute(); \
	\
		void Remove(); \
	\
		void IsSame(Class* TargetObj, void(Class::* CallbackFunc)()) const \
		{ \
			return m_objPtr == TargetObj && CallbackFunction == CallbackFunc; \
		} \
	\
	private: \
		Class* m_objPtr; \
		std::shared_ptr<bool> m_isObjAlive; \
		void (Class::* CallbackFunction)(); \
	}; \
	\
	template<typename Class> \
	inline void DELEGATE_CLASS_NAME<Class>::Bind(Class* TargetObj, void(Class::* CallbackFunc)()) \
	{ \
		m_objPtr = TargetObj; \
		m_isObjAlive = TargetObj->GetValidPtr(); \
		CallbackFunction = CallbackFunc; \
	} \
	\
	template<typename Class> \
	inline void DELEGATE_CLASS_NAME<Class>::Execute() \
	{ \
		if (m_isObjAlive && *m_isObjAlive) \
		{ \
			(m_objPtr->*CallbackFunction)(); \
		} \
	} \
	\
	template<typename Class> \
	inline void DELEGATE_CLASS_NAME<Class>::Remove() \
	{ \
		m_objPtr = nullptr; \
		m_isObjAlive.reset(); \
		CallbackFunction = nullptr; \
	} \
}

#define DECLARE_SINGLE_DELEGATE_ONE_PARAM(DELEGATE_CLASS_NAME, PARAM1) \
namespace suz \
{ \
	template<typename Class, typename PARAM1> \
	class DELEGATE_CLASS_NAME \
	{ \
	public: \
		void Bind(Class* TargetObj, void(Class::* CallbackFunc)(PARAM1)); \
	\
		void Execute(PARAM1 Value1) const; \
	\
		void Remove(); \
	\
		bool IsSame(Class* TargetObj, void(Class::* CallbackFunc)(PARAM1)) const \
		{	\
			return m_objPtr == TargetObj && CallbackFunction == CallbackFunc;	\
		} \
	\
	private:	\
		Class* m_objPtr;	\
		std::shared_ptr<bool> m_isObjAlive;		\
		void (Class::* CallbackFunction)(PARAM1); \
	}; \
	\
	template<typename Class, typename PARAM1> \
	inline void DELEGATE_CLASS_NAME<Class, PARAM1>::Bind(Class* TargetObj, void(Class::* CallbackFunc)(PARAM1)) \
	{ \
		m_objPtr = TargetObj; \
		m_isObjAlive = TargetObj->GetValidPtr(); \
		CallbackFunction = CallbackFunc; \
	} \
	\
	template<typename Class, typename PARAM1> \
	inline void DELEGATE_CLASS_NAME<Class, PARAM1>::Execute(PARAM1 Value1) const \
	{ \
		if (m_isObjAlive && *m_isObjAlive) \
		{ \
			(m_objPtr->*CallbackFunction)(std::forward<PARAM1>(Value1)); \
		} \
	} \
	\
	template<typename Class, typename PARAM1> \
	inline void DELEGATE_CLASS_NAME<Class, PARAM1>::Remove() \
	{ \
		m_objPtr = nullptr; \
		m_isObjAlive.reset(); \
		CallbackFunction = nullptr; \
	} \
}
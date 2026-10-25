#pragma once

#include "SuzzaTypes.h"

#include <vector>
#include <memory>

namespace suz
{
	class IContainerAccessor
	{
	public:
		virtual ~IContainerAccessor() = default;
		virtual usize GetSize() = 0;
		virtual void Resize(usize NewSize) = 0;
		virtual void* GetAccessorElement(usize Index) = 0;
		virtual bool IsValid() = 0;
	};


	template<typename T>
	class VectorAccessor : public IContainerAccessor
	{
	public:
		explicit VectorAccessor(std::vector<T>* InVector) : m_vector(InVector) {}

		virtual usize GetSize() override
		{
			return m_vector->size();
		}

		virtual void Resize(usize NewSize) override
		{
			m_vector->resize(NewSize);
		}

		virtual void* GetAccessorElement(usize Index) override
		{
			return &(*m_vector)[Index];
		}

		virtual bool IsValid() override { return m_vector != nullptr; }

		static std::shared_ptr<IContainerAccessor>& CreateVectorAccessor(void* Obj, usize Offset);
		template<typename ClassT>
		static std::shared_ptr<IContainerAccessor>& CreateVectorAccessor(void* Obj, const std::vector<T>& (ClassT::* GetterFunc)() const);

	private:
		std::vector<T>* m_vector = {};
	};

	template<typename T>
	inline std::shared_ptr<IContainerAccessor>& VectorAccessor<T>::CreateVectorAccessor(void* Obj, usize Offset)
	{
		return std::make_shared<VectorAccessor>(static_cast<std::vector<T>*>((char*)Obj + Offset));
	}

	/* 成员函数返回vector数组引用 */
	template<typename T>
	template<typename ClassT>
	inline std::shared_ptr<IContainerAccessor>& VectorAccessor<T>::CreateVectorAccessor(void* Obj, const std::vector<T>& (ClassT::* GetterFunc)() const)
	{
		ClassT* classObj = static_cast<ClassT*>(Obj);
		return std::make_shared<VectorAccessor>(&(classObj->*GetterFunc)());
	}
}
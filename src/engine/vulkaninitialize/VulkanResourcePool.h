#pragma once

#include "SuzzaTypes.h"
#include "VulkanResourceBase.h"

namespace suz
{
	template<typename T>
	class VulkanResourcePool : public VulkanResourceBase
	{
	public:
		VulkanResourcePool() = default;

		template<typename SpecT>
		T* CreateResource(const String& ResourceName, SpecT& Spec);

		T* GetResource(const String& ResourceName);

		void Destroy();

	private:
		UnorderedMap<String, UniquePtr<T>> m_resourceMap = {};
	};

	template<typename T>
	template<typename SpecT>
	inline T* VulkanResourcePool<T>::CreateResource(const String& ResourceName, SpecT& Spec)
	{
		auto it = m_resourceMap.find(ResourceName);
		if (it != m_resourceMap.end())
		{
			it->second->Create(Spec);
			return it->second.get();
		}

		UniquePtr<T> resourcePtr = std::make_unique<T>();
		if (resourcePtr->Create(Spec))
		{
			m_resourceMap[ResourceName] = std::move(resourcePtr);
			return m_resourceMap[ResourceName].get();
		}

		return nullptr;
	}

	template<typename T>
	inline T* VulkanResourcePool<T>::GetResource(const String& ResourceName)
	{
		auto it = m_resourceMap.find(ResourceName);
		if (it != m_resourceMap.end())
		{
			return it->second.get();
		}

		return nullptr;
	}

	template<typename T>
	inline void VulkanResourcePool<T>::Destroy()
	{
		for (const auto& pair : m_resourceMap)
		{
			pair.second->Destroy();
		}
	}
}
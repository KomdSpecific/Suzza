#pragma once

#include "BaseObject.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace suz
{
	class Descriptor : public BaseObject
	{
	public:
		Descriptor(const VkDevice& Device);
		~Descriptor();

		void InitDescriptor();

		virtual void Destroy() override;

	private:
		void CreateDescriptorPool();
		void CreateDescriptorSets();

		const VkDevice& m_device;

		VkDescriptorPool m_descriptorPool = {};
		std::vector<VkDescriptorSet> m_descriptorSet = {};

	public:
		inline const VkDescriptorPool& GetDescriptorPool() const { return m_descriptorPool; }
		inline const std::vector<VkDescriptorSet>& GetDescriptorSets() const { return m_descriptorSet; }
		inline const VkDescriptorSet& GetDescriptorSetElement(uint32_t Index) const { return m_descriptorSet[Index]; }
	};
}
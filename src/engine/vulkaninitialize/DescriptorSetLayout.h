#pragma once

#include "VulkanResourceBase.h"
#include "VulkanInitialize.h"

namespace suz
{
	struct DescriptorSetLayoutSpec
	{
		DescriptorSetLayoutSpec(const VkDevice& Device)
			: device(Device) {
		}

		const VkDevice& device;

		Vector<VkDescriptorSetLayoutBinding> descriptorBinding{};

		Vector<VkDescriptorBindingFlags> flags{};
		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};

		bool BindingFlagsValid() const;

		static VkDescriptorSetLayoutCreateInfo DefaultDescriptorSetLayoutCreateInfo(const DescriptorSetLayoutSpec& Spec);
	};

	class DescriptorSetLayout : public VulkanResourceBase
	{
	public:
		DescriptorSetLayout() = default;
		~DescriptorSetLayout();

		bool Create(DescriptorSetLayoutSpec& Spec);
		bool CreateDescriptorSetLayout(DescriptorSetLayoutSpec& Spec);

		void Destroy();

	private:
		/* 必要资源句柄 */
		VkDevice m_device = {};

		VkDescriptorSetLayout m_descriptorSetLayout = {};

	public:
		inline const VkDescriptorSetLayout& GetDescriptorLayout() const { return m_descriptorSetLayout; }
	};
}
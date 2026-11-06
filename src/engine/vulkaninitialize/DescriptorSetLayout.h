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

		VkDevice device = {};

		Vector<VkDescriptorSetLayoutBinding> descriptorBinding{};

		Vector<VkDescriptorBindingFlags> flags{};
		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};

		bool BindingFlagsValid() const;

		static VkDescriptorSetLayoutCreateInfo DefaultDescriptorSetLayoutCreateInfo(const DescriptorSetLayoutSpec& Spec);
	};


	/* 描述符布局是给予描述符uniform使用的一部分 */
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
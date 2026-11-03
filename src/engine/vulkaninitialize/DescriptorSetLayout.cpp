#include "DescriptorSetLayout.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	bool DescriptorSetLayoutSpec::BindingFlagsValid() const
	{
		for (const VkDescriptorBindingFlags& flag : flags)
		{
			if (flag != 0)
				return true;
		}

		return false;
	}

	VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutSpec::DefaultDescriptorSetLayoutCreateInfo(const DescriptorSetLayoutSpec& Spec)
	{
		/* 定义描述符集配置信息 */
		VkDescriptorSetLayoutCreateInfo descriotorInfo{};
		descriotorInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriotorInfo.bindingCount = static_cast<uint32>(Spec.descriptorBinding.size());
		descriotorInfo.pBindings = Spec.descriptorBinding.data();

		if (Spec.BindingFlagsValid())
		{
			descriotorInfo.pNext = &Spec.bindingFlags;									// 挂载Binding Flags
			descriotorInfo.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
		}

		return descriotorInfo;
	}

	DescriptorSetLayout::~DescriptorSetLayout()
	{
		if (*m_isAlive)
		{
			SUZZA_LOG_ERROR("Class DescriptorSetLayout Warning: 未调用Destroy手动销毁!");
			Destroy();
		}
	}

	bool DescriptorSetLayout::Create(DescriptorSetLayoutSpec& Spec)
	{
		return CreateDescriptorSetLayout(Spec);
	}

	bool DescriptorSetLayout::CreateDescriptorSetLayout(DescriptorSetLayoutSpec& Spec)
	{
		if (Spec.device == VK_NULL_HANDLE)
		{
			SUZZA_LOG_ERROR("无法创建图形管线: 逻辑设备device不合法!");
			return false;
		}

		m_device = Spec.device;

		if (Spec.BindingFlagsValid())
		{
			Spec.bindingFlags.bindingCount = static_cast<uint32>(Spec.flags.size());
			Spec.bindingFlags.pBindingFlags = Spec.flags.data();
		}

		VkDescriptorSetLayoutCreateInfo descriptorInfo = DescriptorSetLayoutSpec::DefaultDescriptorSetLayoutCreateInfo(Spec);
		if (vkCreateDescriptorSetLayout(Spec.device, &descriptorInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
		{
			SUZZA_LOG_ERROR("创建描述字符集失败!");
			return false;
		}

		return true;
	}

	void DescriptorSetLayout::Destroy()
	{
		if (!(*m_isAlive)) return;

		if (m_device == VK_NULL_HANDLE)
		{
			SUZZA_LOG_ERROR("资源销毁失败: 逻辑设备device不合法!");
			return;
		}

		vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);

		*m_isAlive = false;
	}
}
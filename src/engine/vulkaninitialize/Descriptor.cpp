#include "Descriptor.h"
#include "Engine.h"
#include "VulkanResourceManager.h"
#include "Pipeline.h"
#include "BufferArea.h"
#include "commonmacro/VkMacro.h"

#include <array>
#include <iostream>

namespace suz
{
	Descriptor::Descriptor(const VkDevice& Device)
		: m_device(Device)
	{
	}

	Descriptor::~Descriptor()
	{
		if (*m_isAlive)
		{
			std::cerr << "Class Descriptor Warning: 未调用Destroy手动销毁!" << std::endl;
			Destroy();
		}
	}

	void Descriptor::InitDescriptor()
	{
		CreateCameraDescriptorPool();
		CreateCameraDescriptorSets();
		CreateBindlessDescriptorPool();
		CreateBindlessDescriptorSets();
	}

	void Descriptor::Destroy()
	{
		if (!(*m_isAlive)) return;

		vkDestroyDescriptorPool(m_device, m_cameraDescriptorPool, nullptr);
		vkDestroyDescriptorPool(m_device, m_bindlessDescriptorPool, nullptr);

		*m_isAlive = false;
	}

	void Descriptor::CreateCameraDescriptorPool()
	{
		VkDescriptorPoolSize poolSizes{};
		poolSizes.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes.descriptorCount = static_cast<uint32>(MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPoolCreateInfo poolCreateInfo{};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

		poolCreateInfo.poolSizeCount = 1;
		poolCreateInfo.pPoolSizes = &poolSizes;
		poolCreateInfo.maxSets = static_cast<uint32>(MAX_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(m_device, &poolCreateInfo, nullptr, &m_cameraDescriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("创建描述符池失败!");
		}
	}

	void Descriptor::CreateCameraDescriptorSets()
	{
		m_cameraDescriptorSet.resize(MAX_FRAMES_IN_FLIGHT);

		VulkanResourceManager& vkManager = Engine::GetVulkanResourceManager();

		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, vkManager.GetPipeline()->GetCameraDescriptorSetLayout());
		VkDescriptorSetAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocateInfo.descriptorPool = m_cameraDescriptorPool;
		allocateInfo.descriptorSetCount = static_cast<uint32>(MAX_FRAMES_IN_FLIGHT);
		allocateInfo.pSetLayouts = layouts.data();

		if (vkAllocateDescriptorSets(m_device, &allocateInfo, m_cameraDescriptorSet.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("分配描述符集合失败!");
		}

		const std::vector<VkBuffer>& uniformBuffers = vkManager.GetBufferArea()->GetUniformBuffers();

		/* 配置集合中的每一个描述符 */
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			/* 告诉描述符绑定buffer、偏移量、占用大小 */
			VkDescriptorBufferInfo descriptorInfo{};
			descriptorInfo.buffer = uniformBuffers[i];
			descriptorInfo.offset = 0;
			descriptorInfo.range = sizeof(CameraViewportInfo);

			std::array<VkWriteDescriptorSet, 1> writeDescriptors{};
			writeDescriptors[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptors[0].dstSet = m_cameraDescriptorSet[i];
			writeDescriptors[0].dstBinding = 0;
			writeDescriptors[0].dstArrayElement = 0;
			writeDescriptors[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptors[0].descriptorCount = 1;

			writeDescriptors[0].pBufferInfo = &descriptorInfo;
			writeDescriptors[0].pImageInfo = nullptr;
			writeDescriptors[0].pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(m_device, static_cast<uint32>(writeDescriptors.size()), writeDescriptors.data(), 0, nullptr);
		}
	}

	void Descriptor::CreateBindlessDescriptorPool()
	{
		VkDescriptorPoolSize bindlessPoolSizes{};
		bindlessPoolSizes.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		bindlessPoolSizes.descriptorCount = static_cast<uint32>(MAX_SAMPLED_TEXTURE_IMAGES_SLOTS);

		VkDescriptorPoolCreateInfo poolCreateInfo{};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

		poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
		poolCreateInfo.maxSets = 1;										// 只分配1个全局Bindless集合
		poolCreateInfo.poolSizeCount = 1;
		poolCreateInfo.pPoolSizes = &bindlessPoolSizes;

		if (vkCreateDescriptorPool(m_device, &poolCreateInfo, nullptr, &m_bindlessDescriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("创建描述符池失败!");
		}
	}

	void Descriptor::CreateBindlessDescriptorSets()
	{
		VulkanResourceManager& vkManager = Engine::GetVulkanResourceManager();

		VkDescriptorSetLayout bindlessLayout = vkManager.GetPipeline()->GetBindlessDescriptorSetLayout();
		VkDescriptorSetAllocateInfo bindlessInfo{};
		bindlessInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		bindlessInfo.descriptorPool = m_bindlessDescriptorPool;
		bindlessInfo.descriptorSetCount = 1;
		bindlessInfo.pSetLayouts = &bindlessLayout;

		if (vkAllocateDescriptorSets(m_device, &bindlessInfo, &m_bindlessDescriptorSet) != VK_SUCCESS)
		{
			throw std::runtime_error("分配描述符集合失败!");
		}
	}

	void Descriptor::UpdateBindlessDescriptor(const VkImageView& ImageView, const VkSampler& Sampler, uint32 SlotIndex)
	{
		/* 配置需要加载的图像视图信息 */
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = ImageView;
		imageInfo.sampler = Sampler;

		/* 更改相应的描述符, 可初始化更改也可运行时更改 */
		VkWriteDescriptorSet writeDescriptors{};
		writeDescriptors.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptors.dstSet = m_bindlessDescriptorSet;
		writeDescriptors.dstBinding = 1;
		writeDescriptors.dstArrayElement = SlotIndex;							// 数组指定下标
		writeDescriptors.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptors.descriptorCount = 1;
		writeDescriptors.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(m_device, 1, &writeDescriptors, 0, nullptr);
	}
}
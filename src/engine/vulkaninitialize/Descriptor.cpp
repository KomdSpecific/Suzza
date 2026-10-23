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
		CreateDescriptorPool();
		CreateDescriptorSets();
	}

	void Descriptor::Destroy()
	{
		if (!(*m_isAlive)) return;

		vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);

		*m_isAlive = false;
	}

	void Descriptor::CreateDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPoolCreateInfo poolCreateInfo{};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolCreateInfo.pPoolSizes = poolSizes.data();

		poolCreateInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(m_device, &poolCreateInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("创建描述符池失败!");
		}
	}

	void Descriptor::CreateDescriptorSets()
	{
		m_descriptorSet.resize(MAX_FRAMES_IN_FLIGHT);

		VulkanResourceManager& vkManager = Engine::GetVulkanResourceManager();

		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, vkManager.GetPipeline()->GetDescriptorSetLayout());
		VkDescriptorSetAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocateInfo.descriptorPool = m_descriptorPool;
		allocateInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocateInfo.pSetLayouts = layouts.data();

		if (vkAllocateDescriptorSets(m_device, &allocateInfo, m_descriptorSet.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("分配描述符集合失败!");
		}

		const std::vector<VkBuffer>& uniformBuffers = vkManager.GetBufferArea()->GetUniformBuffers();

		const VkImageView& imageView = vkManager.GetVulkanTexture()->GetTextureImageView();
		const VkSampler& sampler = vkManager.GetVulkanTexture()->GetTextureSampler();

		/* 配置集合中的每一个描述符 */
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			/* 告诉描述符绑定buffer、偏移量、占用大小 */
			VkDescriptorBufferInfo descriptorInfo{};
			descriptorInfo.buffer = uniformBuffers[i];
			descriptorInfo.offset = 0;
			descriptorInfo.range = sizeof(CameraViewportInfo);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = imageView;
			imageInfo.sampler = sampler;

			std::array<VkWriteDescriptorSet, 2> writeDescriptors{};
			writeDescriptors[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptors[0].dstSet = m_descriptorSet[i];
			writeDescriptors[0].dstBinding = 0;
			writeDescriptors[0].dstArrayElement = 0;
			writeDescriptors[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptors[0].descriptorCount = 1;

			writeDescriptors[0].pBufferInfo = &descriptorInfo;
			writeDescriptors[0].pImageInfo = nullptr;
			writeDescriptors[0].pTexelBufferView = nullptr;

			writeDescriptors[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptors[1].dstSet = m_descriptorSet[i];
			writeDescriptors[1].dstBinding = 1;
			writeDescriptors[1].dstArrayElement = 0;
			writeDescriptors[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptors[1].descriptorCount = 1;

			writeDescriptors[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writeDescriptors.size()), writeDescriptors.data(), 0, nullptr);
		}
	}
}
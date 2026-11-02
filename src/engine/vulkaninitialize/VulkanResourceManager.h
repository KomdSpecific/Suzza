#pragma once

#include "VulkanResourceBase.h"
#include "VulkanResourcePool.h"
#include "vulkaninitialize/Device.h"
#include "vulkaninitialize/DescriptorSetLayout.h"
#include "vulkanInitialize/Pipeline.h"
#include "vulkanInitialize/VulkanInitialize.h"
#include "vulkanInitialize/SwapChain.h"
#include "vulkanInitialize/RenderPass.h"
#include "vulkanInitialize/BufferArea.h"
#include "vulkanInitialize/SyncObject.h"
#include "vulkanInitialize/Descriptor.h"

namespace suz
{
	class VulkanResourceManager : public VulkanResourceBase
	{
	public:
		VulkanResourceManager() = default;

		bool InitVulkan();

		void Destroy();

	private:
		VulkanResourceManager(const VulkanResourceManager&) = delete;
		VulkanResourceManager& operator=(const VulkanResourceManager&) = delete;

		std::unique_ptr<Device> m_device = nullptr;
		VulkanResourcePool<DescriptorSetLayout> m_descriptorLayouts = {};
		VulkanResourcePool<Pipeline> m_pipelines = {};
		std::unique_ptr<VulkanInitialize> m_vkInitialize = nullptr;
		std::unique_ptr<SwapChain> m_swapChain = nullptr;
		std::unique_ptr<RenderPass> m_renderPass = nullptr;
		std::unique_ptr<BufferArea> m_bufferArea = nullptr;
		std::unique_ptr<Descriptor> m_descriptor = nullptr;
		std::unique_ptr<SyncObject> m_syncObject = nullptr;

		void CreateTextureSampler();						// 创建采样器
		void DestroyTextureSampler();						// 销毁采样器
		VkSampler m_textureSampler = {};

	public:
		inline Device* GetDevice() { return m_device.get(); }
		inline VulkanInitialize* GetVulkanInitialize() { return m_vkInitialize.get(); }
		inline SwapChain* GetSwapChain() { return m_swapChain.get(); }
		inline RenderPass* GetRenderPass() { return m_renderPass.get(); }
		inline BufferArea* GetBufferArea() { return m_bufferArea.get(); }
		inline Descriptor* GetDescriptor() { return m_descriptor.get(); }
		inline SyncObject* GetSyncObject() { return m_syncObject.get(); }

		inline VulkanResourcePool<Pipeline>& GetPipelines() { return m_pipelines; }
		inline VulkanResourcePool<DescriptorSetLayout>& GetDescriptorLayouts() { return m_descriptorLayouts; }

		inline const VkSampler& GetTextureSampler() const { return m_textureSampler; }
	};
}
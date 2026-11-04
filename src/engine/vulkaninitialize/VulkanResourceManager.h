#pragma once

#include "VulkanResourceBase.h"
#include "VulkanResourcePool.h"
#include "vulkaninitialize/Device.h"
#include "vulkaninitialize/DescriptorSetLayout.h"
#include "vulkanInitialize/Pipeline.h"
#include "vulkanInitialize/VulkanInitialize.h"
#include "vulkanInitialize/SwapChain.h"
#include "vulkanInitialize/RenderPass.h"
#include "vulkaninitialize/CommandContext.h"
#include "vulkaninitialize/BufferContext.h"
#include "vulkanInitialize/BufferArea.h"
#include "vulkaninitialize/FrameBuffer.h"
#include "vulkanInitialize/SyncObject.h"
#include "vulkanInitialize/Descriptor.h"
#include "vulkaninitialize/TextureSampler.h"

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

		UniquePtr<Device> m_device = nullptr;
		VulkanResourcePool<RenderPass> m_renderPasses = {};
		VulkanResourcePool<DescriptorSetLayout> m_descriptorLayouts = {};
		VulkanResourcePool<Pipeline> m_pipelines = {};
		std::unique_ptr<VulkanInitialize> m_vkInitialize = nullptr;
		std::unique_ptr<SwapChain> m_swapChain = nullptr;
		UniquePtr<CommandContext> m_commandContext = nullptr;
		UniquePtr<BufferContext> m_bufferContext = nullptr;
		std::unique_ptr<BufferArea> m_bufferArea = nullptr;
		std::unique_ptr<Descriptor> m_descriptor = nullptr;
		std::unique_ptr<SyncObject> m_syncObject = nullptr;

		void CreateTextureSampler();						// 创建采样器
		void DestroyTextureSampler();						// 销毁采样器
		VkSampler m_textureSampler = {};

		DeviceContext m_deviceContext = {};

	public:
		inline Device* GetDevice() { return m_device.get(); }
		inline VulkanInitialize* GetVulkanInitialize() { return m_vkInitialize.get(); }
		inline SwapChain* GetSwapChain() { return m_swapChain.get(); }
		inline BufferArea* GetBufferArea() { return m_bufferArea.get(); }
		inline Descriptor* GetDescriptor() { return m_descriptor.get(); }
		inline SyncObject* GetSyncObject() { return m_syncObject.get(); }

		inline VulkanResourcePool<RenderPass>& GetRenderPasses() { return m_renderPasses; }
		inline VulkanResourcePool<Pipeline>& GetPipelines() { return m_pipelines; }
		inline VulkanResourcePool<DescriptorSetLayout>& GetDescriptorLayouts() { return m_descriptorLayouts; }
		inline CommandContext* GetCommandContext() { return m_commandContext.get(); }
		inline BufferContext* GetBufferContext() { return m_bufferContext.get(); }

		inline const VkSampler& GetTextureSampler() const { return m_textureSampler; }

		inline const DeviceContext& GetDeviceContext() const { return m_deviceContext; }
	};
}
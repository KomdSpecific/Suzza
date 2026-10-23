#pragma once

#include "BaseObject.h"
#include "vulkanInitialize/Pipeline.h"
#include "vulkanInitialize/VulkanInitialize.h"
#include "vulkanInitialize/SwapChain.h"
#include "vulkanInitialize/RenderPass.h"
#include "vulkanInitialize/BufferArea.h"
#include "vulkanInitialize/SyncObject.h"
#include "vulkanInitialize/Descriptor.h"

#include <memory>

namespace suz
{
	class VulkanResourceManager : public BaseObject
	{
	public:
		VulkanResourceManager() = default;

		bool InitVulkan();

		virtual void Destroy() override;

	private:
		VulkanResourceManager(const VulkanResourceManager&) = delete;
		VulkanResourceManager& operator=(const VulkanResourceManager&) = delete;

		std::unique_ptr<Pipeline> m_pipeline = nullptr;
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
		inline Pipeline* GetPipeline() { return m_pipeline.get(); }
		inline VulkanInitialize* GetVulkanInitialize() { return m_vkInitialize.get(); }
		inline SwapChain* GetSwapChain() { return m_swapChain.get(); }
		inline RenderPass* GetRenderPass() { return m_renderPass.get(); }
		inline BufferArea* GetBufferArea() { return m_bufferArea.get(); }
		inline Descriptor* GetDescriptor() { return m_descriptor.get(); }
		inline SyncObject* GetSyncObject() { return m_syncObject.get(); }

		inline const VkSampler& GetTextureSampler() const { return m_textureSampler; }
	};
}
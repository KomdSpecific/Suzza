#include "VulkanResourceManager.h"
#include "io/FileSystem.h"

#include <string>
#include <filesystem>
#include <iostream>

namespace suz
{
	bool VulkanResourceManager::InitVulkan()
	{
		// TODO: Called And Make Init Function
		/* 初始化并调用Init函数 */
		m_vkInitialize = std::make_unique<VulkanInitialize>();
		m_swapChain = std::make_unique<SwapChain>(
			m_vkInitialize->GetPhysicalDevice(),
			m_vkInitialize->GetDevice(),
			m_vkInitialize->GetSurface()
		);
		m_renderPass = std::make_unique<RenderPass>(
			m_vkInitialize->GetDevice(),
			m_swapChain->GetSwapChainFormat()
		);
		m_pipeline = std::make_unique<Pipeline>(
			*m_vkInitialize,
			m_renderPass->GetRenderPass(),
			std::filesystem::path(FileSystem::GetShadersFolder() / "simple_vertexshader.vert.spv").string(),
			std::filesystem::path(FileSystem::GetShadersFolder() / "simple_fragmentshader.frag.spv").string(),
			Pipeline::DefaultPipelineConfigInfo()
		);
		/* BufferArea初始化 */
		m_bufferArea = std::make_unique<BufferArea>(
			*m_pipeline,
			m_vkInitialize->GetPhysicalDevice(),
			m_vkInitialize->GetDevice(),
			m_vkInitialize->GetSurface(),
			m_swapChain->GetSwapChainExtent(),
			m_renderPass->GetRenderPass()
		);
		m_bufferArea->InitBufferArea();

		m_vkTexture = std::make_unique<VulkanTexture>(m_vkInitialize->GetDevice());
		m_vkTexture->InitVulkanTexture();

		m_descriptor = std::make_unique<Descriptor>(m_vkInitialize->GetDevice());
		m_descriptor->InitDescriptor();

		m_syncObject = std::make_unique<SyncObject>(
			m_vkInitialize->GetDevice()
		);

		return true;
	}

	void VulkanResourceManager::Destroy()
	{
		if (!(*m_isAlive)) return;

		m_syncObject->Destroy();
		m_descriptor->Destroy();
		m_vkTexture->Destroy();
		m_bufferArea->Destroy();
		m_pipeline->Destroy();
		m_renderPass->Destroy();
		m_swapChain->Destroy();
		m_vkInitialize->Destroy();

		*m_isAlive = false;
	}
}
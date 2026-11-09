#include "VulkanResourceManager.h"
#include "file/FileSystem.h"
#include "macros/VkMacro.h"
#include "factory/VulkanInstanceFactories.h"
#include "factory/DeviceFactories.h"
#include "factory/DescriptorSetLayoutFactories.h"
#include "factory/PipelineFactories.h"
#include "factory/RenderPassFactories.h"
#include "factory/CommandPoolFactories.h"
#include "factory/SwapChainFactories.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	bool VulkanResourceManager::InitVulkan()
	{
		/* ==================Vulkan初始化================== */
		{
			m_vulkanInstance = std::make_unique<VulkanInstance>();
			VulkanInstanceSpec instanceSpec = suz_instance::CreateDefaultVulkanInstance();
			m_vulkanInstance->Create(instanceSpec);
		}

		/* ==================创建物理设备=================== */
		{
			m_physicalDevice = std::make_unique<PhysicalDevice>(
				m_vulkanInstance->GetVulkanInstance(),
				m_vulkanInstance->GetSurface(),
				DEVICE_EXTENSIONS
			);
			if (!m_physicalDevice->FindPhyscialDevice())
			{
				SUZZA_LOG_ERROR("Vulkan资源: 物理设备PhyscialDevice初始化失败!");
				return false;
			}
		}

		/* ===================创建逻辑设备=================== */
		{
			m_device = std::make_unique<Device>();
			DeviceSpec deviceSpec = suz_device::CreateDefaultDeviceSpec();
			if (!m_device->CreateDevice(deviceSpec))
			{
				SUZZA_LOG_ERROR("Vulkan资源: 逻辑设备Device初始化失败!");
				return false;
			}
		}

		/* ==================构建设备上下文================= */
		m_deviceContext.device = m_device->GetDevice();
		m_deviceContext.physicalDevice = m_physicalDevice->GetPhysicalDevice();
		m_deviceContext.graphicsQueue = m_device->GetGraphicsQueue();
		m_deviceContext.presentQueue = m_device->GetPresentQueue();
		m_deviceContext.graphicsFamilyIndex = m_device->GetGraphicsFamily();
		m_deviceContext.presentFamilyIndex = m_device->GetPresentFamily();


		/* =================创建命令池上下文================ */
		{
			m_commandContext = std::make_unique<CommandContext>(m_deviceContext);
			m_commandContext->CreateCommandContext(MAX_FRAMES_IN_FLIGHT);
		}


		SwapChainSpec swapChainSpec = suz_swapchain::CreateDefaultSwapChain();

		/* ===================渲染通道创建================== */
		{
			RenderPassSpec renderPassSpec = suz_renderpass::CreateDefaultRenderPassSpec(m_deviceContext, swapChainSpec.surfaceFormat.format);
			m_renderPasses.CreateResource<RenderPassSpec>("DefaultRenderPass", renderPassSpec);
		}

		/* ===================创建交换链=================== */
		{
			m_swapChain = std::make_unique<SwapChain>(m_commandContext.get(), m_renderPasses.GetResource("DefaultRenderPass")->GetRenderPass());
			m_swapChain->InitSwapChain(swapChainSpec);
		}

		/* ==================创建描述符布局================= */
		{
			DescriptorSetLayoutSpec cameraLayoutSpec = suz_descLayout::CreateCameraDescriptorSetLayoutSpec();
			m_descriptorLayouts.CreateResource<DescriptorSetLayoutSpec>("CameraDesc", cameraLayoutSpec);
			DescriptorSetLayoutSpec bindlessLayoutSpec = suz_descLayout::CreateBindlessDescriptorSetLayoutSpec();
			m_descriptorLayouts.CreateResource<DescriptorSetLayoutSpec>("BindlessDesc", bindlessLayoutSpec);
		}

		/* ====================1. 管线创建===================== */
		{
			PipelineLayoutSpec layoutSpec = suz_pipeline::CreateDefaultPipelineLayoutSpec();
			m_pipelines.CreateResource<PipelineLayoutSpec>("DefaultPipeline", layoutSpec);

			PipelineSpec pipelineSpec = suz_pipeline::CreateDefaultGraphicsPipelineSpec(
				std::filesystem::path(FileSystem::GetShadersFolder() / "simple_vertexshader.vert.spv").string(),
				std::filesystem::path(FileSystem::GetShadersFolder() / "simple_fragmentshader.frag.spv").string());
			m_pipelines.CreateResource<PipelineSpec>("DefaultPipeline", pipelineSpec);
		}

		/* =================2. 网格专属管线创建================= */
		{
			PipelineLayoutSpec layoutSpec = suz_pipeline::CreateDefaultPipelineLayoutSpec();
			m_pipelines.CreateResource<PipelineLayoutSpec>("GridPipeline", layoutSpec);

			PipelineSpec pipelineSpec = suz_pipeline::CreateDefaultGridPipelineSpec(
				std::filesystem::path(FileSystem::GetShadersFolder() / "grid_vertexshader.vert.spv").string(),
				std::filesystem::path(FileSystem::GetShadersFolder() / "grid_fragmentshader.frag.spv").string());
			m_pipelines.CreateResource<PipelineSpec>("GridPipeline", pipelineSpec);
		}

		/* ==================创建缓冲上下文===================*/
		{
			m_bufferContext = std::make_unique<BufferContext>(m_deviceContext, m_commandContext.get());
			m_bufferContext->CreateBufferContext(MAX_FRAMES_IN_FLIGHT);
		}

		m_descriptor = std::make_unique<Descriptor>(m_device->GetDevice());
		m_descriptor->InitDescriptor();

		m_syncObject = std::make_unique<SyncObject>(
			m_device->GetDevice()
		);

		/* ===================创建图像采样器=================== */
		{
			TextureSamplerSpec textureSamplerSpec(m_deviceContext);
			
			m_textureSampler = std::make_unique<TextureSampler>();
			m_textureSampler->Create(textureSamplerSpec);
		}

		return true;
	}

	void VulkanResourceManager::Destroy()
	{
		if (!(*m_isAlive)) return;

		m_textureSampler->Destroy();
		m_syncObject->Destroy();
		m_descriptor->Destroy();
		m_bufferContext->Destroy();
		m_pipelines.Destroy();
		m_descriptorLayouts.Destroy();
		m_swapChain->Destroy();
		m_renderPasses.Destroy();
		m_commandContext->Destroy();
		m_device->Destroy();
		m_vulkanInstance->Destroy();

		*m_isAlive = false;
	}
}
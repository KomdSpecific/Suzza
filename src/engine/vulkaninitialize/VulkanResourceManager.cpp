#include "VulkanResourceManager.h"
#include "file/FileSystem.h"
#include "functionLibrary/DeviceFunctionLibrary.h"
#include "factory/DeviceFactories.h"
#include "factory/DescriptorSetLayoutFactories.h"
#include "factory/PipelineFactories.h"

#include "debug/SuzzaLog.h"
#include <filesystem>

namespace suz
{
	bool VulkanResourceManager::InitVulkan()
	{
		/* ===============初始化并调用Init函数=============== */
		m_vkInitialize = std::make_unique<VulkanInitialize>();

		/* ===================创建逻辑设备===================*/
		{
			m_device = std::make_unique<Device>();
			DeviceSpec deviceSpec = suz_device::CreateDefaultDeviceSpec();
			if (!m_device->CreateDevice(deviceSpec))
			{
				SUZZA_LOG_ERROR("Vulkan资源初始化失败!");
				return false;
			}
		}

		m_swapChain = std::make_unique<SwapChain>(m_device->GetDevice());
		m_swapChain->InitSwapChain();

		/* 渲染通道创建 */
		m_renderPass = std::make_unique<RenderPass>(
			m_device->GetDevice(),
			m_swapChain->GetSwapChainFormat()
		);

		/* ==================创建描述符布局================= */
		{
			DescriptorSetLayoutSpec cameraLayoutSpec = suz_descLayout::CreateCameraDescriptorSetLayoutSpec();
			DescriptorSetLayoutSpec bindlessLayoutSpec = suz_descLayout::CreateBindlessDescriptorSetLayoutSpec();
			m_descriptorLayouts.CreateResource<DescriptorSetLayoutSpec>("CameraDesc", cameraLayoutSpec);
			m_descriptorLayouts.CreateResource<DescriptorSetLayoutSpec>("BindlessDesc", bindlessLayoutSpec);
		}

		/* ====================管线创建===================== */
		{
			PipelineLayoutSpec layoutSpec = suz_pipeline::CreateDefaultPipelineLayoutSpec();
			m_pipelines.CreateResource<PipelineLayoutSpec>("DefaultPipeline", layoutSpec);

			PipelineSpec pipelineSpec = suz_pipeline::CreateDefaultGraphicsPipelineSpec(
				std::filesystem::path(FileSystem::GetShadersFolder() / "simple_vertexshader.vert.spv").string(),
				std::filesystem::path(FileSystem::GetShadersFolder() / "simple_fragmentshader.frag.spv").string());
			m_pipelines.CreateResource<PipelineSpec>("DefaultPipeline", pipelineSpec);
		}


		/* BufferArea初始化 */
		m_bufferArea = std::make_unique<BufferArea>(
			*m_pipelines.GetResource("DefaultPipeline"),
			m_device->GetDevice(),
			m_swapChain->GetSwapChainExtent(),
			m_renderPass->GetRenderPass()
		);
		m_bufferArea->InitBufferArea();

		m_descriptor = std::make_unique<Descriptor>(m_device->GetDevice());
		m_descriptor->InitDescriptor();

		m_syncObject = std::make_unique<SyncObject>(
			m_device->GetDevice()
		);

		CreateTextureSampler();

		return true;
	}

	void VulkanResourceManager::Destroy()
	{
		if (!(*m_isAlive)) return;

		DestroyTextureSampler();

		m_syncObject->Destroy();
		m_descriptor->Destroy();
		m_bufferArea->Destroy();
		m_descriptorLayouts.Destroy();
		m_pipelines.Destroy();
		m_renderPass->Destroy();
		m_swapChain->Destroy();
		m_device->Destroy();
		m_vkInitialize->Destroy();

		*m_isAlive = false;
	}

	void VulkanResourceManager::CreateTextureSampler()
	{
		const VkPhysicalDevice& physicalDevice = m_vkInitialize->GetPhysicalDevice();
		VkPhysicalDeviceProperties properties = DeviceFunctionLibrary::GetPhysicalDeviceProperties(physicalDevice);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		/* 当读取超出图像范围时返回窗口背景纯色 */
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;

		/* 开启各向异性过滤 */
		samplerInfo.anisotropyEnable = VK_TRUE;
		/* 传入物理设备支持的纹素样本数量最大值 */
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

		/* 采样超出图像时填充的颜色 */
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		/* mipmapping过滤器 */
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(m_device->GetDevice(), &samplerInfo, nullptr, &m_textureSampler) != VK_SUCCESS)
		{
			throw std::runtime_error("创建纹理采样器失败!");
		}
	}

	void VulkanResourceManager::DestroyTextureSampler()
	{
		vkDestroySampler(m_device->GetDevice(), m_textureSampler, nullptr);
	}
}
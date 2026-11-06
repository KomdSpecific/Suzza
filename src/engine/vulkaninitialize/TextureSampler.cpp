#include "TextureSampler.h"
#include "functionLibrary/DeviceFunctionLibrary.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	VkSamplerCreateInfo TextureSamplerSpec::DefaultSamplerCreateInfo(const TextureSamplerSpec& Spec)
	{
		VkPhysicalDeviceProperties physicalProperties = DeviceFunctionLibrary::GetPhysicalDeviceProperties(Spec.deviceContext.physicalDevice);

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
		samplerInfo.maxAnisotropy = physicalProperties.limits.maxSamplerAnisotropy;

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

		return samplerInfo;
	}

	TextureSampler::~TextureSampler()
	{
		if (*m_isAlive)
		{
			SUZZA_LOG_ERROR("Class TextureSampler Warning: 未调用Destroy手动销毁!");
			Destroy();
		}
	}

	bool TextureSampler::Create(TextureSamplerSpec& Spec)
	{
		return CreateTextureSampler(Spec);
	}

	bool TextureSampler::CreateTextureSampler(TextureSamplerSpec& Spec)
	{
		if (!Spec.deviceContext.PhysicalDeviceValid())
		{
			SUZZA_LOG_ERROR("无法创建采样器: 物理设备physicalDevice不合法!");
			return false;
		}

		if (!Spec.deviceContext.DeviceValid())
		{
			SUZZA_LOG_ERROR("无法创建采样器: 逻辑设备device不合法!");
			return false;
		}

		m_deviceContext = Spec.deviceContext;

		VkSamplerCreateInfo samplerInfo = TextureSamplerSpec::DefaultSamplerCreateInfo(Spec);
		if (vkCreateSampler(Spec.deviceContext.device, &samplerInfo, nullptr, &m_textureSampler) != VK_SUCCESS)
		{
			SUZZA_LOG_ERROR("创建纹理采样器失败!");
			return false;
		}

		return true;
	}

	void TextureSampler::Destroy()
	{
		if (!(*m_isAlive)) return;

		vkDestroySampler(m_deviceContext.device, m_textureSampler, nullptr);

		*m_isAlive = false;
	}
}
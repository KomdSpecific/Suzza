#pragma once

#include <vulkan/vulkan.h>

namespace suz
{
	struct TextureSamplerSpec
	{
		TextureSamplerSpec(const VkPhysicalDevice& PhysicalDevice)
			: physicalDevice(PhysicalDevice) {
		}

		VkPhysicalDevice physicalDevice;

		static VkSamplerCreateInfo DefaultSamplerCreateInfo(const TextureSamplerSpec& Spec);
	};

	class TextureSampler
	{
	public:
		TextureSampler() = default;
		~TextureSampler();

		bool Create(TextureSamplerSpec& Spec);
		bool CreateTextureSampler(TextureSamplerSpec& Spec);

	private:
		VkPhysicalDevice m_physicalDevice = {};

		VkSampler m_textureSampler = {};
	};
}
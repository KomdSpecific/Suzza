#pragma once

#include "DeviceContext.h"
#include "VulkanResourceBase.h"

namespace suz
{
	struct TextureSamplerSpec
	{
		TextureSamplerSpec(const DeviceContext& Context)
			: deviceContext(Context) {
		}

		DeviceContext deviceContext = {};

		static VkSamplerCreateInfo DefaultSamplerCreateInfo(const TextureSamplerSpec& Spec);
	};

	class TextureSampler : public VulkanResourceBase
	{
	public:
		TextureSampler() = default;
		~TextureSampler();

		bool Create(TextureSamplerSpec& Spec);
		bool CreateTextureSampler(TextureSamplerSpec& Spec);

		void Destroy();

	private:
		DeviceContext m_deviceContext = {};

		VkSampler m_textureSampler = {};

	public:
		inline const VkSampler& GetTextureSampler() const { return m_textureSampler; }
	};
}
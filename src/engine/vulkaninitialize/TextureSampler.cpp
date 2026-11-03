#include "TextureSampler.h"

namespace suz
{
	VkSamplerCreateInfo TextureSamplerSpec::DefaultSamplerCreateInfo(const TextureSamplerSpec& Spec)
	{
		VkSamplerCreateInfo samplerInfo{};

		return samplerInfo;
	}
}
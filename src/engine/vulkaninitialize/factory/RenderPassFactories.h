#pragma once

#include "vulkaninitialize/RenderPass.h"

namespace suz
{
	struct DeviceContext;

	namespace suz_renderpass
	{
		RenderPassSpec CreateDefaultRenderPassSpec(const DeviceContext& Context, const VkFormat& SwapChainFormat);
	}
}
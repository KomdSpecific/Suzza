#pragma once

#include "BaseObject.h"

#include <vulkan/vulkan.h>

namespace suz
{
	class RenderPass : public BaseObject
	{
	public:
		RenderPass(const VkDevice& Device, const VkFormat& SwapChainImageFormat);
		~RenderPass();

		virtual void Destroy() override;

	private:
		void CreateRenderPass();

		const VkDevice& m_device;
		const VkFormat& m_swapChainImageFormat;

		VkRenderPass m_renderPass = {};

	public:
		inline const VkRenderPass& GetRenderPass() const { return m_renderPass; }
	};
}
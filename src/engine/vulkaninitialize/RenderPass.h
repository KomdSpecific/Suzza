#pragma once

#include "DeviceContext.h"
#include "VulkanResourceBase.h"

#include <vulkan/vulkan.h>

namespace suz
{
	struct RenderPassSpec
	{
		RenderPassSpec(const DeviceContext& Context, const VkFormat& ImageFormat)
			: deviceContext(Context), swapChainImageFormat(ImageFormat) {
		}

		DeviceContext deviceContext = {};
		VkFormat swapChainImageFormat = {};

		/* 颜色缓冲附件属性 */
		Vector<VkAttachmentDescription> attachmentDesc{};
		/* 子通道描述的颜色附件引用 */
		VkAttachmentReference colorAttachmentReference{};
		VkAttachmentReference depthAttachmentReference{};
		/* 子通道 */
		VkSubpassDescription subpass{};
		/* 子通道依赖 */
		VkSubpassDependency subpassDependency{};

		static VkRenderPassCreateInfo DefaultRenderPassCreateInfo(RenderPassSpec& Spec);
	};

	class RenderPass : public VulkanResourceBase
	{
	public:
		RenderPass() = default;
		~RenderPass();

		bool Create(RenderPassSpec& Spec);
		bool CreateRenderPass(RenderPassSpec& Spec);

		void Destroy();

	private:
		/* 必要资源句柄 */
		DeviceContext m_deviceContext = {};

		VkRenderPass m_renderPass = {};

	public:
		inline const VkRenderPass& GetRenderPass() const { return m_renderPass; }
	};
}
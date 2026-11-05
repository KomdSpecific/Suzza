#pragma once

#include "DeviceContext.h"
#include "VulkanResourceBase.h"

#include <vulkan/vulkan.h>

namespace suz
{
	struct FrameBufferSpec
	{
		FrameBufferSpec(const DeviceContext& Context, const VkRenderPass& RenderPass)
			: deviceContext(Context), renderPass(RenderPass) {
		};

		DeviceContext deviceContext = {};
		VkRenderPass renderPass = {};

		VkExtent2D swapChainExtent = { 0,0 };
		/* 每一个帧缓冲的附件 */
		Vector<VkImageView> attachments = {};

		static VkFramebufferCreateInfo DefaultFrameBufferCreateInfo(const FrameBufferSpec& Spec);
	};

	class FrameBuffer : public VulkanResourceBase
	{
	public:
		FrameBuffer() = default;
		~FrameBuffer();

		bool Create(FrameBufferSpec& Spec);
		bool CreateFrameBuffer(FrameBufferSpec& Spec);

		void Destroy();

	private:
		DeviceContext m_deviceContext = {};

		VkFramebuffer m_frameBuffer = {};

	public:
		inline const VkFramebuffer& GetFrameBuffer() const { return m_frameBuffer; }
	};
}
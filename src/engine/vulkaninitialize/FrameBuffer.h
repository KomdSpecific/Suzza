#pragma once

#include "DeviceContext.h"
#include "VulkanResourceBase.h"

#include <vulkan/vulkan.h>

namespace suz
{
	struct FrameBufferSpec
	{
		FrameBufferSpec(const DeviceContext& Context)
			: deviceContext(Context) {
		};

		DeviceContext deviceContext = {};

		/* 每一个帧缓冲的附件 */
		Vector<VkImageView> attachments = {};
	};

	class FrameBuffer
	{
	public:
		FrameBuffer() = default;
		~FrameBuffer();

		bool Create(FrameBufferSpec& Spec);
		bool CreateFrameBuffer(FrameBufferSpec& Spec);

	private:
		DeviceContext m_deviceContext = {};
	};
}
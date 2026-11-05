#include "FrameBuffer.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	VkFramebufferCreateInfo FrameBufferSpec::DefaultFrameBufferCreateInfo(const FrameBufferSpec& Spec)
	{
		VkFramebufferCreateInfo frameBufferInfo{};
		frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferInfo.renderPass = Spec.renderPass;
		frameBufferInfo.width = Spec.swapChainExtent.width;
		frameBufferInfo.height = Spec.swapChainExtent.height;
		frameBufferInfo.attachmentCount = static_cast<uint32>(Spec.attachments.size());
		frameBufferInfo.pAttachments = Spec.attachments.data();
		frameBufferInfo.layers = 1;

		return frameBufferInfo;
	}

	FrameBuffer::~FrameBuffer()
	{
		if (*m_isAlive)
		{
			SUZZA_LOG_ERROR("Class FrameBuffer Warning: 未调用Destroy手动销毁!");
			Destroy();
		}
	}

	bool FrameBuffer::Create(FrameBufferSpec& Spec)
	{
		return CreateFrameBuffer(Spec);
	}

	bool FrameBuffer::CreateFrameBuffer(FrameBufferSpec& Spec)
	{
		if (!Spec.deviceContext.DeviceValid())
		{
			SUZZA_LOG_ERROR("无法创建帧缓冲: 逻辑设备device不合法!");
			return false;
		}

		m_deviceContext = Spec.deviceContext;

		VkFramebufferCreateInfo frameBufferInfo = FrameBufferSpec::DefaultFrameBufferCreateInfo(Spec);
		if (vkCreateFramebuffer(Spec.deviceContext.device, &frameBufferInfo, nullptr, &m_frameBuffer) != VK_SUCCESS)
		{
			SUZZA_LOG_ERROR("创建帧缓冲失败!");
			return false;
		}

		return true;
	}

	void FrameBuffer::Destroy()
	{
		if (!(*m_isAlive)) return;

		vkDestroyFramebuffer(m_deviceContext.device, m_frameBuffer, nullptr);

		*m_isAlive = false;
	}
}
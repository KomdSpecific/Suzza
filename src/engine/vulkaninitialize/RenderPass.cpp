#include "RenderPass.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	VkRenderPassCreateInfo RenderPassSpec::DefaultRenderPassCreateInfo(RenderPassSpec& Spec)
	{
		Spec.subpass.colorAttachmentCount = 1;
		Spec.subpass.pColorAttachments = &Spec.colorAttachmentReference;				// 绑定颜色附件
		Spec.subpass.pDepthStencilAttachment = &Spec.depthAttachmentReference;			// 绑定深度附件

		/* 定义渲染通道对象信息 */
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32>(Spec.attachmentDesc.size());
		renderPassInfo.pAttachments = Spec.attachmentDesc.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &Spec.subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &Spec.subpassDependency;

		return renderPassInfo;
	}

	RenderPass::~RenderPass()
	{
		if (*m_isAlive)
		{
			SUZZA_LOG_ERROR("Class RenderPass Warning: 未调用Destroy手动销毁!");
			Destroy();
		}
	}

	bool RenderPass::Create(RenderPassSpec& Spec)
	{
		return CreateRenderPass(Spec);
	}

	bool RenderPass::CreateRenderPass(RenderPassSpec& Spec)
	{
		if (!Spec.deviceContext.DeviceValid())
		{
			SUZZA_LOG_ERROR("无法创建渲染通道: 逻辑设备device不合法!");
			return false;
		}

		m_deviceContext = Spec.deviceContext;

		VkRenderPassCreateInfo renderPassInfo = RenderPassSpec::DefaultRenderPassCreateInfo(Spec);
		if (vkCreateRenderPass(Spec.deviceContext.device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
		{
			SUZZA_LOG_ERROR("创建渲染通道失败!");
			return false;
		}

		return true;
	}

	void RenderPass::Destroy()
	{
		if (!(*m_isAlive)) return;

		vkDestroyRenderPass(m_deviceContext.device, m_renderPass, nullptr);

		*m_isAlive = false;
	}
}
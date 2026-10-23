#include "RenderPass.h"
#include "vulkaninitialize/BufferArea.h"

#include "debug/SuzzaLog.h"

#include <stdexcept>

namespace suz
{
	RenderPass::RenderPass(const VkDevice& Device, const VkFormat& SwapChainImageFormat)
		: m_device(Device), m_swapChainImageFormat(SwapChainImageFormat)
	{
		CreateRenderPass();
	}

	RenderPass::~RenderPass()
	{
		if (*m_isAlive)
		{
			SUZZA_LOG_ERROR("Class RenderPass Warning: 未调用Destroy手动销毁!");
			Destroy();
		}
	}

	void RenderPass::Destroy()
	{
		if (!(*m_isAlive)) return;

		vkDestroyRenderPass(m_device, m_renderPass, nullptr);

		*m_isAlive = false;
	}

	void RenderPass::CreateRenderPass()
	{
		/* ==========定义颜色缓冲附件属性========== */
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		/* 确定渲染前后如何处理附件中的数据 */
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;					// 设置颜色, 在开始时清除为常量
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;					// 设置深度数据, 渲染内容存储在内存中
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;		// 不关心模板缓冲
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;			// 在交换链中呈现的图像

		VkFormat depthFormat = BufferArea::FindDepthFormat();
		/* ==========定义深度缓冲附件属性========== */
		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = depthFormat;									// 深度格式
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;						// 无多重采样
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;					// 每帧开始清除深度值
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;				// 绘制结束不保留深度数据
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;		// 不使用模板加载
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;		// 不使用模板存储
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;				// 初始布局: 丢弃旧数据
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;	// 最终布局: 保持附件布局

		/* ==========定义子通道描述的颜色附件引用========== */
		VkAttachmentReference colorAttachmentReference{};
		colorAttachmentReference.attachment = 0;								// 引用附件数组0号元素
		colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		/* ==========定义子通道描述的深度附件引用========== */
		VkAttachmentReference depthAttachmentReference{};
		depthAttachmentReference.attachment = 1;								// 引用附件数组1号元素
		depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;	// 深度附件最优布局

		/* 定义子通道 */
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;			// 该通道是图形子通道
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentReference;					// 绑定颜色附件
		subpass.pDepthStencilAttachment = &depthAttachmentReference;			// 绑定深度附件

		/* 定义子通道依赖 */
		/* 保证布局转换在子通道阶段之前完成, 避免读写冲突 */
		VkSubpassDependency subpassDependency{};
		subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependency.dstSubpass = 0;
		/* 指定需要等待的操作以及这些操作发生的阶段 */
		subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |	// 颜色输出 + 早期片段测试
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		subpassDependency.srcAccessMask = 0;
		subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |	// 颜色输出 + 早期片段测试
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |			// 颜色写 + 深度附件读写
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachment = { colorAttachment,depthAttachment };

		/* 定义渲染通道对象信息 */
		VkRenderPassCreateInfo renderCreateInfo{};
		renderCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderCreateInfo.attachmentCount = static_cast<uint32_t>(attachment.size());
		renderCreateInfo.pAttachments = attachment.data();
		renderCreateInfo.subpassCount = 1;
		renderCreateInfo.pSubpasses = &subpass;
		renderCreateInfo.dependencyCount = 1;
		renderCreateInfo.pDependencies = &subpassDependency;

		if (vkCreateRenderPass(m_device, &renderCreateInfo, nullptr, &m_renderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("创建渲染队列失败!");
		}
	}
}
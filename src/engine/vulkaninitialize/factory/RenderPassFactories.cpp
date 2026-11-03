#include "RenderPassFactories.h"
#include "Engine.h"
#include "vulkaninitialize/VulkanResourceManager.h"
#include "vulkaninitialize/BufferArea.h"
#include "vulkaninitialize/functionLibrary/VulkanFunctionLibrary.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	namespace suz_renderpass
	{
		RenderPassSpec CreateDefaultRenderPassSpec()
		{
			VulkanResourceManager& manager = Engine::GetVulkanResourceManager();

			const VkDevice& device = manager.GetDevice()->GetDevice();
			const VkFormat& swapChainImageFormat = manager.GetSwapChain()->GetSwapChainFormat();

			RenderPassSpec spec(device, swapChainImageFormat);
			spec.attachmentDesc.resize(2);

			/* ==========定义颜色缓冲附件属性========== */
			spec.attachmentDesc[0].format = swapChainImageFormat;
			spec.attachmentDesc[0].samples = VK_SAMPLE_COUNT_1_BIT;					// 无多重采样
			/* 确定渲染前后如何处理附件中的数据 */
			spec.attachmentDesc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;			// (重要)设置颜色, 在开始时清除为常量
			spec.attachmentDesc[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;			// (重要)设置深度数据, 渲染内容存储在内存中
			spec.attachmentDesc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;// 不关心模板缓冲
			spec.attachmentDesc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			spec.attachmentDesc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			spec.attachmentDesc[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;	// 在交换链中呈现的图像

			VkFormat depthFormat = VulkanFunctionLibrary::FindDepthFormat(
				Engine::GetVulkanResourceManager().GetVulkanInitialize()->GetPhysicalDevice()
			);
			/* ==========定义深度缓冲附件属性========== */
			spec.attachmentDesc[1].format = depthFormat;
			spec.attachmentDesc[1].samples = VK_SAMPLE_COUNT_1_BIT;					// 无多重采样
			spec.attachmentDesc[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;			// (重要)每帧开始清除深度值
			spec.attachmentDesc[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;		// (重要)绘制结束不保留深度数据
			spec.attachmentDesc[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;	// 不使用模板加载
			spec.attachmentDesc[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;// 不使用模板存储
			spec.attachmentDesc[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		// 初始布局: 丢弃旧数据
			spec.attachmentDesc[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;	// // 最终布局: 保持模板附件布局

			/* ==========定义子通道描述的颜色附件引用========== */
			spec.colorAttachmentReference.attachment = 0;							// 引用附件数组0号元素
			spec.colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			/* ==========定义子通道描述的深度附件引用========== */
			spec.depthAttachmentReference.attachment = 1;							// 引用附件数组1号元素
			spec.depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;// 深度附件最优布局(模板布局)

			/* 定义子通道 */
			spec.subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;		// 该通道是图形子通道

			/* 定义子通道依赖 */
			/* 保证布局转换在子通道阶段之前完成, 避免读写冲突 */
			spec.subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			spec.subpassDependency.dstSubpass = 0;
			/* 指定需要等待的操作以及这些操作发生的阶段 */
			spec.subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;							// 颜色输出 + 早期片段测试
			spec.subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;							// 颜色输出 + 早期片段测试
			spec.subpassDependency.srcAccessMask = 0;
			spec.subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;						// // 颜色写 + 深度附件写

			return spec;
		}
	}
}
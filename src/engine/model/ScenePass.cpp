#include "ScenePass.h"
#include "model/Mesh.h"
#include "vulkaninitialize/Pipeline.h"
#include "vulkaninitialize/SwapChain.h"
#include "vulkaninitialize/RenderPass.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	ScenePass::ScenePass(SwapChain* InSwapChain, RenderPass* InRenderPass)
		: m_swapChain(InSwapChain), m_renderPass(InRenderPass)
	{
	}

	ScenePass::~ScenePass()
	{
	}

	void ScenePass::RecordCommandBuffer(const VkCommandBuffer& CommandBuffer, const Vector<VkDescriptorSet>& DescriptorSets, const Vector<DrawStage>& Stages, uint8 ImageIndex)
	{
		VkCommandBufferBeginInfo commandBegin{};
		commandBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBegin.flags = 0;
		commandBegin.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(CommandBuffer, &commandBegin) != VK_SUCCESS)
		{
			SUZZA_LOG_ERROR("开始记录命令缓冲区失败!");
			return;
		}

		/* 配置背景颜色 */
		Vector<VkClearValue> clearValues(2);
		clearValues[0].color = { 0.0f,0.0f,0.0f,1.0f };								// 设置颜色清除值 (背景颜色)
		clearValues[1].depthStencil = { 1.0f,0 };									// 深度 + 模板清除值

		/* 配置渲染通道信息 */
		VkRenderPassBeginInfo renderPassBegin = DefaultBeginRenderPass(clearValues, ImageIndex);
		/* 将命令缓冲区和渲染通道绑定 */
		vkCmdBeginRenderPass(CommandBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);

		ViewportConfigInfo viewportConfig = DefaultViewportConfig();
		/* 设置图形视口和裁剪矩形区域 */
		vkCmdSetViewport(CommandBuffer, 0, 1, &viewportConfig.viewport);
		vkCmdSetScissor(CommandBuffer, 0, 1, &viewportConfig.scissor);

		for (const DrawStage& stage : Stages)
		{
			if (!stage.pipeline)
				continue;

			/* 绑定图形管线 */
			vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, stage.pipeline->GetGraphicsPipeline());

			/* 绑定描述符集合, 并从映射内存读取数据 */
			/* 绑定摄像机描述符和Bindless描述符 */
			vkCmdBindDescriptorSets(CommandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS, stage.pipeline->GetPipelineLayout(),
				0, static_cast<uint32>(DescriptorSets.size()), DescriptorSets.data(),
				0, nullptr
			);

			if (stage.pipelinePushConstant)
			{
				vkCmdPushConstants(CommandBuffer, stage.pipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, stage.constantSize, stage.pipelinePushConstant);
			}

			if (!stage.drawList || stage.drawList->IsEmpty())
			{
				vkCmdDraw(CommandBuffer, 3, 1, 0, 0);
				continue;
			}

			for (const auto& [mesh, modelConstants] : stage.drawList->GetDrawQueue())
			{
				/* 绑定顶点缓冲 */
				VkDeviceSize offset = 0;												// 定义读取顶点数据的字节偏移量
				vkCmdBindVertexBuffers(CommandBuffer, 0, 1, &mesh->GetMeshVertexBuffer(), &offset);
				vkCmdBindIndexBuffer(CommandBuffer, mesh->GetMeshIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

				for (const ModelConstant& constant : modelConstants)
				{
					vkCmdPushConstants(CommandBuffer, stage.pipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ModelConstant), &constant);

					/* 发出绘制命令 */
					/* 告诉Vulkan使用索引缓冲区 */
					vkCmdDrawIndexed(CommandBuffer, mesh->GetIndexBufferSize(), 1, 0, 0, 0);
				}
			}
		}

		/* 广播执行命令缓冲区委托 */
		m_commandBufferDelegate.Execute(CommandBuffer);

		/* 结束渲染通道 */
		vkCmdEndRenderPass(CommandBuffer);
		/* 结束记录命令缓冲区 */
		if (vkEndCommandBuffer(CommandBuffer) != VK_SUCCESS)
		{
			SUZZA_LOG_ERROR("结束记录命令缓冲区失败!");
		}
	}

	VkRenderPassBeginInfo ScenePass::DefaultBeginRenderPass(const Vector<VkClearValue>& ClearValues, uint8 ImageIndex)
	{
		/* 配置渲染通道信息 */
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_renderPass->GetRenderPass();
		renderPassInfo.framebuffer = m_swapChain->GetFrameBuffer(ImageIndex).GetFrameBuffer();

		renderPassInfo.renderArea.offset = { 0,0 };									// 定义加载原点 (加载矩形左上角)
		renderPassInfo.renderArea.extent = m_swapChain->GetSwapChainExtent();

		/* 配置背景颜色 */
		renderPassInfo.clearValueCount = static_cast<uint32>(ClearValues.size());
		renderPassInfo.pClearValues = ClearValues.data();

		return renderPassInfo;
	}

	ViewportConfigInfo ScenePass::DefaultViewportConfig()
	{
		VkExtent2D swapChainExtent = m_swapChain->GetSwapChainExtent();

		ViewportConfigInfo viewport{};
		/* 配置视口信息, 定义管线坐标映射到图像的输出坐标 */
		viewport.viewport.x = 0.0f;
		viewport.viewport.y = 0.0f;
		/* 定义视口宽度和高度 */
		viewport.viewport.width = static_cast<float>(swapChainExtent.width);
		viewport.viewport.height = static_cast<float>(swapChainExtent.height);
		viewport.viewport.minDepth = 0.0f;
		viewport.viewport.maxDepth = 1.0f;

		/* 定义裁剪矩形, 在窗口裁剪出extent大小 */
		viewport.scissor.offset = { 0,0 };
		viewport.scissor.extent = { swapChainExtent.width,swapChainExtent.height };

		return viewport;
	}
}
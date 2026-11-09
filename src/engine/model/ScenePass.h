#pragma once

#include "DrawList.h"
#include "SingleDelegateDeclareMacro.h"

#include <vulkan/vulkan.h>

DECLARE_SINGLE_DELEGATE_ONE_PARAM(RecordCommandBufferDelegate, VkCommandBuffer)

namespace suz
{
	class SwapChain;
	class RenderPass;
	class Pipeline;

	struct ViewportConfigInfo
	{
		VkViewport viewport;
		VkRect2D scissor;
	};

	struct DrawStage
	{
		Pipeline* pipeline = nullptr;
		DrawList* drawList = nullptr;

		/* 网格这种"每条 pipeline 只有一个常量"的场合用它 */
		const void* pipelinePushConstant = nullptr;
		uint32 constantSize = 0;
	};

	class ScenePass
	{
	public:
		ScenePass(SwapChain* InSwapChain, RenderPass* InRenderPass);
		~ScenePass();

		void RecordCommandBuffer(const VkCommandBuffer& CommandBuffer, const Vector<VkDescriptorSet>& DescriptorSets, const Vector<DrawStage>& Stages, uint8 ImageIndex);

	private:
		VkRenderPassBeginInfo DefaultBeginRenderPass(const Vector<VkClearValue>& ClearValues, uint8 ImageIndex);
		ViewportConfigInfo DefaultViewportConfig();

		/* 命令缓冲区记录单播委托 */
		RecordCommandBufferDelegate m_commandBufferDelegate;

		SwapChain* m_swapChain = nullptr;
		RenderPass* m_renderPass = nullptr;

	public:
		inline RecordCommandBufferDelegate& GetRecordCommandBufferDelegate() { return m_commandBufferDelegate; }
	};
}
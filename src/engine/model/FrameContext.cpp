#include "FrameContext.h"
#include "macros/VkMacro.h"
#include "vulkaninitialize/Device.h"
#include "vulkanInitialize/SwapChain.h"
#include "vulkaninitialize/CommandContext.h"
#include "vulkanInitialize/SyncObject.h"

#include "debug/SuzzaLog.h"

namespace suz
{
	FrameContext::FrameContext(Device* InDevice, SwapChain* InSwapChain, CommandContext* InCommandContext, SyncObject* InSyncObject)
		: m_device(InDevice), m_swapChain(InSwapChain), m_commandContext(InCommandContext), m_syncObject(InSyncObject)
	{
	}

	bool FrameContext::BeginFrame()
	{
		/* 1. 等待上一帧完成接收到围栏(Fence)信号, 并设置超时阙值 */
		vkWaitForFences(m_device->GetDevice(), 1, &m_syncObject->GetInFrameFence()[m_currentFrame], VK_TRUE, UINT64_MAX);

		/* 2. 从交换链获取图像 */
		/* 此处使用图像之后发送图像信号量 */
		VkResult result = vkAcquireNextImageKHR(m_device->GetDevice(), m_swapChain->GetSwapChain(), UINT64_MAX,
			m_syncObject->GetImageAvailableSemaphore()[m_currentFrame],
			VK_NULL_HANDLE,
			&m_imageIndex
		);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)							// 如果交换链过期可重建
		{
			SUZZA_LOG_WARN("交换链已过期, 需重建!");
			m_swapChain->ReCreateSwapChain();
			return false;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			SUZZA_LOG_ERROR("从交换链获取图像失败!");
			return false;
		}

		/* 重置当前帧的围栏信号, 防止围栏重置后提前返回导致没有信号发出, 造成死锁 */
		vkResetFences(m_device->GetDevice(), 1, &m_syncObject->GetInFrameFence()[m_currentFrame]);

		/* 重置命令缓冲区 */
		vkResetCommandBuffer(m_commandContext->GetCommandBuffer()[m_currentFrame], 0);

		return true;
	}

	bool FrameContext::SubmitFrame(VkSemaphore& OutSemaphore)
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		/* 指定需要等待的图像信号量和等待对应的管线阶段 */
		VkSemaphore waitSemapore[] = { m_syncObject->GetImageAvailableSemaphore()[m_currentFrame] };
		VkPipelineStageFlags waitPipelineStage[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemapore;
		submitInfo.pWaitDstStageMask = waitPipelineStage;

		/* 指定实际提交执行的当前帧主命令缓冲区 */
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_commandContext->GetCommandBuffer()[m_currentFrame];

		/* 指定提交队列完成后需要发出的信号量 */
		OutSemaphore = m_syncObject->GetRenderFinishedSemaphore()[m_currentFrame];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &OutSemaphore;

		/* 提交命令缓冲区到图形队列, 完成提交后并发送当前帧的围栏信号 */
		if (vkQueueSubmit(m_device->GetGraphicsQueue(), 1, &submitInfo, m_syncObject->GetInFrameFence()[m_currentFrame]) != VK_SUCCESS)
		{
			SUZZA_LOG_ERROR("提交图形队列失败!");
			OutSemaphore = VK_NULL_HANDLE;
			return false;
		}

		return true;
	}

	bool FrameContext::PresentFrame(const VkSemaphore& WaitSemaphore)
	{
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		/* 等待绘制完成的信号接收到才可以执行 */
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &WaitSemaphore;
		/* 指定将要呈现的交换链, 以及交换链需要呈现的图像索引 */
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_swapChain->GetSwapChain();
		presentInfo.pImageIndices = &m_imageIndex;

		/* 最终: 请求提交图像到呈现队列 */
		VkResult result = vkQueuePresentKHR(m_device->GetPresentQueue(), &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_swapChain->IsFrameBufferResized())
		{
			m_swapChain->DisableBufferResized();
			m_swapChain->ReCreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			SUZZA_LOG_ERROR("呈现交换链图像失败!");
			return false;
		}

		m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

		return true;
	}
}
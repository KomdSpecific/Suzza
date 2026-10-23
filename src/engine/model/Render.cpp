#include "Render.h"
#include "Engine.h"
#include "macros/VkMacro.h"
#include "ModelResourceManager.h"
#include "vulkaninitialize/VulkanResourceManager.h"
#include "vulkanInitialize/Pipeline.h"
#include "vulkanInitialize/VulkanInitialize.h"
#include "vulkanInitialize/SwapChain.h"
#include "vulkanInitialize/RenderPass.h"
#include "vulkanInitialize/BufferArea.h"
#include "vulkaninitialize/Descriptor.h"
#include "vulkanInitialize/SyncObject.h"

#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan.h>
#include <stdexcept>

namespace suz
{
	Render::Render()
		: m_bindlessSlotsAllocator(Engine::GetBindlessSlotsAllocator())
	{
		VulkanResourceManager& vkManager = Engine::GetVulkanResourceManager();
		m_pipeline = vkManager.GetPipeline();
		m_vkInitialize = vkManager.GetVulkanInitialize();
		m_swapChain = vkManager.GetSwapChain();
		m_renderPass = vkManager.GetRenderPass();
		m_bufferArea = vkManager.GetBufferArea();
		m_descriptor = vkManager.GetDescriptor();
		m_syncObject = vkManager.GetSyncObject();

		m_bufferArea->RegisterBindlessDescriptorSet(m_descriptor->GetBindlessDescriptorSets());
	};

	void Render::SubmitMesh(Mesh* TargetMesh, const ModelConstant& ModelMatrix)
	{
		m_meshQueue[TargetMesh].push_back(ModelMatrix);
	}

	void Render::SubmitCameraViewport(const CameraViewportInfo& ViewportInfo)
	{
		m_cameraViewportInfo = ViewportInfo;
	}

	void Render::Draw(float DeltaTime)
	{
		/* 1. 等待上一帧完成接收到围栏(Fence)信号, 并设置超时阙值 */
		vkWaitForFences(m_vkInitialize->GetDevice(), 1, &m_syncObject->GetInFrameFence()[m_currentFrame], VK_TRUE, UINT64_MAX);

		m_bindlessSlotsAllocator.ReleaseSlots();

		/* 2. 从交换链获取图像 */
		uint32 imageIndex;
		/* 此处使用图像之后发送图像信号量 */
		VkResult result = vkAcquireNextImageKHR(m_vkInitialize->GetDevice(), m_swapChain->GetSwapChain(), UINT64_MAX, m_syncObject->GetImageAvailableSemaphore()[m_currentFrame], VK_NULL_HANDLE, &imageIndex);
		/* 如果交换链过期可重建 */
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			m_swapChain->ReCreateSwapChain(m_bufferArea);
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("从交换链获取图像失败!");
		}

		/* 重置围栏信号, 防止围栏重置后提前返回导致没有信号发出, 造成死锁 */
		vkResetFences(m_vkInitialize->GetDevice(), 1, &m_syncObject->GetInFrameFence()[m_currentFrame]);

		/* 3. 记录命令缓冲, 传入图像索引 */
		vkResetCommandBuffer(m_bufferArea->GetCommandBuffer()[m_currentFrame], 0);
		m_bufferArea->RecordCommandBuffer(
			m_bufferArea->GetCommandBuffer()[m_currentFrame],
			m_descriptor->GetCameraDescriptorSetElement(m_currentFrame),
			m_meshQueue,
			imageIndex
		);

		UpdateModelTransform(DeltaTime);

		/* 4. 提交命令缓冲 */
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		/* 指定需要等待的图像信号量和等待对应的管线阶段 */
		VkSemaphore waitSemaphores[]{ m_syncObject->GetImageAvailableSemaphore()[m_currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		/* 指定实际提交执行的命令缓冲区 */
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_bufferArea->GetCommandBuffer()[m_currentFrame];
		/* 指定命令缓冲区完成执行需要发出的信号量 */
		VkSemaphore sendSemaphores[] = { m_syncObject->GetRenderFinishedSemaphore()[m_currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = sendSemaphores;

		/* 5. 提交命令缓冲区到图形队列 */
		if (vkQueueSubmit(m_vkInitialize->GetGraphicsQueue(), 1, &submitInfo, m_syncObject->GetInFrameFence()[m_currentFrame]) != VK_SUCCESS)
		{
			throw std::runtime_error("提交命令缓冲失败!");
		}

		/* 6. 提交结果到交换链以此最终在屏幕上显示 */
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		/* 等待绘制完成的信号接收到才可以执行 */
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = sendSemaphores;
		/* 指定将要呈现的交换链, 以及交换链需要呈现的图像索引 */
		VkSwapchainKHR swapchains[] = { m_swapChain->GetSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &imageIndex;

		/* 最终: 请求提交图像到呈现队列 */
		result = vkQueuePresentKHR(m_vkInitialize->GetPresentQueue(), &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_bufferArea->IsFrameBufferResized())
		{
			m_bufferArea->DisableBufferResized();
			m_swapChain->ReCreateSwapChain(m_bufferArea);
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("呈现交换链图像失败!");
		}

		m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void Render::UpdateModelTransform(float DeltaTime)
	{
		/* 将摄像机数据传入映射内存 */
		memcpy(m_bufferArea->GetUniformBuffersMapped()[m_currentFrame], &m_cameraViewportInfo, sizeof(m_cameraViewportInfo));
	}
}
#include "EditorUI.h"
#include "Engine.h"
#include "imgui/ImGuiUIRender.h"
#include "macros/VkMacro.h"

#ifdef EDITOR

namespace suz
{
	void EditorUI::Init(
		GLFWwindow* window,
		const VkInstance& Instance,
		const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device,
		uint32 GraphicsFamily, const VkQueue& GraphicsQueue,
		const VkRenderPass& RenderPass,
		const VkCommandPool& CommandPool)
	{
		std::unique_ptr<ImGuiUIRender> uiRender = std::make_unique<ImGuiUIRender>();
		uiRender->Init(window, Instance, PhysicalDevice, Device, GraphicsFamily, GraphicsQueue, RenderPass, CommandPool);
		m_uiRender = std::unique_ptr<ImGuiUIRender>(std::move(uiRender));

		/* 执行引擎全局广播委托 */
		EngineUIRenderEventDelegate& engineDelegate = Engine::GetEngineInstance().GetEngineUIRenderEventDelegate();
		engineDelegate.Bind([this](const VkCommandBuffer& CommandBuffer)
			{
				SuzzaApplication* application = Engine::GetEngineInstance().GetApplication();

				this->DrawSurface(application);
				this->RecordDrawCommandBuffer(CommandBuffer);
			});
	}

	void EditorUI::BeginFrame()
	{
		m_uiRender->BeginFrame();
	}

	void EditorUI::DrawSurface(SuzzaApplication* Application)
	{
		m_uiRender->DrawSurfacePanels(Application);
	}

	void EditorUI::RecordDrawCommandBuffer(const VkCommandBuffer& CommandBuffer)
	{
		m_uiRender->RecordDrawCommandBuffer(CommandBuffer);
	}

	void EditorUI::Destroy()
	{
		if (!(*m_isAlive)) return;

		m_uiRender->Destroy();

		*m_isAlive = false;
	}

	VkCommandBuffer EditorUI::BeginSingleTimeCommands(const VkDevice& Device, const VkCommandPool& CommandPool)
	{
		/* 命令缓冲区分配 */
		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandPool = CommandPool;
		allocateInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer{};
		vkAllocateCommandBuffers(Device, nullptr, &commandBuffer);

		/* 定义命令缓冲信息 */
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;					// 该命令缓冲只提交一次

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void EditorUI::EndSingleTimeCommands(const VkDevice& Device, const VkCommandPool& CommandPool, const VkQueue& GraphicsQueue, const VkCommandBuffer& CommandBuffer)
	{
		vkEndCommandBuffer(CommandBuffer);

		/* 执行命令缓冲提交 */
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &CommandBuffer;

		vkQueueSubmit(GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(GraphicsQueue);

		/* 清理临时命令缓冲 */
		vkFreeCommandBuffers(Device, CommandPool, 1, &CommandBuffer);
	}
}
#endif
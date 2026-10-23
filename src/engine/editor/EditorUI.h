#pragma once

#include "BaseObject.h"
#include "SuzzaTypes.h"
#include "macros/SuzzaMacros.h"
#include "UIRender.h"

#include <memory>

#ifdef EDITOR
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace suz
{
	class EditorUI : public BaseObject
	{
		DECLARE_CLASS(EditorUI, BaseObject);
	public:
		void Init(
			GLFWwindow* window,
			const VkInstance& Instance,
			const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device,
			uint32 GraphicsFamily, const VkQueue& GraphicsQueue,
			const VkRenderPass& RenderPass,
			const VkCommandPool& CommandPool);

		void BeginFrame();

		void DrawSurface();

		void RecordDrawCommandBuffer(const VkCommandBuffer& CommandBuffer);

		virtual void Destroy() override;

	private:
		VkCommandBuffer BeginSingleTimeCommands(const VkDevice& Device, const VkCommandPool& CommandPool);
		void EndSingleTimeCommands(const VkDevice& Device, const VkCommandPool& CommandPool, const VkQueue& GraphicsQueue, const VkCommandBuffer& CommandBuffer);

		std::unique_ptr<IUIRender> m_uiRender = nullptr;
	};
}
#endif
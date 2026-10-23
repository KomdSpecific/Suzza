#pragma once

#include "SuzzaTypes.h"
#include "UIRender.h"

#ifdef EDITOR
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

namespace suz
{
	class ImGuiUIRender : public IUIRender
	{
	public:
		virtual bool BeginWindow(const char* Title) override;
		virtual void EndWindow() override;

		virtual void Text(const char* Format, ...) override;
		virtual bool Button(const char* Label) override;

		/* Ö¡ÉúÃüÖÜÆÚ */
		virtual void BeginFrame() override;
		virtual void DrawSurface() override;
		virtual void RecordDrawCommandBuffer(const VkCommandBuffer& CommandBuffer) override;

#pragma region LifeCycle
		void Init(GLFWwindow* window,
			const VkInstance& Instance,
			const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device,
			uint32 GraphicsFamily, const VkQueue& GraphicsQueue,
			const VkRenderPass& RenderPass,
			const VkCommandPool& CommandPool);

		virtual void Destroy() override;

#pragma endregion
	};
}
#endif

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

		virtual bool TreeNodeEx(const char* Label, uint32 flags) override;
		virtual void TreePop() override;

		virtual bool IsItemLeftClicked() override;
		virtual bool IsItemRightClicked() override;

		virtual bool BeginPopupContextItem() override;
		virtual void EndPopup() override;
		virtual bool MenuItem(const char* Label) override;

		virtual void Text(const char* Format, ...) override;
		virtual void SeparatorText(const char* Label) override;
		virtual bool Button(const char* Label) override;
		virtual bool ShowFloat(const char* Label, float* Value, const float Speed = 1.0f) override;
		virtual bool ShowVector2Float(const char* Label, const glm::vec2& Vector, const float Speed = 1.0f) override;
		virtual bool ShowVector3Float(const char* Label, const glm::vec3& Vector, const float Speed = 1.0f) override;
		virtual bool ShowVector3Float(const char* Label, float* Vector, const float Speed = 1.0f) override;
		/* Ö¡ÉúÃüÖÜÆÚ */
		virtual void BeginFrame() override;
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

#pragma once

#include "BaseObject.h"
#include "SuzzaTypes.h"
#include "SuzzaApplication.h"
#include "UIRender.h"

#include <memory>

#ifdef EDITOR
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace suz
{
	struct FieldInfo;
	
	class EditorUI : public BaseObject
	{
	public:
		void Init(
			GLFWwindow* window,
			const VkInstance& Instance,
			const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device,
			uint32 GraphicsFamily, const VkQueue& GraphicsQueue,
			const VkRenderPass& RenderPass,
			const VkCommandPool& CommandPool);

		void BeginFrame();

		void DrawSurface(SuzzaApplication* Application);

		void RecordDrawCommandBuffer(const VkCommandBuffer& CommandBuffer);

		virtual void Destroy() override;

	private:
		void DrawHierarchyPanel(SuzzaApplication* Application);
		void DrawInspectorPanel();

		void DrawActorTreeNode(Actor* TargetActor);

		void DrawObjField(void* TargetObj, const FieldInfo& TargetField);
		void DrawPrimitiveField(void* TargetObj, const FieldInfo& TargetField);
		void DrawContainerField(void* TargetObj, const FieldInfo& TargetField);

		std::unique_ptr<IUIRender> m_uiRender = nullptr;
		Actor* m_selectedActor = nullptr;
	};
}
#endif
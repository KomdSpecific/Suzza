#pragma once

#include "BaseObject.h"

#ifdef EDITOR
#include <vulkan/vulkan.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace suz
{
	enum UITreeNodeFlags
	{
		UITreeNodeFlags_Selected = 1 << 0,
		UITreeNodeFlags_Framed = 1 << 1,
		UITreeNodeFlags_OpenOnArrow = 1 << 7,
		UITreeNodeFlags_Leaf = 1 << 8,
		UITreeNodeFlags_SpanAvailWidth = 1 << 11
	};

	class IUIRender : public BaseObject
	{
	public:
		~IUIRender() = default;

		virtual bool BeginWindow(const char* Title) = 0;
		virtual void EndWindow() = 0;

		virtual bool TreeNodeEx(const char* Label, uint32 flags) = 0;
		virtual void TreePop() = 0;

		virtual bool IsItemLeftClicked() = 0;
		virtual bool IsItemRightClicked() = 0;

		virtual bool BeginPopupContextItem() = 0;
		virtual void EndPopup() = 0;
		virtual bool MenuItem(const char* Label) = 0;

		/* 基础控件 */
		virtual void Text(const char* Format, ...) = 0;
		virtual void SeparatorText(const char* Label) = 0;
		virtual bool Button(const char* Label) = 0;
		virtual bool ShowFloat(const char* Label, float* Value, const float Speed = 1.0f) = 0;
		virtual bool ShowVector2Float(const char* Label, const glm::vec2& Vector, const float Speed = 1.0f) = 0;
		virtual bool ShowVector3Float(const char* Label, const glm::vec3& Vector, const float Speed = 1.0f) = 0;
		virtual bool ShowVector2Float(const char* Label, float* Vector, const float Speed = 1.0f) = 0;
		virtual bool ShowVector3Float(const char* Label, float* Vector, const float Speed = 1.0f) = 0;

		/* 帧生命周期 */
		virtual void BeginFrame() = 0;
		virtual void RecordDrawCommandBuffer(const VkCommandBuffer& CommandBuffer) = 0;
	};
}
#endif

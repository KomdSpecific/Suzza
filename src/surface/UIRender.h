#pragma once

#include "BaseObject.h"

#ifdef EDITOR
#include <vulkan/vulkan.h>

namespace suz
{
	class IUIRender : public BaseObject
	{
	public:
		~IUIRender() = default;

		virtual bool BeginWindow(const char* Title) = 0;
		virtual void EndWindow() = 0;

		/* 基础控件 */
		virtual void Text(const char* Format, ...) = 0;
		virtual bool Button(const char* Label) = 0;

		/* 帧生命周期 */
		virtual void BeginFrame() = 0;
		virtual void DrawSurface() = 0;
		virtual void RecordDrawCommandBuffer(const VkCommandBuffer& CommandBuffer) = 0;
	};
}
#endif

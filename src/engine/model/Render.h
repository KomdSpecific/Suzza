#pragma once

#include "DrawList.h"

namespace suz
{
	class Mesh;
	class Device;
	class Pipeline;
	class VulkanInitialize;
	class SwapChain;
	class RenderPass;
	class BufferArea;
	class CommandContext;
	class BufferContext;
	class Descriptor;
	class SyncObject;

	class BindlessSlotsAllocator;

	class Render
	{
	public:
		Render();

		void SubmitMesh(Mesh* TargetMesh, const ModelConstant& Constant);
		void SubmitCameraViewport(const CameraViewportInfo& ViewportInfo);

		void Draw(float DeltaTime);

		void UpdateModelTransform(float DeltaTime);

	private:
		Device* m_device;
		//VulkanInitialize* m_vkInitialize;
		SwapChain* m_swapChain;
		RenderPass* m_renderPass;
		BufferArea* m_bufferArea;
		CommandContext* m_commandContext;
		BufferContext* m_bufferContext;
		Descriptor* m_descriptor;
		SyncObject* m_syncObject;

		BindlessSlotsAllocator& m_bindlessSlotsAllocator;

		Pipeline* m_pipeline;
		DrawList m_drawList = {};
		CameraViewportInfo m_cameraViewportInfo = {};

		Pipeline* m_gridPipeline;
		glm::mat4 m_gridViewProjectionMatrix = glm::mat4(1.0f);

		uint32 m_currentFrame = 0;
	};
}
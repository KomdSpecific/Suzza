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
		Pipeline* m_pipeline;
		VulkanInitialize* m_vkInitialize;
		SwapChain* m_swapChain;
		RenderPass* m_renderPass;
		BufferArea* m_bufferArea;
		Descriptor* m_descriptor;
		SyncObject* m_syncObject;

		BindlessSlotsAllocator& m_bindlessSlotsAllocator;

		DrawList m_drawList = {};
		CameraViewportInfo m_cameraViewportInfo = {};

		uint32 m_currentFrame = 0;
	};
}
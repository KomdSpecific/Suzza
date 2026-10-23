#pragma once

#include "components/CameraComponent.h"
#include "model/Mesh.h"
#include "SuzzaTypes.h"

namespace suz
{
	class Pipeline;
	class VulkanInitialize;
	class SwapChain;
	class RenderPass;
	class BufferArea;
	class Descriptor;
	class SyncObject;

	class BindlessSlotsAllocator;

	struct ModelConstant
	{
		glm::mat4 modelMatrix;
		uint16 slotIndex;
	};

	class Render
	{
	public:
		Render();

		void SubmitMesh(Mesh* TargetMesh, const ModelConstant& ModelMatrix);
		void SubmitCameraViewport(const CameraViewportInfo& ViewportInfo);

		void Draw(float DeltaTime);

		void UpdateModelTransform(float DeltaTime);

	private:
		Pipeline* m_pipeline;
		VulkanInitialize* m_vkInitialize;
		SwapChain* m_swapChain;
		RenderPass* m_renderPass;
		BufferArea* m_bufferArea;
		Descriptor* m_descriptor;
		SyncObject* m_syncObject;

		BindlessSlotsAllocator& m_bindlessSlotsAllocator;

		std::unordered_map<Mesh*, std::vector<ModelConstant>> m_meshQueue = {};
		CameraViewportInfo m_cameraViewportInfo = {};

		uint32 m_currentFrame = 0;
	};
}
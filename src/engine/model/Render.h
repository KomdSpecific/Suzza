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

	class Render
	{
	public:
		Render();

		void SubmitMesh(Mesh* TargetMesh, const glm::mat4& ModelMatrix);
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

		std::unordered_map<Mesh*, std::vector<glm::mat4>> m_meshQueue = {};
		CameraViewportInfo m_cameraViewportInfo = {};

		uint32 m_currentFrame = 0;
	};
}
#pragma once

#include "DrawList.h"

namespace suz
{
	class Mesh;
	class Pipeline;
	class CommandContext;
	class BufferContext;
	class Descriptor;

	class BindlessSlotsAllocator;

	class FrameContext;
	class ScenePass;

	class Render
	{
	public:
		Render();
		~Render();

		void SubmitMesh(Mesh* TargetMesh, const ModelConstant& Constant);
		void SubmitCameraViewport(const CameraViewportInfo& ViewportInfo);

		void Draw(float DeltaTime);

		void UpdateModelTransform(float DeltaTime);

	private:
		CommandContext* m_commandContext;
		BufferContext* m_bufferContext;
		Descriptor* m_descriptor;

		BindlessSlotsAllocator& m_bindlessSlotsAllocator;

		Pipeline* m_pipeline;
		DrawList m_drawList = {};
		CameraViewportInfo m_cameraViewportInfo = {};

		Pipeline* m_gridPipeline;
		glm::mat4 m_gridViewProjectionMatrix = glm::mat4(1.0f);

		UniquePtr<FrameContext> m_frameContext = nullptr;
		UniquePtr<ScenePass> m_scenePass = nullptr;

	public:
		ScenePass* GetScenePass();
	};
}
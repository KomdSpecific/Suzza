#include "Render.h"
#include "FrameContext.h"
#include "ScenePass.h"
#include "Engine.h"
#include "ModelResourceManager.h"
#include "vulkaninitialize/VulkanResourceManager.h"

#include "debug/SuzzaLog.h"

#include <glm/gtc/matrix_transform.hpp>

namespace suz
{
	Render::Render()
		: m_bindlessSlotsAllocator(Engine::GetBindlessSlotsAllocator())
	{
		VulkanResourceManager& vkManager = Engine::GetVulkanResourceManager();
		Device* device = vkManager.GetDevice();
		SwapChain* swapChain = vkManager.GetSwapChain();
		RenderPass* renderPass = vkManager.GetRenderPasses().GetResource("DefaultRenderPass");
		SyncObject* syncObject = vkManager.GetSyncObject();

		m_commandContext = vkManager.GetCommandContext();
		m_bufferContext = vkManager.GetBufferContext();
		m_descriptor = vkManager.GetDescriptor();

		m_pipeline = vkManager.GetPipelines().GetResource("DefaultPipeline");
		m_gridPipeline = vkManager.GetPipelines().GetResource("GridPipeline");

		m_frameContext = std::make_unique<FrameContext>(device, swapChain, m_commandContext, syncObject);
		m_scenePass = std::make_unique<ScenePass>(swapChain, renderPass);
	}

	Render::~Render()
	{
	}

	void Render::SubmitMesh(Mesh* TargetMesh, const ModelConstant& Constant)
	{
		if (!TargetMesh) return;

		m_drawList.SubmitMesh(TargetMesh, Constant);
	}

	void Render::SubmitCameraViewport(const CameraViewportInfo& ViewportInfo)
	{
		m_cameraViewportInfo = ViewportInfo;
	}

	void Render::Draw(float DeltaTime)
	{
		struct DrawListCleanerGuard
		{
			~DrawListCleanerGuard() { drawList.Clear(); }

			DrawList& drawList;
		} drawListCleaner{ m_drawList };

		/* 1. 开启新的一帧 */
		if (!m_frameContext->BeginFrame())
		{
			SUZZA_LOG_ERROR("本帧建立失败, 提交已作废!");
			return;
		}
		/* 2. 释放上一帧的纹理插槽 */
		m_bindlessSlotsAllocator.ReleaseSlots();
		/* 拷贝摄像机矩阵数据到uniform缓冲区映射内存 */
		UpdateModelTransform(DeltaTime);

		/* 3. 组装 stage */
		Vector<DrawStage> stage(2);
		stage[0] = { m_pipeline,&m_drawList,nullptr,0 };

		m_gridViewProjectionMatrix = glm::inverse(m_cameraViewportInfo.projectionMatrix * m_cameraViewportInfo.viewMatrix);
		stage[1] = { m_gridPipeline,nullptr,&m_gridViewProjectionMatrix,sizeof(glm::mat4) };

		/* 4. 记录命令缓冲, 传入图像索引 */
		m_scenePass->RecordCommandBuffer(
			m_commandContext->GetCommandBuffer()[m_frameContext->GetFrameIndex()],
			{ m_descriptor->GetCameraDescriptorSetElement(m_frameContext->GetFrameIndex()),m_descriptor->GetBindlessDescriptorSets() },
			stage,
			m_frameContext->GetImageIndex()
		);

		/* 5. 提交命令缓冲 */
		VkSemaphore submitSemaphore;
		if (!m_frameContext->SubmitFrame(submitSemaphore))
		{
			SUZZA_LOG_ERROR("当前帧提交图形队列失败, 当前帧作废!");
			return;
		}

		/* 6. 提交交换链图像到呈现队列, 最终在屏幕上显示 */
		if (!m_frameContext->PresentFrame(submitSemaphore))
		{
			SUZZA_LOG_ERROR("当前帧提交呈现队列失败, 当前帧作废!");
		}
	}

	void Render::UpdateModelTransform(float DeltaTime)
	{
		/* 将摄像机数据传入映射内存 */
		memcpy(m_bufferContext->GetUniformBuffersMapped()[m_frameContext->GetFrameIndex()], &m_cameraViewportInfo, sizeof(m_cameraViewportInfo));
	}

	ScenePass* Render::GetScenePass()
	{
		return m_scenePass.get();
	}
}
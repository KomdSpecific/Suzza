#include "CameraComponent.h"
#include "Engine.h"
#include "vulkaninitialize/VulkanResourceManager.h"
#include "Actor.h"
#include "math/Math.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>

namespace suz
{
	CameraComponent::CameraComponent()
	{
	}

	void CameraComponent::UpdateWindowAspect(int Width, int Height)
	{
		m_aspect = static_cast<float>(Width) / static_cast<float>(Height);
	}

	void CameraComponent::BeginPlay()
	{
		Engine& engInstance = Engine::GetEngineInstance();
		m_aspect = static_cast<float>(engInstance.WIDTH) / static_cast<float>(engInstance.HEIGHT);

		/* 绑定委托 */
		engInstance.GetMainWindow()->GetWindowSizeChangedDelegate().Bind<ThisClass>(this, &ThisClass::UpdateWindowAspect);
	}

	void CameraComponent::Update(float DeltaTime)
	{
		CameraViewportInfo viewportInfo{};
		viewportInfo.viewMatrix = GetViewMatrix();
		viewportInfo.projectionMatrix = GetProjectionMatrix(m_aspect);

		viewportInfo.projectionMatrix[1][1] *= -1;

		Engine::GetEngineInstance().GetRender()->SubmitCameraViewport(viewportInfo);
	}

	void CameraComponent::Destroy()
	{
	}

	glm::mat4 CameraComponent::GetViewMatrix() const
	{
		const glm::vec3& worldLocation = m_actorOwner->GetActorWorldLocation();
		const glm::vec3& forward = m_actorOwner->GetActorForwardVector();		// 偏航角 左右摇头 水平旋转
		const glm::vec3& up = m_actorOwner->GetActorUpVector();

		/* 返回: 世界坐标看向前方单位向量的目标 */
		return Math::GameSceneLookAt(worldLocation, worldLocation + forward, up);
	}

	glm::mat4 CameraComponent::GetProjectionMatrix(float Aspect) const
	{
		return glm::perspective(glm::radians(m_fovRad), Aspect, m_nearClip, m_farClip);
	}
}
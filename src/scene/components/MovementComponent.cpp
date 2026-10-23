#include "MovementComponent.h"
#include "Actor.h"
#include "Engine.h"

#include <iostream>

namespace suz
{
	MovementComponent::MovementComponent()
		: Super(), m_inputManager(Engine::GetEngineInstance().GetMainWindow()->GetInputManager())
	{
	}

	MovementComponent::MovementComponent(const std::string& ComponentName)
		: Super(ComponentName), m_inputManager(Engine::GetEngineInstance().GetMainWindow()->GetInputManager())
	{
	}

	void MovementComponent::BeginPlay()
	{
	}

	void MovementComponent::Update(float DeltaTime)
	{
		if (m_inputManager.IsMouseKeyPressed(GLFW_MOUSE_BUTTON_LEFT))
		{
			glm::quat actorRot = m_actorOwner->GetActorQuatRotation();

			const glm::vec2 cursorOldLoc = m_inputManager.GetMouseOldLocation();
			const glm::vec2 cursorCurrentLoc = m_inputManager.GetMouseCurrentLocation();

			float deltaX = cursorCurrentLoc.x - cursorOldLoc.x;
			float deltaY = cursorCurrentLoc.y - cursorOldLoc.y;

			glm::vec3 rotationDeg = m_actorOwner->GetActorRotation();
			/* 缓存欧拉角叠加鼠标增量 */
			/* 鼠标y增量为负数, 需要取反再相加 */
			rotationDeg.z += deltaX * m_sensitivity;
			rotationDeg.y -= deltaY * m_sensitivity;

			/* 限制俯仰, 防止过渡旋转 */
			const float maxPitch = 89.0f;
			rotationDeg.y = glm::clamp(rotationDeg.y, -maxPitch, maxPitch);

			/* Yaw角度归一化, 防止数值无线累积 */
			rotationDeg.z = NormalizeAngle(rotationDeg.z);
			/* 设置欧拉角旋转 */
			m_actorOwner->SetActorRotation(rotationDeg);
		}

		/* 得到相机的前向向量 */
		glm::vec3 forward = m_actorOwner->GetActorForwardVector();
		glm::vec3 right = m_actorOwner->GetActorRightVector();

		/* 计算移动变化量 */
		glm::vec3 deltaMove = glm::vec3(0.0f);

		if (m_inputManager.IsKeywordKeyPressed(GLFW_KEY_A))
		{
			deltaMove -= right * m_moveSpeed * DeltaTime;
		}
		else if (m_inputManager.IsKeywordKeyPressed(GLFW_KEY_D))
		{
			deltaMove += right * m_moveSpeed * DeltaTime;
		}

		if (m_inputManager.IsKeywordKeyPressed(GLFW_KEY_W))
		{
			deltaMove += forward * m_moveSpeed * DeltaTime;
		}
		else if (m_inputManager.IsKeywordKeyPressed(GLFW_KEY_S))
		{
			deltaMove -= forward * m_moveSpeed * DeltaTime;
		}

		glm::vec3 location = m_actorOwner->GetActorLocation();
		glm::vec3 newLocation = deltaMove + location;
		m_actorOwner->SetActorLocation(newLocation);
	}

	void MovementComponent::Destroy()
	{
	}

	float MovementComponent::NormalizeAngle(float Angle)
	{
		Angle = fmodf(Angle, 360.0f);
		if (Angle > 180.0f)
		{
			Angle -= 360.0f;
		}
		else if (Angle < -180.0f)
		{
			Angle += 360.0f;
		}

		return Angle;
	}
}
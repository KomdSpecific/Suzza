#pragma once

#include "Component.h"
#include "input/InputManager.h"
#include "macros/GameSceneMacro.h"

namespace suz
{
	class MovementComponent : public Component
	{
		DECLARE_CLASS(MovementComponent, Component)
		COMPONENT(MovementComponent)
	public:
		MovementComponent();

		virtual void BeginPlay() override;
		virtual void Update(float DeltaTime) override;
		virtual void Destroy() override;

	private:
		float NormalizeAngle(float Angle);

		suz::InputManager& m_inputManager;

		float m_moveSpeed = 2.0f;						// 移动速度
		float m_sensitivity = 0.1f;						// 鼠标旋转灵敏度
	};
}

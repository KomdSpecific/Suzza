#pragma once

#include "Component.h"
#include "input/InputManager.h"
#include "macros/ReflectMacros.h"
#include "MovementComponent.generated.h"

namespace suz
{
	class MovementComponent : public Component
	{
		SUZZA_REFLECT_CLASS();
		COMPONENT(MovementComponent)
	public:
		MovementComponent();
		MovementComponent(const std::string& ComponentName);

		virtual void BeginPlay() override;
		virtual void Update(float DeltaTime) override;
		virtual void Destroy() override;

	private:
		float NormalizeAngle(float Angle);

		suz::InputManager& m_inputManager;

		SUZZA_PROPERTY(Editability::Editable)
		float m_moveSpeed = 2.0f;						// 移动速度
		SUZZA_PROPERTY(Editability::Editable)
		float m_sensitivity = 0.1f;						// 鼠标旋转灵敏度
	};
}

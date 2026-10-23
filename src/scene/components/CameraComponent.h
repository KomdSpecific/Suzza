#pragma once

#include "Component.h"
#include "macros/ReflectMacros.h"
#include "CameraComponent.generated.h"

#include <glm/glm.hpp>

namespace suz
{
	struct CameraViewportInfo
	{
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
	};

	class CameraComponent : public Component
	{
		SUZZA_REFLECT_CLASS();
		COMPONENT(CameraComponent)
	public:
		CameraComponent();
		CameraComponent(const std::string& ComponentName);

		void UpdateWindowAspect(int Width, int Height);

		virtual void BeginPlay() override;
		virtual void Update(float DeltaTime) override;
		virtual void Destroy() override;

		glm::mat4 GetViewMatrix() const;
		glm::mat4 GetProjectionMatrix(float Aspect) const;	// Aspect为宽高比

	private:
		SUZZA_PROPERTY(Editability::Editable)
		float m_fovRad = 70.0f;								// 视场角 物体放大
		float m_nearClip = 0.1f;
		float m_farClip = 1000.0f;

		float m_aspect = 0.0f;
	};
}

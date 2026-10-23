#pragma once

#include "Component.h"

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <type_traits>

namespace suz
{
	class Scene;
	class Mesh;

	class Actor
	{
	public:
		Actor();
		Actor(Scene* OwnerScene);

		virtual void BeginPlay();
		virtual void Update(float DeltaTime);
		virtual void Destroy();

		void RegisterOwnerScene(Scene* OwnerScene);
		void BatchRegisterAllSubComponents();

		/* 注册Actor组件 */
		template<typename Comp>
		bool RegisterComponent(Comp* TargetComponent);
		/* 获取Actor组件 */
		template<typename Comp>
		Comp* GetActorComponent();

#pragma region Transform
		void SetActorLocation(const glm::vec3& NewLocation);
		const glm::vec3& GetActorLocation() const;
		const glm::vec3& GetActorWorldLocation() const;

		void SetActorRotation(const glm::vec3& NewRotation);
		const glm::vec3 GetActorRotation() const;
		void SetActorQuatRotation(const glm::quat& NewRotation);
		const glm::quat& GetActorQuatRotation() const;

		void SetActorScale(const glm::vec3& NewScale);
		const glm::vec3& GetActorScale() const;

		glm::vec3 GetActorForwardVector() const;
		glm::vec3 GetActorRightVector() const;
		glm::vec3 GetActorUpVector() const;

		const glm::mat4 GetActorLocalTransform() const;
#pragma endregion

		const glm::mat4 GetActorModelMatrix() const;

	protected:
		Scene* m_scene = nullptr;

	private:
		std::vector<Component*> m_components = {};

		glm::vec3 m_location = glm::vec3(0.0f);
		glm::quat m_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 m_scale = glm::vec3(1.0f);

		float m_yaw = -90.0f;								// 偏航角 左右摇头 水平旋转
		float m_pitch = 0.0f;								// 俯仰角 抬头低头
	};

	template<typename Comp>
	inline bool Actor::RegisterComponent(Comp* TargetComponent)
	{
		if constexpr (std::is_base_of_v<Component, Comp>)
		{
			m_components.push_back(static_cast<Component*>(TargetComponent));

			return true;
		}

		return false;
	}

	template<typename Comp>
	inline Comp* Actor::GetActorComponent()
	{
		if constexpr (!std::is_base_of_v<Component, Comp>) return nullptr;

		/* 获取组件对应的ID */
		size_t typeId = GetStaticTypeId<Comp>();

		for (Component* comp : m_components)
		{
			if (comp->GetTypeId() == typeId)
			{
				return comp;
			}
		}

		return nullptr;
	}
}

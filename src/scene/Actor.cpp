#include "Actor.h"
#include "Scene.h"

namespace suz
{
	Actor::Actor()
	{
	}

	Actor::Actor(Scene* OwnerScene)
		: m_scene(OwnerScene)
	{
	}

	void Actor::BeginPlay()
	{
		for (Component* comp : m_components)
		{
			comp->BeginPlay();
		}
	}

	void Actor::Update(float DeltaTime)
	{
		for (Component* comp : m_components)
		{
			comp->Update(DeltaTime);
		}
	}

	void Actor::Destroy()
	{
		for (Component* comp : m_components)
		{
			comp->Destroy();

			/* 内存回收 */
			delete comp;
		}

		m_components.clear();
	}

	void Actor::RegisterOwnerScene(Scene* OwnerScene)
	{
		m_scene = OwnerScene;
	}

	void Actor::BatchRegisterAllSubComponents()
	{
		for (Component* comp : m_components)
		{
			comp->RegisterOwnerActor(this);
		}
	}

	void Actor::SetActorLocation(const glm::vec3& NewLocation)
	{
		m_location = NewLocation;
	}

	const glm::vec3& Actor::GetActorLocation() const
	{
		return m_location;
	}

	const glm::vec3& Actor::GetActorWorldLocation() const
	{
		return m_location;
	}

	void Actor::SetActorRotation(const glm::vec3& NewRotation)
	{
		float roll = glm::radians(NewRotation.x);
		float pitch = glm::radians(NewRotation.y);
		float yaw = glm::radians(NewRotation.z);

		/* 使用游戏坐标轴构造四元数 */
		glm::quat qRoll = glm::angleAxis(-roll, glm::vec3(1, 0, 0));
		glm::quat qPitch = glm::angleAxis(-pitch, glm::vec3(0, 1, 0));
		glm::quat qYaw = glm::angleAxis(yaw, glm::vec3(0, 0, 1));

		m_rotation = glm::normalize(qYaw * qPitch * qRoll);
	}

	const glm::vec3 Actor::GetActorRotation() const
	{
		glm::vec3 euler = glm::vec3(0.0f);

		const glm::quat& quat = m_rotation;

		/* 从游戏坐标四元数反解欧拉角 */
		float sinPitch = 2 * (quat.w * quat.y - quat.z * quat.x);
		if (fabs(sinPitch) >= 1)
		{
			euler.y = -copysign(glm::pi<float>() / 2, sinPitch);
		}
		else
		{
			euler.y = glm::degrees(-asin(sinPitch));
		}

		float sinYaw = 2 * (quat.w * quat.z + quat.x * quat.y);
		float cosYaw = 1 - 2 * (quat.y * quat.y + quat.z * quat.z);
		euler.z = glm::degrees(atan2(sinYaw, cosYaw));

		float sinRoll = 2 * (quat.w * quat.x + quat.y * quat.z);
		float cosRoll = 1 - 2 * (quat.x * quat.x + quat.y * quat.y);
		euler.x = glm::degrees(-atan2(sinRoll, cosRoll));

		return euler;
	}

	void Actor::SetActorQuatRotation(const glm::quat& NewRotation)
	{
		m_rotation = glm::normalize(NewRotation);
	}

	const glm::quat& Actor::GetActorQuatRotation() const
	{
		return m_rotation;
	}

	void Actor::SetActorScale(const glm::vec3& NewScale)
	{
		m_scale = NewScale;
	}

	const glm::vec3& Actor::GetActorScale() const
	{
		return m_scale;
	}

	glm::vec3 Actor::GetActorForwardVector() const
	{
		const glm::quat& actorRot = GetActorQuatRotation();
		return glm::normalize(actorRot * glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::vec3 Actor::GetActorRightVector() const
	{
		const glm::quat& actorRot = GetActorQuatRotation();
		return glm::normalize(actorRot * glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 Actor::GetActorUpVector() const
	{
		const glm::quat& actorRot = GetActorQuatRotation();
		return glm::normalize(actorRot * glm::vec3(0.0f, 0.0f, 1.0f));
	}

	const glm::mat4 Actor::GetActorLocalTransform() const
	{
		glm::mat4 matTranslate = glm::translate(glm::mat4(1.0f), m_location);
		glm::mat4 matRotate = glm::mat4(m_rotation);
		glm::mat4 matScale = glm::scale(glm::mat4(1.0f), m_scale);

		/* 计算顺序: 平移 * 旋转 * 缩放 */
		return matTranslate * matRotate * matScale;
	}

	const glm::mat4 Actor::GetActorModelMatrix() const
	{
		return GetActorLocalTransform();
	}
}
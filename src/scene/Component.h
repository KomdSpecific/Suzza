#pragma once

#include "SuzzaTypes.h"

#include <memory>
#include <string>

namespace suz
{
	class Actor;
	struct ClassInfo;

	class Component
	{
	public:
		Component();
		Component(const std::string& ComponentName);
		~Component();
		virtual uint16 GetComponentId() const = 0;

		virtual void BeginPlay() = 0;
		virtual void Update(float DeltaTime) = 0;
		virtual void Destroy() = 0;

		virtual const ClassInfo& GetClassInfo() const = 0;

		inline std::shared_ptr<bool> GetValidPtr() { return m_isAlive; }

		virtual void RegisterOwnerActor(Actor* OwnerActor);

		template<typename T>
		static uint16 GetStaticComponentId()
		{
			static const uint16 componentId = nextId++;
			return componentId;
		}

	protected:
		Actor* m_actorOwner = nullptr;

	private:
		static inline uint16 nextId = 0;
		std::shared_ptr<bool> m_isAlive;

		std::string m_componentName = {};

	public:
		inline Actor* GetOwnerActor() { return m_actorOwner; }
	};
}

#define COMPONENT(Comp) \
public: \
	static suz::uint16 GetComponentClassId() { return GetStaticComponentId<Comp>(); } \
	virtual suz::uint16 GetComponentId() const override { return GetStaticComponentId<Comp>(); }

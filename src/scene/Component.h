#pragma once

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
		virtual size_t GetTypeId() const = 0;

		virtual void BeginPlay() = 0;
		virtual void Update(float DeltaTime) = 0;
		virtual void Destroy() = 0;

		virtual const ClassInfo& GetClassInfo() const = 0;

		inline std::shared_ptr<bool> GetValidPtr() { return m_isAlive; }

		virtual void RegisterOwnerActor(Actor* OwnerActor);

		template<typename T>
		static size_t GetStaticTypeId()
		{
			static size_t typeId = nextId++;
			return typeId;
		}

	protected:
		Actor* m_actorOwner = nullptr;

	private:
		static size_t nextId;
		std::shared_ptr<bool> m_isAlive;

		std::string m_componentName = {};

	public:
		inline Actor* GetOwnerActor() { return m_actorOwner; }
	};
}

#define COMPONENT(Comp) \
public: \
	static size_t GetComponentTypeId() { return GetStaticTypeId<Comp>(); } \
	virtual size_t GetTypeId() const override { return GetComponentTypeId(); }

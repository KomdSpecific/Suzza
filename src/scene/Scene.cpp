#include "Scene.h"

namespace suz
{
	void Scene::RegisterActor(Actor* TargetActor)
	{
		if (!TargetActor) return;

		/* 防止重复注册 */
		for (std::unique_ptr<Actor>& actor : m_rootActorList)
		{
			if (actor.get() == TargetActor) return;
		}

		m_rootActorList.push_back(std::unique_ptr<Actor>(TargetActor));
	}

	void Scene::MarkedActorDestroy(Actor* TargetActor)
	{
		/* 非立即删除, 而是标记为待删除列表 */
		if (!TargetActor) return;
		m_actorForDestroy.push_back(TargetActor);
	}

	void Scene::BeginPlay()
	{
		for (std::unique_ptr<Actor>& actor : m_rootActorList)
		{
			actor->BatchRegisterAllSubComponents();
			actor->BeginPlay();
		}
	}

	void Scene::Update(float DeltaTime)
	{
		for (std::unique_ptr<Actor>& actor : m_rootActorList)
		{
			actor->Update(DeltaTime);
		}

		for (Actor* actorToDestroy : m_actorForDestroy)
		{
			actorToDestroy->Destroy();

			for (auto it = m_rootActorList.begin(); it != m_rootActorList.end();)
			{
				if (it->get() == actorToDestroy)
				{
					/* unique_ptr自动析构对象 */
					it = m_rootActorList.erase(it);
					break;
				}
				else
				{
					it++;
				}
			}
		}

		m_actorForDestroy.clear();
	}

	void Scene::Destroy()
	{
		for (std::unique_ptr<Actor>& actor : m_rootActorList)
		{
			actor->Destroy();
		}

		m_actorForDestroy.clear();
		m_rootActorList.clear();
	}
}
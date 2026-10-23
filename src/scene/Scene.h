#pragma once

#include "Actor.h"

#include <vector>
#include <memory>

namespace suz
{
	class Scene
	{
	public:
		void RegisterActor(Actor* TargetActor);
		void MarkedActorDestroy(Actor* TargetActor);

		void BeginPlay();
		void Update(float DeltaTime);
		void Destroy();

	private:
		std::vector<std::unique_ptr<Actor>> m_rootActorList = {};	// 存放场景所有根节点对象
		std::vector<Actor*> m_actorForDestroy = {};					// 指针所有权仍然在ActorList中
	};
}
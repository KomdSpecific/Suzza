#pragma once

#include "Actor.h"

#include <vector>
#include <memory>
#include <string>

namespace suz
{
	class Scene
	{
	public:
		bool RegisterActor(Actor* TargetActor);
		bool RegisterActor(Actor* TargetActor, Actor* ParentActor, bool bRegisterComponents = false);
		void MarkedActorDestroy(Actor* TargetActor);

		void LoadModel(const std::string& FilePath);

		void BeginPlay();
		void Update(float DeltaTime);
		void Destroy();

	private:
		bool CheckWouldCreateCycle(Actor* TargetActor, Actor* ParentActor);

		std::vector<std::unique_ptr<Actor>> m_actorList = {};		// 存放场景所有Actor对象
		std::vector<Actor*> m_rootActorList = {};					// 存放场景所有根Actor对象

	public:
		inline const std::vector<Actor*>& GetSceneRootActorList() const { return m_rootActorList; }
	};
}
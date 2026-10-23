#pragma once

#include "Actor.h"
#include "macros/GameSceneMacro.h"

namespace suz
{
	class PlayerController : public Actor
	{
		DECLARE_CLASS(PlayerController, Actor)
		ACTOR_REGISTER_SCENE_CONSTRUCT_METHOD(PlayerController)
	public:
		PlayerController();

		virtual void BeginPlay() override;
		virtual void Update(float DeltaTime) override;
		virtual void Destroy() override;

	private:

	};
}

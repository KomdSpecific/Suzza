#pragma once

#include "Actor.h"
#include "macros/ReflectMacros.h"
#include "PlayerController.generated.h"

namespace suz
{
	class PlayerController : public Actor
	{
		SUZZA_REFLECT_CLASS();
	public:
		PlayerController();
		PlayerController(Scene* OwnerScene);

		virtual void BeginPlay() override;
		virtual void Update(float DeltaTime) override;
		virtual void Destroy() override;

	private:

	};
}

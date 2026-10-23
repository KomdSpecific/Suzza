#include "PlayerController.h"
#include "Scene.h"
#include "components/MovementComponent.h"

#include <iostream>

namespace suz
{
	PlayerController::PlayerController()
	{
		MovementComponent* movementComponent = new MovementComponent();

		if (!RegisterComponent<MovementComponent>(movementComponent))
		{
			std::cout << "retgister movement falied!" << std::endl;
		}
	}

	PlayerController::PlayerController(Scene* OwnerScene)
	{
		CALL_REGISTER_OWNERSCENE_METHOD;

		MovementComponent* movementComponent = new MovementComponent();

		if (!RegisterComponent<MovementComponent>(movementComponent))
		{
			std::cout << "retgister movement failed!" << std::endl;
		}
	}

	void PlayerController::BeginPlay()
	{
		Super::BeginPlay();
	}

	void PlayerController::Update(float DeltaTime)
	{
		Super::Update(DeltaTime);
	}

	void PlayerController::Destroy()
	{
		Super::Destroy();
	}
}
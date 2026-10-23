#pragma once

#include "SuzzaMacros.h"

#define ACTOR_REGISTER_SCENE_CONSTRUCT_METHOD(ACTOR_NAME) \
public: \
	ACTOR_NAME(Scene* OwnerScene);

#define CALL_REGISTER_OWNERSCENE_METHOD \
	SetOwnerScene(OwnerScene);
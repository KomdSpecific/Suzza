#pragma once

#define ACTOR_REGISTER_SCENE_CONSTRUCT_METHOD(ACTOR_NAME) \
public: \
	ACTOR_NAME(Scene* OwnerScene);

#define CALL_REGISTER_OWNERSCENE_METHOD \
	RegisterOwnerScene(OwnerScene);

#define DECLARE_CLASS(THISCLASS_NAME, SUPER_CLASS_NAME) \
	using ThisClass = THISCLASS_NAME; \
	using Super = SUPER_CLASS_NAME;
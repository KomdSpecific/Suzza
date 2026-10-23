#include "Component.h"
#include "Actor.h"

namespace suz
{
	size_t Component::nextId = 0;

	Component::Component()
		: m_isAlive(std::make_shared<bool>(true))
	{
	}

	Component::~Component()
	{
		*m_isAlive = false;
	}

	void Component::RegisterOwnerActor(Actor* OwnerActor)
	{
		m_actorOwner = OwnerActor;
	}
}
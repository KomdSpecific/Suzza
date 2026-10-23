#pragma once

#include "Scene.h"
#include "model/Mesh.h"

class SuzzaApplication
{
public:
	SuzzaApplication() = default;

	bool Init();

	void BeginPlay();

	void Update(float DeltaTime);

	void Destroy();

private:
	SuzzaApplication(SuzzaApplication&&) = delete;
	SuzzaApplication(const SuzzaApplication&) = delete;
	SuzzaApplication& operator=(SuzzaApplication&&) = delete;
	SuzzaApplication& operator=(const SuzzaApplication&) = delete;

	std::unique_ptr<suz::Scene> m_scene = nullptr;

public:
	inline suz::Scene* GetScene() const { return m_scene.get(); }
};
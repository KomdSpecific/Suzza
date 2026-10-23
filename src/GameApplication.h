#pragma once

#include "Scene.h"
#include "model/Mesh.h"

class GameApplication
{
public:
	GameApplication() = default;

	bool Init();

	void BeginPlay();

	void Update(float DeltaTime);

	void Destroy();

private:
	GameApplication(GameApplication&&) = delete;
	GameApplication(const GameApplication&) = delete;
	GameApplication& operator=(GameApplication&&) = delete;
	GameApplication& operator=(const GameApplication&) = delete;

	std::unique_ptr<suz::Scene> m_scene = nullptr;
};
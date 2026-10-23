#include "GameApplication.h"
#include "Engine.h"
#include "model/ModelResourceManager.h"
#include "actor/PlayerController.h"
#include "components/MeshComponent.h"

#include <vector>
#include <iostream>

bool GameApplication::Init()
{
	const std::vector<suz::Vertex> vertices = {
		// -------- 面1：+X面（向前面，X=+0.5f）4顶点
		{{ 0.5f, -0.5f, -0.5f }, {1,1,1}, {0,0}},
		{{ 0.5f,  0.5f, -0.5f }, {1,1,1}, {1,0}},
		{{ 0.5f,  0.5f,  0.5f }, {1,1,1}, {1,1}},
		{{ 0.5f, -0.5f,  0.5f }, {1,1,1}, {0,1}},

		// -------- 面2：‑X面（朝后，X=-0.5f）
		{{-0.5f, -0.5f, -0.5f }, {1,1,1}, {0,0}},
		{{-0.5f,  0.5f, -0.5f }, {1,1,1}, {1,0}},
		{{-0.5f,  0.5f,  0.5f }, {1,1,1}, {1,1}},
		{{-0.5f, -0.5f,  0.5f }, {1,1,1}, {0,1}},

		// -------- 面3：+Y面（向右，Y=+0.5f）
		{{-0.5f,  0.5f, -0.5f }, {1,1,1}, {0,0}},
		{{ 0.5f,  0.5f, -0.5f }, {1,1,1}, {1,0}},
		{{ 0.5f,  0.5f,  0.5f }, {1,1,1}, {1,1}},
		{{-0.5f,  0.5f,  0.5f }, {1,1,1}, {0,1}},

		// -------- 面4：‑Y面（向左，Y=-0.5f）
		{{-0.5f, -0.5f, -0.5f }, {1,1,1}, {0,0}},
		{{ 0.5f, -0.5f, -0.5f }, {1,1,1}, {1,0}},
		{{ 0.5f, -0.5f,  0.5f }, {1,1,1}, {1,1}},
		{{-0.5f, -0.5f,  0.5f }, {1,1,1}, {0,1}},

		// -------- 面5：+Z面（顶面，Z=+0.5f）
		{{-0.5f, -0.5f,  0.5f }, {1,1,1}, {0,0}},
		{{ 0.5f, -0.5f,  0.5f }, {1,1,1}, {1,0}},
		{{ 0.5f,  0.5f,  0.5f }, {1,1,1}, {1,1}},
		{{-0.5f,  0.5f,  0.5f }, {1,1,1}, {0,1}},

		// -------- 面6：‑Z面（底面，Z=-0.5f）
		{{-0.5f, -0.5f, -0.5f }, {1,1,1}, {0,0}},
		{{ 0.5f, -0.5f, -0.5f }, {1,1,1}, {1,0}},
		{{ 0.5f,  0.5f, -0.5f }, {1,1,1}, {1,1}},
		{{-0.5f,  0.5f, -0.5f }, {1,1,1}, {0,1}},
	};

	const std::vector<uint16_t> indices =
	{
		//前面 (0,1,2), (0,2,3)
		0,1,2, 0,2,3,
		//后面 (4,5,6), (4,6,7)
		4,5,6, 4,6,7,
		//左面 (8,9,10), (8,10,11)
		8,9,10, 8,10,11,
		//右面 (12,13,14), (12,14,15)
		12,13,14, 12,14,15,
		//底面 (16,17,18), (16,18,19)
		16,17,18, 16,18,19,
		//顶面 (20,21,22), (20,22,23)
		20,21,22, 20,22,23
	};

	m_scene = std::make_unique<suz::Scene>();

	/* 加载Mesh */
	suz::Mesh* mesh = new suz::Mesh(vertices, indices);
	suz::ModelResourceManager& modelManager = suz::Engine::GetModelResourceManager();
	const std::string& meshId = modelManager.LoadMesh(mesh);

	/* 创建Actor */
	suz::Actor* actor1 = new suz::Actor();
	m_scene->RegisterActor(actor1);

	suz::Actor* actor2 = new suz::Actor();
	m_scene->RegisterActor(actor2);

	suz::MeshComponent* meshComponentActor1 = new suz::MeshComponent();
	meshComponentActor1->CreateMesh(meshId);
	actor1->RegisterComponent<suz::MeshComponent>(meshComponentActor1);
	actor1->SetActorLocation({ 3.0f,0.0f,0.0f });

	suz::MeshComponent* meshComponentActor2 = new suz::MeshComponent();
	meshComponentActor2->CreateMesh(meshId);
	actor2->RegisterComponent<suz::MeshComponent>(meshComponentActor2);
	actor2->SetActorLocation({ 3.0f,2.0f,0.0f });
	actor2->SetActorRotation({ 0.0f,0.0f,45.0f });

	/* 定义玩家Actor */
	suz::PlayerController* player = new suz::PlayerController(m_scene.get());
	m_scene->RegisterActor(player);

	suz::CameraComponent* camera = new suz::CameraComponent();
	player->RegisterComponent<suz::CameraComponent>(camera);
	player->SetActorLocation({ 0.0f,0.0f,0.0f });
	player->SetActorRotation({ 0.0f,0.0f,0.0f });

	return true;
}

void GameApplication::BeginPlay()
{
	m_scene->BeginPlay();
}

void GameApplication::Update(float DeltaTime)
{
	m_scene->Update(DeltaTime);
}

void GameApplication::Destroy()
{
	m_scene->Destroy();
}
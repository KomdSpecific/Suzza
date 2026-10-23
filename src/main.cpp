#include "engine/Engine.h"
#include "SuzzaApplication.h"

#include <iostream>
#include <cstdlib>
#include <stdexcept>

int main()
{
	SuzzaApplication* suzApp = new SuzzaApplication();
	suz::Engine& engine = suz::Engine::GetEngineInstance();
	engine.WIDTH = 800;
	engine.HEIGHT = 600;

	engine.SetApplication(suzApp);

	if (!engine.Init())
	{
		std::cerr << "³ÌÐò³õÊ¼»¯Ê§°Ü!" << std::endl;
		return EXIT_FAILURE;
	}

	try
	{
		engine.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	engine.Destroy();

	return EXIT_SUCCESS;
}
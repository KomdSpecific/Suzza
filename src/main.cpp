#include "engine/Engine.h"
#include "SuzzaApplication.h"

#include "debug/SuzzaLog.h"

#include <cstdlib>
#include <stdexcept>

int main()
{
	SuzzaApplication* suzApp = new SuzzaApplication();
	suz::Engine& engine = suz::Engine::GetEngineInstance();
	engine.WIDTH = 1600;
	engine.HEIGHT = 1200;

	engine.SetApplication(suzApp);

	if (!engine.Init())
	{
		SUZZA_LOG_ERROR("≥Ã–Ú≥ı ºªØ ß∞‹!");
		return EXIT_FAILURE;
	}

	try
	{
		engine.Run();
	}
	catch (const std::exception& e)
	{
		SUZZA_LOG_ERROR(e.what());
		return EXIT_FAILURE;
	}

	engine.Destroy();

	return EXIT_SUCCESS;
}
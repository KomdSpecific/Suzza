#include "engine/Engine.h"
#include "SuzzaApplication.h"

#include "debug/SuzzaLog.h"

#ifdef WIN32
#include <Windows.h>
#endif

static void SetWindowsConsoleCoding()
{
#ifdef WIN32
	SetConsoleOutputCP(CP_UTF8);
#endif
}

int main()
{
	SetWindowsConsoleCoding();

	SuzzaApplication* suzApp = new SuzzaApplication();
	suz::Engine& engine = suz::Engine::GetEngineInstance();
	engine.WIDTH = 1600;
	engine.HEIGHT = 1200;

	engine.SetApplication(suzApp);

	if (!engine.Init())
	{
		SUZZA_LOG_ERROR("程序初始化失败!");
		return EXIT_FAILURE;
	}

	engine.Run();

	engine.Destroy();

	return EXIT_SUCCESS;
}
#include "StdAfx.h"
#include "main.h"

#include "misc.h"
#include "config.h"
#include "scene.h"

extern char path_app[MAX_PATH];

// Точка входа.
// Ничего интересного здесь смотреть.
// Двиньте вперед.
int	main(int argc, char *argv[])
{
	UNUSED_ARG(argc);
	UNUSED_ARG(argv);
	#ifdef FIND_MEM_LEAKS
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	InitPaths();
	ChangeDir(path_app);
	CheckPaths();
	CleanupLogs();
	LogPaths();

	InitVKeysArrays();
	CheckDefaultConfig();

	srand((unsigned)time(NULL));

	scene::InitScene();
	scene::MainLoop();
	scene::FreeScene();

	ClearVKeysArrays();

	EndupLogs();

	return 0x00;
}

#ifndef __GAME_H_
#define __GAME_H_

namespace game
{
	void InitGame();
	void FreeGame(bool fullDestroy);

	void UpdateGame(UINT dt);
	void DrawGame();

	void TogglePause();
	void TogglePause( bool state );
	bool GetPause();

	enum GameStates
	{
		GAME_NONE,
		GAME_INITIALIZING,
		GAME_RUNNNIG,
		GAME_DESTROYING,
		GAME_PAUSED
	};

#ifdef USE_TIME_DEPENDENT_PHYS
#define TARGET_GAMETICK 12
#endif // USE_TIME_DEPENDENT_PHYS

#ifdef GAMETICK_FROM_CONFIG
#define GAMETICK cfg.gametick
#define PASSIVE_OBJ_TICK 10*GAMETICK
#else
#define GAMETICK 10
#endif // GAMETICK_FROM_CONFIG
}


#endif // __GAME_H_

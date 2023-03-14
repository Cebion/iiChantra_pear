#ifndef __LUATHREAD_H_
#define __LUATHREAD_H_

#include "script.h"

class LuaThread
{
public:
	LuaThread(void)
	{
		lThread = NULL;
		refKey = LUA_NOREF;
		pausable = false;
	}

	lua_State* lThread;				// Coroutine

	int refKey;

	bool pausable;				//  огда тред ждет (Wait), врем€ не считаетс€, пока game_state == GAME_PAUSED
};

void ProcessThread(int r);
UINT NewThread(lua_State* L);

void RemoveAllThreads();

#endif // __LUATHREAD_H_
#include "StdAfx.h"

#include "../misc.h"

#include "api.h"
#include "timerevent.h"

#include "luathread.h"

//////////////////////////////////////////////////////////////////////////

extern lua_State* lua;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Список тредов
list<LuaThread*> threads;
typedef list<LuaThread*>::iterator ThreadIter;	// Тип итератора по списку тредов

// Возвращает итератор на тред в списке
ThreadIter GetThread(lua_State* c)
{
	list<LuaThread*>::iterator it;
	for(list<LuaThread*>::iterator it = threads.begin();
		it != threads.end();
		it++)
	{
		LuaThread* lt = *it;
		if (lt->lThread == c)
			return it;
	}
	return threads.end();
}

// Создает новый тред, беря в качестве базового L/
// Действует аналогично coroutine.create, помещая в стек созданный coroutine.
// Возвращает ссылку на coroutine, сохраненную в реестре луа.
UINT NewThread(lua_State* L)
{	
	// Стек L: func pausable
	ASSERT(lua_gettop(L) == 1 || lua_gettop(L) == 2);
	bool pausable = false;
	if (lua_gettop(L) == 2)
	{
		pausable = lua_toboolean(L, 2) != 0;
		lua_pop(L, 1);
	}

	// Код из функции luaB_cocreate файла lbaselib.c
	// Стек L: func
	lua_State *NL = lua_newthread(L);	// Стек L: func thread
										// Стек NL: 

	luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1), 1,
		"Lua function expected");

	lua_pushvalue(L, 1);  // Стек L: func thread func
	lua_xmove(L, NL, 1);  /* move function from L to NL */
	
	// Конец кода из функции luaB_cocreate файла lbaselib.c

	// Стек L: func thread
	// Стек NL: func
	
	LuaThread* lt = new LuaThread();

	lua_pushvalue(L, -1);					// Стек L: func thread thread 
	// А если L == lua, то переноса не будет, с вершины стека просто снимется 1 копия thread
	lua_xmove(L, lua, 1);					// Стек L: func thread
											// Стек lua: thread
	lt->refKey = SCRIPT::AddToRegistry();	// Стек lua: 
	lt->lThread = NL;
	lt->pausable = pausable;
	threads.push_back(lt);

	sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_INFO_EV, "created thread 0x%p, master_thread = 0x%p, refKey = %d", NL, L, lt->refKey);

	return lt->refKey;
}

// Создает новую coroutine. Аналогична команде базовой библиотеки
// lua coroutines.create, за исключением того, что
// запоминает созданную coroutine.
int scriptApi::NewThread(lua_State* L)
{
	luaL_argcheck(L, lua_isfunction(L, 1), 1, "function expected");
	luaL_argcheck(L, lua_isboolean(L, 2) || lua_isnoneornil(L, 2), 2, "boolean or none expected");
	// Стек L: func pausable
	::NewThread(L);
	// Стек L: func thread
	return 1;
}

// Удаляет тред и его события таймера.
void RemoveThread(ThreadIter it)
{
	LuaThread* lt = *it;
	DeleteTimerEvent(lt->refKey);
	sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_INFO_EV, "removed thread 0x%p, refKey = %d", lt->lThread, lt->refKey);
	SCRIPT::RemoveFromRegistry(lt->refKey);
	DELETESINGLE(lt);
	threads.erase(it);
}

// Удаляет все треды.
void RemoveAllThreads()
{
	sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_INFO_EV, "Уничтожаем существующие потоки." );

	LuaThread* lt = NULL;
	ThreadIter it;
	for(it = threads.begin();
		it != threads.end();
		it++)
	{
		lt = *it;

		SCRIPT::RemoveFromRegistry(lt->refKey);
		DELETESINGLE(lt);
	}
	threads.clear();

	DeleteAllThreadEvents();
}

// Удаляет все треды. Биндинг для луа. 
int scriptApi::StopAllThreads(lua_State* L)
{
	UNUSED_ARG(L);
	::RemoveAllThreads();
	return 0;
}

// Удаляет тред. Биндинг для луа.
// Возвращает true или false.
int scriptApi::StopThread(lua_State* L)
{
	luaL_argcheck(L, lua_isthread(L, 1), 1, "Thread expected");
	lua_State* t = lua_tothread(L, 1);
	ThreadIter it = GetThread(t);
	if (it != threads.end())
	{
		::RemoveThread(it);
		lua_pushboolean(L, 1);
		return 1;
	}
	else
	{
		lua_pushboolean(L, 0);
		lua_pushstring(L, "Thread is not registered");
		return 2;
	}	
}


// Функция приостанавливает coroutine, из которой вызвана. Аналогична
// команде базовой библиотеки lua coroutines.yield, но требует первый
// аргумент - число милисекунд, на которое coroutine будет приостановлена.
// Однако, передавать параметры для возврата бессмысленно, resume ничего не вернет.
int scriptApi::Wait(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1,
		"Number expected");

	if (lua_pushthread(L))
	{
		// lua_pushthread вернет true, если мы пытаемся остановить основной поток
		luaL_where(L, 0);
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_SCRIPT_EV, "%s Попытка приостновить основной поток.", lua_tostring(L, -1));
		lua_pop(L, lua_gettop(L));
		return 0;
	}
	else
	{
		// lua_pushthread вернула false, мы не в основном потоке, убираем из стека не нужное значение
		lua_pop(L, 1);
	}

	ThreadIter it = GetThread(L);
	if (it != threads.end())
	{
		AddThreadTimerEvent(lua_tointeger(L, 1), (*it)->refKey, (*it)->pausable);
	}
	lua_remove(L, 1);


	return lua_yield(L, lua_gettop(L));
}

// Продолжает выполнение приостановленной coroutine, являющейся
// тредом, за котором следит игра.
// В ОТЛИЧИЕ от coroutine.resume, ничего не возвращает.
int Resume(ThreadIter it, lua_State* L)
{
	LuaThread* lt = *it;
	//SCRIPT::GetFromRegistry(lt->refKey);
	//SCRIPT::StackDumpToLog(lt->masterState);
	//UINT res = luaB_coresume(lt->masterState);
	
	if (L)
	{
		// Стек L:		thread arg1 arg2 ... argN
		// Cтек thread: [func]
		// func на стеке только в первый раз
		lua_xmove(L, lt->lThread, lua_gettop(L)-1);
		// Стек L:		thread 
		// Cтек thread: [func] arg1 arg2 ... argN
	}


	//SCRIPT::StackDumpToLog(lt->masterState);
	//SCRIPT::StackDumpToLog(lt->lThread);
	lua_resume(lt->lThread, lua_gettop(lt->lThread) - 1);

	if (lua_status(lt->lThread) != LUA_YIELD)
	{
		if (lua_status(lt->lThread) != 0)		// Всякие ошибки
		{
			const char* err = lua_tostring(lt->lThread, -1);
			sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "thread 0x%p: %s", lt->lThread, err);
		}
		RemoveThread(it);
	}

	return 0;
}

// Продолжает выполнение приостановленной coroutine.
// Аналогична команде coroutines.resume, однако работает
// несколько дольше, так как сначала ищет среди созданных игрой тредов.
// Не возвращает аргументы, преданные через Wait
int scriptApi::Resume(lua_State* L)
{
	lua_State *co = lua_tothread(L, 1);
	luaL_argcheck(L, co, 1, "coroutine expected");
	ThreadIter it = GetThread(co);
	if (it != threads.end())
	{
		return ::Resume(it, L);
	}


	return luaB_coresume(L);
}

// Вызывается при настплении события таймера, вызывает продолжение работы треда
void ProcessThread(int r)
{
	LuaThread* lt = NULL;
	ThreadIter it;
	for(it = threads.begin();
		it != threads.end();
		it++)
	{
		lt = *it;
		if (lt->refKey == r)
			break;
	}

	if (lt)
	{
		Resume(it, NULL);
	}
}

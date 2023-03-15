#include "StdAfx.h"

#include "../misc.h"

#include "script.h"
#include "api.h"
#include "luathread.h"
#include <vector>


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

lua_State* lua = NULL;

extern char path_app[MAX_PATH];
extern char path_scripts[MAX_PATH];

//////////////////////////////////////////////////////////////////////////
// Инициализирует скриптовую систему
bool SCRIPT::InitScript()
{
	if (lua)
	{
		FreeScript();
	}

	lua = luaL_newstate();			// Создаем виртуальную машину Lua
	if ( lua == NULL )
	{
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Ошибка создания Lua" );
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_ERROR_EV, "Ошибка создания Lua" );
		return true;
	}
	sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_INFO_EV, "Lua создан.");

	luaL_openlibs ( lua );                              // open standart libraries

	lua_atpanic(lua, SCRIPT::Panic);

	scriptApi::RegisterAPI(lua);			// Инициализируем константы, привязываем C-функции.

	return false;
}

// Освобождает скриптовую систему
void SCRIPT::FreeScript()
{
	if (lua)
	{
		RemoveAllThreads();

		lua_close(lua);
		lua = NULL;
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_INFO_EV, "Lua уничтожен." );
	}
}

// Срабатывает при критических ошибках Lua
int SCRIPT::Panic(lua_State* L)
{
	const char* msg = lua_tostring(L, -1);
	sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_ERROR_EV,
		"Lua Panic: %s", msg);
	sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV,
		"Lua Panic. Exiting.");
	//luaL_where(lua, 1);
	//msg = lua_tostring(L, -1);
	//sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_ERROR_EV,
	//	"Traceback: %s", msg);
	return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Загружает файл и оставляет чанк на вершине стека
bool SCRIPT::LoadFile( const char* filename )
{
	if (!filename)
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV,
			"Ошибка исполнения файла. Передана пустая строка-имя файла." );
		return true;
	}

	if (!lua)
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV,
			"Ошибка загрузки файла \"%s\". Lua не создана.", filename);
		return true;
	}

	//SetCurrentDirectory(path_scripts);
	ChangeDir(path_scripts);

	if(luaL_loadfile(lua, filename))
	{
		const char* err = lua_tostring(lua, -1);
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_ERROR_EV, "%s", err);
		return true;
	}

	//SetCurrentDirectory(path_app);
	ChangeDir(path_app);
	// Стек: func
	return false;
}

// Загружает строку скрипта и оставляет чанк на вершине стека
bool SCRIPT::LoadString( const char* str )
{
	if (!str)
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка загрузки команды. Передана пустая строка." );
		return true;
	}

	if (!lua)
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV,
			"Ошибка загрузки команды: %s. Lua не создана.", str);
		return true;
	}

	if(luaL_loadstring(lua, str))
	{
		const char* err = lua_tostring(lua, -1);
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "%s", err);
		return true;
	}

	return false;
}

// Выполняет функцию (чанк) в конце стека
bool SCRIPT::ExecChunk()
{
	return SCRIPT::ExecChunk(0);
}

bool SCRIPT::ExecChunk(UINT args)
{
	if (lua_pcall(lua, args, LUA_MULTRET, 0))
	{
		const char* err = lua_tostring(lua, -1);
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_ERROR_EV, "In ExecChunk(UINT args): %s", err);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// Заносит в реестр то, что лежит на вершине стека, и возвращает ссылку на него
// При этом с вершины стека оно удаляется
int SCRIPT::AddToRegistry()
{
	// Стек: ... data
	return luaL_ref(lua, LUA_REGISTRYINDEX);	// Стек:
}

// Удаляет из реестра содержимое по ссылке
void SCRIPT::RemoveFromRegistry(int& r)
{
	lua_unref(lua, r);
	r = LUA_NOREF;
}

// Помещает на верхушку стека значенее из реестра
void SCRIPT::GetFromRegistry(lua_State* L, int r)
{
	//lua_rawgeti(L, LUA_REGISTRYINDEX, r);
	lua_rawgeti(lua, LUA_REGISTRYINDEX, r);
	lua_xmove(lua, L, 1);
}

//////////////////////////////////////////////////////////////////////////

// Помещает в procref ссылку реестра луа на функцию, находящуюся в стеке под номером n
void SCRIPT::RegProc(lua_State* L, int* procref, int n)
{
	luaL_argcheck(L, lua_isfunction(L, n) || lua_isnil(L, n), n, "Function or nil expected");

	SCRIPT::RemoveFromRegistry(*procref);
	*procref = LUA_NOREF;

	if(!lua_isnil(L, n))
	{
		// TODO: сделать перемещение функции на самый верх стека и забирать ее оттуда
		// Сейчас же просто забирается одно верхнее значение со стека, это может быть и не функция
		lua_xmove(L, lua, 1);
		*procref = SCRIPT::AddToRegistry();
	}
}

//////////////////////////////////////////////////////////////////////////

// Очищает стек и исполняет файл скрипта
bool SCRIPT::ExecFile( const char* filename )
{
	if (lua)
	{
		lua_pop(lua, lua_gettop(lua));
		return !LoadFile(filename) ? ExecChunk() : true;
	}
	return false;
}

// Очищает стек и исполняет строку скрипта
bool SCRIPT::ExecString( const char* str )
{
	return !LoadString(str) ? ExecChunk() : true;
}

// Загружает файл скрипта и заносит его в реестр
int SCRIPT::LoadFileToReg(const char* filename)
{
	return !LoadFile(filename) ? AddToRegistry() : 1;
}

// Загружает строку скрипта и заносит её в реестр
int SCRIPT::LoadStringToReg(const char* str)
{
	return !LoadString(str) ? AddToRegistry() : 1;
}

// Берет функцию из реестра и выполяняет ее
bool SCRIPT::ExecChunkFromReg(int r, UINT args)
{
	GetFromRegistry(lua, r);

	int i = lua_gettop(lua) - args;
	if (i <= 0)
		i = 1;

	lua_insert(lua, i);

	return ExecChunk(args);
}

bool SCRIPT::ExecChunkFromReg(int r)
{
	return SCRIPT::ExecChunkFromReg(r, 0);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Загружает в стек таблицу и берет из нее значение переменной с заданным именем.
// Возвращает стек в исххожное состояние.
void SCRIPT::GetUIntFieldByName(lua_State* L, const char* table, const char* name, UINT* val)
{
	// Стек: ...
	lua_getglobal(L, table);		// Стек: ..., table
	if (lua_istable(L, -1))
	{
		lua_getfield(L, -1, name);	// Стек: ..., table, name
		if (lua_isnumber(L, -1))
		{
			*val = (UINT)lua_tonumber(L, -1);
		}
		lua_pop(L, 1);		// Стек: ..., table
	}
	lua_pop(L, 1); // Стек: ...
}

// Берет из таблицы на вершине стека значение переменной с заданным именем.
// Возвращает стек в исходное состояние.
void SCRIPT::GetIntFieldByName(lua_State* L, const char* name, int* val)
{
	// Стек: ..., table
	lua_getfield(L, -1, name);	// Стек: ..., table, name
	if (lua_isnumber(L, -1))
	{
		*val = lua_tointeger(L, -1);
	}
	lua_pop(L, 1);		// Стек: ..., table
}

// Берет из таблицы на вершине стека значение переменной с заданным именем.
// Возвращает стек в исходное состояние.
void SCRIPT::GetUIntFieldByName(lua_State* L, const char* name, UINT* val)
{
	// Стек: ..., table
	lua_getfield(L, -1, name);	// Стек: ..., table, name
	if (lua_isnumber(L, -1))
	{
		*val = (UINT)lua_tonumber(L, -1);
	}
	lua_pop(L, 1);		// Стек: ..., table
}

// Берет из таблицы на вершине стека значение переменной с заданным именем.
// Возвращает стек в исходное состояние.
void SCRIPT::GetFloatFieldByName(lua_State* L, const char* name, float* val)
{
	// Стек: ..., table
	lua_getfield(L, -1, name);	// Стек: ..., table, name
	if (lua_isnumber(L, -1))
	{
		*val = (float)lua_tonumber(L, -1);
	}
	lua_pop(L, 1);		// Стек: ..., table
}

// Берет из таблицы на вершине стека значение переменной с заданным именем.
// Возвращает стек в исходное состояние.
void SCRIPT::GetStringFieldByName(lua_State* L, const char* name, char** val)
{
	// Стек: ..., table
	lua_getfield(L, -1, name);	// Стек: ..., table, name
	if (lua_isstring(L, -1))
	{
		const char* str = lua_tostring(L, -1);
		*val = new char[strlen(str) + 1];
		strcpy(*val, str);
	}
	lua_pop(L, 1);		// Стек: ..., table

}

// Берет из таблицы на вершине стека таблицу с заданным именем и пытается
// из нее загрущить первые четыре значения массива.
// Возвращает стек в исходное состояние.
void SCRIPT::GetColorFieldByName(lua_State* L, const char* name, RGBAf& val)
{
	// Стек: ..., table
	lua_getfield(L, -1, name);	// Стек: ..., table, name
	SCRIPT::GetColorFromTable(L, -1, val);
	lua_pop(L, 1);		// Стек: ..., table
}

// Пытается загрузить первые четыре значения массива из таблице в стеке на позиции n.
void SCRIPT::GetColorFromTable(lua_State* L, int n, RGBAf& val)
{
	if (lua_istable(L, n))
	{
		// Стек: ..., color, ...
		lua_rawgeti(L, n, 1);	val.r = (float)lua_tonumber(L, -1); lua_pop(L, 1);
		lua_rawgeti(L, n, 2);	val.g = (float)lua_tonumber(L, -1); lua_pop(L, 1);
		lua_rawgeti(L, n, 3);	val.b = (float)lua_tonumber(L, -1); lua_pop(L, 1);
		lua_rawgeti(L, n, 4);	val.a = (float)lua_tonumber(L, -1); lua_pop(L, 1);
	}
}

void SCRIPT::GetIntVectorFieldByName(lua_State* L, const char* name, vector<int>& val, bool add)
{
	// Стек: ..., table
	lua_getfield(L, -1, name);	// Стек: ..., table, name
	//val = new vector<int>;
	if (lua_istable(L, -1))
	{
		size_t size = lua_objlen( L, -1 );
		if ( !add && val.size() > 0 )
			val.clear();
		for ( size_t i = 1; i <= size; i++ )
		{
			lua_rawgeti( L, -1, i );
			if ( lua_isnumber(L, -1) )
				val.push_back( lua_tointeger(L, -1) );
			lua_pop( L, 1 );
		}
	}
	lua_pop(L, 1);		// Стек: ..., table
}

//////////////////////////////////////////////////////////////////////////

// Выводит в лог дамп содержимого стека. Использовать для отладки функций.
void SCRIPT::StackDumpToLog(lua_State *L)
{
	int i;
	int top = lua_gettop(L);
	string s("");
	char buf[20];
#ifdef WIN32
	sprintf_s(buf, 20, "0x%p: ", (void*)L);
#else
	snprintf(buf, 20, "0x%p: ", (void*)L);
#endif // WIN32
	s += buf;

	for (i = 1; i <= top; i++)
	{  /* repeat for each level */
		int t = lua_type(L, i);
		switch (t)
		{
		case LUA_TSTRING: /* strings */
			s += string("'") + lua_tostring(L, i) + "'";
			break;

		case LUA_TBOOLEAN: /* booleans */
			s += lua_toboolean(L, i) ? "true" : "false";
			break;

		case LUA_TNUMBER: /* numbers */
			// TODO: sprintf_s и snprintf немного отличаются поведением.
			// sprintf_s гарантирует, что в конце буефера будет 0.
#ifdef WIN32
			sprintf_s(buf, 20, "%f", lua_tonumber(L, i));
#else
			snprintf(buf, 20, "%f", lua_tonumber(L, i));
#endif // WIN32
			s += string(buf);
			break;

		case LUA_TTHREAD:
			//char buf[20];
			// TODO: sprintf_s и snprintf немного отличаются поведением.
			// sprintf_s гарантирует, что в конце буефера будет 0.
#ifdef WIN32
			sprintf_s(buf, 20, "0x%p", (void*)lua_tothread(L, i));
#else
			snprintf(buf, 20, "0x%p", (void*)lua_tothread(L, i));
#endif // WIN32

			s += "thread " + string(buf);
			break;

		default: /* other values */
			s += lua_typename(L, t);
			break;

		}
		s += ", "; /* put a separator */
	}
	sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_SCRIPT_EV, s.c_str());
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Помещает в стек таблицу, представляющую вектор
void SCRIPT::PushVector(lua_State* L, Vector2& v)
{
	// Стек: obj
	lua_createtable(L, 0, 2);	// Стек: obj vector

	lua_pushnumber(L, v.x); lua_setfield(L, -2, "x");
	lua_pushnumber(L, v.y); lua_setfield(L, -2, "y");
}

// Помещает в стек таблицу, представляющую AABB
void SCRIPT::PushAABB(lua_State* L, CAABB& a)
{
	// Стек: obj
	lua_createtable(L, 0, 3);	// Стек: obj aabb

	PushVector(L, a.p);		lua_setfield(L, -2, "p");
	lua_pushnumber(L, a.H); lua_setfield(L, -2, "H");
	lua_pushnumber(L, a.W); lua_setfield(L, -2, "W");
}

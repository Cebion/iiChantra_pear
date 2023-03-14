#ifndef __SCRIPT_H_
#define __SCRIPT_H_

#include "lua/lua.hpp"

#include "../render/types.h"
#include "../game/phys/phys_misc.h"

namespace SCRIPT
{
	bool InitScript();
	void FreeScript();
	int Panic(lua_State* L);
	
	bool LoadFile(const char* filename);
	bool LoadString(const char* str);
	bool ExecChunk();
	bool ExecChunk(UINT args);

	int AddToRegistry();
	void RemoveFromRegistry(int& r);
	void GetFromRegistry(lua_State* L, int r);

	void RegProc(lua_State* L, int* procref, int n);

	bool ExecFile(const char* filename);
	bool ExecString(const char* str);
	bool ExecChunkFromReg(int r);
	bool ExecChunkFromReg(int r, UINT args);
	int LoadFileToReg(const char* filename);
	int LoadStringToReg(const char* str);


	void GetUIntFieldByName(lua_State* L, const char* table, const char* name, UINT* val);
	void GetUIntFieldByName(lua_State* L, const char* name, UINT* val);
	void GetIntFieldByName(lua_State* L, const char* name, int* val);
	void GetFloatFieldByName(lua_State* L, const char* name, float* val);
	void GetStringFieldByName(lua_State* L, const char* name, char** val);
	void GetColorFieldByName(lua_State* L, const char* name, RGBAf& val);
	void GetColorFromTable(lua_State* L, int n, RGBAf& val);
	void GetIntVectorFieldByName(lua_State* L, const char* name, vector<int>& val, bool add);

	void StackDumpToLog(lua_State *L);

	void PushVector(lua_State* L, Vector2& v);
	void PushAABB(lua_State* L, CAABB& a);

}

//#define SCRIPTLOG(x) LogToFile( DEFAULT_SCRIPT_LOG_NAME, string(x)+"\n" );

#ifdef _DEBUG
#define STACK_CHECK_INIT(L) int stackCountBefore = lua_gettop(L); int stackCountAfter = -1;
#define STACK_CHECK(L)  { stackCountAfter = lua_gettop(L); assert(stackCountBefore == stackCountAfter); }
#else
#define STACK_CHECK_INIT(L)		{(void)L;}
#define STACK_CHECK(L)			{(void)L;}
#endif // _DEBUG

#endif // __SCRIPT_H_
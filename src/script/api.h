#ifndef __SCRIPT_API_H_
#define __SCRIPT_API_H_

#include "script.h"

namespace scriptApi
{
	void RegisterAPI(lua_State* L);

	void MapVarAdd( lua_State* L, char const * var_name, int amount );

	int LoadConfig (lua_State* L);
	int SaveConfig(lua_State* L);

	int LoadTexture (lua_State* L);
	int LoadFont(lua_State* L);
	int LoadPrototype(lua_State* L);

	int InitNewGame(lua_State* L);
	int DestroyGame(lua_State* L);
	int ExitGame(lua_State* L);

	int CreateMap(lua_State* L);
	int CreateColorBox(lua_State* L);
	int CreateSprite(lua_State* L);
	int CreateEffect(lua_State* L);
	int CreatePlayer(lua_State* L);
	int RevivePlayer(lua_State* L);
	int CreateEnemy(lua_State* L);
	int CreateSpawner(lua_State* L);
	int CreateItem(lua_State* L);
	int CreateWaypoint(lua_State* L);
	int CreateSpecialWaypoint(lua_State* L);
	int CreateParticleSystem(lua_State *L);
	int CreateGroup(lua_State *L);
	int AddParticleArea(lua_State *L);
	int SetParticleWind(lua_State *L);
	int GetParticleWind(lua_State *L);
	int CreateEnvironment(lua_State* L);
	int CreateRay(lua_State* L);
	int setDefaultEnvironment(lua_State* L);

	int SetNextWaypoint(lua_State* L);
	int SetEnemyWaypoint(lua_State* L);

	int AddTimerEvent(lua_State* L);

	int Log(lua_State* L);

	int GetCurTime(lua_State* L);

	int SetCamLag(lua_State* L);
	int SetCamAttachedObj(lua_State* L);
	int CamMoveToPos(lua_State* L);
	int SetCamFocusOnObjPos(lua_State* L);
	int SetCamObjOffset(lua_State* L);
	int SetCamAttachedAxis(lua_State* L);
	int SetCamBounds(lua_State* L);
	int SetCamUseBounds(lua_State* L);
	int GetCamPos(lua_State* L);
	int GetCamFocusShift(lua_State* L);

	int GetMousePos(lua_State* L);
	int IsConfKeyPressed(lua_State* L);
	int IsConfKeyHolded(lua_State* L);
	int IsConfKeyReleased(lua_State* L);
	int IsConfKey(lua_State* L);


	int CreateRibbon(lua_State* L);
	int SetRibbonZ(lua_State* L);
	int SetRibbonAttatachToY(lua_State* L);
	int SetRibbonBounds(lua_State* L);
	int CreateRibbonObj(lua_State* L);
	int SetRibbonObjBounds(lua_State* L);
	int SetRibbonObjRepitition(lua_State* L);
	int SetRibbonObjK(lua_State* L);

	int ListDirContents(lua_State* L);

//////////////////////////////////////////////////////////////////////////

	int GetPlayer(lua_State* L);
	int GetPlayerNum(lua_State* L);
	int GetObject(lua_State* L);
	int GetWaypoint(lua_State* L);
	int GetChildren(lua_State* L);

	int GetCharHealth(lua_State* L);
	int SetCharHealth(lua_State* L);
	
//////////////////////////////////////////////////////////////////////////

	int SetObjDead(lua_State* L);
	int SetObjPos(lua_State* L);
	int SetDynObjGravity(lua_State* L);
	int SetDynObjAcc(lua_State* L);
	int SetDynObjVel(lua_State* L);
	int SetDynObjMovement(lua_State* L);
	int SetDynObjMoveDirX(lua_State* L);
	int SetDynObjJumpVel(lua_State* L);
	int GetDynObjShadow(lua_State* L);
	int SetDynObjShadow(lua_State* L);


	int SetObjSpriteMirrored(lua_State* L);
	int SetObjSpriteColor(lua_State* L);
	int SetObjAnim(lua_State* L);
	int SetObjSolidTo(lua_State* L);
	int SetObjGhostTo(lua_State* L);
	int SetObjSolidToByte(lua_State* L);
	int SetObjGhostToByte(lua_State* L);

	int SetRayPos(lua_State* L);
	int SetRaySearchDistance(lua_State* L);

	int EnablePlayerControl(lua_State* L);
	int GetPlayerControlState(lua_State* L);
	
	int SetObjProcessor(lua_State* L);

	int GroupObjects(lua_State* L);

	int SetPhysObjBorderColor(lua_State* L);

//////////////////////////////////////////////////////////////////////////

	int DamageObject(lua_State* L);
	int SetPlayerStats(lua_State* L);
	int ReplacePrimaryWeapon(lua_State* L);
	int SetEnvironmentStats(lua_State* L);
	int SetPlayerHealth(lua_State* L);
	int SetPlayerAmmo(lua_State* L);
	int SetPlayerAltWeapon(lua_State* L);
	int SetPlayerRevivePoint(lua_State* L);
	int SetOnChangePlayerProcessor(lua_State* L);
	int SetOnPlayerDeathProcessor(lua_State* L);
	int BlockPlayerChange(lua_State* L);
	int UnblockPlayerChange(lua_State* L);
	int AddBonusHealth(lua_State* L);
	int AddBonusAmmo(lua_State* L);
	int AddBonusSpeed(lua_State* L);
	int SwitchPlayer(lua_State* L);
	int SwitchWeapon(lua_State* L);

//////////////////////////////////////////////////////////////////////////

	int CreateWidget(lua_State* L);
	int DestroyWidget(lua_State* L);
	int WidgetSetCaption(lua_State* L);
	int WidgetSetMaxTextfieldSize(lua_State* L);
	int WidgetGetCaption(lua_State* L);
	int WidgetSetCaptionColor(lua_State* L);
	int WidgetSetCaptionFont(lua_State* L);
	int GetCaptionSize(lua_State* L);
	int WidgetStopTyper(lua_State* L);
	int WidgetStartTyper(lua_State* L);
	int WidgetUseTyper(lua_State* L);
	int WidgetSetOnTyperEndedProc(lua_State* L);	
	int WidgetGetVisible(lua_State* L);
	int WidgetGetSize(lua_State* L);
	int WidgetGetPos(lua_State* L);
	int WidgetSetVisible(lua_State* L);
	int WidgetSetFixedPosition(lua_State* L);
	int WidgetSetFocusable(lua_State* L);
	int WidgetSetBorder(lua_State* L);
	int WidgetSetBorderColor(lua_State* L);
	int WidgetSetLMouseClickProc(lua_State* L);
	int WidgetSetRMouseClickProc(lua_State* L);
	int WidgetSetMouseEnterProc(lua_State* L);
	int WidgetSetMouseLeaveProc(lua_State* L);
	int WidgetSetKeyDownProc(lua_State* L);
	int WidgetSetKeyPressProc(lua_State* L);
	int WidgetSetKeyInputProc(lua_State* L);
	int WidgetSetFocusProc(lua_State* L);
	int WidgetSetUnFocusProc(lua_State* L);
	int WidgetSetResizeProc(lua_State* L);
	int WidgetSetSprite(lua_State* L);
	int WidgetSetColorBox(lua_State* L);
	int WidgetSetSpriteRenderMethod(lua_State* L);
	int WidgetSetAnim(lua_State* L);
	int WidgetSetSize(lua_State* L);
	int WidgetSetPos(lua_State* L);
	int WidgetSetZ(lua_State* L);
	int WidgetBringToFront(lua_State* L);
	int WidgetGetName(lua_State* L);
	int WidgetGainFocus(lua_State* L);
	int PushWidget(lua_State* L);
	int GamePaused(lua_State* L);

	int GlobalSetKeyDownProc(lua_State* L);
	int GlobalSetKeyReleaseProc(lua_State* L);
	int GlobalGetKeyDownProc(lua_State* L);
	int GlobalGetKeyReleaseProc(lua_State* L);
	int GlobalSetMouseKeyDownProc(lua_State* L);
	int GlobalSetMouseKeyReleaseProc(lua_State* L);
	int GlobalGetMouseKeyDownProc(lua_State* L);
	int GlobalGetMouseKeyReleaseProc(lua_State* L);

//////////////////////////////////////////////////////////////////////////

	int LoadSound(lua_State* L);
	int PlaySnd(lua_State* L);
	int PauseSnd(lua_State* L);
	int StopSnd(lua_State* L);
	int StopAllSnd(lua_State* L);
	int GetMasterSoundVolume(lua_State* L);
	int SetMasterSoundVolume(lua_State* L);
	int GetMasterSFXVolume(lua_State* L);
	int SetMasterSFXVolume(lua_State* L);
	int GetMasterMusicVolume(lua_State* L);
	int SetMasterMusicVolume(lua_State* L);
	int GetSoundVolume(lua_State* L);
	int SetSoundVolume(lua_State* L);
	int PlayBackMusic(lua_State* L);
	int PauseBackMusic(lua_State* L);
	int StopBackMusic(lua_State* L);

//////////////////////////////////////////////////////////////////////////

	int SendHighscores(lua_State* L);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Для файла threads.cpp
// Функции создания отдельных "потоков" на основе coroutines, которые
// можно приостанавливать.

	int NewThread(lua_State* L);
	int Wait(lua_State* L);	
	int Resume(lua_State* L);
	int StopAllThreads(lua_State* L);
	int StopThread(lua_State* L);

	int PushButton(lua_State* L);
	int TogglePause(lua_State* L);
	int DumpKeys(lua_State* L);
	int PushKeyName(lua_State* L);
	int GetField(lua_State* L);
	int SetField(lua_State* L);
	int GetFirstButtonPressed(lua_State* L);

	int PushScreenInfo(lua_State* L);
	int PushInt(lua_State* L);
	int SetObjectInvincible(lua_State* L);
	int SetObjectInvisible(lua_State* L);
	int SetPlayerRecoveryTime(lua_State* L);

//////////////////////////////////////////////////////////////////////////
	int EditorGetObjects(lua_State* L);
	int RegEditorGetObjectsProc(lua_State* L);
	int EditorToggleBorders(lua_State* L);

	int GetNearestWaypoint(lua_State* L);

}


//////////////////////////////////////////////////////////////////////////

enum MapObjType
{
	otSprite, otPlayer, otEnemy, otBox, otGroup, otItem, otSecret, otTile, otNone, otSpawner, otRibbon
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// game.cpp
#include "../game/game.h"
extern game::GameStates game_state;
// 
#define CHECKGAME if (game_state != game::GAME_RUNNNIG && game_state != game::GAME_PAUSED) { lua_pop(L, lua_gettop(L)); lua_pushstring(L, "GAME not running"); return 1; } 

class GameObject;
void PushObject(lua_State* L, GameObject* o);

#endif // __SCRIPT_API_H_

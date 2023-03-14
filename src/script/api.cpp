#include "StdAfx.h"

#include "../game/player.h"

#include "../game/game.h"
#include "../config.h"
#include "../scene.h"
#include "../misc.h"
#include "../input_mgr.h"

#include "../render/texture.h"
#include "../render/font.h"
#include "../render/draw.h"

#include "../game/proto.h"

#include "../game/camera.h"
#include "../game/ribbon.h"
#include "../game/editor.h"

#include "../game/objects/object_character.h"
#include "../game/objects/object_spawner.h"

#include "../gui/gui.h"

#include "../sound/snd.h"

#include "api.h"
#include "timerevent.h"

#include "../resource_mgr.h"

#include "../game/highscores.h"

#ifdef WIN32
	#include "../dirent/dirent.h"
#else
	#include "dirent.h"
#endif //WIN32

//////////////////////////////////////////////////////////////////////////

// scene.cpp
extern config cfg;
extern ProgrammStates current_state;
extern InputMgr inpmgr;

// misc.cpp
extern char path_app[MAX_PATH];
extern char path_config[MAX_PATH];
extern char path_textures[MAX_PATH];
extern char path_protos[MAX_PATH];
extern char path_levels[MAX_PATH];
extern char path_scripts[MAX_PATH];
extern char path_sounds[MAX_PATH];


// camera.cpp
extern CameraFocusObjectPoint CAMERA_FOCUS_ON_OBJ_POS;
extern float CAMERA_X;
extern float CAMERA_Y;

// resource_mgr.cpp
extern ResourceMgr<Proto> * protoMgr;
extern ResourceMgr<Texture> * textureMgr;
extern SoundMgr* soundMgr;
extern Player* playerControl;

//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void scriptApi::RegisterAPI(lua_State* L)
{
	// Меняем значение package.path, чтобы команда require искала фалы в папке скриптов
	lua_getglobal(L, "package");		// Стек: package
	char* new_path = new char[strlen(path_scripts) + 1+6];
	memset(new_path,'\0', strlen(path_scripts) + 1+6);
	sprintf(new_path, "%s\?.lua", path_scripts);
	lua_pushstring(L, new_path);		// Стек: package new_path
	lua_setfield(L, -2, "path");		// Стек: package
	lua_pop(L, 1);						// Стек:
	DELETEARRAY(new_path);

	// Привязываем С-функции
	lua_register(L, "LoadConfig", &scriptApi::LoadConfig);
	lua_register(L, "SaveConfig", &scriptApi::SaveConfig);
	lua_register(L, "LoadTexture", &scriptApi::LoadTexture);
	lua_register(L, "LoadFont", &scriptApi::LoadFont);
	lua_register(L, "LoadPrototype", &scriptApi::LoadPrototype);
	lua_register(L, "InitNewGame", &scriptApi::InitNewGame);
	lua_register(L, "DestroyGame", &scriptApi::DestroyGame);
	lua_register(L, "AddTimerEvent", &scriptApi::AddTimerEvent);

	lua_register(L, "Log", &scriptApi::Log);
	lua_register(L, "GetCurTime", &scriptApi::GetCurTime);

	lua_register(L, "GetNearestWaypoint", &scriptApi::GetNearestWaypoint);

	lua_register(L, "SetCamLag", &scriptApi::SetCamLag);
	lua_register(L, "SetCamAttachedObj", &scriptApi::SetCamAttachedObj);
	lua_register(L, "CamMoveToPos", &scriptApi::CamMoveToPos);
	lua_register(L, "SetCamFocusOnObjPos", &scriptApi::SetCamFocusOnObjPos);
	lua_register(L, "SetCamObjOffset", &scriptApi::SetCamObjOffset);
	lua_register(L, "SetCamAttachedAxis", &scriptApi::SetCamAttachedAxis);
	lua_register(L, "SetCamUseBounds", &scriptApi::SetCamUseBounds);
	lua_register(L, "SetCamBounds", &scriptApi::SetCamBounds);
	lua_register(L, "GetCamPos", &scriptApi::GetCamPos);
	lua_register(L, "GetCamFocusShift", &scriptApi::GetCamFocusShift);
	lua_register(L, "GetMousePos", &scriptApi::GetMousePos);

	lua_register(L, "CreateRibbon", &scriptApi::CreateRibbon);
	lua_register(L, "SetRibbonBounds", &scriptApi::SetRibbonBounds);
	lua_register(L, "SetRibbonAttatachToY", &scriptApi::SetRibbonAttatachToY);
	lua_register(L, "SetRibbonZ", &scriptApi::SetRibbonZ);
	lua_register(L, "CreateRibbonObj", &scriptApi::CreateRibbonObj);
	lua_register(L, "SetRibbonObjBounds", &scriptApi::SetRibbonObjBounds);
	lua_register(L, "SetRibbonObjK", &scriptApi::SetRibbonObjK);
	lua_register(L, "SetRibbonObjRepitition", &scriptApi::SetRibbonObjRepitition);

	lua_register(L, "ListDirContents", &scriptApi::ListDirContents);

	lua_register(L, "CreatePlayer", &scriptApi::CreatePlayer);
	lua_register(L, "CreatePlayerZ", &scriptApi::CreatePlayer);

	lua_register(L, "CreateWaypoint", &scriptApi::CreateWaypoint);
	lua_register(L, "CreateSpecialWaypoint", &scriptApi::CreateSpecialWaypoint);
	lua_register(L, "SetNextWaypoint", &scriptApi::SetNextWaypoint);
	lua_register(L, "SetEnemyWaypoint", &scriptApi::SetEnemyWaypoint);
	lua_register(L, "GetWaypoint", &scriptApi::GetWaypoint);

	lua_register(L, "RevivePlayer", &scriptApi::RevivePlayer);

	lua_register(L, "CreateParticleSystem", scriptApi::CreateParticleSystem);
	lua_register(L, "AddParticleArea", scriptApi::AddParticleArea);
	lua_register(L, "SetWind", scriptApi::SetParticleWind);
	lua_register(L, "GetWind", scriptApi::GetParticleWind);

	lua_register(L, "CreateMap", &scriptApi::CreateMap);
	lua_register(L, "CreateColorBox", &scriptApi::CreateColorBox);
	lua_register(L, "CreateSprite", &scriptApi::CreateSprite);
	lua_register(L, "CreateEffect", &scriptApi::CreateEffect);
	lua_register(L, "CreateGroup", &scriptApi::CreateGroup);
	lua_register(L, "CreateSpriteZ", &scriptApi::CreateSprite);
	lua_register(L, "CreateSpriteF", &scriptApi::CreateSprite);
	lua_register(L, "CreateSpriteZF", &scriptApi::CreateSprite);
	lua_register(L, "CreateEnemy", &scriptApi::CreateEnemy);
	lua_register(L, "CreateSpawner", &scriptApi::CreateSpawner);
	lua_register(L, "CreateItem", &scriptApi::CreateItem);
	lua_register(L, "CreateEnvironment", &scriptApi::CreateEnvironment);
	lua_register(L, "CreateRay", &scriptApi::CreateRay);
	lua_register(L, "SetDefaultEnvironment", &scriptApi::setDefaultEnvironment);

	lua_register(L, "SetObjAnim", &scriptApi::SetObjAnim);
	lua_register(L, "SetObjSolidTo", &scriptApi::SetObjSolidTo);
	lua_register(L, "SetObjGhostTo", &scriptApi::SetObjGhostTo);
	lua_register(L, "SetObjSolidToByte", &scriptApi::SetObjSolidToByte);
	lua_register(L, "SetObjGhostToByte", &scriptApi::SetObjGhostToByte);

	lua_register(L, "GetPlayer", &scriptApi::GetPlayer);
	lua_register(L, "GetPlayerNum", &scriptApi::GetPlayerNum);
	lua_register(L, "GetObject", &scriptApi::GetObject);
	lua_register(L, "GetChildren", &scriptApi::GetChildren);

	lua_register(L, "GetCharHealth", &scriptApi::GetCharHealth);
	lua_register(L, "SetCharHealth", &scriptApi::SetCharHealth);
	
	lua_register(L, "GroupObjects", &scriptApi::GroupObjects);

	lua_register(L, "SetObjDead", &scriptApi::SetObjDead);
	lua_register(L, "SetObjPos", &scriptApi::SetObjPos);
	lua_register(L, "SetDynObjAcc", &scriptApi::SetDynObjAcc);
	lua_register(L, "SetDynObjGravity", &scriptApi::SetDynObjGravity);
	lua_register(L, "SetDynObjVel", &scriptApi::SetDynObjVel);
	lua_register(L, "SetDynObjJumpVel", &scriptApi::SetDynObjJumpVel);
	lua_register(L, "SetDynObjShadow", &scriptApi::SetDynObjShadow);
	lua_register(L, "GetDynObjShadow", &scriptApi::GetDynObjShadow);
	lua_register(L, "EnablePlayerControl", &scriptApi::EnablePlayerControl);
	lua_register(L, "GetPlayerControlState", &scriptApi::GetPlayerControlState);
	lua_register(L, "SetDynObjMovement", &scriptApi::SetDynObjMovement);
	lua_register(L, "SetDynObjMoveDirX", &scriptApi::SetDynObjMoveDirX);
	lua_register(L, "SetObjSpriteMirrored", &scriptApi::SetObjSpriteMirrored);
	lua_register(L, "SetObjSpriteColor", &scriptApi::SetObjSpriteColor);
	lua_register(L, "SetObjAnim", &scriptApi::SetObjAnim);
	lua_register(L, "SetRayPos", &scriptApi::SetRayPos);
	lua_register(L, "SetRaySearchDistance", &scriptApi::SetRaySearchDistance);
	lua_register(L, "AddBonusHealth", &scriptApi::AddBonusHealth);
	lua_register(L, "AddBonusAmmo", &scriptApi::AddBonusAmmo);
	lua_register(L, "AddBonusSpeed", &scriptApi::AddBonusSpeed);

	lua_register(L, "SetObjProcessor", &scriptApi::SetObjProcessor);
	
	lua_register(L, "SetPhysObjBorderColor", &scriptApi::SetPhysObjBorderColor);

	lua_register(L, "DamageObject", &scriptApi::DamageObject);
	lua_register(L, "SetPlayerStats", &scriptApi::SetPlayerStats);
	lua_register(L, "ReplacePrimaryWeapon", &scriptApi::ReplacePrimaryWeapon);
	lua_register(L, "SwitchCharacter", &scriptApi::SwitchPlayer);
	lua_register(L, "SwitchWeapon", &scriptApi::SwitchWeapon);
	lua_register(L, "SetEnvironmentStats", &scriptApi::SetEnvironmentStats);
	lua_register(L, "SetPlayerHealth", &scriptApi::SetPlayerHealth);
	lua_register(L, "SetObjInvincible", &scriptApi::SetObjectInvincible);
	lua_register(L, "SetObjInvisible", &scriptApi::SetObjectInvisible);
	lua_register(L, "SetPlayerRecoveryTime", &scriptApi::SetPlayerRecoveryTime);
	lua_register(L, "SetPlayerAmmo", &scriptApi::SetPlayerAmmo);
	lua_register(L, "SetPlayerAltWeapon", &scriptApi::SetPlayerAltWeapon);
	lua_register(L, "SetPlayerRevivePoint", &scriptApi::SetPlayerRevivePoint);
	lua_register(L, "SetOnChangePlayerProcessor", &scriptApi::SetOnChangePlayerProcessor);
	lua_register(L, "SetOnPlayerDeathProcessor", &scriptApi::SetOnPlayerDeathProcessor);
	lua_register(L, "BlockPlayerChange", &scriptApi::BlockPlayerChange);
	lua_register(L, "UnblockPlayerChange", &scriptApi::UnblockPlayerChange);

	lua_register(L, "NewThread", &scriptApi::NewThread);
	lua_register(L, "Wait", &scriptApi::Wait);
	lua_register(L, "Resume", &scriptApi::Resume);
	lua_register(L, "StopThread", &scriptApi::StopThread);
	lua_register(L, "StopAllThreads", &scriptApi::StopAllThreads);

	lua_register(L, "CreateWidget", &scriptApi::CreateWidget);
	lua_register(L, "DestroyWidget", &scriptApi::DestroyWidget);
	lua_register(L, "WidgetSetMaxTextfieldSize", &scriptApi::WidgetSetMaxTextfieldSize);
	lua_register(L, "WidgetSetCaption", &scriptApi::WidgetSetCaption);
	lua_register(L, "WidgetGetCaption", &scriptApi::WidgetGetCaption);
	lua_register(L, "WidgetSetCaptionColor", &scriptApi::WidgetSetCaptionColor);
	lua_register(L, "WidgetSetCaptionFont", &scriptApi::WidgetSetCaptionFont);
	lua_register(L, "GetCaptionSize", &scriptApi::GetCaptionSize);
	lua_register(L, "WidgetUseTyper", &scriptApi::WidgetUseTyper);
	lua_register(L, "WidgetStartTyper", &scriptApi::WidgetStartTyper);
	lua_register(L, "WidgetStopTyper", &scriptApi::WidgetStopTyper);
	lua_register(L, "WidgetSetOnTyperEndedProc", &scriptApi::WidgetSetOnTyperEndedProc);
	lua_register(L, "WidgetSetVisible", &scriptApi::WidgetSetVisible);
	lua_register(L, "WidgetGetVisible", &scriptApi::WidgetGetVisible);
	lua_register(L, "WidgetSetFixedPosition", &scriptApi::WidgetSetFixedPosition);
	lua_register(L, "WidgetSetFocusable", &scriptApi::WidgetSetFocusable);
	lua_register(L, "WidgetSetBorder", &scriptApi::WidgetSetBorder);
	lua_register(L, "WidgetSetBorderColor", &scriptApi::WidgetSetBorderColor);
	lua_register(L, "WidgetSetLMouseClickProc", &scriptApi::WidgetSetLMouseClickProc);
	lua_register(L, "WidgetSetRMouseClickProc", &scriptApi::WidgetSetRMouseClickProc);
	lua_register(L, "WidgetSetMouseEnterProc", &scriptApi::WidgetSetMouseEnterProc);
	lua_register(L, "WidgetSetMouseLeaveProc", &scriptApi::WidgetSetMouseLeaveProc);
	lua_register(L, "WidgetSetKeyDownProc", &scriptApi::WidgetSetKeyDownProc);
	lua_register(L, "WidgetSetKeyInputProc", &scriptApi::WidgetSetKeyInputProc);
	lua_register(L, "WidgetSetKeyPressProc", &scriptApi::WidgetSetKeyPressProc);
	lua_register(L, "WidgetSetFocusProc", &scriptApi::WidgetSetFocusProc);
	lua_register(L, "WidgetSetUnFocusProc", &scriptApi::WidgetSetUnFocusProc);
	lua_register(L, "WidgetSetResizeProc", &scriptApi::WidgetSetResizeProc);	
	lua_register(L, "WidgetSetSize", &scriptApi::WidgetSetSize);
	lua_register(L, "WidgetSetPos", &scriptApi::WidgetSetPos);
	lua_register(L, "WidgetSetZ", &scriptApi::WidgetSetZ);
	lua_register(L, "WidgetGainFocus", &scriptApi::WidgetGainFocus);
	lua_register(L, "WidgetBringToFront", &scriptApi::WidgetBringToFront);
	lua_register(L, "PushWidget", &scriptApi::PushWidget);
	lua_register(L, "WidgetSetSprite", &scriptApi::WidgetSetSprite);
	lua_register(L, "WidgetSetColorBox", &scriptApi::WidgetSetColorBox);
	lua_register(L, "WidgetSetSpriteRenderMethod", &scriptApi::WidgetSetSpriteRenderMethod);
	lua_register(L, "WidgetSetAnim", &scriptApi::WidgetSetAnim);
	lua_register(L, "GetWidgetName", &scriptApi::WidgetGetName);
	lua_register(L, "WidgetGetSize", &scriptApi::WidgetGetSize);
	lua_register(L, "WidgetGetPos", &scriptApi::WidgetGetPos);
	lua_register(L, "ObjectPushInt", &scriptApi::PushInt);

	lua_register(L, "GlobalSetKeyDownProc", &scriptApi::GlobalSetKeyDownProc);
	lua_register(L, "GlobalSetKeyReleaseProc", &scriptApi::GlobalSetKeyReleaseProc);
	lua_register(L, "GlobalGetKeyDownProc", &scriptApi::GlobalGetKeyDownProc);
	lua_register(L, "GlobalGetKeyReleaseProc", &scriptApi::GlobalGetKeyReleaseProc);
	lua_register(L, "GlobalSetMouseKeyDownProc", &scriptApi::GlobalSetMouseKeyDownProc);
	lua_register(L, "GlobalSetMouseKeyReleaseProc", &scriptApi::GlobalSetMouseKeyReleaseProc);
	lua_register(L, "GlobalGetMouseKeyDownProc", &scriptApi::GlobalGetMouseKeyDownProc);
	lua_register(L, "GlobalGetMouseKeyReleaseProc", &scriptApi::GlobalGetMouseKeyReleaseProc);

	lua_register(L, "LoadSound", &scriptApi::LoadSound);
	lua_register(L, "PlaySnd", &scriptApi::PlaySnd);
	lua_register(L, "PauseSnd", &scriptApi::PauseSnd);
	lua_register(L, "StopSnd", &scriptApi::StopSnd);
	lua_register(L, "StopAllSnd", &scriptApi::StopAllSnd);
	lua_register(L, "GetMasterSoundVolume", &scriptApi::GetMasterSoundVolume);
	lua_register(L, "SetMasterSoundVolume", &scriptApi::SetMasterSoundVolume);
	lua_register(L, "GetMasterSFXVolume", &scriptApi::GetMasterSFXVolume);
	lua_register(L, "SetMasterSFXVolume", &scriptApi::SetMasterSFXVolume);
	lua_register(L, "GetMasterMusicVolume", &scriptApi::GetMasterMusicVolume);
	lua_register(L, "SetMasterMusicVolume", &scriptApi::SetMasterMusicVolume);
	lua_register(L, "GetSoundVolume", &scriptApi::GetSoundVolume);
	lua_register(L, "SetSoundVolume", &scriptApi::SetSoundVolume);
	lua_register(L, "PlayBackMusic", &scriptApi::PlayBackMusic);
	lua_register(L, "PauseBackMusic", &scriptApi::PauseBackMusic);
	lua_register(L, "StopBackMusic", &scriptApi::StopBackMusic);
	lua_register(L, "ExitGame", &scriptApi::ExitGame);

	lua_register(L, "SendHighscores", &scriptApi::SendHighscores);

	lua_register(L, "ButtonPressed", &scriptApi::PushButton);
	lua_register(L, "TogglePause", &scriptApi::TogglePause);
	lua_register(L, "GetKeysState", &scriptApi::DumpKeys);
	lua_register(L, "GetKeyName", &scriptApi::PushKeyName);
	lua_register(L, "GetField", &scriptApi::GetField);
	lua_register(L, "SetField", &scriptApi::SetField);
	lua_register(L, "GetFirstButtonPressed", &scriptApi::GetFirstButtonPressed);
	lua_register(L, "GetScreenInfo", &scriptApi::PushScreenInfo);
	lua_register(L, "GamePaused", &scriptApi::GamePaused);
	lua_register(L, "IsConfKeyPressed", &scriptApi::IsConfKeyPressed);
	lua_register(L, "IsConfKeyHolded", &scriptApi::IsConfKeyHolded);
	lua_register(L, "IsConfKeyReleased", &scriptApi::IsConfKeyReleased);
	lua_register(L, "IsConfKey", &scriptApi::IsConfKey);

	lua_register(L, "EditorGetObjects", &scriptApi::EditorGetObjects);
	lua_register(L, "EditorDumpMap", &EditorDumpMap);
	lua_register(L, "EditorResize", &EditorResize);
	lua_register(L, "EditorCloneObject", &EditorCopyObject);
	lua_register(L, "EditorToggleBorders", &scriptApi::EditorToggleBorders);
	lua_register(L, "RegEditorGetObjectsProc", &scriptApi::RegEditorGetObjectsProc);

	// TODO: Зарегестрировать разные константы движка, чтобы были доступны в скриптах.
	// Стандартный шрифт. Виртуал-кейс и т. п. Было в одном из уроков на ilovelua

	// Регистрируем константные названия для клавиш.
	lua_newtable(L);			// Создаем новую таблицу. Стек: таблица
	{
		char ch[MAX_VKEY_NAME_LEN] = "";	// Буфер для названий

		for (UINT i = 1; i < inpmgr.keys_count; i++)		// Цикл по всем возможным кодам
		{
			GetVKeyByNum(i, ch);
			lua_pushinteger(L,i);		// Помещаем в стек код клавиши. Стек: таблица, i
			lua_setfield(L, -2, ch);	// Заносим в таблицу перменную с именем и значением. Стек: таблица
		}
	}
	lua_setglobal(L, "keys");		// Заносим таблицу в глобальную переменную. Стек:

	lua_newtable(L);			// Создаем новую таблицу. Стек: таблица
	{
		for (size_t i = 0; i < cakLastKey; i++)		// Цикл по всем возможным кнопкам в конфиге
		{
			lua_pushinteger(L, (int)i);
			lua_setfield(L, -2, GetConfigKeyName((ConfigActionKeys)i));
		}
	}
	lua_setglobal(L, "config_keys");		// Заносим таблицу в глобальную переменную. Стек:

	// Различные именованые константы
	lua_newtable(L);		// Стек: таблица
	{
		lua_pushinteger(L, protoNullBeh);		lua_setfield(L, -2, "NullBehaviour");
		lua_pushinteger(L, protoPlayer);		lua_setfield(L, -2, "PlayerBehaviour");
		lua_pushinteger(L, protoEnemy);			lua_setfield(L, -2, "EnemyBehaviour");
		lua_pushinteger(L, protoPowerup);		lua_setfield(L, -2, "PowerUpBehaviour");
		lua_pushinteger(L, protoSprite);		lua_setfield(L, -2, "SpriteBehaviour");

		// Параметры для SetCamFocusOnObjPos
		lua_pushinteger(L, CamFocusLeftBottomCorner);	lua_setfield(L, -2, "CamFocusLeftBottomCorner");
		lua_pushinteger(L, CamFocusLeftCenter);			lua_setfield(L, -2, "CamFocusLeftCenter");
		lua_pushinteger(L, CamFocusLeftTopCorner);		lua_setfield(L, -2, "CamFocusLeftTopCorner");
		lua_pushinteger(L, CamFocusRightBottomCorner);	lua_setfield(L, -2, "CamFocusRightBottomCorner");
		lua_pushinteger(L, CamFocusRightCenter);		lua_setfield(L, -2, "CamFocusRightCenter");
		lua_pushinteger(L, CamFocusRightTopCorner);		lua_setfield(L, -2, "CamFocusRightTopCorner");
		lua_pushinteger(L, CamFocusBottomCenter);		lua_setfield(L, -2, "CamFocusBottomCenter");
		lua_pushinteger(L, CamFocusCenter);				lua_setfield(L, -2, "CamFocusCenter");
		lua_pushinteger(L, CamFocusTopCenter);			lua_setfield(L, -2, "CamFocusTopCenter");

		// Параметры для CreateMap
		lua_pushinteger(L, otSprite);		lua_setfield(L, -2, "ObjSprite");
		lua_pushinteger(L, otPlayer);		lua_setfield(L, -2, "ObjPlayer");
		lua_pushinteger(L, otEnemy);		lua_setfield(L, -2, "ObjEnemy");
		lua_pushinteger(L, otBox);			lua_setfield(L, -2, "ObjBox");
		lua_pushinteger(L, otGroup);		lua_setfield(L, -2, "ObjGroup");
		lua_pushinteger(L, otItem);			lua_setfield(L, -2, "ObjItem");
		lua_pushinteger(L, otSecret);		lua_setfield(L, -2, "ObjSecret");
		lua_pushinteger(L, otTile);			lua_setfield(L, -2, "ObjTile");
		lua_pushinteger(L, otSpawner);		lua_setfield(L, -2, "ObjSpawner");
		lua_pushinteger(L, otRibbon);		lua_setfield(L, -2, "ObjRibbon");
		lua_pushinteger(L, otNone);			lua_setfield(L, -2, "ObjNone");

		// Команды анимаций
		lua_pushinteger(L, afcNone);					lua_setfield(L, -2, "AnimComNone");
		lua_pushinteger(L, afcBreakpoint);				lua_setfield(L, -2, "AnimComBreakpoint");
		lua_pushinteger(L, afcSetDamageMult);			lua_setfield(L, -2, "AnimComSetDamageMult");
		lua_pushinteger(L, afcSetSolid);				lua_setfield(L, -2, "AnimComSetSolid");
		lua_pushinteger(L, afcSetWaypoint);				lua_setfield(L, -2, "AnimComSetWaypoint");
		lua_pushinteger(L, afcPop);						lua_setfield(L, -2, "AnimComPop");
		lua_pushinteger(L, afcSetShadow);				lua_setfield(L, -2, "AnimComSetShadow");
		lua_pushinteger(L, afcMirror);					lua_setfield(L, -2, "AnimComMirror");
		lua_pushinteger(L, afcReplaceWithRandomTile);	lua_setfield(L, -2, "AnimComReplaceWithRandomTile");
		lua_pushinteger(L, afcRandomAngledSpeed);		lua_setfield(L, -2, "AnimComRandomAngledSpeed");
		lua_pushinteger(L, afcSetShielding);			lua_setfield(L, -2, "AnimComSetShielding");
		lua_pushinteger(L, afcEnemyClean);				lua_setfield(L, -2, "AnimComEnemyClean");
		lua_pushinteger(L, afcStartDying);				lua_setfield(L, -2, "AnimComStartDying");
		lua_pushinteger(L, afcSetBulletCollidable);		lua_setfield(L, -2, "AnimComSetBulletCollidable");
		lua_pushinteger(L, afcJumpIfCloseToCamera);		lua_setfield(L, -2, "AnimComJumpIfCloseToCamera");
		lua_pushinteger(L, afcJumpIfCloseToCameraLeft);	lua_setfield(L, -2, "AnimComJumpIfCloseToCameraLeft");
		lua_pushinteger(L, afcJumpIfCloseToCameraRight);lua_setfield(L, -2, "AnimComJumpIfCloseToCameraRight");
		lua_pushinteger(L, afcJumpIfCloseToCameraUp);	lua_setfield(L, -2, "AnimComJumpIfCloseToCameraUp");
		lua_pushinteger(L, afcJumpIfCloseToCameraDown);	lua_setfield(L, -2, "AnimComJumpIfCloseToCameraDown");
		lua_pushinteger(L, afcJumpIfObjectExists);		lua_setfield(L, -2, "AnimComJumpIfObjectExists");
		lua_pushinteger(L, afcJumpIfXGreater);			lua_setfield(L, -2, "AnimComJumpIfXGreater");
		lua_pushinteger(L, afcJumpIfWaypointClose);		lua_setfield(L, -2, "AnimComJumpIfWaypointClose");
		lua_pushinteger(L, afcSetNearestWaypoint);		lua_setfield(L, -2, "AnimComSetNearestWaypoint");
		lua_pushinteger(L, afcLocalJumpIfIntEquals);	lua_setfield(L, -2, "AnimComLocalJumpIfIntEquals");
		lua_pushinteger(L, afcLocalJumpIfNextIntEquals);lua_setfield(L, -2, "AnimComLocalJumpIfNextIntEquals");
		lua_pushinteger(L, afcJumpIfXLess);				lua_setfield(L, -2, "AnimComJumpIfXLess");
		lua_pushinteger(L, afcJumpCheckFOV);			lua_setfield(L, -2, "AnimComJumpCheckFOV");
		lua_pushinteger(L, afcCreateEnemy);				lua_setfield(L, -2, "AnimComCreateEnemy");
		lua_pushinteger(L, afcSummonObject);			lua_setfield(L, -2, "AnimComSummonObject");
		lua_pushinteger(L, afcStopMorphing);			lua_setfield(L, -2, "AnimComStopMorphing");
		lua_pushinteger(L, afcJumpIfStackIsNotEmpty);	lua_setfield(L, -2, "AnimComJumpIfStackIsNotEmpty");
		lua_pushinteger(L, afcRandomOverlayColor);		lua_setfield(L, -2, "AnimComRandomOverlayColor");
		lua_pushinteger(L, afcControlledOverlayColor);	lua_setfield(L, -2, "AnimComControlledOverlayColor");
		lua_pushinteger(L, afcSetColor);				lua_setfield(L, -2, "AnimComSetColor");
		lua_pushinteger(L, afcRealX);					lua_setfield(L, -2, "AnimComRealX");
		lua_pushinteger(L, afcRealY);					lua_setfield(L, -2, "AnimComRealY");
		lua_pushinteger(L, afcRealW);					lua_setfield(L, -2, "AnimComRealW");
		lua_pushinteger(L, afcRealH);					lua_setfield(L, -2, "AnimComRealH");
		lua_pushinteger(L, afcInitW);					lua_setfield(L, -2, "AnimComInitW");
		lua_pushinteger(L, afcInitH);					lua_setfield(L, -2, "AnimComInitH");
		lua_pushinteger(L, afcLoop);					lua_setfield(L, -2, "AnimComLoop");
		lua_pushinteger(L, afcDrop);					lua_setfield(L, -2, "AnimComDrop");
		lua_pushinteger(L, afcShootX);					lua_setfield(L, -2, "AnimComShootX");
		lua_pushinteger(L, afcShootY);					lua_setfield(L, -2, "AnimComShootY");
		lua_pushinteger(L, afcShootDir);				lua_setfield(L, -2, "AnimComShootDir");
		lua_pushinteger(L, afcShootBeh);				lua_setfield(L, -2, "AnimComShootBeh");
		lua_pushinteger(L, afcShoot);					lua_setfield(L, -2, "AnimComShoot");
		lua_pushinteger(L, afcSetAnim);					lua_setfield(L, -2, "AnimComSetAnim");
		lua_pushinteger(L, afcSetAnimIfGunDirection);	lua_setfield(L, -2, "AnimComSetAnimIfGunDirection");
		lua_pushinteger(L, afcSetAnimOnTargetPos);		lua_setfield(L, -2, "AnimComSetAnimOnTargetPos");
		lua_pushinteger(L, afcSetAnimIfWeaponNotReady);	lua_setfield(L, -2, "AnimComSetAnimIfWeaponNotReady");
		lua_pushinteger(L, afcPushString);				lua_setfield(L, -2, "AnimComPushStr");
		lua_pushinteger(L, afcPushInt);					lua_setfield(L, -2, "AnimComPushInt");
		lua_pushinteger(L, afcPushRandomInt);			lua_setfield(L, -2, "AnimComPushRandomInt");
		lua_pushinteger(L, afcJump);					lua_setfield(L, -2, "AnimComJump");
		lua_pushinteger(L, afcJumpIfIntEquals);			lua_setfield(L, -2, "AnimComJumpIfIntEquals");
		lua_pushinteger(L, afcJumpIfYSpeedGreater);		lua_setfield(L, -2, "AnimComJumpIfYSpeedGreater");
		lua_pushinteger(L, afcJumpIfXSpeedGreater);		lua_setfield(L, -2, "AnimComJumpIfXSpeedGreater");
		lua_pushinteger(L, afcJumpIfOnPlane);			lua_setfield(L, -2, "AnimComJumpIfOnPlane");
		lua_pushinteger(L, afcJumpRandom);				lua_setfield(L, -2, "AnimComJumpRandom");
		lua_pushinteger(L, afcJumpIfPlayerId);			lua_setfield(L, -2, "AnimComJumpIfPlayerId");
		lua_pushinteger(L, afcRecover);					lua_setfield(L, -2, "AnimComRecover");
		lua_pushinteger(L, afcCreateEffect);			lua_setfield(L, -2, "AnimComCreateEffect");
		lua_pushinteger(L, afcCreateObject);			lua_setfield(L, -2, "AnimComCreateObject");
		lua_pushinteger(L, afcDestroyObject);			lua_setfield(L, -2, "AnimComDestroyObject");
		lua_pushinteger(L, afcCreateParticles);			lua_setfield(L, -2, "AnimComCreateParticles");
		lua_pushinteger(L, afcCreateItem);				lua_setfield(L, -2, "AnimComCreateItem");
		lua_pushinteger(L, afcJumpIfWeaponReady);		lua_setfield(L, -2, "AnimComJumpIfWeaponReady");
		lua_pushinteger(L, afcMountPointSet);			lua_setfield(L, -2, "AnimComMPSet");
		lua_pushinteger(L, afcSetHealth);				lua_setfield(L, -2, "AnimComSetHealth");
		lua_pushinteger(L, afcSetGravity);				lua_setfield(L, -2, "AnimComSetGravity");
		lua_pushinteger(L, afcSetLifetime);				lua_setfield(L, -2, "AnimComSetLifetime");
		lua_pushinteger(L, afcWait);				lua_setfield(L, -2, "AnimComWait");
		lua_pushinteger(L, afcWaitForTarget);			lua_setfield(L, -2, "AnimComWaitForTarget");
		lua_pushinteger(L, afcClearTarget);				lua_setfield(L, -2, "AnimComClearTarget");
		lua_pushinteger(L, afcWaitForEnemy);			lua_setfield(L, -2, "AnimComWaitForEnemy");
		lua_pushinteger(L, afcFlyToWaypoint);			lua_setfield(L, -2, "AnimComFlyToWaypoint");
		lua_pushinteger(L, afcTeleportToWaypoint);		lua_setfield(L, -2, "AnimComTeleportToWaypoint");
		lua_pushinteger(L, afcMoveToTargetX);			lua_setfield(L, -2, "AnimComMoveToTargetX");
		lua_pushinteger(L, afcMoveToTargetY);			lua_setfield(L, -2, "AnimComMoveToTargetY");
		lua_pushinteger(L, afcMoveToTarget);			lua_setfield(L, -2, "AnimComMoveToTarget");
		lua_pushinteger(L, afcJumpIfTargetX);			lua_setfield(L, -2, "AnimComJumpIfTargetX");
		lua_pushinteger(L, afcJumpIfTargetY);			lua_setfield(L, -2, "AnimComJumpIfTargetY");
		lua_pushinteger(L, afcJumpIfTargetClose);		lua_setfield(L, -2, "AnimComJumpIfTargetClose");
		lua_pushinteger(L, afcJumpIfTargetCloseByX);	lua_setfield(L, -2, "AnimComJumpIfTargetCloseByX");
		lua_pushinteger(L, afcJumpIfTargetCloseByY);	lua_setfield(L, -2, "AnimComJumpIfTargetCloseByY");
		lua_pushinteger(L, afcCreateEnemyRay);			lua_setfield(L, -2, "AnimComCreateEnemyRay");
		lua_pushinteger(L, afcCreateEnemyBullet);		lua_setfield(L, -2, "AnimComCreateEnemyBullet");
		lua_pushinteger(L, afcAdjustAim);				lua_setfield(L, -2, "AnimComAdjustAim");
		lua_pushinteger(L, afcAdjustHomingAcc);			lua_setfield(L, -2, "AnimComAdjustHomingAcc");
		lua_pushinteger(L, afcAimedShot);				lua_setfield(L, -2, "AnimComAimedShot");
		lua_pushinteger(L, afcAngledShot);				lua_setfield(L, -2, "AnimComAngledShot");
		lua_pushinteger(L, afcFaceTarget);				lua_setfield(L, -2, "AnimComFaceTarget");
		lua_pushinteger(L, afcPlaySound);				lua_setfield(L, -2, "AnimComPlaySound");
		lua_pushinteger(L, afcEnvSound);				lua_setfield(L, -2, "AnimComEnvSound");
		lua_pushinteger(L, afcEnvSprite);				lua_setfield(L, -2, "AnimComEnvSprite");
		lua_pushinteger(L, afcGiveHealth);				lua_setfield(L, -2, "AnimComGiveHealth");
		lua_pushinteger(L, afcGiveWeapon);				lua_setfield(L, -2, "AnimComGiveWeapon");
		lua_pushinteger(L, afcGiveAmmo);				lua_setfield(L, -2, "AnimComGiveAmmo");
		lua_pushinteger(L, afcDamage);					lua_setfield(L, -2, "AnimComDamage");
		lua_pushinteger(L, afcDealDamage);				lua_setfield(L, -2, "AnimComDealDamage");
		lua_pushinteger(L, afcReduceHealth);			lua_setfield(L, -2, "AnimComReduceHealth");
		lua_pushinteger(L, afcSetZ);					lua_setfield(L, -2, "AnimComSetZ");
		lua_pushinteger(L, afcSetInvincible);			lua_setfield(L, -2, "AnimComSetInvincible");
		lua_pushinteger(L, afcSetInvisible);			lua_setfield(L, -2, "AnimComSetInvisible");
		lua_pushinteger(L, afcMapVarFieldAdd);			lua_setfield(L, -2, "AnimComMapVarAdd");
		lua_pushinteger(L, afcSetAccY);					lua_setfield(L, -2, "AnimComSetAccY");
		lua_pushinteger(L, afcAdjustAccY);				lua_setfield(L, -2, "AnimComAdjustAccY");
		lua_pushinteger(L, afcSetVelY);					lua_setfield(L, -2, "AnimComSetVelY");
		lua_pushinteger(L, afcSetAccX);					lua_setfield(L, -2, "AnimComSetAccX");
		lua_pushinteger(L, afcSetRelativeVelX);			lua_setfield(L, -2, "AnimComSetRelativeVelX");
		lua_pushinteger(L, afcSetRelativePos);			lua_setfield(L, -2, "AnimComSetRelativePos");
		lua_pushinteger(L, afcSetVelX);					lua_setfield(L, -2, "AnimComSetVelX");
		lua_pushinteger(L, afcAdjustX);					lua_setfield(L, -2, "AnimComAdjustX");
		lua_pushinteger(L, afcAdjustY);					lua_setfield(L, -2, "AnimComAdjustY");
		lua_pushinteger(L, afcStop);					lua_setfield(L, -2, "AnimComStop");
		lua_pushinteger(L, afcSetMaxVelX);				lua_setfield(L, -2, "AnimComSetMaxVelX");
		lua_pushinteger(L, afcSetMaxVelY);				lua_setfield(L, -2, "AnimComSetMaxVelY");
		lua_pushinteger(L, afcSetTouchable);			lua_setfield(L, -2, "AnimComSetTouchable");
		lua_pushinteger(L, afcJumpIfSquashCondition);	lua_setfield(L, -2, "AnimComJumpIfSquashCondition");
		lua_pushinteger(L, afcBounceObject);			lua_setfield(L, -2, "AnimComBounceObject");
		lua_pushinteger(L, afcPushObject);				lua_setfield(L, -2, "AnimComPushObject");
		lua_pushinteger(L, afcCallFunction);			lua_setfield(L, -2, "AnimComCallFunction");
		lua_pushinteger(L, afcCallFunctionWithStackParameter);	lua_setfield(L, -2, "AnimComCallFunctionWithStackParameter");

		// Параметры для A	nimComShootBeh
		lua_pushinteger(L, csbNoShooting);		lua_setfield(L, -2, "csbNoShooting");
		lua_pushinteger(L, csbFreeShooting);	lua_setfield(L, -2, "csbFreeShooting");
		lua_pushinteger(L, csbOnAnimCommand);	lua_setfield(L, -2, "csbOnAnimCommand");

		// Параметры для AnimComShootDir
		lua_pushinteger(L, cgdNone);			lua_setfield(L, -2, "cgdNone");
		lua_pushinteger(L, cgdUp);				lua_setfield(L, -2, "cgdUp");
		lua_pushinteger(L, cgdDown);			lua_setfield(L, -2, "cgdDown");

		// Параметры для CreateWidget
		lua_pushinteger(L, wt_Widget);			lua_setfield(L, -2, "wt_Widget");
		lua_pushinteger(L, wt_Button);			lua_setfield(L, -2, "wt_Button");
		lua_pushinteger(L, wt_Picture);			lua_setfield(L, -2, "wt_Picture");
		lua_pushinteger(L, wt_Label);			lua_setfield(L, -2, "wt_Label");
		lua_pushinteger(L, wt_Textfield);		lua_setfield(L, -2, "wt_Textfield");

		// Методы обнаружения касаний
		lua_pushinteger(L, tdtAlways);			lua_setfield(L, -2, "tdtAlways");
		lua_pushinteger(L, tdtFromBottom);		lua_setfield(L, -2, "tdtFromBottom");
		lua_pushinteger(L, tdtFromEverywhere);	lua_setfield(L, -2, "tdtFromEverywhere");
		lua_pushinteger(L, tdtFromTop);			lua_setfield(L, -2, "tdtFromTop");
		lua_pushinteger(L, tdtFromSides);		lua_setfield(L, -2, "tdtFromSides");
		lua_pushinteger(L, tdtTopAndSides);		lua_setfield(L, -2, "tdtTopAndSides");

		// Параметры для GuiSetNavMode
		lua_pushinteger(L, gnm_None);			lua_setfield(L, -2, "gnm_None");
		lua_pushinteger(L, gnm_Normal);			lua_setfield(L, -2, "gnm_Normal");

		//Уровни логов
		lua_pushinteger(L, logLevelNone);		lua_setfield(L, -2, "logLevelNone");
		lua_pushinteger(L, logLevelError);		lua_setfield(L, -2, "logLevelError");
		lua_pushinteger(L, logLevelWarning);	lua_setfield(L, -2, "logLevelWarning");
		lua_pushinteger(L, logLevelInfo);		lua_setfield(L, -2, "logLevelInfo");

		//Поведение объектов за кадром
		lua_pushinteger(L, cobNone);		lua_setfield(L, -2, "offscreenDoNothing");
		lua_pushinteger(L, cobSleep);		lua_setfield(L, -2, "offscreenSleep");
		lua_pushinteger(L, cobDie);			lua_setfield(L, -2, "offscreenDestroy");
		lua_pushinteger(L, cobAnim);		lua_setfield(L, -2, "offscreenSwitchAnim");

		//Соотношение направления взгляда и отражения спрайта
		lua_pushinteger(L, ofNormal);		lua_setfield(L, -2, "facingNormal");
		lua_pushinteger(L, ofMoonwalking);	lua_setfield(L, -2, "facingMoonwalking");
		lua_pushinteger(L, ofFixed);		lua_setfield(L, -2, "facingFixed");

		//Траектории частиц и выстрелов.
		lua_pushinteger(L, pttLine);		lua_setfield(L, -2, "pttLine");
		lua_pushinteger(L, pttSine);		lua_setfield(L, -2, "pttSine");
		lua_pushinteger(L, pttCosine);		lua_setfield(L, -2, "pttCosine");
		lua_pushinteger(L, pttRipple);		lua_setfield(L, -2, "pttRipple");
		lua_pushinteger(L, pttRandom);		lua_setfield(L, -2, "pttRandom");
		lua_pushinteger(L, pttPseudoDepth);	lua_setfield(L, -2, "pttPseudoDepth");
		lua_pushinteger(L, pttTwist);		lua_setfield(L, -2, "pttTwist");
		lua_pushinteger(L, pttOrbit);		lua_setfield(L, -2, "pttOrbit");
		lua_pushinteger(L, pttGlobalSine);	lua_setfield(L, -2, "pttGlobalSine");

		//Направления.
		lua_pushinteger(L, dirNone);		lua_setfield(L, -2, "dirNone");
		lua_pushinteger(L, dirLeft);		lua_setfield(L, -2, "dirLeft");
		lua_pushinteger(L, dirRight);		lua_setfield(L, -2, "dirRight");
		lua_pushinteger(L, dirDown);		lua_setfield(L, -2, "dirDown");
		lua_pushinteger(L, dirUp);			lua_setfield(L, -2, "dirUp");
		lua_pushinteger(L, dirAny);			lua_setfield(L, -2, "dirAny");
		lua_pushinteger(L, dirHorizontal);	lua_setfield(L, -2, "dirHorizontal");
		lua_pushinteger(L, dirVertical);	lua_setfield(L, -2, "dirVertical");

		// Количество наборов кнопок в конфиге
		lua_pushinteger(L, KEY_SETS_NUMBER);lua_setfield(L, -2, "configKeySetsNumber");

		// Методы рендеринга спрайтов
		lua_pushinteger(L, rsmStandart);	lua_setfield(L, -2, "rsmStandart");
		lua_pushinteger(L, rsmStretch);		lua_setfield(L, -2, "rsmStretch");
		lua_pushinteger(L, rsmCrop);		lua_setfield(L, -2, "rsmCrop");
		lua_pushinteger(L, rsmRepeatX);		lua_setfield(L, -2, "rsmRepeatX");
		lua_pushinteger(L, rsmRepeatY);		lua_setfield(L, -2, "rsmRepeatY");
		lua_pushinteger(L, rsmRepeatXY);	lua_setfield(L, -2, "rsmRepeatXY");
	}
	lua_setglobal(L, "constants");		// Заносим таблицу в глобальную переменную. Стек:
}

//////////////////////////////////////////////////////////////////////////

int scriptApi::LoadConfig(lua_State* L)
{
	UNUSED_ARG(L);
	::LoadConfig();
	return 0;
}

int scriptApi::SaveConfig(lua_State* L)
{
	UNUSED_ARG(L);
	::SaveConfig();
	return 0;
}

//////////////////////////////////////////////////////////////////////////

int scriptApi::LoadTexture (lua_State* L)
{
	return 0;

	if (!textureMgr)
	{
		sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Менеджер текстур не создан");
		return 0;
	}

	luaL_argcheck(L, lua_isstring(L, 1), 1, "Ожидается имя текстуры");

	const char* tex_name = lua_tostring(L, 1);

	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Загружаем текстуру: %s", tex_name);

	if ( textureMgr->GetByName(tex_name) == NULL)
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Ошибка загрузки текстуры.");

	return 0;
}

// Загрузка шрифта. Принимает 2 или 4 аргумента в зависимости
// от типа шрифта (текстурный или Windows-шрифт).
// Возврщает результат загрузки true или false
int scriptApi::LoadFont (lua_State* L)
{
	int num = lua_gettop ( L );
	bool res = false;

	luaL_argcheck(L, lua_isstring(L, 1), 1, "Ожидается имя шрифта");
	luaL_argcheck(L, lua_isstring(L, num), num, "Ожидается имя шрифта для игры");

	const char* font_name = lua_tostring(L, 1);
	const char* out_name = lua_tostring(L, num);

	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Загружаем шрифт: %s", font_name);

#ifdef WIN32
	if (num == 4)
	{
		luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается высота шрифта");
		luaL_argcheck(L, lua_isnumber(L, 3), 3, "Ожидается ширина шрифта");

		UINT height = (UINT)lua_tointeger(L, 2);
		UINT weight = (UINT)lua_tointeger(L, 3);

		res = LoadWindowsFont(font_name, (BYTE)height, (long)weight, out_name);
	}
	else
#endif // WIN32
		if (num == 2)
		{
			res = LoadTextureFont(font_name, out_name);
		}

		if (!res)
			sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Ошибка загрузки шрифта.");

		lua_settop(L, num);
		lua_pushboolean(L, res);
		return 1;
}


// Загруза прототипа игрового объекта из файла
int scriptApi::LoadPrototype(lua_State* L)
{
	if (!protoMgr)
	{
		sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Менеджер прототипов не создан");
		return 0;
	}

	luaL_argcheck(L, lua_isstring(L, 1), 1, "Ожидается имя прототипа");

	// Стек: proto_name
	const char* proto_name = lua_tostring(L, 1);
	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Загружаем прототип: %s", proto_name);

	if ( protoMgr->GetByName(proto_name) == NULL)
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Ошибка загрузки прототипа.");

	return 0;
}

int scriptApi::LoadSound(lua_State* L)
{
	luaL_argcheck(L, lua_isstring(L, 1), 1, "Ожидается имя звука");

	const char* file_name = lua_tostring(L, 1);
	char* snd_file = new char[strlen(path_sounds) + strlen(file_name) + 1];
	memset(snd_file, '\0', strlen(path_sounds) + strlen(file_name) + 1);
	sprintf(snd_file, "%s%s", path_sounds, file_name);

	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Загружаем звук: %s", file_name);

	if (!soundMgr || !::soundMgr->GetByName(snd_file))
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Ошибка загрузки звука.");

	DELETEARRAY(snd_file);

	return 0;
}






//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int scriptApi::InitNewGame(lua_State* L)
{
	game::InitGame();
	lua_pop(L, lua_gettop(L));
	return 0;
}

int scriptApi::DestroyGame(lua_State* L)
{
	UNUSED_ARG(L);
	// TODO: опасно. Можно вызвать из обработки объекта - тогда игра упадет.
	// Надо сделать что-то вроде need_destroying. 
	game::FreeGame(true);
	return 0;
}

int scriptApi::ExitGame(lua_State* L)
{
	UNUSED_ARG(L);
	current_state = PROGRAMM_EXITING;
	return 0;
}

////////////////////////////////////

int scriptApi::AddTimerEvent(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается интервал времени");
	luaL_argcheck(L, lua_isfunction(L, 2), 2, "Ожидается функция");

	//SCRIPT::StackDumpToLog(L);

	UINT dt = (UINT)lua_tointeger(L, 1);
	UINT period = 0;
	UINT maxCalls = 0;

	int num = lua_gettop ( L );	// Берем количество элементов в стеке (количество переданных аргументов)
	if (num == 4)
	{
		luaL_argcheck(L, lua_isnumber(L, 3), 3, "Ожидается период");
		luaL_argcheck(L, lua_isnumber(L, 4), 4, "Ожидается количество повторений");

		period = (UINT)lua_tointeger(L, 3);
		maxCalls = (UINT)lua_tointeger(L, 4);
	}


	lua_settop(L, 2);	// Таким образом функция окажется на вершине стека
	//SCRIPT::StackDumpToLog(L);
	int action = SCRIPT::AddToRegistry();

	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Создаем событие таймера.");

	int result = 0;
	if (num == 2)
		result = ::AddTimerEvent(dt, action);
	else
		result = ::AddTimerEvent(dt, action, period, maxCalls);

	if (!result)
	{
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Ошибка создания события таймера.");
	}

	return 0;
}


///////////////////////////////////////

// Запись в лог. Работает как print
int scriptApi::Log(lua_State* L)
{
	int num = lua_gettop ( L );	// Берем количество элементов в стеке (количество переданных аргументов)

	string buf("");
	char char_buf[20];
	for (int i = -num; i < 0; i++)
	{
		int t = lua_type(L, i);
		switch (t)
		{
		case LUA_TSTRING: /* strings */
			{
				buf += lua_tostring(L, i);
			}
			break;

		case LUA_TBOOLEAN: /* booleans */
			{
				buf += lua_toboolean(L, i) ? "true" : "false";
			}
			break;

		case LUA_TNUMBER: /* numbers */
			{
				// TODO: sprintf_s и snprintf немного отличаются поведением.
				// sprintf_s гарантирует, что в конце буефера будет 0.
#ifdef WIN32
				sprintf_s(char_buf, 20, "%f", lua_tonumber(L, i));
#else
				snprintf(char_buf, 20, "%f", lua_tonumber(L, i));
#endif // WIN32
				buf += string(char_buf);
			}
			break;

		case LUA_TNIL:
			buf += "nil";
			break;

		default:
			{
				// TODO: sprintf_s и snprintf немного отличаются поведением.
				// sprintf_s гарантирует, что в конце буефера будет 0.
#ifdef WIN32
				sprintf_s(char_buf, 20, "0x%p", lua_topointer(L, i));
#else
				snprintf(char_buf, 20, "0x%p", lua_topointer(L, i));
#endif // WIN32
				buf += lua_typename(L, t) + string(": ") + string(char_buf);
			}

		}

	}

	sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_SCRIPT_EV, buf.c_str());

	lua_pop(L, num);	// Стек:
	return 0;
}

//////////////////////////////////////////////////////////////////////////

// Задаёт коэффициент отставания камеры
int scriptApi::SetCamLag(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "CAMERA_LAG expected!");

	float k = (float)lua_tonumber(L, 1);

	CameraSetLag(k);

	return 0;
}

// Фоксирует камеру на объекте. Объект задается по его id
int scriptApi::SetCamAttachedObj(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Object id expected");

	UINT id = lua_tointeger(L, 1);

	GameObject* obj = NULL;
	if (id != 0)
	{
		obj = GetGameObject(id);
	}

	CameraAttachToObject(obj);

	return 0;
}

// Камера будет смещаться только по заданным осям, следя за объектом
int scriptApi::SetCamAttachedAxis(lua_State* L)
{
	luaL_argcheck(L, lua_isboolean(L, 1), 1, "Boolean expected");
	luaL_argcheck(L, lua_isboolean(L, 2), 1, "Boolean expected");

	CameraAttachToAxis(lua_toboolean(L, 1) != 0, lua_toboolean(L, 2) != 0);

	return 0;
}

// Фокусирует камеру на точке, заданной координатами
int scriptApi::CamMoveToPos(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Number expected");
	luaL_argcheck(L, lua_isnumber(L, 2), 1, "Number expected");

	CameraMoveToPos((float)lua_tonumber(L, 1), (float)lua_tonumber(L, 2));

	return 0;
}


// ЗАдает смещение камеры относительно объекта
int scriptApi::SetCamObjOffset(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Number expected");
	luaL_argcheck(L, lua_isnumber(L, 2), 1, "Number expected");

	SetCameraAttachedObjectOffset((float)lua_tonumber(L, 1), (float)lua_tonumber(L, 2));

	return 0;
}


// Задает току объекта, на котрой фокусируется камера
int scriptApi::SetCamFocusOnObjPos(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Number expected");

	CAMERA_FOCUS_ON_OBJ_POS = (CameraFocusObjectPoint)lua_tointeger(L, 1);

	return 0;
}

int scriptApi::SetCamUseBounds(lua_State* L)
{
	luaL_argcheck(L, lua_isboolean(L, 1), 1, "Boolean expected");
	::CameraUseBounds(lua_toboolean(L,1) != 0);
	return 0;
}

int scriptApi::SetCamBounds(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Number expected");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Number expected");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Number expected");
	luaL_argcheck(L, lua_isnumber(L, 4), 4, "Number expected");

	::CameraSetBounds((float)lua_tonumber(L, 1), (float)lua_tonumber(L, 2),
		(float)lua_tonumber(L, 3), (float)lua_tonumber(L, 4));
	return 0;
}

int scriptApi::GetCamPos(lua_State* L)
{
	lua_pushnumber(L, CAMERA_X);
	lua_pushnumber(L, CAMERA_Y);

	return 2;
}

int scriptApi::GetCamFocusShift(lua_State* L)
{
	float x = 0, y = 0;
	GetCameraAttachedFocusShift(x,y);

	lua_pushnumber(L, x);
	lua_pushnumber(L, y);
	return 2;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int scriptApi::CreateRibbon(lua_State* L)
{
	CHECKGAME;

	luaL_argcheck(L, lua_isstring(L, 1), 1, "Ожидается имя прототипа");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается координата X");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Ожидается координата Y");
	luaL_argcheck(L, lua_isnumber(L, 4), 4, "Ожидается коэффициент скорости по x");
	luaL_argcheck(L, lua_isnumber(L, 5), 5, "Ожидается коэффициент скорости по y");
	const char* proto_name = lua_tostring(L, 1);
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	float fact = (float)lua_tonumber(L, 4);
	float fact_y = (float)lua_tonumber(L, 5);
	lua_pop(L, lua_gettop ( L ));	// Стек:

	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Cоздаем Ribbon: %s", proto_name);
	Ribbon* r = NULL;
	r = ::CreateRibbon(proto_name, Vector2(x, y), fact, fact_y);
	if (r)
	{

		lua_pushnumber(L, r->id);	// Стек: r->id
	}
	else
	{
		lua_pushnil(L);				// Стек: nil
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Ошибка cоздания спрайта");
	}

	return 1;
}

int scriptApi::SetRibbonZ(lua_State* L)
{
	CHECKGAME;

	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Номер ленты");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается координата Z");

	Ribbon* r = GetRibbon(lua_tointeger(L,1));
	if (r && r->sprite)
	{
		r->sprite->z = (float)lua_tonumber(L, 2);
	}
	return 0;
}

int scriptApi::SetRibbonAttatachToY(lua_State* L)
{
	CHECKGAME;

	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Номер ленты");
	luaL_argcheck(L, lua_isboolean(L, 2), 2, "bool");

	Ribbon* r = GetRibbon(lua_tointeger(L,1));
	if (r)
	{
		r->y_attached = lua_toboolean(L, 2) != 0;
	}
	return 0;
}

int scriptApi::SetRibbonBounds(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Номер ленты");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается координата X1 границ");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Ожидается координата Y1 границ");
	luaL_argcheck(L, lua_isnumber(L, 4), 4, "Ожидается координата X2 границ");
	luaL_argcheck(L, lua_isnumber(L, 5), 5, "Ожидается координата Y2 границ");

	Ribbon* r = GetRibbon(lua_tointeger(L,1));
	if (r)
	{
		r->SetBounds((float)lua_tonumber(L, 2), (float)lua_tonumber(L, 3),
			(float)lua_tonumber(L, 4), (float)lua_tonumber(L, 5));
	}
	return 0;
}




//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int scriptApi::PlaySnd(lua_State* L)
{
	luaL_argcheck(L, lua_isstring(L, 1), 1, "Ожидается имя звука");
	luaL_argcheck(L, lua_isboolean(L, 2), 2, "Ожидается bool");
	luaL_argcheck(L, lua_isnumber(L, 3)||lua_isnil(L, 3)||lua_isnone(L, 3), 2, "Ожидается x");
	luaL_argcheck(L, lua_isnumber(L, 4)||lua_isnil(L, 4)||lua_isnone(L, 4), 2, "Ожидается y");
	const char* sound_name = lua_tostring(L, 1);
	bool restart = lua_toboolean(L, 2) != 0;
	if (soundMgr)
	{
		if ( lua_isnumber(L, 3) && lua_isnumber(L, 4)  )
			soundMgr->PlaySnd(string(sound_name), restart, Vector2( (scalar)lua_tonumber(L, 3), (scalar)lua_tonumber(L, 4) ));
		else
			soundMgr->PlaySnd(string(sound_name), restart);
	}
	return 0;
}

int scriptApi::PauseSnd(lua_State* L)
{
	luaL_argcheck(L, lua_isstring(L, 1), 1, "Ожидается имя звука");
	const char* sound_name = lua_tostring(L, 1);
	if (soundMgr)
		soundMgr->PauseSnd(string(sound_name));
	return 0;
}

int scriptApi::StopSnd(lua_State* L)
{
	luaL_argcheck(L, lua_isstring(L, 1), 1, "Ожидается имя звука");
	const char* sound_name = lua_tostring(L, 1);
	if (soundMgr)
		soundMgr->StopSnd(string(sound_name));
	return 0;
}

int scriptApi::StopAllSnd(lua_State* L)
{
	UNUSED_ARG(L);
	if (soundMgr)
		soundMgr->StopAll();
	return 0;
}

int scriptApi::GetMasterSoundVolume(lua_State* L)
{
	if (soundMgr)
		lua_pushnumber(L, soundMgr->GetVolume());
	else
		lua_pushnumber(L, 0);
	return 1;
}

int scriptApi::SetMasterSoundVolume(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается уровень громкости");
	if (soundMgr)
		soundMgr->SetVolume((float)lua_tonumber(L, 1));
	return 0;
}

int scriptApi::GetMasterSFXVolume(lua_State* L)
{
	if (soundMgr)
		lua_pushnumber(L, soundMgr->GetSoundVolume());
	else
		lua_pushnumber(L, 0);
	return 1;
}

int scriptApi::SetMasterSFXVolume(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается уровень громкости");
	if (soundMgr)
		soundMgr->SetSoundVolume((float)lua_tonumber(L, 1));
	return 0;
}

int scriptApi::GetMasterMusicVolume(lua_State* L)
{
	if (soundMgr)
		lua_pushnumber(L, soundMgr->GetMusicVolume());
	else
		lua_pushnumber(L, 0);
	return 1;
}

int scriptApi::SetMasterMusicVolume(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается уровень громкости");
	if (soundMgr)
		soundMgr->SetMusicVolume((float)lua_tonumber(L, 1));
	return 0;
}

int scriptApi::GetSoundVolume(lua_State* L)
{
	luaL_argcheck(L, lua_isstring(L, 1), 1, "Ожидается имя звука");
	if (soundMgr)
		lua_pushnumber(L, soundMgr->GetSndVolume(string(lua_tostring(L, 1))) );
	else
		lua_pushnumber(L, 0);
	return 1;
}

int scriptApi::SetSoundVolume(lua_State* L)
{
	luaL_argcheck(L, lua_isstring(L, 1), 1, "Ожидается имя звука");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается уровень громкости");
	if (soundMgr)
		soundMgr->SetSndVolume(string(lua_tostring(L, 1)), (float)lua_tonumber(L, 2));
	return 0;
}

int scriptApi::PlayBackMusic(lua_State* L)
{
	luaL_argcheck(L, lua_isstring(L, 1), 1, "Ожидается имя звука");
	if (soundMgr)
		soundMgr->PlayBackMusic(string(lua_tostring(L, 1)));
	return 0;

}

int scriptApi::PauseBackMusic(lua_State* L)
{
	UNUSED_ARG(L);
	if (soundMgr)
		soundMgr->PauseBackMusic();
	return 0;
}

int scriptApi::StopBackMusic(lua_State* L)
{
	UNUSED_ARG(L);
	if (soundMgr)
		soundMgr->StopBackMusic();
	return 0;
}

void scriptApi::MapVarAdd( lua_State* L, char const * var_name, int amount )
{
	lua_getglobal( L, "mapvar" );
	bool nt = false;
	if ( !lua_istable(L, -1) )
	{
		lua_newtable(L);
		nt = true;
	}
	{
		int oldValue = 0;
		if ( !nt )
		{
			lua_getfield(L, -1, var_name);
			oldValue = lua_tointeger(L, -1);
		}
		lua_pushinteger(L, oldValue + amount);
		if (nt) lua_setfield(L, -2, var_name);
		else
		{
			lua_setfield(L, -3, var_name);
			lua_pop(L, 1);
		}
	}
	lua_setglobal( L, "mapvar" );
	lua_getglobal( L, "varchange" );
	if ( lua_isfunction(L, -1) )
	{
		lua_pushstring( L, var_name );
		lua_pushinteger( L, amount );
		SCRIPT::ExecChunk(2);;
	}
}
/////////////////////////////////////////////////////////////

int scriptApi::IsConfKeyPressed(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается номер кнопки в конфиге (из config_keys)");
	lua_pushboolean(L, inpmgr.IsPressed( ((ConfigActionKeys)lua_tointeger(L, 1)) ) );
	return 1;
}

int scriptApi::IsConfKeyHolded(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается номер кнопки в конфиге (из config_keys)");
	lua_pushboolean(L, inpmgr.IsHolded( ((ConfigActionKeys)lua_tointeger(L, 1)) ) );
	return 1;
}

int scriptApi::IsConfKeyReleased(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается номер кнопки в конфиге (из config_keys)");
	lua_pushboolean(L, inpmgr.IsReleased( ((ConfigActionKeys)lua_tointeger(L, 1)) ) );
	return 1;
}

int scriptApi::IsConfKey(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается кнопка");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается номер кнопки в конфиге (из config_keys)");
	bool is_key = false;
	UINT key = (UINT)lua_tointeger(L, 1);
	ConfigActionKeys cak = (ConfigActionKeys)lua_tointeger(L, 2);
	for (size_t i = 0; i < KEY_SETS_NUMBER; i++)
	{
		is_key = is_key || (cfg.cfg_keys[i][cak] == key);
	}
	lua_pushboolean(L, is_key);
	return 1;
}

int scriptApi::PushButton(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 2, "Ожидается кнопка");
	UINT keynum = (UINT)lua_tointeger(L, 1);
	lua_pushboolean(L, (int)inpmgr.IsHolded( keynum ));
	return 1;
}

int scriptApi::TogglePause(lua_State* L)
{
	luaL_argcheck(L, lua_isboolean(L,1)||lua_isnone(L,1)||lua_isnil(L,1), 1, "Ожидается состояние или ничего.");
	if ( lua_isboolean(L,1) )
			game::TogglePause( lua_toboolean(L,1) != 0 );
	else
			game::TogglePause();
	return 0;
}

int scriptApi::DumpKeys(lua_State* L)
{
	for ( UINT i = 0; i < inpmgr.keys_count; i++ )
		lua_pushboolean(L, inpmgr.IsHolded( i ));

	return inpmgr.keys_count;
}

int scriptApi::PushKeyName(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается кнопка");
	int keynum = lua_tointeger(L, 1);
	char ch[MAX_VKEY_NAME_LEN] = "";
	GetVKeyByNum(keynum, ch);
	lua_pushstring(L, ch);
	return 1;
}

int scriptApi::GetField(lua_State* L)
{
	luaL_argcheck(L, lua_istable(L, 1), 1, "Ожидается таблица");
	luaL_argcheck(L, lua_isstring(L, 2), 2,"Ожидается название поля");
	//lua_gettable(L, 1);
	lua_getfield(L, 1, lua_tostring(L, 2));
	return 1;
}

int scriptApi::SetField(lua_State* L)
{
	luaL_argcheck(L, lua_istable(L, 1), 1, "Ожидается таблица");
	luaL_argcheck(L, lua_isstring(L, 2), 2, "Ожидается название поля");
	lua_pushvalue(L, 3);
	lua_setfield(L, 1, lua_tostring(L, 2));
	return 0;
}

int scriptApi::GetFirstButtonPressed(lua_State* L)
{
	for ( UINT i = 0; i < inpmgr.keys_count; i++ )
	{
		if (inpmgr.IsHolded(i))
		{
			lua_pushinteger(L, i);
			return 1;
		}
	}
	lua_pushnil(L);
	return 1;
}

int scriptApi::GamePaused(lua_State* L)
{
	lua_pushboolean( L, game::GetPause() );
	return 1;
}

extern float CAMERA_X;
extern float CAMERA_Y;

int scriptApi::PushScreenInfo(lua_State* L)
{
	lua_newtable(L);
	lua_pushnumber(L, CAMERA_X);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, CAMERA_Y);
	lua_setfield(L, -2, "y");
	return 1;
}

//////////////////////////////////////////////////////////////////////////

extern UINT internal_time;
int scriptApi::GetCurTime(lua_State* L)
{
	lua_pushinteger(L, internal_time);
	return 1;
}

extern float CAMERA_LEFT;
extern float CAMERA_TOP;

int scriptApi::GetMousePos(lua_State* L)
{
	lua_pushnumber(L, CAMERA_LEFT + inpmgr.mouseX);
	lua_pushnumber(L, CAMERA_TOP + inpmgr.mouseY);

	return 2;
}

//////////////////////////////////////////////////////////////////////////


int scriptApi::EditorGetObjects(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается x1");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается y1");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Ожидается x2");
	luaL_argcheck(L, lua_isnumber(L, 4), 4, "Ожидается y2");
	
	float x1 = (float)lua_tonumber(L, 1);
	float y1 = (float)lua_tonumber(L, 2);
	float x2 = (float)lua_tonumber(L, 3);
	float y2 = (float)lua_tonumber(L, 4);


	if (x1 == x2) x2 = x1 + 0.1f;
	if (y1 == y2) y2 = y1 + 0.1f;
	
	EditorGetObjects(CAABB(x1, y1, x2, y2));


	return 0;
}


int scriptApi::RegEditorGetObjectsProc(lua_State* L)
{
	luaL_argcheck(L, lua_isfunction(L, 1), 1, "Ожидается proc");
	EditorRegAreaSelectProc(L);
	return 0;
}

#ifdef MAP_EDITOR
extern bool editor_ShowBorders;
#endif //MAP_EDITOR

int scriptApi::EditorToggleBorders(lua_State* L)
{
#ifdef MAP_EDITOR
	luaL_argcheck(L, lua_isboolean(L, 1), 1, "Ожидается значение");
	editor_ShowBorders = lua_toboolean(L, 1);
#else
	UNUSED_ARG(L);
#endif //MAP_EDITOR
	return 0;
}

//////////////////////////////////////////////////////////////////////////

int scriptApi::ListDirContents(lua_State* L)
{
	luaL_argcheck(L, lua_isstring(L, 1), 1, "Ожидается путь");

	const char* path = lua_tostring(L, 1);
	char* root_path = new char[ strlen(path_app) + 1 + strlen(path) + 1 + 1 ];
	sprintf(root_path, "%s/%s/", path_app, path);

	DIR *dp;
	struct dirent *ep;
	
	list<string> dirs;
	list<string> files;

	dp = opendir(root_path);
	if (dp == NULL)
	{
		lua_pushnil(L);
		sLog(DEFAULT_SCRIPT_LOG_NAME, logLevelWarning, "Ошибка ListDirContents. Невозможно открыть папку.");
		return 1;
	}
	
	while ( (ep = readdir(dp)) != NULL )
	{
		if (ep->d_name[0] == '.')
			continue;

		if (ep->d_type == DT_DIR)
		{
			dirs.push_back(string(ep->d_name));
		}
		else
		{
			files.push_back(string(ep->d_name));
		}
	}
	closedir(dp);
	DELETEARRAY(root_path);


	UINT dirs_count = (UINT)dirs.size();
	lua_newtable(L);
	list<string>::iterator it;
	int i = 1;
	for (it = dirs.begin(); it != dirs.end(); it++, i++)
	{
		lua_pushstring(L, (*it).c_str());
		lua_rawseti(L, -2, i);
	}
	for (it = files.begin(); it != files.end(); it++, i++)
	{
		lua_pushstring(L, (*it).c_str());
		lua_rawseti(L, -2, i);
	}

	lua_pushinteger(L, dirs_count);
	return 2;
}

//////////////////////////////////////////////////////////////////////////

int scriptApi::SendHighscores(lua_State* L)
{
	luaL_argcheck(L, lua_isstring(L, 1), 1, "nickname");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "points");
	luaL_argcheck(L, lua_isnumber(L, 2), 3, "seconds");

	//lua_pushboolean(L, true);
	lua_pushboolean(L, hsSendScore(lua_tostring(L, 1), lua_tointeger(L, 2), lua_tointeger(L, 3)));
	return 1;
}
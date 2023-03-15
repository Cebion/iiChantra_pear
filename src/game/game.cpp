#include "StdAfx.h"

#include "../misc.h"

#include "../script/script.h"

#include "phys/phys_misc.h"

#include "../render/texture.h"
#include "animation.h"
#include "sprite.h"
#include "proto.h"
#include "../config.h"
#include "../sound/snd.h"
#include "objects/object.h"
#include "objects/object_sprite.h"
#include "objects/object_environment.h"
#include "objects/object_character.h"
#include "objects/object_ray.h"
#include "player.h"
#include "particle_system.h"

#include "phys/sap/OPC_ArraySAP.h"

Opcode::ArraySAP *asap = NULL;

#include "camera.h"
#include "ribbon.h"

#include "game.h"

#include "../input_mgr.h"

#include "../resource_mgr.h"
#include "net.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

extern char path_levels[MAX_PATH];
extern InputMgr inpmgr;

extern ResourceMgr<Proto> * protoMgr;
extern SoundMgr* soundMgr;

UINT spare_physics_time;		//Время, оставшееся необработанное физикой с прошлого цикла.

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

game::GameStates game_state = game::GAME_NONE;
Player* playerControl = NULL;

UINT last_passive_obj_update_tick = 0;		// Время предыдущего тика обновления пассивных объектов


#ifdef DEBUG_DRAW_COLLISION_PAIRS
list<Vector2> collPairs;
#endif // DEBUG_DRAW_COLLISION_PAIRS

bool isNewGameCreated = false;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Об удалении игры
// При удалении игры, игровые объекты должны быть удалены из SAP и, затем, освобождены.
// Удаление игры происходит при создании новой, при вызове из скрипта или при 
// завершении работы движка.
// 1) Если game::InitGame game::FreeGame вызвана из скрипта, запущенного движком 
// по событию таймера или в процессе выполнения DEFAULT_INIT_SCRIPT, то в таком 
// случае удаление SAP и игровых объектов не несет никакой опасности.
// 2) Если game::InitGame game::FreeGame вызвана из скрипта, запущенного при 
// обработке объекта(скрипт объекта или команда анимации, вызывающая код lua), то
// нельзя просто удалить SAP и объекты, так как будет испорчен стек вызовов.
// Поэтому, объекты в FreeGame подготавливаются для удаления, а затем, потом 
// после полного создания новой игры в game::UpdateGame вызывается RemoveAllObjects, 
// в которой происходит полное удаление.

// TODO: сделать нормальное удаление игры из скрипта, не создавая новую игру. См. scriptApi::DestroyGame

void game::FreeGame(bool fullDestroy)
{
	game_state = game::GAME_DESTROYING;
	ClearFactions();
	// Тут внутри мы делаем asap = NULL, и удаляем те обхекты, которые не могли 
	// обрабатываться в данный момент.
	PrepareRemoveAllObjects();
	if (fullDestroy)
	{
		// Можно делать полное удаление.
		RemoveAllObjects();
		Net::Close();
	}

	DELETESINGLE(playerControl);
	
	DeleteAllRibbons();

	assert(protoMgr);
	protoMgr->UnloadAllUnUsed();

	game_state = game::GAME_NONE;
}

extern config cfg;

void game::InitGame()
{
	FreeGame(false);

	DefaultEnvSet( NULL );

	asap = new Opcode::ArraySAP();

	game_state = game::GAME_INITIALIZING;
	spare_physics_time = 0;

	CameraMoveToPos(0,0);

	playerControl = new Player;

	game_state = game::GAME_RUNNNIG;

	SetParticlesWind( Vector2(0, 0) );

	isNewGameCreated = true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////



void game::UpdateGame(UINT dt)
{
	if (isNewGameCreated)
	{
		// См. выше, почему тут вызывается RemoveAllObjects
		RemoveAllObjects(); 
		isNewGameCreated = false;
	}

	if (game_state == GAME_RUNNNIG)
	{
		playerControl->Update();

		dt += spare_physics_time;
#ifdef USE_TIME_DEPENDENT_PHYS
		while (dt >= TARGET_GAMETICK && !isNewGameCreated)
		{
			CameraUpdateFocus();
			ProcessAllActiveObjects();
			dt -= TARGET_GAMETICK;
		}
#endif // USE_TIME_DEPENDENT_PHYS
		spare_physics_time = dt;


		if (internal_time - last_passive_obj_update_tick >= GAMETICK && !isNewGameCreated)
		{
			ProcessAllPassiveObjects();

			last_passive_obj_update_tick = internal_time;
		}
	}
	else if ( game_state == GAME_PAUSED )
	{
		//UpdatePhysTime();
	}

	inpmgr.ProlongedArrayFlush();
}

void game::TogglePause()
{
		if ( game_state == GAME_RUNNNIG) game_state = GAME_PAUSED;
		else if ( game_state == GAME_PAUSED ) game_state = GAME_RUNNNIG;
}

void game::TogglePause( bool state )
{
		if ( game_state != GAME_RUNNNIG && game_state != GAME_PAUSED )
			return;
		if ( state ) game_state = GAME_PAUSED;
		else game_state = GAME_RUNNNIG;
}

bool game::GetPause()
{
	return ( game_state == GAME_PAUSED );
}

void game::DrawGame()
{
	if (game_state == GAME_RUNNNIG || game_state == GAME_PAUSED)
	{
		CameraUpdatePosition();

		DrawRibbons();

		DrawAllObjects();


#ifdef DEBUG_DRAW_COLLISION_PAIRS
		for (list<Vector2>::const_iterator it = collPairs.begin(); it != collPairs.end(); it++)
		{
			const Vector2& p1 = *it;
			it++;
			const Vector2& p2 = *it;
			RenderLine(p1.x, p1.y, p2.x, p2.y, 1.0f, RGBAf(DEBUG_COLLISION_PAIRS_COLOR) );
		}
#endif // DEBUG_DRAW_COLLISION_PAIRS
	}
}


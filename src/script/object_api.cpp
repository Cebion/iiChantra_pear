#include "StdAfx.h"

#include "../misc.h"

#include "../game/player.h"
#include "../game/objects/object.h"
#include "../game/objects/object_enemy.h"
#include "../game/objects/object_environment.h"
#include "../game/objects/object_sprite.h"
#include "../game/objects/object_effect.h"
#include "../game/objects/object_waypoint.h"
#include "../game/objects/object_spawner.h"
#include "../game/objects/object_item.h"
#include "../game/objects/object_particle_system.h"
#include "../game/objects/object_ray.h"
#include "../game/objects/object_ribbon.h"

#include "../game/phys/phys_collisionsolver.h"

#include "../game/sprite.h"
#include "../render/texture.h"

#include "api.h"
#include "luathread.h"

//////////////////////////////////////////////////////////////////////////
extern Player* playerControl;

extern lua_State* lua;
//////////////////////////////////////////////////////////////////////////


void PushSpawner(lua_State* L, ObjSpawner* o);
void PushPhysic(lua_State* L, ObjPhysic* o);
void PushDynamic(lua_State* L, ObjDynamic* o);
void PushCharacter(lua_State* L, ObjCharacter* o);
void PushPlayer(lua_State* L, ObjPlayer* o);
void PushRibbon(lua_State* L, ObjRibbon* o);
//////////////////////////////////////////////////////////////////////////



// Помещает в стек таблицу, представлющую игровой объект
void PushObject(lua_State* L, GameObject* o)
{
	lua_newtable(L);	// Стек: obj

	lua_pushinteger(L, o->id);		lua_setfield(L, -2, "id");
	lua_pushinteger(L, o->type);	lua_setfield(L, -2, "type");
	SCRIPT::PushAABB(L, o->aabb);	lua_setfield(L, -2, "aabb");

	lua_newtable(L);	// Стек: obj flags
	lua_pushboolean(L, o->IsPhysic());	lua_setfield(L, -2, "physic");
	lua_pushboolean(L, o->IsDead());	lua_setfield(L, -2, "dead");
	lua_setfield(L, -2, "flags");	// Стек: obj

#ifdef MAP_EDITOR
	lua_pushstring(L, o->proto_name);	lua_setfield(L, -2, "proto");
	lua_pushboolean(L, o->grouped);		lua_setfield(L, -2, "group");
	SCRIPT::PushVector(L, o->creation_shift);	lua_setfield(L, -2, "creation_shift");
#endif //MAP_EDITOR

	if(o->sprite)
	{
		lua_newtable(L);	// Стек: obj sprite
		lua_pushboolean(L, o->sprite->IsMirrored());	lua_setfield(L, -2, "mirrored");
		lua_pushboolean(L, o->sprite->IsFixed());		lua_setfield(L, -2, "fixed");
		lua_pushboolean(L, o->sprite->IsVisible());		lua_setfield(L, -2, "visible");
		lua_pushnumber(L, o->sprite->z);				lua_setfield(L, -2, "z");
		lua_pushboolean(L, o->sprite->IsAnimDone());	lua_setfield(L, -2, "animDone");
		lua_pushinteger(L, o->sprite->animsCount);		lua_setfield(L, -2, "animsCount");
		lua_pushstring(L, o->sprite->cur_anim.c_str()); lua_setfield(L, -2, "cur_anim");
		Animation* a = o->sprite->GetAnimation(o->sprite->cur_anim_num);
		lua_pushinteger(L, (a ? a->current : -1));		lua_setfield(L, -2, "currentFrame");
		lua_pushinteger(L, o->sprite->currentFrame);	lua_setfield(L, -2, "frame");
		lua_pushinteger(L, o->sprite->frameWidth);		lua_setfield(L, -2, "frameWidth");
		lua_pushinteger(L, o->sprite->frameHeight);		lua_setfield(L, -2, "frameHeight");
		lua_newtable(L);	//Стек: obj sprite color
		lua_pushnumber(L, o->sprite->color.r);			lua_rawseti(L, -2, 1);
		lua_pushnumber(L, o->sprite->color.g);			lua_rawseti(L, -2, 2);
		lua_pushnumber(L, o->sprite->color.b);			lua_rawseti(L, -2, 3);
		lua_pushnumber(L, o->sprite->color.a);			lua_rawseti(L, -2, 4);
		lua_setfield(L, -2, "color");  // Стек: obj sprite
#ifdef MAP_EDITOR
		if (o->sprite->tex != NULL)
		{
			lua_pushinteger(L, o->sprite->tex->framesCount);	lua_setfield(L, -2, "frames");
		}
#endif //MAP_EDITOR
		lua_setfield(L, -2, "sprite");	// Стек: obj
	}

	if(o->IsPhysic())
		PushPhysic(L, (ObjPhysic*)o);

	if (o->type == objPlayer)
		PushPlayer(L, (ObjPlayer*)o);

	if (o->type == objSpawner)
		PushSpawner(L, (ObjSpawner*)o);

	if (o->type == objRibbon)
		PushRibbon(L, (ObjRibbon*)o);

	// Стек: obj
}

void PushSpawner(lua_State* L, ObjSpawner* o)
{
	// Стек: obj
	lua_pushinteger(L, o->maximumEnemies);	lua_setfield(L, -2, "maximumEnemies");
	lua_pushinteger(L, o->enemySpawnDelay);	lua_setfield(L, -2, "enemySpawnDelay");
	lua_pushinteger(L, o->size);			lua_setfield(L, -2, "enemySize");
	lua_pushstring(L, o->enemyType);		lua_setfield(L, -2, "enemyType");
	lua_pushinteger(L, o->respawn_dist);	lua_setfield(L, -2, "respawn_dist");
	lua_pushinteger(L, o->direction);		lua_setfield(L, -2, "direction");
}

void PushRibbon(lua_State* L, ObjRibbon* o)
{
	// Стек: obj
	lua_pushnumber(L, o->bl);	lua_setfield(L, -2, "bl");
	lua_pushnumber(L, o->br);	lua_setfield(L, -2, "br");
	lua_pushnumber(L, o->bt);	lua_setfield(L, -2, "bt");
	lua_pushnumber(L, o->bb);	lua_setfield(L, -2, "bb");
	SCRIPT::PushVector(L, o->k);	lua_setfield(L, -2, "k");
	lua_pushboolean(L, o->ubl);	lua_setfield(L, -2, "ubl");
	lua_pushboolean(L, o->ubr);	lua_setfield(L, -2, "ubr");
	lua_pushboolean(L, o->ubt);	lua_setfield(L, -2, "ubt");
	lua_pushboolean(L, o->ubb);	lua_setfield(L, -2, "ubb");
	lua_pushboolean(L, o->repeat_x);	lua_setfield(L, -2, "repeat_x");
	lua_pushboolean(L, o->repeat_y);	lua_setfield(L, -2, "repeat_y");
#ifdef MAP_EDITOR
	lua_pushboolean(L, o->from_proto);	lua_setfield(L, -2, "from_proto");
#endif //MAP_EDITOR
}

// Добавляет в таблице на вершине стека поля физического игрового объекта
void PushPhysic(lua_State* L, ObjPhysic* o)
{
	// Стек: obj
	lua_newtable(L);	// Стек: obj phFlags
	lua_pushboolean(L, o->IsSolid());	lua_setfield(L, -2, "solid");
	lua_pushboolean(L, o->IsBulletCollidable());	lua_setfield(L, -2, "bulletCollideable");
	lua_pushboolean(L, o->IsDynamic());	lua_setfield(L, -2, "dynamic");
	lua_setfield(L, -2, "phFlags");	// Стек: obj

	if (o->IsDynamic())
		PushDynamic(L, (ObjDynamic*)o);
}

// Добавляет в таблице на вершине стека поля динамического физического игрового объекта
void PushDynamic(lua_State* L, ObjDynamic* o)
{
	// Стек: obj
	SCRIPT::PushVector(L, o->acc);				lua_setfield(L, -2, "acc");
	SCRIPT::PushVector(L, o->vel);				lua_setfield(L, -2, "vel");
	lua_pushnumber(L, o->walk_acc);				lua_setfield(L, -2, "walk_acc");
	SCRIPT::PushVector(L, o->gravity);			lua_setfield(L, -2, "gravity");
	lua_pushboolean(L, o->IsOnPlane()); lua_setfield(L, -2, "onPlane");
	lua_pushnumber(L, o->jump_vel);			lua_setfield(L, -2, "jump_vel");
	lua_pushnumber(L, o->max_x_vel);			lua_setfield(L, -2, "max_x_vel");
	lua_pushnumber(L, o->max_y_vel);			lua_setfield(L, -2, "max_y_vel");
	lua_pushboolean(L, o->movementDirectionX);	lua_setfield(L, -2, "movementDirectionX");

	lua_pushinteger(L, o->movement);			lua_setfield(L, -2, "movement");
	lua_pushinteger(L, o->activity);			lua_setfield(L, -2, "activity");
}

// Добавляет в таблице на вершине стека поля игрового объекта персонажа
void PushCharacter(lua_State* L, ObjCharacter* o)
{
	// Стек: obj
	lua_pushinteger(L, o->health);			lua_setfield(L, -2, "health");
	lua_pushinteger(L, o->gunDirection);	lua_setfield(L, -2, "gunDirection");
	lua_pushboolean(L, o->is_invincible);	lua_setfield(L, -2, "isInvincible");
}

// Добавляет в таблице на вершине стека поля игрового объекта игрока
void PushPlayer(lua_State* L, ObjPlayer* o)
{
	// Стек: obj
	PushCharacter(L, o);
	lua_pushboolean(L, o->controlEnabled);	lua_setfield(L, -2, "controlEnabled");
	lua_pushinteger(L, o->ammo);
	lua_setfield(L, -2, "ammo");
	if ( o->weapon )
	{
		lua_pushboolean(L, o->cur_weapon == o->alt_weapon);
		lua_setfield(L, -2, "altWeaponActive");
		lua_pushboolean(L, playerControl->current == o);
		lua_setfield(L, -2, "active");
		if (o->alt_weapon) lua_pushstring(L, o->alt_weapon->bullet_proto->name.c_str());
		else lua_pushstring(L, "none");
		lua_setfield(L, -2, "altWeaponName");
	}
	else
	{
		lua_pushboolean(L, false);
		lua_setfield(L, -2, "altWeaponActive");
		lua_pushstring(L, "none");
		lua_setfield(L, -2, "altWeaponName");
	}
}

//////////////////////////////////////////////////////////////////////////

// Возвращает таблицу, предствавлющую собой объект игрока
int scriptApi::GetPlayer(lua_State* L)
{
	// TODO: CHECKGAME здесь нужен. Просто переписывать нормально loader.lua долго.
	// Костыли во все поля.
	//CHECKGAME;

	luaL_argcheck(L, lua_isnumber(L, 1) || lua_isnone(L, 1), 1, "player num or none");

	ObjPlayer* plr = NULL;
	if (playerControl)
	{
		if (lua_isnone(L, 1))
		{
			plr = playerControl->current;
		}
		else 
		{
			plr = playerControl->GetByNum(lua_tointeger(L, 1));
		}
	}

	if (plr)
		PushObject(L, plr);
	else
	{
		lua_pushnil(L);
		//sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка. Игрока нет.");
	}

	return 1;
}

int scriptApi::GetPlayerNum(lua_State* L)
{
	CHECKGAME;

	if (playerControl)
	{
		if ( playerControl->current == playerControl->first )
			lua_pushinteger(L, 1);
		else
			lua_pushinteger(L, 2);
	}
	else
		lua_pushnil(L);

	return 1;
}

// Возвращает таблицу, предствавлющую собой игровой объект
int scriptApi::GetObject(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Object id expected");

	UINT id = lua_tointeger(L, 1);

	GameObject* obj = GetGameObject(id);
	if (obj)
	{
		PushObject(L, obj);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV,
			"GetObject: Ошибка. Объекта с id=%d нет.", id);
		return 0;
	}

	return 1;
}

int scriptApi::GetCharHealth(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "char id");
	GameObject* obj = GetGameObject(lua_tointeger(L, 1));
	if (obj && (obj->type == objPlayer || obj->type == objEnemy))
	{
		lua_pushinteger(L, ((ObjCharacter*)obj)->health);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV,
			"GetCharHealth: объекта с id=%d нет.", lua_tointeger(L, 1));
		lua_pushnil(L);
	}
	return 1;
}

int scriptApi::SetCharHealth(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "char id");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "health");
	GameObject* obj = GetGameObject(lua_tointeger(L, 1));
	if (obj && (obj->type == objPlayer || obj->type == objEnemy))
	{
		((ObjCharacter*)obj)->health = lua_tointeger(L, 2);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV,
			"GetCharHealth: объекта с id=%d нет.", lua_tointeger(L, 1));
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////

int scriptApi::SetObjDead(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Object id expected");
	GameObject* obj = GetGameObject( lua_tointeger(L, 1) );
	if (obj)
	{
		obj->SetDead();
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV,
			"Ошибка SetObjDead. Объекта с таким id нет.");
	}

	return 0;
}


// Изменяет координаты игрового объекта
int scriptApi::SetObjPos(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Object id expected");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "X coord expected");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Y coord expected");
	luaL_argcheck(L, lua_isnumber(L, 4)||lua_isnil(L, 4)||lua_isnone(L, 4), 4, "Z coord expected");
	luaL_argcheck(L, lua_isboolean(L, 5)||lua_isnil(L, 5)||lua_isnone(L, 5), 5, "editor flag expected");

	UINT id = lua_tointeger(L, 1);
	scalar x = (scalar)lua_tonumber(L, 2);
	scalar y = (scalar)lua_tonumber(L, 3);

	GameObject* obj = GetGameObject(id);
	if (obj)
	{
		if ( lua_isboolean(L, 5) && (lua_toboolean(L, 5) != 0) )
		{
			obj->aabb.p.x = x + obj->aabb.W;
			obj->aabb.p.y = y + obj->aabb.H;
		}
		else
		{
			obj->aabb.p.x = x;
			obj->aabb.p.y = y;
		}
		if ( lua_isnumber(L, 4) && obj->sprite )
			obj->sprite->z = (float)lua_tonumber(L, 4);

		// TODO: как-то лучше оно должно быть. Не место здесь для обновления в сап.
#ifndef MAP_EDITOR
		if (obj->IsPhysic())
		{
			if ( !((ObjPhysic*)obj)->IsDynamic() )
			{
				UpdateSAPObject( ((ObjPhysic*)obj)->sap_handle, obj->aabb.GetASAP_AABB() );
			}
		}
#endif //MAP_EDITOR
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV,
			"Ошибка изменения координат объекта. Объекта с таким id нет.");
	}

	return 0;
}

int scriptApi::SetDynObjGravity(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Object id expected");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "X acceleration expected");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Y acceleration expected");

	UINT id = lua_tointeger(L, 1);
	GameObject* obj = GetGameObject(id);
	if (obj && obj->IsPhysic() && ((ObjPhysic*)obj)->IsDynamic())
	{
		((ObjDynamic*)obj)->gravity.x = (float)lua_tonumber(L, 2);
		((ObjDynamic*)obj)->gravity.y = (float)lua_tonumber(L, 3);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV,
			"Ошибка изменения ускорения объекта. Объекта с таким id нет или он не динамический.");
	}

	return 0;
}

// Изменяет ускорение динамического игрового объекта
int scriptApi::SetDynObjAcc(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Object id expected");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "X acceleration expected");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Y acceleration expected");

	UINT id = lua_tointeger(L, 1);
	GameObject* obj = GetGameObject(id);
	if (obj && obj->IsPhysic() && ((ObjPhysic*)obj)->IsDynamic())
	{
		((ObjDynamic*)obj)->acc.x = (float)lua_tonumber(L, 2);
		((ObjDynamic*)obj)->acc.y = (float)lua_tonumber(L, 3);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV,
			"Ошибка изменения ускорения объекта. Объекта с таким id нет или он не динамический.");
	}

	return 0;
}

// Изменяет скорость динамическго игрового объекта
int scriptApi::SetDynObjVel(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Object id expected");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "X acceleration expected");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Y acceleration expected");

	UINT id = lua_tointeger(L, 1);
	GameObject* obj = GetGameObject(id);
	if (obj && obj->IsPhysic() && ((ObjPhysic*)obj)->IsDynamic())
	{
		((ObjDynamic*)obj)->vel.x = (float)lua_tonumber(L, 2);
		((ObjDynamic*)obj)->vel.y = (float)lua_tonumber(L, 3);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV,
			"Ошибка изменения скорости объекта. Объекта с таким id нет или он не динамический.");
	}

	return 0;
}

// Включает/отключает контроль игрока
int scriptApi::EnablePlayerControl(lua_State* L)
{
	CHECKGAME;

	if (playerControl->current)
	{
		luaL_argcheck(L, lua_isboolean(L, 1), 1, "Boolean expected");

		playerControl->current->controlEnabled = lua_toboolean(L, 1) != 0;
	}

	return 0;
}

// Включает/отключает контроль игрока
int scriptApi::GetPlayerControlState(lua_State* L)
{
	CHECKGAME;

	if (playerControl->current)
	{
		lua_pushboolean(L, playerControl->current->controlEnabled);
	}

	return 1;
}

// Устанавливает тип движения динамического игрового объекта
int scriptApi::SetDynObjMovement(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Object id expected");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Movement type expected");

	UINT id = lua_tointeger(L, 1);

	GameObject* obj = GetGameObject(id);
	if (obj && obj->IsPhysic() && ((ObjPhysic*)obj)->IsDynamic())
	{
		((ObjDynamic*)obj)->movement = (ObjectMovementType)lua_tointeger(L, 2);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV,
			"Ошибка изменения типа движения объекта. Объекта с таким id нет или он не динамический.");
	}

	return 0;
}

// Устанавливает направление движения динамического игрового объекта
int scriptApi::SetDynObjMoveDirX(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Object id expected");
	luaL_argcheck(L, lua_isboolean(L, 2), 2, "Movement type expected");

	UINT id = lua_tointeger(L, 1);

	GameObject* obj = GetGameObject(id);
	if (obj && obj->IsPhysic() && ((ObjPhysic*)obj)->IsDynamic())
	{
		((ObjDynamic*)obj)->movementDirectionX = lua_toboolean(L, 2) != 0;
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV,
			"Ошибка изменения направления движения объекта. Объекта с таким id нет или он не динамический.");
	}

	return 0;
}

int scriptApi::SetObjSpriteMirrored(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Object id expected");
	luaL_argcheck(L, lua_isboolean(L, 2), 2, "Boolean expected");

	UINT id = lua_tointeger(L, 1);

	GameObject* obj = GetGameObject(id);
	if (obj)
	{
		if (lua_toboolean(L, 2))
			obj->SetFacing( true );
		else
			obj->SetFacing( false );
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV,
			"Ошибка SetObjSpriteMirrored. Объекта с таким id нет или он не динамический.");
	}

	return 0;
}

int scriptApi::SetObjSpriteColor(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Object id expected");
	luaL_argcheck(L, lua_istable(L, 2), 2, "Color table expected");

	UINT id = lua_tointeger(L, 1);
	RGBAf col; SCRIPT::GetColorFromTable(L, 2, col);

	GameObject* obj = GetGameObject(id);
	if (obj && obj->sprite)
	{
		SCRIPT::GetColorFromTable(L, 2, obj->sprite->color);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV,
			"Ошибка SetObjSpriteColor. Объекта с таким id нет или у него нет спрайта.");
	}

	return 0;
}

int scriptApi::SetObjAnim(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Object id expected");
	luaL_argcheck(L, lua_isstring(L, 2)||lua_isnumber(L, 2), 2, "String or number expected");
	luaL_argcheck(L, lua_isboolean(L, 3), 3, "Boolean expected");

	UINT id = lua_tointeger(L, 1);

	GameObject* obj = GetGameObject(id);
	if (obj)
	{
		if ( lua_isnumber(L, 2) )
		{
			UINT tilenum = lua_tointeger(L, 2);
			obj->sprite->SetCurrentFrame(tilenum);
			obj->sprite->animsCount = 0;
			obj->type = objTile;
			obj->aabb.W = (scalar)obj->sprite->frameWidth / 2.0f;
			obj->aabb.H = (scalar)obj->sprite->frameHeight / 2.0f;
		}
		else
			obj->SetAnimation(string(lua_tostring(L, 2)), lua_toboolean(L, 3) != 0 );
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV,
			"Ошибка SetObjAnim. Объекта с таким id нет или он не динамический.");
	}


	return 0;
}

int scriptApi::SetObjProcessor(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Object id expected");

	UINT id = lua_tointeger(L, 1);

	GameObject* obj = GetGameObject(id);
	if (obj)
	{
		SCRIPT::RegProc(L, &obj->scriptProcess, 2);		
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV,
			"Ошибка изменения обработчика объекта. Объекта с таким id нет или он не динамический.");
	}

	return 0;
}

int scriptApi::SetDynObjJumpVel(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Object id expected");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "jump vel expected");

	GameObject* obj = GetGameObject(lua_tointeger(L, 1));
	if (obj)
	{
		if (obj->IsPhysic() && ((ObjPhysic*)obj)->IsDynamic())
		{
			((ObjDynamic*)obj)->jump_vel = (float)lua_tonumber(L, 2);
		}
	}
	return 0;
}

int scriptApi::SetDynObjShadow(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Object id expected");
	luaL_argcheck(L, lua_isnumber(L, 2) || lua_isnil(L, 2), 2, "width or nil expected");

	GameObject* obj = GetGameObject(lua_tointeger(L, 1));
	if (obj)
	{
		if (obj->IsPhysic() && ((ObjPhysic*)obj)->IsDynamic())
		{
			if (lua_isnil(L, 2))
				((ObjDynamic*)obj)->drops_shadow = false;
			else
			{
				((ObjDynamic*)obj)->drops_shadow = true;
				((ObjDynamic*)obj)->shadow_width = (float)lua_tonumber(L, 2);
			}
		}
	}
	return 0;
}

int scriptApi::GetDynObjShadow(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Object id expected");
	GameObject* obj = GetGameObject(lua_tointeger(L, 1));
	if (obj)
	{
		if (obj->IsPhysic() && ((ObjPhysic*)obj)->IsDynamic())
		{
			if (((ObjDynamic*)obj)->drops_shadow)
			{
				lua_pushnumber(L, ((ObjDynamic*)obj)->shadow_width);
				return 1;
			}
		}
	}
	lua_pushnil(L);
	return 1;
}

int scriptApi::GroupObjects(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Object id expected");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Second object id expected");

	::GroupObjects( lua_tointeger(L, 1), lua_tointeger(L, 2) );

	return 0;
}

int scriptApi::SetRayPos(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Object id expected");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "x0");
	luaL_argcheck(L, lua_isnumber(L, 3), 2, "y0");
	luaL_argcheck(L, lua_isnumber(L, 4), 2, "x1");
	luaL_argcheck(L, lua_isnumber(L, 5), 2, "y1");

	GameObject* obj = GetGameObject(lua_tointeger(L, 1));
	if (obj)
	{
		if (obj->type == objRay)
		{
			((ObjRay*)obj)->ray = CRay(
				(scalar)lua_tonumber(L, 2), (scalar)lua_tonumber(L, 3), 
				(scalar)lua_tonumber(L, 4), (scalar)lua_tonumber(L, 5) );
		}
	}
	return 0;
}

int scriptApi::SetRaySearchDistance(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Object id expected");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Distance expected");

	GameObject* obj = GetGameObject(lua_tointeger(L, 1));
	if (obj)
	{
		if (obj->type == objRay)
		{
			((ObjRay*)obj)->searchDistance = (scalar)lua_tonumber(L, 2);
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int scriptApi::CreateColorBox(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается координата X1");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается координата Y1");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Ожидается координата X2");
	luaL_argcheck(L, lua_isnumber(L, 4), 4, "Ожидается координата Y2");
	luaL_argcheck(L, lua_isnumber(L, 5), 5, "Ожидается координата Z");
	luaL_argcheck(L, lua_istable(L, 6), 6, "Ожидается таблица color");
	
	float x1 = (float)lua_tonumber(L, 1);
	float y1 = (float)lua_tonumber(L, 2);
	float x2 = (float)lua_tonumber(L, 3);
	float y2 = (float)lua_tonumber(L, 4);
	float z = (float)lua_tonumber(L, 5);
	RGBAf col; SCRIPT::GetColorFromTable(L, 6, col);

	lua_pop(L, lua_gettop(L));	// Стек:

	GameObject* obj = ::CreateColorBox(CAABB(x1, y1, x2, y2), z, col);

	lua_pushnumber(L, obj->id);
	return 1;
}

int scriptApi::CreateMap(lua_State* L)
{
	CHECKGAME;

	luaL_argcheck(L, lua_istable(L,1), 1, "Ожидается таблица карты");

	int type = -1;
	float x1, y1, x2, y2, z;
	bool b1, b2, b3, b4;
	UINT count, delay, dir, ssize, sdist;
	int tilenum;
	RGBAf col;
	//UINT num = 1;
	const char* proto_name;
	
	// Стек: table
	const UINT size = lua_objlen(L, 1);

	GameObject* obj1 = NULL;
	GameObject* obj2 = NULL;
	vector<int> secretIDs;

	for (UINT i = 1; i <= size; i++) 
	{
		// Получаем в стеке i-тый элемент массива
		lua_pushnumber(L, i);	// Стек: table i
		lua_gettable(L, -2);	// Стек: table table[i]		

		luaL_argcheck(L, lua_istable(L,1), 1, "Что-то странное в таблицы карты!");
		lua_rawgeti(L, -1, 1);		// Стек: table table[i]	type
		type = lua_tointeger(L, -1);
		lua_pop(L, 1);				// Стек: table table[i]
		switch ( type )
		{
			case otSprite: //Спрайт
					lua_rawgeti(L, -1, 2);				// Стек: table table[i] proto_name
					proto_name = lua_tostring(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 3);				// Стек: table table[i] x1 
					x1 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 4);				// Стек: table table[i] y1
					y1 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);						// Стек: table table[i]
					obj1 = ::CreateSprite(proto_name, Vector2(x1, y1), false, NULL);
					lua_rawgeti(L, -1, 5);
					if ( lua_isnumber(L, -1) && obj1 && obj1->sprite )
					{
						z = (float)lua_tonumber(L, -1);
						obj1->sprite->z = z;
					}
					lua_pop(L, 1);
					break;
			case otTile: //Спрайт
					lua_rawgeti(L, -1, 2);				// Стек: table table[i] proto_name
					proto_name = lua_tostring(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 3);				// Стек: table table[i] x1 
					x1 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 4);				// Стек: table table[i] y1
					y1 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 5);				// Стек: table table[i] tilenum
					tilenum = lua_tointeger(L, -1);
					obj1 = ::CreateSprite(proto_name, Vector2(x1, y1), false, NULL);
					if (obj1 && obj1->sprite)
					{
						obj1->sprite->SetCurrentFrame(tilenum);
						obj1->sprite->animsCount = 0;
						obj1->aabb.W  = (scalar)obj1->sprite->frameWidth / 2.0f;
						obj1->aabb.H  = (scalar)obj1->sprite->frameHeight / 2.0f;
					}
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 6);				// Стек: table table[i] z
					if ( lua_isnumber(L, -1) && obj1 && obj1->sprite )
					{
						z = (float)lua_tonumber(L, -1);
						obj1->sprite->z = z;
					}
					obj1->type = objTile;
					lua_pop(L, 1);						// Стек: table table[i]
					break;
			case otPlayer: //Игрок
					lua_rawgeti(L, -1, 2);
					proto_name = lua_tostring(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 3);
					x1 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 4);
					y1 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					playerControl->Create(proto_name, Vector2(x1, y1), NULL);
					break;
			case otEnemy: //Противник
					lua_rawgeti(L, -1, 2);
					proto_name = lua_tostring(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 3);
					x1 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 4);
					y1 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					obj1 = ::CreateEnemy(proto_name, Vector2(x1, y1), NULL);
					lua_rawgeti(L, -1, 5);
					if ( obj1 && obj1->sprite && lua_isnumber(L, -1) )
						obj1->sprite->z = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					break;
			case otSpawner:
					lua_rawgeti(L, -1, 2);
					proto_name = lua_tostring(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 3);
					x1 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 4);
					y1 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 5);
					count = lua_tointeger(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 6);
					delay = lua_tointeger(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 7);
					dir = lua_tointeger(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 8);
					if ( lua_isnumber(L, -1) )
					{
						ssize = lua_tointeger(L, -1);
						lua_pop(L, 1);
						lua_rawgeti(L, -1, 9);
						if ( lua_isnumber(L, -1) )
							sdist = lua_tointeger(L, -1);
						else
							sdist = ssize;
					}
					else
					{
						ssize = 0;
						sdist = 0;
					}
					lua_pop(L, 1);
					::CreateSpawner( Vector2(x1, y1), proto_name, count, delay, (directionType)dir, ssize, sdist );
					break;
			case otBox: //Коробка
					lua_rawgeti(L, -1, 2);
					x1 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 3);
					y1 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 4);
					x2 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 5);
					y2 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 6);
					z = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 7);
					SCRIPT::GetColorFromTable(L, -1, col);
					lua_pop(L, 1);
					::CreateColorBox(CAABB(x1, y1, x2, y2), z, col);
					break;
			case otGroup: //Группа
				    lua_rawgeti(L, -1, 2);
					proto_name = lua_tostring(L, -1);
					lua_pop(L,1);
					lua_rawgeti(L, -1, 3);
					x1 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 4);
					y1 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 5);
					x2 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 6);
					y2 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					obj1 = ::CreateSprite(proto_name, Vector2(x1, y1), false, NULL);
					obj2 = ::CreateSprite(proto_name, Vector2(x2, y2), false, NULL);
					if ( obj1 && obj2 )
						::GroupObjects(obj1->id, obj2->id);
					break;
			case otItem: //Предмет
					lua_rawgeti(L, -1, 2);
					proto_name = lua_tostring(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 3);
					x1 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 4);
					y1 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					::CreateItem(proto_name, Vector2(x1, y1), "init");
					break;
			case otSecret: //Секретный тайл
					lua_rawgeti(L, -1, 2);				// Стек: table table[i] proto_name
					proto_name = lua_tostring(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 3);				// Стек: table table[i] x1 
					x1 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 4);				// Стек: table table[i] y1
					y1 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);						// Стек: table table[i]
					obj1 = ::CreateSprite(proto_name, Vector2(x1, y1), false, NULL);
					if (obj1) secretIDs.push_back(obj1->id);
					break;
			case otRibbon:
				{
					lua_rawgeti(L, -1, 2);
					proto_name = lua_tostring(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 3);
					x1 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 4);
					y1 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 5);
					x2 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 6);
					y2 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 7);
					z = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 8);
					b1 = lua_toboolean(L, -1) != 0;
					lua_pop(L, 1);
					ObjRibbon* rib = CreateRibbon( proto_name, Vector2(x1, y1), Vector2(x2, y2), z, b1);
					lua_rawgeti(L, -1, 9);
					x1 = 0;
					if ( (b1 = (lua_isnumber(L, -1) != 0)) ) x1 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 10);
					y1 = 0;
					if ( (b2 = (lua_isnumber(L, -1) != 0)) ) y1 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 11);
					x2 = 0;
					if ( (b3 = (lua_isnumber(L, -1) != 0)) ) x2 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 12);
					y2 = 0;
					if ( (b4 = (lua_isnumber(L, -1) != 0)) ) y2 = (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
					rib->setBounds( x1, y1, x2, y2 );
					rib->setBoundsUsage( b1, b2, b3, b4 );
					lua_rawgeti(L, -1, 13);
					b1 = lua_toboolean(L, -1) != 0;
					lua_pop(L, 1);
					lua_rawgeti(L, -1, 14);
					b2 = lua_toboolean(L, -1) != 0;
					lua_pop(L, 1);
					rib->setRepitition( b1, b2 );
					break;
				}
			case otNone: //На этом уровне сложности - ничего
					break;
			default:
					sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Что-то странное в таблице карты (неверный тип объекта)!");
					return 0;
		}

		lua_pop(L,1);		// Стек: table

	}
	if ( secretIDs.size() > 0 )
	{
		lua_newtable(L);
		for ( size_t i = 0; i < secretIDs.size(); i ++ )
		{
			lua_pushinteger(L, secretIDs[i]);
			lua_rawseti(L, -2, i+1);
		}
	}
	else lua_pushnil(L);
	lua_setglobal(L, "secrets");
	return 0;
}

int scriptApi::CreateSprite(lua_State* L)
{
	CHECKGAME;

	luaL_argcheck(L, lua_isstring(L, 1), 1, "Ожидается имя прототипа");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается координата X");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Ожидается координата Y");
	luaL_argcheck(L, lua_isstring(L, 4) || lua_isnil(L, 4) || lua_isnone(L, 4), 4, "Ожидается start_anim");

	int num = lua_gettop ( L );
	bool isFixed = false;
	float z = 0;
	bool override_z = false;
	const char* start_anim = lua_tostring(L, 4);

	if (num == 5)
	{
		if (lua_isboolean(L, 5))
		{
			isFixed = lua_toboolean(L, 5) != 0;
		}
		else if (lua_isnumber(L, 5))
		{
			z = (float)lua_tonumber(L, 5);
			override_z = true;
		}
		else
			luaL_argcheck(L, false, 5, "Boolean or number expected");
	}
	else if (num >= 6)
	{
		luaL_argcheck(L, lua_isnumber(L, 5), 5, "Number expected");
		luaL_argcheck(L, lua_isboolean(L, 6), 6, "Boolean expected");

		z = (float)lua_tonumber(L, 5);
		isFixed = lua_toboolean(L, 6) != 0;
		override_z = true;
	}

	const char* proto_name = lua_tostring(L, 1);
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);

	//lua_pop(L, num);	// Стек: Why?!

	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Cоздаем спрайт: %s", proto_name);

	GameObject* obj = NULL;
	obj = ::CreateSprite(proto_name, Vector2(x, y), isFixed, start_anim);
	if (obj && obj->id)
	{
		if (obj->sprite)
		{
			if (override_z)
				obj->sprite->z = z;
		}

		lua_pushnumber(L, obj->id);	// Стек: obj->id
	}
	else
	{
		lua_pushnil(L);				// Стек: nil
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Ошибка cоздания спрайта");
	}

	return 1;
}

int scriptApi::CreateEffect(lua_State* L)
{
	CHECKGAME;

	luaL_argcheck(L, lua_isstring(L, 1), 1, "Ожидается имя прототипа");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается координата X");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Ожидается координата Y");
	luaL_argcheck(L, lua_isnumber(L, 4), 4, "Ожидается объект-родитель");
	luaL_argcheck(L, lua_isnumber(L, 5), 5, "Ожидается номер точки");

	const char* proto_name = lua_tostring(L, 1);
	//float x = (float)lua_tonumber(L, 2);
	//float y = (float)lua_tonumber(L, 3);
	GameObject* parent = GetGameObject( (UINT)lua_tointeger(L, 4) );
	int num = lua_tointeger(L, 5);

	GameObject* obj = NULL;
	obj = ::CreateEffect(proto_name, false, (ObjDynamic*)parent, num, Vector2(0,0));
	if (obj && obj->id)
	{
		lua_pushnumber(L, obj->id);	// Стек: obj->id
	}
	else
	{
		lua_pushnil(L);				// Стек: nil
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Ошибка cоздания спрайта");
	}

	return 1;
}

int scriptApi::CreateGroup(lua_State* L)
{
	CHECKGAME;

	luaL_argcheck(L, lua_isstring(L, 1), 1, "Ожидается имя прототипа");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается x1");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Ожидается y1");
	luaL_argcheck(L, lua_isnumber(L, 4), 4, "Ожидается x2");
	luaL_argcheck(L, lua_isnumber(L, 5), 5, "Ожидается y2");
	luaL_argcheck(L, lua_isnumber(L, 6), 6, "Ожидается z");
	luaL_argcheck(L, lua_isboolean(L, 7), 7, "Ожидается параметр активности");

	GameObject* obj = NULL;
	float x1 = (float)lua_tonumber(L, 2);
	float y1 = (float)lua_tonumber(L, 3);
	float x2 = (float)lua_tonumber(L, 4);
	float y2 = (float)lua_tonumber(L, 5);
	if ( lua_toboolean(L, 7) == 0 )
		obj = ::CreateSprite(lua_tostring(L, 1), Vector2(0, 0), false, NULL, CAABB(x1, y1, x2, y2));
	else
		obj = ::CreateItem(lua_tostring(L, 1), Vector2(0, 0), NULL, CAABB(x1, y1, x2, y2));
	if (obj)
	{
		if (obj->sprite)
		{
			obj->sprite->z = (float)lua_tonumber(L, 6);
		}

		if (obj->id != 0)
			lua_pushnumber(L, obj->id);	// Стек: obj->id
		else
			lua_pushnil(L);
	}
	else
	{
		lua_pushnil(L);				// Стек: nil
	}

	return 1;
}

int scriptApi::CreateWaypoint(lua_State* L)
{
	CHECKGAME;

	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается координата X");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается координата Y");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Ожидается размер X");
	luaL_argcheck(L, lua_isnumber(L, 4), 4, "Ожидается размер Y");
	luaL_argcheck(L, lua_isfunction(L, 5)|lua_isnil(L,5)|lua_isnone(L,5), 5, "Ожидается событие");
	
	Vector2 coord = Vector2( (float)lua_tonumber(L, 1), (float)lua_tonumber(L, 2) );
	Vector2 size = Vector2( (float)lua_tonumber(L, 3), (float)lua_tonumber(L, 4) );
	
	ObjWaypoint* obj = ::CreateWaypoint( coord, size );
	if (obj)
	{
		if ( lua_isfunction(L, 5) )
			SCRIPT::RegProc(L, &obj->reach_event, 5);
		lua_pushnumber(L, obj->id);
	}
	else
	{
		lua_pushnil(L);	
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Ошибка cоздания путевой точки");
	}

	return 1;
}

int scriptApi::CreateSpecialWaypoint(lua_State* L)
{
	CHECKGAME;


	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается координата X");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается координата Y");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Ожидается размер X");
	luaL_argcheck(L, lua_isnumber(L, 4), 4, "Ожидается размер Y");
	luaL_argcheck(L, lua_isnumber(L, 5) || lua_isnil(L, 5) || lua_isnone(L, 5), 5, "Ожидается номер");
	luaL_argcheck(L, lua_isfunction(L, 6) || lua_isnil(L, 6) || lua_isnone(L, 6), 6, "Ожидается событие");
	
	Vector2 coord = Vector2( (float)lua_tonumber(L, 1), (float)lua_tonumber(L, 2) );
	Vector2 size = Vector2( (float)lua_tonumber(L, 3), (float)lua_tonumber(L, 4) );
	
	ObjWaypoint* obj = ::CreateWaypoint( coord, size );
	if ( obj )
	{
		UINT wp_id;
		if ( lua_isnumber(L, 5) )
		{
			wp_id = lua_tointeger(L, 5);
			::AddWaypoint(obj, wp_id);
		}
		else wp_id = ::AddWaypoint(obj);
		if ( lua_isfunction(L, 6) )
			SCRIPT::RegProc(L, &obj->reach_event, 6);

		lua_pushinteger(L, obj->id);
		lua_pushinteger(L, wp_id);
	}
	else
	{
		lua_pushnil(L);
		lua_pushnil(L);
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Ошибка cоздания путевой точки");
	}

	return 2;
}


int scriptApi::CreateItem(lua_State* L)
{
	CHECKGAME;

	luaL_argcheck(L, lua_isstring(L, 1), 1, "Ожидается имя прототипа");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается координата X");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Ожидается координата Y");
	luaL_argcheck(L, lua_isnumber(L, 4) || lua_isnil(L, 4) || lua_isnone(L, 4)  , 4, "Ожидается start_anim");

	const char* proto_name = lua_tostring(L, 1);
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	const char* start_anim = lua_tostring(L, 4);

	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Cоздаем предмет: %s", proto_name);

	int result = -1;
	GameObject* obj = NULL;

	obj = ::CreateItem(proto_name, Vector2(x, y), start_anim);

	if (obj)
	{
		result = obj->id;
	}
	else
	{
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Ошибка cоздания предмета");
	}

	if (result == -1)
	{
		lua_pushnil(L);				// Стек: nil
	}
	else
	{
		lua_pushnumber(L, result);	// Стек: result
	}
	return 1;
}

int scriptApi::CreateEnvironment(lua_State* L)
{
	CHECKGAME;

	luaL_argcheck(L, lua_isstring(L, 1), 1, "Ожидается имя прототипа");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается координата X");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Ожидается координата Y");

	const char* proto_name = lua_tostring(L, 1);
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);

	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Cоздаем окружение: %s", proto_name);

	int result = -1;
	GameObject* obj = NULL;

	obj = ::CreateEnvironment(proto_name, Vector2(x, y));

	if (obj)
	{
		result = obj->id;
	}
	else
	{
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Ошибка cоздания окружения!");
	}

	if (result == -1)
	{
		lua_pushnil(L);				// Стек: nil
	}
	else
	{
		lua_pushnumber(L, result);	// Стек: result
	}
	return 1;
}

int scriptApi::CreatePlayer(lua_State* L)
{
	CHECKGAME;

	luaL_argcheck(L, lua_isstring(L, 1), 1, "Ожидается имя прототипа");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается координата X");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Ожидается координата Y");
	luaL_argcheck(L, lua_isnumber(L, 4) || lua_isnil(L, 4) || lua_isnone(L, 4)  , 4, "Ожидается start_anim");

	const char* proto_name = lua_tostring(L, 1);
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	const char* start_anim = lua_tostring(L, 4);

	bool override_z = false;
	float z = 0;

	int num = lua_gettop(L);
	if (num == 5)
	{
		luaL_argcheck(L, lua_isnumber(L, 5), 5, "Ожидается координата Z");

		z = (float)lua_tonumber(L, 5);
		override_z = true;
	}


	lua_pop(L, num);

	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Cоздаем игрока: %s", proto_name);

	int result = -1;
	GameObject* obj = NULL;

	obj = playerControl->Create(proto_name, Vector2(x, y), start_anim);

	if (obj)
	{
		if (obj->sprite)
		{
			if (override_z)
				obj->sprite->z = z;
		}

		result = obj->id;
	}
	else
	{
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Ошибка cоздания игрока");
	}

	if (result == -1)
	{
		lua_pushnil(L);				// Стек: nil
	}
	else
	{
		lua_pushnumber(L, result);	// Стек: result
	}
	return 1;
}

int scriptApi::RevivePlayer(lua_State* L)
{
	CHECKGAME;

	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается номер игрока");
	luaL_argcheck(L, lua_isstring(L, 2) || lua_isnil(L, 2) || lua_isnone(L, 2), 2, "Ожидается start_anim");
	
	int num = lua_tointeger(L, 1);
	const char* start_anim = lua_tostring(L, 2);

	lua_pop(L, lua_gettop(L));

	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Оживляем игрока: %d", num);

	int result = -1;
	ObjPlayer* obj = playerControl->Revive(num, start_anim);
	if (obj)
	{
		result = obj->id;
	}
	else
	{
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Ошибка оживления игрока");
	}

	if (result == -1)
	{
		lua_pushnil(L);				// Стек: nil
	}
	else
	{
		lua_pushnumber(L, result);	// Стек: result
	}
	return 1;
}

int scriptApi::GetChildren(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "id expected");
	int num = 1;

	GameObject* obj = GetGameObject( lua_tointeger(L, 1) );
	if ( obj != NULL && obj->childrenConnection != NULL && obj->childrenConnection->children.size() > 0  )
	{
		lua_newtable(L);
		for ( vector<GameObject*>::iterator iter = obj->childrenConnection->children.begin(); iter != obj->childrenConnection->children.end(); ++iter )
		{
				PushObject(L, *iter);
				lua_rawseti(L, -2, num);
				num = num + 1;
		}
	}
	else
		lua_pushnil(L);

	return 1;
}

int scriptApi::CreateEnemy(lua_State* L)
{
	CHECKGAME;

	luaL_argcheck(L, lua_isstring(L, 1), 1, "Ожидается имя прототипа");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается координата X");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Ожидается координата Y");
	luaL_argcheck(L, lua_isstring(L, 4) || lua_isnil(L, 4) || lua_isnone(L, 4)  , 4, "Ожидается start_anim");
	luaL_argcheck(L, lua_isnumber(L, 5) || lua_isnil(L, 5) || lua_isnone(L, 5)  , 4, "Ожидается z");
	luaL_argcheck(L, lua_isnumber(L, 6) || lua_isnil(L, 6) || lua_isnone(L, 6)  , 4, "Ожидается parent");
	
	const char* proto_name = lua_tostring(L, 1);
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	const char* start_anim = lua_tostring(L, 4);

	bool override_z = false;
	float z = 0;

	if (lua_isnumber(L, 5))
	{
		z = (float)lua_tonumber(L, 5);
		override_z = true;
	}

	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Cоздаем противника: %s", proto_name);

	int result = -1;
	GameObject* obj = NULL;

	obj = ::CreateEnemy(proto_name, Vector2(x, y), start_anim);

	if (obj)
	{
		if (obj->sprite)
		{
			if (override_z)
				obj->sprite->z = z;
		}

		result = obj->id;
	}
	else
	{
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Ошибка cоздания противника");
	}

	if (result != -1 && lua_isnumber(L, 6) )
	{
		GameObject* obj2 = GetGameObject( lua_tointeger(L, 6) );
		if ( obj2 != NULL )
			obj2->AddChild( obj );
	}

	if (result == -1)
	{
		lua_pushnil(L);				// Стек: nil
	}
	else
	{
		lua_pushnumber(L, result);	// Стек: result
	}
	return 1;
}

int scriptApi::CreateSpawner(lua_State* L)
{
	CHECKGAME;

	luaL_argcheck(L, lua_isstring(L, 1), 1, "Ожидается имя прототипа противника");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается координата X");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Ожидается координата Y");
	luaL_argcheck(L, lua_isnumber(L, 4), 4, "Ожидается количество противников");
	luaL_argcheck(L, lua_isnumber(L, 5), 5, "Ожидается направление");
	luaL_argcheck(L, lua_isnumber(L, 6), 6, "Ожидается задержка");
	luaL_argcheck(L, lua_isnumber(L, 7) || lua_isnil(L, 7) || lua_isnone(L, 7), 7, "Ожидается размер");
	luaL_argcheck(L, lua_isnumber(L, 8) || lua_isnil(L, 8) || lua_isnone(L, 8), 7, "Ожидается расстояние для сброса");

	const char* proto_name = lua_tostring(L, 1);
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	UINT count = lua_tointeger(L, 4);
	UINT dir = lua_tointeger(L, 5);
	UINT delay = lua_tointeger(L, 6);
	UINT size = 0;
	UINT dist = 0;
	if ( lua_isnumber(L, 7) ) size = lua_tointeger(L, 7);
	if ( lua_isnumber(L, 8) ) dist = lua_tointeger(L, 8);

	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Cоздаем спаунер: %s", proto_name);

	ObjSpawner* obj = NULL;

	obj = ::CreateSpawner( Vector2(x,y), proto_name, count, delay, (directionType)dir, size, dist);

	if (obj)
	{
		lua_pushinteger(L, obj->id);
	}
	else
	{
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Ошибка cоздания спаунера");
		lua_pushnil(L);
	}

	return 1;
}


int scriptApi::CreateParticleSystem(lua_State *L)
{
	CHECKGAME;

	luaL_argcheck(L, lua_isstring(L, 1), 1, "Ожидается имя прототипа");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается координата X");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Ожидается координата Y");

	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	const char* proto_name = lua_tostring(L, 1);

	GameObject* obj = ::CreateParticleSystem( proto_name, Vector2(x, y), NULL, 0 );
	if ( !obj )
		lua_pushnil(L);
	else
		lua_pushinteger(L, obj->id);

	return 1;
}

int scriptApi::AddParticleArea(lua_State* L)
{
	CHECKGAME;

	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Particle system id expected");
	GameObject* ps = GetGameObject( lua_tointeger(L, 1) );
	luaL_argcheck(L, lua_istable(L, 2), 1, "Area table expeced");
	lua_rawgeti(L, 2, 1);
	float x1 = (float)lua_tonumber(L, -1);
	lua_rawgeti(L, 2, 2);
	float y1 = (float)lua_tonumber(L, -1);
	lua_rawgeti(L, 2, 3);
	float x2 = (float)lua_tonumber(L, -1);
	lua_rawgeti(L, 2, 4);
	float y2 = (float)lua_tonumber(L, -1);

	if ( ps->ParticleSystem->area == NULL )
		ps->ParticleSystem->area = new vector<CAABB>;
	if ( ps->ParticleSystem->drop_area == NULL )
		ps->ParticleSystem->drop_area = new vector<CAABB>;
	ps->ParticleSystem->area->push_back( CAABB( x1, y1, x2, y2 ) );
	ps->ParticleSystem->drop_area->push_back( CAABB( x1, y2-30, x2, y2 ) );

	return 0;
}

int scriptApi::SetParticleWind(lua_State* L)
{
	CHECKGAME;

	luaL_argcheck(L, lua_istable(L, 1), 1, "Ожидается табица ветра");
	lua_rawgeti(L, 1, 1);
	float x = (float)lua_tonumber(L, -1);
	lua_rawgeti(L, 1, 2);
	float y = (float)lua_tonumber(L, -1);
	
	SetParticlesWind(Vector2(x, y));

	return 0;
}

int scriptApi::GetParticleWind(lua_State* L)
{
	lua_newtable(L);
	Vector2 wind = GetParticlesWind();
	lua_pushnumber(L, wind.x);
	lua_rawseti(L, -2, 1);
	lua_pushnumber(L, wind.y);
	lua_rawseti(L, -2, 2);
	return 1;
}

int scriptApi::CreateRay(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается координата X1");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается координата Y1");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Ожидается координата X2");
	luaL_argcheck(L, lua_isnumber(L, 4), 4, "Ожидается координата Y2");

	float x1 = (float)lua_tonumber(L, 1);
	float y1 = (float)lua_tonumber(L, 2);
	float x2 = (float)lua_tonumber(L, 3);
	float y2 = (float)lua_tonumber(L, 4);
	
	lua_pop(L, lua_gettop(L));	// Стек:

	ObjRay* obj = ::CreateRay(x1, y1, x2, y2);

	lua_pushnumber(L, obj->id);
	return 1;
}

//////////////////////////////////////////////////////////////////////////

int scriptApi::SetPlayerHealth(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "player num");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "health");

	int num = lua_tointeger(L, 1);
	UINT health = lua_tointeger(L, 2);

	ObjPlayer* plr = playerControl->GetByNum(num);
	if (plr)
	{
		plr->health = health;
	}
	return 0;
}

int scriptApi::DamageObject(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "player id");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "damage amount");

	int num = lua_tointeger(L, 1);
	GameObject* obj = GetGameObject(num);
	if (obj && (obj->type == objPlayer || obj->type == objEnemy))
	{
		((ObjCharacter*)obj)->ReceiveDamage(lua_tointeger(L, 2), 0);
	}
	return 0;
}

int scriptApi::SetPlayerStats(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "player id");
	luaL_argcheck(L, lua_istable(L, 2), 2, "player stats table");

	int num = lua_tointeger(L, 1);
	GameObject* obj = GetGameObject(num);
	if ( !obj || obj->type != objPlayer ) return 0;
	ObjPlayer* plr = (ObjPlayer*)obj;
	lua_getfield(L, 2, "jump_vel");
	plr->jump_vel = (float)lua_tonumber(L, -1);
	lua_getfield(L, 2, "walk_acc");
	plr->walk_acc = (float)lua_tonumber(L, -1);
	lua_getfield(L, 2, "max_health");
	plr->health_max = lua_tointeger(L, -1);
	lua_getfield(L, 2, "max_x_vel");
	plr->max_x_vel = (float)lua_tonumber(L, -1);
	if ( plr->health > plr->health_max ) plr->health = plr->health_max;
	return 0;
}

int scriptApi::AddBonusHealth(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "player id");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "amount");
	int num = lua_tointeger(L, 1);
	GameObject* obj = GetGameObject(num);
	((ObjCharacter*)obj)->health += lua_tointeger(L, 2);
	return 0;
}

int scriptApi::AddBonusAmmo(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "player id");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "amount");
	int num = lua_tointeger(L, 1);
	GameObject* obj = GetGameObject(num);
	((ObjPlayer*)obj)->ammo += lua_tointeger(L, 2);
	return 0;
}

int scriptApi::AddBonusSpeed(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "player id");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "amount");

	int num = lua_tointeger(L, 1);
	GameObject* obj = GetGameObject(num);
	((ObjCharacter*)obj)->max_x_vel += lua_tointeger(L, 2);
	((ObjCharacter*)obj)->walk_acc += lua_tointeger(L, 2);
	return 0;
}

int scriptApi::ReplacePrimaryWeapon(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "player id");
	luaL_argcheck(L, lua_isstring(L, 2), 2, "weapon name");

	GameObject* obj = GetGameObject(lua_tointeger(L, 1));
	if (obj && obj->type == objPlayer)
		((ObjPlayer*)obj)->GiveWeapon(lua_tostring(L, 2), true);

	return 0;
}

int scriptApi::SetEnvironmentStats(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "environment id");
	luaL_argcheck(L, lua_istable(L, 2), 2, "environment stats table");

	int num = lua_tointeger(L, 1);
	GameObject* obj = GetGameObject(num);
	if ( !obj || obj->type != objEnvironment ) return 0;
	ObjEnvironment* env = (ObjEnvironment*)obj;
	lua_getfield(L, 2, "bounce_bonus");
	if ( !lua_isnil(L, -1) )
		env->bounce_bonus = (float)lua_tonumber(L, -1);

	lua_getfield(L, 2, "gravity_bonus_x");
	if ( !lua_isnil(L, -1) )
		env->gravity_bonus.x = (float)lua_tonumber(L, -1);

	lua_getfield(L, 2, "gravity_bonus_y");
	if ( !lua_isnil(L, -1) )
		env->gravity_bonus.y = (float)lua_tonumber(L, -1);

	lua_getfield(L, 2, "on_use");
	if ( !lua_isnil(L, -1) )
		env->script_on_use = StrDupl( lua_tostring(L, -1) );

	lua_getfield(L, 2, "on_enter");
	if ( !lua_isnil(L, -1) )
		env->script_on_enter = StrDupl( lua_tostring(L, -1) );

	lua_getfield(L, 2, "on_stay");
	if ( !lua_isnil(L, -1) )
		env->script_on_stay = StrDupl( lua_tostring(L, -1) );

	lua_getfield(L, 2, "on_leave");
	if ( !lua_isnil(L, -1) )
		env->script_on_leave = StrDupl( lua_tostring(L, -1) );

	lua_getfield(L, 2, "material");
	if ( !lua_isnil(L, -1) )
		env->material = lua_tointeger(L, -1);

	return 0;
}

int scriptApi::setDefaultEnvironment(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "environment id");

	int num = lua_tointeger(L, 1);
	GameObject* obj = GetGameObject(num);
	if ( !obj || obj->type != objEnvironment ) return 0;
	DefaultEnvSet( (ObjEnvironment*)obj );

	return 0;
}

int scriptApi::SetObjectInvincible(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "object num");
	luaL_argcheck(L, lua_isboolean(L, 2), 2, "invincibility flag");
	
	// TODO: Номер не используется, однако скиптами передается. Надо точно разобраться и поправить.
	//int num = lua_tointeger(L, 1);
	bool inv = lua_toboolean(L, 2) != 0;

	ObjPlayer* obj = playerControl->second;
	ObjPlayer* obj2 = playerControl->first;
	if (obj)
		obj->is_invincible = inv;
	if (obj2)
		obj2->is_invincible = inv;
	return 0;
}

int scriptApi::SetObjectInvisible(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "object num");
	luaL_argcheck(L, lua_isboolean(L, 2), 2, "invisbility flag");
	
	// TODO: Номер не используется, однако скиптами передается. Надо точно разобраться и поправить.
	int num = lua_tointeger(L, 1);
	bool inv = lua_toboolean(L, 2) != 0;
	GameObject* obj = GetGameObject( num );
	if ( obj == NULL )
		return 0;
	
	if ( inv )
		obj->sprite->ClearVisible();
	else
		obj->sprite->SetVisible();
	return 0;
}

int scriptApi::SetPlayerRecoveryTime(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "object num");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "invincibility time");

	int num = lua_tointeger(L, 1);
	float ammount = (float)lua_tonumber(L, 2);

	GameObject* go = GetGameObject( num );
	if ( !go || go->type != objPlayer )
		return 0;
	ObjPlayer* op = (ObjPlayer*)go;
	op->recovery_time = max( op->recovery_time, ammount );
	if ( ammount == -1 )
	{
		op->recovery_time = 0;
		op->is_invincible = false;
	}
	if ( ammount > 0 ) op->is_invincible = true;
	
	return 0;
}

int scriptApi::SetObjSolidToByte(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "object num");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "ghost_to byte");

	int num = lua_tointeger(L, 1);
	BYTE solid_to = (BYTE)lua_tointeger(L, 2);

	GameObject* go = GetGameObject( num );
	if ( !go || !go->IsPhysic() )
		return 0;
	((ObjPhysic*)go)->solid_to = solid_to;
	
	return 0;
}

int scriptApi::SetObjGhostToByte(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "object num");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "ghost_to byte");

	int num = lua_tointeger(L, 1);
	BYTE solid_to = (BYTE)lua_tointeger(L, 2);

	GameObject* go = GetGameObject( num );
	if ( !go || !go->IsPhysic() )
		return 0;
	((ObjPhysic*)go)->solid_to = ~solid_to;
	
	return 0;
}

int scriptApi::SetObjSolidTo(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "object num");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "ghost_to byte");

	int num = lua_tointeger(L, 1);
	BYTE solid_to = (BYTE)lua_tointeger(L, 2);

	GameObject* go = GetGameObject( num );
	if ( !go || !go->IsPhysic() )
		return 0;
	((ObjPhysic*)go)->solid_to |= solid_to;
	
	return 0;
}

int scriptApi::SetObjGhostTo(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "object num");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "ghost_to byte");

	int num = lua_tointeger(L, 1);
	BYTE solid_to = (BYTE)lua_tointeger(L, 2);

	GameObject* go = GetGameObject( num );
	if ( !go || !go->IsPhysic() )
		return 0;
	((ObjPhysic*)go)->solid_to &= ~solid_to;
	
	return 0;
}

int scriptApi::SetNextWaypoint(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "object num");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "next id");

	int num = lua_tointeger(L, 1);
	int id = lua_tointeger(L, 2);

	GameObject* obj = GetGameObject( num );
	if ( obj->type != objWaypoint )
	{
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "SetNextWaypont: object %i is not a waypoint!", num);
		return 0;
	}
	((ObjWaypoint*)obj)->next_id = id;
	return 0;
}

int scriptApi::SetEnemyWaypoint(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "object num");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "next id");

	int num = lua_tointeger(L, 1);
	int id = lua_tointeger(L, 2);

	GameObject* obj = GetGameObject( num );
	if (!obj || obj->type != objEnemy )
	{
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "SetNextWaypont: object %i is not exists or not an enemy!", num);
		return 0;
	}
	
	((ObjEnemy*)obj)->current_waypoint = id;
	return 0;
}

int scriptApi::GetWaypoint(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "waypoint id");
	UINT num = lua_tointeger(L, 1);
	ObjWaypoint* wp = ::GetWaypoint( num );
	if ( wp )
		lua_pushinteger( L, wp->id );
	else
	{
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "GetWaypoint: no special waypoint %i!", num);
		lua_pushnil( L );
	}
	return 1;
}


int scriptApi::SetPlayerAltWeapon(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "player num");
	luaL_argcheck(L, lua_isstring(L, 2), 2, "weapon name");

	int num = lua_tointeger(L, 1);
	const char* wn = lua_tostring(L, 2);

	ObjPlayer* plr = playerControl->GetByNum(num);
	if (plr)
	{
		plr->GiveWeapon(wn, false);
	}

	return 0;
}

int scriptApi::SetPlayerAmmo(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "player num");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "ammo amount");

	int num = lua_tointeger(L, 1);
	UINT ammo = lua_tointeger(L, 2);

	ObjPlayer* plr = playerControl->GetByNum(num);
	if (plr)
	{
		plr->SetAmmo(ammo);
	}

	return 0;
}

int scriptApi::SwitchPlayer(lua_State* L)
{
	CHECKGAME;

	luaL_argcheck(L, lua_isboolean(L, 1)||lua_isnil(L, 1)||lua_isnone(L, 1), 1, "Immediate change flag expected.");

	bool immediate = false;
	if ( lua_isboolean(L, 1) )
	{
		immediate = lua_toboolean(L, 1) != 0;
	}

	playerControl->Changer( true, immediate );
	return 0;
}

int scriptApi::SwitchWeapon(lua_State* L)
{
	CHECKGAME;

	luaL_argcheck(L, lua_isnumber(L, 1), 1, "player num");
	int num = lua_tointeger(L, 1);

	ObjPlayer* op = playerControl->GetByNum(num);
	if (op)
		op->cur_weapon = ( op->alt_weapon && op->cur_weapon == op->alt_weapon ? op->weapon : op->alt_weapon );
	return 0;
}

int scriptApi::SetPlayerRevivePoint(lua_State* L)
{
	CHECKGAME;
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "x coord");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "x coord");

	playerControl->revivePoint = Vector2((float)lua_tonumber(L,1), (float)lua_tonumber(L, 2));

	return 0;
}

int scriptApi::SetOnChangePlayerProcessor(lua_State* L)
{
	CHECKGAME;
	SCRIPT::RegProc(L, &playerControl->onChangePlayerProcessor, 1);
	return 0;
}

int scriptApi::SetOnPlayerDeathProcessor(lua_State* L)
{
	CHECKGAME;
	SCRIPT::RegProc(L, &playerControl->onPlayerDeathProcessor, 1);
	return 0;
}

int scriptApi::BlockPlayerChange(lua_State* L)
{
	CHECKGAME;
	playerControl->BlockChange();
	return 0;
}

int scriptApi::UnblockPlayerChange(lua_State* L)
{
	CHECKGAME;
	playerControl->UnblockChange();
	return 0;
}


int scriptApi::SetPhysObjBorderColor(lua_State* L)
{
#ifdef MAP_EDITOR
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Object id expected");
	luaL_argcheck(L, lua_istable(L, 2), 2, "Ожидается таблица color");
	luaL_argcheck(L, lua_isboolean(L, 3), 3, "Ожидается bool show");

	GameObject* obj = GetGameObject((UINT)lua_tointeger(L, 1));
	if (obj && obj->IsPhysic())
	{
		SCRIPT::GetColorFromTable(L, 2, static_cast<ObjPhysic*>(obj)->border_color);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка SetPhysObjBorderColor: объект id=%d не существует или не фищический", (UINT)lua_tointeger(L, 1));
	}

	static_cast<ObjPhysic*>(obj)->show_border = lua_toboolean(L, 3);

#endif // MAP_EDITOR
	return 0;
}


//////////////////////////////////////////////////////////////////////////

int scriptApi::PushInt(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Object id expected");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "value expected");

	GameObject* obj = GetGameObject(lua_tointeger(L, 1));
	if (obj)
	{
		if (obj->sprite)
			obj->sprite->stack->Push((int)lua_tointeger(L, 2));
	}
	return 0;
}

int scriptApi::CreateRibbonObj(lua_State* L)
{
	luaL_argcheck(L, lua_isstring(L, 1), 1, "Texture or prototype name expected");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "k_x expected");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "k_y expected");
	luaL_argcheck(L, lua_isnumber(L, 4), 4, "x expected");
	luaL_argcheck(L, lua_isnumber(L, 5), 5, "y expected");
	luaL_argcheck(L, lua_isnumber(L, 6), 6, "z expected");
	luaL_argcheck(L, lua_isboolean(L, 7), 7, "Prototype usage flag expected");
	ObjRibbon* rib = CreateRibbon( lua_tostring(L, 1), Vector2((scalar)lua_tonumber(L, 2), (scalar)lua_tonumber(L, 3)), Vector2((scalar)lua_tonumber(L, 4), (scalar)lua_tonumber(L, 5)), (float)lua_tonumber(L, 6), lua_toboolean(L, 7) != 0);
	lua_pushinteger( L, rib->id );
	return 1;
}

int scriptApi::SetRibbonObjBounds(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "id expected");
	luaL_argcheck(L, lua_isnumber(L, 2) || lua_isnil(L, 2), 2, "x1 expected");
	luaL_argcheck(L, lua_isnumber(L, 3) || lua_isnil(L, 3), 3, "y1 expected");
	luaL_argcheck(L, lua_isnumber(L, 4) || lua_isnil(L, 4), 4, "x2 expected");
	luaL_argcheck(L, lua_isnumber(L, 5) || lua_isnil(L, 5), 5, "y2 expected");
	GameObject* obj = GetGameObject( lua_tointeger(L, 1) );
	if ( obj == NULL || obj->type != objRibbon ) return 0;
	ObjRibbon* rib = (ObjRibbon*)obj;
	float x1 = 0.0f;

	float y1 = 0.0f;
	float x2 = 0.0f;
	float y2 = 0.0f;
	bool ux1, ux2, uy1, uy2;
	if ( (ux1 = (lua_isnumber(L, 2) != 0)) ) x1 = (float)lua_tonumber( L, 2 );
	if ( (uy1 = (lua_isnumber(L, 3) != 0)) ) y1 = (float)lua_tonumber( L, 3 );
	if ( (ux2 = (lua_isnumber(L, 4) != 0)) ) x2 = (float)lua_tonumber( L, 4 );
	if ( (uy2 = (lua_isnumber(L, 5) != 0)) ) y2 = (float)lua_tonumber( L, 5 );
	rib->setBounds( x1, y1, x2, y2 );
	rib->setBoundsUsage( ux1, uy1, ux2, uy2 );
	return 0;
}

int scriptApi::SetRibbonObjK(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "id expected");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "kx expected");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "ky expected");
	GameObject* obj = GetGameObject( lua_tointeger(L, 1) );
	if ( obj == NULL || obj->type != objRibbon ) return 0;
	ObjRibbon* rib = (ObjRibbon*)obj;
	rib->k.x = (scalar)lua_tonumber(L, 2);
	rib->k.y = (scalar)lua_tonumber(L, 3);
	return 0;
}

int scriptApi::SetRibbonObjRepitition(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "id expected");
	luaL_argcheck(L, lua_isboolean(L, 2), 1, "x flag expected");
	luaL_argcheck(L, lua_isboolean(L, 3), 2, "y flag expected");
	GameObject* obj = GetGameObject( lua_tointeger(L, 1) );
	if ( obj == NULL || obj->type != objRibbon ) return 0;
	ObjRibbon* rib = (ObjRibbon*)obj;
	rib->setRepitition( lua_toboolean(L, 2) != 0, lua_toboolean(L, 3) != 0 );
	return 0;
}

extern GameObject* Waypoints;

int scriptApi::GetNearestWaypoint(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "id expected");
	GameObject* obj = GetGameObject( lua_tointeger(L, 1) );
	if ( obj == NULL ) return 0;
	ObjWaypoint* wp = NULL;
	ObjWaypoint* target = NULL;
	float dist = 0.0f;
	for ( vector<GameObject*>::iterator iter = Waypoints->childrenConnection->children.begin(); 
		iter != Waypoints->childrenConnection->children.end(); ++iter)
	{
			wp = (ObjWaypoint*)(*iter);
			if ( target == NULL || (wp->aabb.p - obj->aabb.p).Length() < dist )
			{
					target = wp;
					dist = (wp->aabb.p - obj->aabb.p).Length();
			}
	}
	PushObject(L, target);
	return 1;
}

#include "StdAfx.h"

#include "object_bullet.h"
#include "../phys/sap/OPC_ArraySAP.h"
#include "weapon.h"
#include "object_ray.h"

#include "../../resource_mgr.h"

//////////////////////////////////////////////////////////////////////////

extern ResourceMgr<Proto> * protoMgr;

extern UINT internal_time;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Загрузка оружия из прототипа
BOOL LoadWeaponFromProto(const Proto* proto, Weapon* weapon)
{
	if (!proto || !weapon)
		return TRUE;

	weapon->bullets_count = (USHORT)proto->bullets_count;
	if (proto->bullets_count <= 0)
	{
		// Бесконечный боезапас
		weapon->is_infinite = true;
	}

	weapon->reload_time = proto->reload_time;
	weapon->clip_reload_time = proto->clip_reload_time;
	weapon->shots_per_clip = proto->shots_per_clip;
	weapon->clip = weapon->shots_per_clip;
	weapon->bullet_proto = proto;

	weapon->is_ray = proto->is_ray_weapon != 0;

	return FALSE;
}

// Создание оружия
Weapon* CreateWeapon(const Proto* proto)
{
	if (!proto)
		return NULL;

	Weapon* weapon = new Weapon();

	if (LoadWeaponFromProto(proto, weapon))
	{
		DELETESINGLE(weapon);
		return NULL;
	}

	return weapon;
}

Weapon* CreateWeapon(const char* proto_name)
{
	if (!proto_name)
		return NULL;

	return CreateWeapon(protoMgr->GetByName(proto_name, "weapons/"));
}

extern Opcode::ArraySAP *asap;

//////////////////////////////////////////////////////////////////////////

// Выстрел из оружия
void Weapon::Fire(ObjCharacter* shooter, Vector2 coord)
{
	if (!IsReady())
		return;
	
	if ( shots_per_clip > 0 ) clip--;

	// Определение направления стрельбы
	WeaponDirection wd = wdRight;
	switch (shooter->gunDirection)
	{
	case cgdDown:
		wd = !shooter->GetFacing() ? wdDownRight : wdDownLeft;
		break;
	case cgdUp:
		wd = !shooter->GetFacing() ? wdUpRight : wdUpLeft;
		break;
	case cgdNone:
		wd = !shooter->GetFacing() ? wdRight : wdLeft;
	}

	if (is_ray)
	{
		/*ObjRay* ray = */CreateRay(this->bullet_proto, coord, shooter, wd);
	}
	else
	{
		ObjBullet* ob = CreateBullet(this->bullet_proto, coord, shooter, wd);
		ob->vel += shooter->vel;
		//ob->max_x_vel += abs(shooter->vel.x)*0.25f;
		//ob->max_y_vel += abs(shooter->vel.y);
		ob->acc += shooter->acc;
	}
	last_reload_tick = internal_time;
	//bullets_count--; В обработке стрельбы игроком.
}

// Проверка на перезаряженность.
bool Weapon::IsReloaded()
{
	if ( clip == 0 && shots_per_clip != 0 ) return false;
	return internal_time - last_reload_tick >= reload_time;
}

bool Weapon::ClipReloaded(bool hold)
{
	if ( hold ) return internal_time - last_reload_tick >= clip_reload_time*2;
	return internal_time - last_reload_tick >= clip_reload_time;
}

// Проверка на готовность оружия к стрельбе
bool Weapon::IsReady()
{

	return IsReloaded();
}

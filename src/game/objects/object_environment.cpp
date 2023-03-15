#include "StdAfx.h"

#include "object_environment.h"
#include "object_dynamic.h"
#include "../../script/script.h"

#include "../../resource_mgr.h"

//////////////////////////////////////////////////////////////////////////

extern ResourceMgr<Proto> * protoMgr;
extern lua_State* lua;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ObjEnvironment* default_environment = NULL;

ObjEnvironment* CreateEnvironment(const char* proto_name, Vector2 coord)
{
	const Proto* proto = protoMgr->GetByName( proto_name, "environments/" );
	if (!proto)
		return NULL;

	ObjEnvironment* env = new ObjEnvironment();

	
	env->bounce_bonus = proto->bounce_bonus;
	env->walk_vel_multiplier = proto->walk_vel_multiplier;
	env->jump_vel_multiplier = proto->jump_vel_multiplier;
	env->gravity_bonus = proto->gravity_bonus;
	env->material = proto->env_material;
	if ( proto->env_onenter )
	{
		env->script_on_enter = StrDupl(proto->env_onenter);
	}
	if ( proto->env_onleave )
	{
		env->script_on_leave = StrDupl(proto->env_onleave);
	}
	if ( proto->env_onstay)
	{
		env->script_on_stay = StrDupl(proto->env_onstay);
	}
	if ( proto->env_onuse)
	{
		env->script_on_use = StrDupl(proto->env_onuse);
	}

	int i = 0;
	for(vector<char*>::const_iterator it = proto->sprites.begin();
			it != proto->sprites.end();
			it++)
		{
			char *val = new char[strlen(*it)+1];
			strcpy(val, *it);
			env->sprites[i] = val;
			if (++i == 32) break;
		}

	i = 0;
	for(vector<char*>::const_iterator it = proto->sounds.begin();
			it != proto->sounds.end();
			it++)
		{
			char *val = new char[strlen(*it)+1];
			strcpy(val, *it);
			env->sounds[i] = val;
			if (++i == 32) break; 
		}
	if (LoadObjectFromProto(proto, (GameObject*)env))
	{
		DELETESINGLE(env);
		return NULL;
	}

	env->aabb.p = coord;
	//env->sprite = NULL; <- Плохая строчка, плохая! Вызывает утечку! Почему...?

	env->type = objEnvironment;
	env->SetPhysic();

	AddObject(env);

#ifdef MAP_EDITOR
	env->proto_name = new char[ strlen(proto_name) + 1 ];
	strcpy( env->proto_name, proto_name );
#endif //MAP_EDITOR

	return env;
}

char* ObjEnvironment::GetSound( int index )
{
	return *(sounds+index);
}

char* ObjEnvironment::GetSprite( int index )
{
	return *(sprites+index);
}

Vector2 TraceVelBack( ObjDynamic* eob, ObjEnvironment* area, bool leaving )
{
	Vector2 pnt = Vector2(0, 0);
	float x = ( (leaving && eob->vel.x > 0) && (!leaving && eob->vel.x < 0) ) ? area->aabb.Left() : area->aabb.Right();
	float y = ( (leaving && eob->vel.y > 0) && (!leaving && eob->vel.y < 0) ) ? area->aabb.Top() : area->aabb.Bottom();
	float sx = (y-eob->aabb.p.y)*((eob->aabb.p.x/(y-eob->aabb.p.y))*eob->vel.y+eob->vel.x)/eob->vel.y;
	float sy = (eob->aabb.p.x-x)*((eob->aabb.p.y/(eob->aabb.p.x-x))*eob->vel.x-eob->vel.y)/eob->vel.x;
	if ( Vector2( x, sy ).Length() > 
		 Vector2( sx, y ).Length() )
	{
		pnt.x = sx;
		pnt.y = y;
	}
	else
	{
		pnt.x = x;
		pnt.y = sy;
	}
	return pnt;
}

void ObjEnvironment::OnEnter( GameObject* obj )
{
	if ( !this->script_on_enter ) return;
	ObjDynamic* eob = (ObjDynamic*)obj;

	//Ищем точку для передачи параметром
	Vector2 pnt = obj->aabb.p;
	//Сначала ищем возможное пересечение внутри
	float top = this->aabb.Top();
	float bottom = this->aabb.Bottom();
	if ( obj->aabb.Bottom() > top && top > obj->aabb.Top() && bottom > obj->aabb.Bottom() )
		pnt.y = top;
	else if ( obj->aabb.Bottom() > bottom && bottom > obj->aabb.Top() && top < obj->aabb.Top() )
		pnt.y = bottom;
	float left = this->aabb.Left();
	float right = this->aabb.Right();
	if ( obj->aabb.Right() > right && right > obj->aabb.Left() && left < obj->aabb.Left() )
		pnt.x = right;
	else if ( obj->aabb.Right() > left && left > obj->aabb.Left() && right > obj->aabb.Right() )
		pnt.y = left;
	//Внутренних пересечений не было, отслеживаем скорость назад.
	if ( pnt.x == obj->aabb.p.x && pnt.y == obj->aabb.p.y )
		pnt = TraceVelBack(eob, this, false);

	lua_getglobal(lua, this->script_on_enter);
	lua_pushinteger(lua, eob->id);
	lua_pushnumber(lua, pnt.x);
	lua_pushnumber(lua, pnt.y);
	//lua_pushinteger(lua, eob->old_env->material);
	lua_pushinteger(lua, this->material);
	SCRIPT::ExecChunk(4);
}

void ObjEnvironment::OnLeave( GameObject* obj )
{
	if ( !this->script_on_leave ) return;
	ObjDynamic* eob = (ObjDynamic*)obj;
	//Ищем точку для передачи параметром, пересечения внутри точно нет.
	Vector2 pnt = TraceVelBack(eob, this, true);

	lua_getglobal(lua, this->script_on_leave);
	lua_pushinteger(lua, eob->id);
	lua_pushnumber(lua, pnt.x);
	lua_pushnumber(lua, pnt.y);
	lua_pushinteger(lua, this->material);
	lua_pushinteger(lua, eob->env->material);
	SCRIPT::ExecChunk(5);
}

void ObjEnvironment::OnStay( GameObject* obj )
{
	if ( !this->script_on_stay ) return;
	ObjDynamic* eob = (ObjDynamic*)obj;
	
	lua_getglobal(lua, this->script_on_stay);
	lua_pushinteger(lua, eob->id);
	lua_pushnumber(lua, eob->aabb.p.x);
	lua_pushnumber(lua, eob->aabb.p.y);
	SCRIPT::ExecChunk(3);
}

void ObjEnvironment::OnUse( GameObject* obj )
{
	if ( !this->script_on_use ) return;
	ObjDynamic* eob = (ObjDynamic*)obj;

	lua_getglobal(lua, this->script_on_use);
	lua_pushinteger(lua, eob->id);
	lua_pushnumber(lua, eob->aabb.p.x);
	lua_pushnumber(lua, eob->aabb.p.y);
	SCRIPT::ExecChunk(3);
}

void DefaultEnvDelete()
{
	if (default_environment) DELETESINGLE(default_environment);
}

void DefaultEnvSet( ObjEnvironment* env )
{
	if ( env )
	{
		default_environment->walk_vel_multiplier = env->walk_vel_multiplier;
		default_environment->jump_vel_multiplier = env->jump_vel_multiplier;
		default_environment->bounce_bonus = env->bounce_bonus;
		default_environment->gravity_bonus = env->gravity_bonus;
		default_environment->material = env->material;
		for ( int i = 0; i < 32; i++ )
		{
			if (default_environment->sprites[i]) DELETESINGLE(default_environment->sprites[i]);
			if (default_environment->sounds[i]) DELETESINGLE(default_environment->sounds[i]);
			if (env->sprites[i]) 
			{
				default_environment->sprites[i] = new char[strlen(env->sprites[i]) + 1];
				strcpy(default_environment->sprites[i], env->sprites[i]);
			}
			if (env->sounds[i]) 
			{
				default_environment->sounds[i] = new char[strlen(env->sounds[i]) + 1];
				strcpy(default_environment->sounds[i], env->sounds[i]);
			}
		}
		//memcpy(default_environment, env, sizeof(ObjEnvironment)); <= не сработает, потому что строки, на которые ссылаются списки могут быть удалены со старым окружением.
	}
	else 
	{
		DefaultEnvDelete();
		default_environment = new ObjEnvironment();
	}
}
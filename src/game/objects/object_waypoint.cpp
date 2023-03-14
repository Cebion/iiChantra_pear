#include "StdAfx.h"

#include "object.h"
#include "object_waypoint.h"
#include "object_enemy.h"

#include <stack>
#include <vector>

extern lua_State* lua;
map<UINT, ObjWaypoint*> MapWaypoints;
stack<UINT> free_wp_ids;

//<HALlOWEEN>
GameObject* Waypoints = NULL;
//</HALlOWEEN>

UINT ObjWaypoint::pointReached( GameObject* obj )
{
	assert(obj);
	
	if ( this->reach_event != LUA_NOREF )
	{
		lua_pushinteger( lua, obj->id );
		lua_pushinteger( lua, this->id );
		SCRIPT::ExecChunkFromReg(this->reach_event, 2);
		if ( lua_isnumber(lua, -1) )
			return lua_tointeger(lua, -1);
	}

	return this->next_id;
}

void ObjWaypoint::setNextID( UINT id )
{
	this->next_id = id;
}

ObjWaypoint::~ObjWaypoint()
{
	if ( unique_map_id > 0 )
	{
		MapWaypoints.erase(unique_map_id);
		free_wp_ids.push( unique_map_id );
	}
}

ObjWaypoint* CreateWaypoint( Vector2 coord, Vector2 size )
{
	//<HALLOWEEN>
	if ( Waypoints == NULL )
	{
		Waypoints = new GameObject();
		AddObject( Waypoints );
	}
	//</HALLOWEEN>

	ObjWaypoint* waypoint = new ObjWaypoint( Waypoints );
	waypoint->aabb.p = coord;
	waypoint->aabb.W = size.x * 0.5f;
	waypoint->aabb.H = size.y * 0.5f;

#ifdef MAP_EDITOR
	waypoint->SetPhysic();
#endif // MAP_EDITOR

	AddObject( waypoint );
	return waypoint;
}

UINT AddWaypoint( ObjWaypoint* waypoint )
{
	UINT wp_id = 1;
	if ( !free_wp_ids.empty() )
	{
		wp_id = free_wp_ids.top();
		free_wp_ids.pop();
	}

	MapWaypoints[wp_id] = waypoint;
	waypoint->unique_map_id = wp_id;

	if ( free_wp_ids.empty() )
		free_wp_ids.push( wp_id + 1 );

	return wp_id;
}

void AddWaypoint( ObjWaypoint* waypoint, UINT unique_map_id )
{
	map<UINT, ObjWaypoint*>::iterator it = MapWaypoints.find(unique_map_id);

	if ( it != MapWaypoints.end() )
		it->second->SetDead();
	
	MapWaypoints[unique_map_id] = waypoint;
	waypoint->unique_map_id = unique_map_id;

	if ( !free_wp_ids.empty() )
	{
		UINT wp_id = free_wp_ids.top();
		if ( wp_id == unique_map_id )
		{
			wp_id++;
			free_wp_ids.pop();
			free_wp_ids.push( wp_id );
		}
	}
}

ObjWaypoint* GetWaypoint( UINT wp_id )
{
	map<UINT, ObjWaypoint*>::iterator it = MapWaypoints.find(wp_id);
	if (it != MapWaypoints.end())
		return it->second;
	
	return NULL;
}

void EmptyWPStack()
{
	while ( !free_wp_ids.empty() )
		free_wp_ids.pop();
}

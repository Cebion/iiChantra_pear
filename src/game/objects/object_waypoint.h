#ifndef __OBJECT_WAYPOINT_H_
#define __OBJECT_WAYPINT_H_

#include "../../defines.h"

#include "object.h"

#ifdef MAP_EDITOR
#include "object_physic.h"
#endif // MAP_EDITOR

class ObjWaypoint :	
#ifdef MAP_EDITOR
	public ObjPhysic
#else
	public GameObject
#endif
{
public:

	UINT next_id;
	UINT unique_map_id;
	int reach_event;

	ObjWaypoint( GameObject* parent )
	{
		next_id = 0;
		reach_event = LUA_NOREF;
		unique_map_id = 0;

		aabb.W = 0.5f;
		aabb.H = 0.5f;

		type = objWaypoint;

		//<HALLOWEEN>
		if ( parent )
			ObjectConnection::addChild( parent, this );
		//</HALLOWEEN>
	}

	~ObjWaypoint();
	
	UINT pointReached( GameObject* obj );
	void setNextID( UINT id );
};

ObjWaypoint* CreateWaypoint( Vector2 coord, Vector2 size );
UINT AddWaypoint( ObjWaypoint* waypoint );
void AddWaypoint( ObjWaypoint* waypoint, UINT unique_map_id );
ObjWaypoint* GetWaypoint( UINT wp_id );
void EmptyWPStack();

#endif // __OBJECT_WAYPOINT_H_

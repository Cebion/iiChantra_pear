#ifndef __OBJECT_ENEMY_H_
#define __OBJECT_ENEMY_H_

#include "object_character.h"
#include "object_player.h"

#define FACTION_NEUTRAL 0
#define FACTION_PLAYER -1
#define FACTION_ENEMY 1

class ObjEnemy : public ObjCharacter
{
public:
	ObjectConnection* target;
	Vector2 aim;
	bool attached;
	UINT current_waypoint;
	int waypoint_mode;
	float waypoint_speed;
	float waypoint_speed_limit;
	Vector2 waypoint_start;
	UINT waypoint_global;

	void Process();
	virtual bool ReduceHealth( UINT damage );
	void Touch(ObjPhysic *obj);
	void ParentEvent( ObjectEventInfo info );
	void UpdateTarget( ObjCharacter* target );
	ObjCharacter* GetTarget();

	ObjEnemy()
	{
		this->current_waypoint = 0;
		this->type = objEnemy;
		this->aim = Vector2(0,0);
		attached = false;
		health = 1;						 //ѕросто чтобы не погибнуть сразу при создании.
		target = NULL;
	}

	~ObjEnemy()
	{
		if ( target )
			target->childDead( this->id );
	}
};

ObjEnemy* CreateEnemy(const char* proto_name, Vector2 coord, const char* start_anim);

#endif // __OBJECT_ENEMY_H_
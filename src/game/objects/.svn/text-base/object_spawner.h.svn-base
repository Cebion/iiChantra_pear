#ifndef __OBJECT_SPANWER_H_
#define __OBJECT_SPAWNER_H_

#include "object.h"

enum directionType { dirNone, dirLeft, dirRight, dirUp, dirDown, dirAny, dirHorizontal, dirVertical };

#ifdef MAP_EDITOR
#include "object_physic.h"
#endif // MAP_EDITOR

class ObjSpawner :	
#ifdef MAP_EDITOR
	public ObjPhysic
#else
	public GameObject
#endif
{
public:
#ifdef MAP_EDITOR
	GameObject* spriteObj;
#endif // MAP_EDITOR
	char* enemyType;
	UINT maximumEnemies;
	UINT enemySpawnDelay;
	UINT size;
	UINT respawn_dist;
	directionType direction;

	UINT cooldown;
	UINT charges;

	void Process();
	bool onScreen(float size);
	
	ObjSpawner( const char* enemyType, UINT maximumEnemies, UINT enemySpawnDelay, directionType direction, UINT size, UINT respawn_dist )
	{
		this->enemyType = new char[ strlen(enemyType) + 1 ];
		strcpy( this->enemyType, enemyType );
		this->maximumEnemies = maximumEnemies;
		this->enemySpawnDelay = enemySpawnDelay;
		this->direction = direction;
		this->cooldown = 0;
		this->charges = maximumEnemies;
		this->childrenConnection = new ObjectConnection( this );
		this->type = objSpawner;
		this->size = size;
		this->respawn_dist = respawn_dist;
	}

	~ObjSpawner()
	{
		DELETESINGLE( this->enemyType );
#ifdef MAP_EDITOR
		spriteObj->SetDead();
#endif // MAP_EDITOR
	}
};

ObjSpawner* CreateSpawner(Vector2 coord, const char* enemy_type, UINT max_enemies, UINT delay, directionType direction, UINT size );
ObjSpawner* CreateSpawner(Vector2 coord, const char* enemy_type, UINT max_enemies, UINT delay, directionType direction, UINT size, UINT respawn_dist );

#endif //__OBJECT_SPAWNER_H
#include "StdAfx.h"
#include "object_spawner.h"
#include "object_enemy.h"
#include "../camera.h"

extern float CAMERA_LEFT;
extern float CAMERA_RIGHT;
extern float CAMERA_TOP;
extern float CAMERA_BOTTOM;
extern float CAMERA_X;
extern float CAMERA_Y;

bool ObjSpawner::onScreen( float size )
{
	if ( aabb.p.x - size < CAMERA_RIGHT && aabb.p.x + size > CAMERA_LEFT &&
		 aabb.p.y + size > CAMERA_TOP   && aabb.p.y - size < CAMERA_BOTTOM )
		 return true;
	return false;
}

void ObjSpawner::Process()
{
	#ifdef MAP_EDITOR
		spriteObj->aabb.p = this->aabb.p;
		RenderBox( aabb.Left(), aabb.Top(), 1, 2*size, 2*size, RGBAf(1,1,1,1));
	#endif // MAP_EDITOR

	if ( charges > 0 && cooldown == 0 && onScreen((float)size) && this->childrenConnection->children.size() < maximumEnemies )
	{
		if ( direction == dirHorizontal  && abs(aabb.p.y - CAMERA_Y) > size )
			return;
		if ( direction == dirVertical  && abs(aabb.p.x - CAMERA_X) > size )
			return;
		charges--;
		cooldown = enemySpawnDelay;
		ObjEnemy* oe = CreateEnemy( enemyType, Vector2(0, 0), NULL);
		Vector2 coord = this->aabb.p;
		switch ( direction )
		{
			case dirDown:
				{
					coord.y = CAMERA_BOTTOM + oe->aabb.H;
					break;
				}
			case dirUp:
				{
					coord.y = CAMERA_TOP - oe->aabb.H;
					break;
				}
			case dirLeft:
				{
					coord.x = CAMERA_LEFT - oe->aabb.W;
					break;
				}
			case dirRight:
				{
					coord.x = CAMERA_RIGHT + oe->aabb.W;
					break;
				}
			default: break;
		}
		oe->aabb.p = coord;
		ObjectConnection::addChild( this, oe );
	}
	if ( cooldown > 0 ) cooldown--;
	switch ( direction )
	{
		case dirDown:
			{
				if (CAMERA_BOTTOM - respawn_dist < aabb.p.y) charges = maximumEnemies;
				break;
			}
		case dirUp:
			{
				if (CAMERA_TOP + respawn_dist > aabb.p.y) charges = maximumEnemies;
				break;
			}
		case dirLeft:
			{
				if (CAMERA_LEFT + respawn_dist > aabb.p.x) charges = maximumEnemies;
				break;
			}
		case dirRight:
			{
				if (CAMERA_RIGHT - respawn_dist < aabb.p.x) charges = maximumEnemies;
				break;
			}
		case dirHorizontal:
		case dirVertical:
		case dirAny:
			{
				if ( !onScreen( (float)respawn_dist ) )
					charges = maximumEnemies - childrenConnection->children.size();
			}
		default: break;
	}
}

ObjSpawner* CreateSpawner(Vector2 coord, const char* enemy_type, UINT max_enemies, UINT delay, directionType direction, UINT size )
{
	return CreateSpawner( coord, enemy_type, max_enemies, delay, direction, size, size );
}

ObjSpawner* CreateSpawner(Vector2 coord, const char* enemy_type, UINT max_enemies, UINT delay, directionType direction, UINT size, UINT respawn_dist )
{
	ObjSpawner* spawner = new ObjSpawner(enemy_type, max_enemies, delay, direction, size, respawn_dist);
	spawner->aabb.p = coord;

#ifdef MAP_EDITOR
	spawner->SetPhysic();
#else
	spawner->ClearPhysic();
#endif // MAP_EDITOR

#ifdef MAP_EDITOR
		spawner->spriteObj = CreateEnemy( enemy_type, coord, NULL);
		spawner->spriteObj->type = objNone;
		spawner->spriteObj->sprite->color.a = 0.5f;
		AddObject( spawner->spriteObj );
#endif // MAP_EDITOR

	AddObject(spawner);

#ifdef MAP_EDITOR
	spawner->proto_name = new char[ strlen(enemy_type) + 1 ];
	strcpy( spawner->proto_name, enemy_type );
#endif //MAP_EDITOR

	return spawner;
}

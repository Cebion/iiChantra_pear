#ifndef __OBJECT_H_
#define __OBJECT_H_

enum ObjectType { objNone, objPlayer, objSprite, objBullet, objEffect, objEnemy, objItem, objEnvironment, objRay, objParticleSystem, objWaypoint, objSpawner, objTile, objRibbon };
enum TouchDetectionType { tdtAlways, tdtFromEverywhere, tdtFromTop, tdtFromBottom, tdtFromSides, tdtTopAndSides };
enum ObjectActivityType { oatIdling, oatShooting, oatDying, oatInjuring, oatUndead, oatMorphing, oatUsing };
enum ObjectEventType { eventError, eventDead, eventCollisionPushed, eventMoved, eventFacingChange, eventMPChanged };

#include "../../script/script.h"

#include "../phys/phys_misc.h"
#include "../sprite.h"

#include "../proto.h"

#include "../particle_system.h"

class ObjectEventInfo
{
public:
	ObjectEventType type;
	Vector2 coord;
	UINT num;

	ObjectEventInfo() { this->type = eventError; }
	ObjectEventInfo( ObjectEventType type )	{ this->type = type; }
	ObjectEventInfo( ObjectEventType type, Vector2 coord ) { this->type = type; this->coord = coord; }
	ObjectEventInfo( ObjectEventType type, Vector2 coord, UINT num ) { this->type = type; this->coord = coord; this->num = num; }
};

class GameObject
{
protected:
	class ObjectConnection
	{
	protected:
		void _addChild( GameObject* child );

	public:
		GameObject* parent;
		vector<GameObject*> children;
		bool orphan;
		static void addChild( GameObject* parent, GameObject* child );
		void parentDead();
		void childDead( UINT id );
		void Event( ObjectEventInfo event_info );
		
		GameObject* getParent();

		ObjectConnection( GameObject* parent )
		{
			this->parent = parent;
			this->orphan = NULL == parent;
		}

		ObjectConnection( GameObject* parent, GameObject* child )
		{
			this->parent = parent;
			this->orphan = NULL == parent;
			this->children.push_back( child );
		}

		~ObjectConnection()
		{
		}

		static void removeConnection( ObjectConnection* instance )
		{
			delete instance;
		}
	};

public:
	UINT id;

	ObjectType type;

	Sprite* sprite;			// Спрайт
	CParticleSystem *ParticleSystem; // facepalm.jpg
#ifdef MAP_EDITOR
	char* proto_name;
	bool grouped;
	Vector2 creation_shift;
#endif //MAP_EDITOR

	CAABB aabb;				// Прямоугольник, описывающий физический объект; координаты тела

	int scriptProcess;

	ObjectActivityType activity;		// Тип действий объекта

	// Флаги
	// dead		1		Объект подлежит удолени.
	// physic	2		Физический, добавлен в ASAP.
	// sleep	4
	BYTE objFlags;
	__INLINE BYTE IsDead()		{ return objFlags & 1; }
	__INLINE void SetDead()		{ objFlags |= 1; }
	__INLINE void ClearDead()	{ objFlags &= ~1; }

	__INLINE BYTE IsPhysic()		{ return objFlags & 2; }
	__INLINE void SetPhysic()		{ objFlags |= 2; }
	__INLINE void ClearPhysic()	{ objFlags &= ~2; }

	__INLINE BYTE IsSleep()		{ return objFlags & 4; }
	__INLINE void SetSleep()		{ objFlags |= 4; }
	__INLINE void ClearSleep()	{ objFlags &= ~4; }

	UINT mem_frame;
	UINT mem_tick;
	char * mem_anim;

	TouchDetectionType touch_detection;

	ObjectConnection* parentConnection;
	ObjectConnection* childrenConnection;

	GameObject()
	{
		type = objNone;
		id = 0;
		sprite = NULL;
		ParticleSystem = NULL;
#ifdef MAP_EDITOR
		proto_name = NULL;
		grouped = false;
		creation_shift = Vector2(0, 0);
#endif //MAP_EDITOR

		parentConnection = childrenConnection = NULL;

		mem_anim = NULL;
		
		objFlags = 0;		
		scriptProcess = LUA_NOREF;

		touch_detection = tdtAlways;

		activity = oatIdling;
	}

	virtual ~GameObject();

	virtual void Process()
	{
	}

	virtual void ParentEvent( ObjectEventInfo info )
	{
		UNUSED_ARG(info);
	}

	virtual bool GetFacing()
	{
		if ( sprite ) return sprite->IsMirrored() != 0;
		return false;
	}

	virtual void SetFacing( bool mirrored )
	{
		if ( sprite ) 
		{
			if ( mirrored )
				sprite->SetMirrored();
			else
				sprite->ClearMirrored();
		}
	}

	void ProcessSprite();
	void SetAnimation(string anim_name, bool restart);
	void SetAnimationIfPossible(string anim_name, bool restart);
	virtual void AddChild( GameObject* child )
	{
		ObjectConnection::addChild( this, child );
	}

	virtual void Draw();
};	

void AddObject(GameObject* obj);
void RemoveObject(GameObject* obj);

void PrepareRemoveAllObjects();
void RemoveAllObjects();

void BatchAdd();
void BatchRemove();

GameObject* GetGameObject(UINT id);
void GroupObjects( int objGroup, int objToAdd );

void ProcessAllActiveObjects();
void ProcessAllPassiveObjects();
void UpdatePhysTime();
void DrawAllObjects();

BOOL LoadObjectFromProto(const Proto* proto, GameObject* obj);

#endif // __OBJECT_H_

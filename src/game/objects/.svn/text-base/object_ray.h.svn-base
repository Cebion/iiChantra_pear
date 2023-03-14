#ifndef __OBJECT_RAY_H_
#define __OBJECT_RAY_H_

#include "object.h"
#include "object_physic.h"
#include "../phys/phys_misc.h"
#include "object_character.h"

// Размер масива, выделяемого для сохранения объектов, с котрыми персекаетсял луч
// Большее количество объектов луч за раз пересечь не сможет
const UINT maxRayIntersectionsCount = 512;

class ObjRay : public GameObject
{
public:
	CRay ray;	

	RGBAf debug_color;

	float searchDistance;		// Расстояние, на котром ведется поиск пересечений.
								// Если равно 0, то тогда поиск не ограничен.

	// TODO: 
	// 1. Указывать режим прекращения поиска:
	//		а) на i-тоим статическом/динамическом/враге и т.п.
	//		б) проверять до конца интервала поиска

	UINT collisionsCount;

	BYTE rayFlags;

	int next_shift_y;

	// bullet code
	UINT damage;
	UINT shooter_id;
	ObjectType shooter_type;
	UINT damage_type;
	bool multiple_targets;
	bool hurts_same_type;
	float push_force;

	UINT timeToLive;

	char* end_effect;

	bool Hit(ObjPhysic* obj);
	bool CheckIntersection(ObjPhysic* obj);
	// /bullet code

	Vector2 ray_end_point;


	__INLINE BYTE IsUseCollisionCounter()		{ return rayFlags & 1; }
	__INLINE void SetUseCollisionCounter()		{ rayFlags |= 1; }
	__INLINE void ClearUseCollisionCounter()	{ rayFlags &= ~1; }

	__INLINE BYTE IsUseTTLCounter()			{ return rayFlags & 2; }
	__INLINE void SetUseTTLCounter()		{ rayFlags |= 2; }
	__INLINE void ClearUseTTLCounter()		{ rayFlags &= ~2; }

	__INLINE BYTE IsDrawRayEndPoint()		{ return rayFlags & 4; }
	__INLINE void SetDrawRayEndPoint()		{ rayFlags |= 4; }
	__INLINE void ClearDrawRayEndPoint()	{ rayFlags &= ~4; }

	ObjRay();
	ObjRay( GameObject* shooter );
	~ObjRay();

	virtual void Process();
	void PhysProcess();

	virtual void Draw();
	
private:
};

ObjRay* CreateRay(float x1, float y1, float x2, float y2);
ObjRay* CreateRay(const char* proto_name, Vector2 coord, ObjCharacter* shooter, WeaponDirection wd);
ObjRay* CreateRay(const Proto* proto, Vector2 coord, ObjCharacter* shooter, WeaponDirection wd);
#endif // __OBJECT_RAY_H_
#ifndef __OBJECT_BULLET_H_
#define __OBJECT_BULLET_H_

#include "object_dynamic.h"
#include "object_character.h"
#include "../particle_system.h"

class ObjBullet : public ObjDynamic
{
public:
	UINT damage;
	//ObjCharacter* shooter; <- так нельзя, потому что он может быть удалён и информации не получим.
	ObjectType shooter_type;

	//Shielding bit - поглощает другие выстрелы при встрече. Другие shielding выстрелы игнорируются.
	__INLINE void SetShielding()		{ physFlags |= 128; }
	__INLINE void ClearShielding()		{ physFlags &= ~128; }
	__INLINE bool IsShielding()			{ return (physFlags & 128) != 0; }

	UINT damage_type;
	float push_force;
	bool multiple_targets;
	bool hurts_same_type;
	bool hurts_same_faction;

	ObjBullet()
	{
		damage = 0;
		push_force = 0;

		this->type = objBullet;
		this->damage_type = 0;
		this->trajectory = pttLine;
		this->t_value = 0;
		this->t_param1 = 0;
		this->t_param2 = 0;

		multiple_targets = hurts_same_type = false;

		this->SetBullet();
		this->parentConnection = new ObjectConnection( NULL );
	}

	ObjBullet( GameObject* shooter )
	{
		damage = 0;
		push_force = 0;

		this->type = objBullet;
		this->damage_type = 0;
		this->trajectory = pttLine;
		this->t_value = 0;
		this->t_param1 = 0;
		this->t_param2 = 0;

		multiple_targets = hurts_same_type = false;

		this->SetBullet();

		if (shooter)
		{
			ObjectConnection::addChild( shooter, this );
			this->shooter_type = shooter->type;
		}
	}


	virtual void Process();
	void PhysProcess();

	void Hit(ObjPhysic* obj);
};


ObjBullet* CreateBullet(const char* proto, Vector2 coord, ObjCharacter* shooter, Vector2 aim, int angle);
ObjBullet* CreateBullet(const char* proto_name, Vector2 coord, ObjCharacter* shooter, WeaponDirection wd);
ObjBullet* CreateBullet(const Proto* proto, Vector2 coord, ObjCharacter* shooter, WeaponDirection wd);
ObjBullet* CreateAngledBullet(const char* proto_name, Vector2 coord, GameObject* shooter, bool mirror, int angle, int dir);

#endif // __OBJECT_BULLET_H_

#ifndef __OBJECT_DYNAMIC_H_
#define __OBJECT_DYNAMIC_H_

#include "object_physic.h"
#include "object_environment.h"

enum ObjectMovementType { omtIdling, omtWalking, omtSitting, omtJumping, omtLanding/*, omtAimingUp, omtAimingDown, omtWalkingAimingUp*/, omtDropSitting, omtMovingToWaypoint };
enum ObjectFacingType { ofNormal, ofMoonwalking, ofFixed };


extern ObjEnvironment* default_environment;

class ObjDynamic : public ObjPhysic
{
public:
	CAABB old_aabb;
	Vector2 vel;
	Vector2 acc;
	Vector2 gravity;
	TrajectoryType trajectory;
	float t_value;
	float t_param1;
	float t_param2;
	scalar max_x_vel;
	scalar max_y_vel;
	ObjPhysic* suspected_plane;
	ObjPhysic* drop_from;			// Ипользуется для того, чтобы понять, что спрыгиваем с платформы.
	ObjEnvironment* env;
	//ObjEnvironment* old_env;
	float shadow_width;
	bool drops_shadow;
	bool facing_left;
	float lifetime;
	float mass;

	scalar walk_acc;
	scalar jump_vel;

	bool ghostlike;			// Объект "призрачный", для него не решаются
							// столкновения и он проходит сквозь solid-объекты

	bool movementDirectionX;		// Напавление движения объекта


	ObjectMovementType movement;		// Тип движения объекта
	ObjectFacingType facing;
	


	// Флаги
	// on_plane		16		Объект находится на поверхности
	// bullet		32		Объект - пуля
	// dropping		128		Объект спрыгивает с платформы
	__INLINE BYTE IsOnPlane()		{ return physFlags & 16; }
	__INLINE BYTE IsBullet()		{ return physFlags & 32; }
	__INLINE BYTE IsDropping()		{ return physFlags & 128; }

	__INLINE void SetOnPlane()		{ physFlags |= 16; }
	__INLINE void SetBullet()		{ physFlags |= 32; }
	__INLINE void SetDropping()		{ physFlags |= 128; drop_from = suspected_plane; }

	__INLINE void ClearOnPlane()	{ physFlags &= ~16; }
	__INLINE void ClearBullet()		{ physFlags &= ~32; }
	__INLINE void ClearDropping()	{ physFlags &= ~128; }

	ObjDynamic()
	{
		this->SetDynamic();

		suspected_plane = NULL;
		drop_from = NULL;
		env = default_environment;
		//old_env = default_environment;

		trajectory = pttLine;
		t_param1 = 0.0f;
		t_param2 = 0.0f;

		mass = 1.0f;

		max_x_vel = 0.0f;
		max_y_vel = 0.0f;

		walk_acc = 0.0f;
		jump_vel = 0.0f;
		drops_shadow = false;
		shadow_width = 0.25f;

		lifetime = 0.0f;
		facing_left = false;

		movementDirectionX = true;

		movement = omtIdling;
		facing = ofNormal;
		

		ghostlike = false;
	}

	void SetFacing( bool mirrored )
	{
		if ( this->childrenConnection && facing_left ^ mirrored )
			this->childrenConnection->Event( ObjectEventInfo( eventFacingChange ) );
		facing_left = mirrored;
		if ( !sprite ) return;
		if ( facing == ofFixed ) return;
		if ( mirrored )
			ofMoonwalking == facing ? this->sprite->ClearMirrored() : this->sprite->SetMirrored();
		else
			ofMoonwalking == facing ? this->sprite->SetMirrored() : this->sprite->ClearMirrored();
	}

	bool GetFacing()
	{
		return facing_left;
	}

	void PhysProcess();

	bool CheckSuspectedPlane();
	void CheckUnderlyngPlane();
	void DropShadow();

	void SetEnv(ObjEnvironment* e);
};


#endif // __OBJECT_DYNAMIC_H_

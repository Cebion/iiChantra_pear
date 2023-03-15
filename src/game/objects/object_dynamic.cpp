#include "StdAfx.h"

// #include "../phys/phys_misc.h"

#include "object_dynamic.h"

#include "../phys/phys_collisionsolver.h"
#include "../phys/sap/OPC_ArraySAP.h"
#include "../../config.h"

extern Opcode::ArraySAP *asap;
extern float CAMERA_RIGHT;
extern float CAMERA_LEFT;
extern config cfg;


bool ObjDynamic::CheckSuspectedPlane()
{
	if ( !suspected_plane || suspected_plane->slopeType != 0 ) return false;
	if ( this->IsDropping() && suspected_plane->IsOneSide() ) 
		return false;
	return ( !( this->aabb.Left() > suspected_plane->aabb.Right() || this->aabb.Right() < suspected_plane->aabb.Left() )
		&&  this->aabb.Bottom()+2 >= suspected_plane->aabb.Top() && this->aabb.Top() < suspected_plane->aabb.Top() );
}

void ObjDynamic::PhysProcess()
{
	if (IsSleep())
		return;
	bool movement_occured = false;

	switch ( trajectory )
		{
			case pttRipple:
				{
					t_value++;
					float l = vel.Length();
					if ( l == 0 ) break;
					float tr_x = t_param1*( -vel.x/l )*sin(t_value*t_param2 );
					float tr_y = t_param1*( vel.y/l )*sin(t_value*t_param2);
					aabb.p += Vector2( tr_x, tr_y );
					movement_occured = true;
					break;
				}
			case pttCosine:
				{
					t_value++;
					Vector2 tr = vel.GetPerp().Normalized() * (t_param1 * cos(t_value*t_param2));
					aabb.p += tr;
					movement_occured = true;
					break;
				}
			case pttGlobalSine:
				{
					t_value++;
					Vector2 tr = Vector2(0, 1) * (t_param1 * cos(t_value*t_param2));
					aabb.p += tr;
					movement_occured = true;
					break;
				}
			case pttSine:
				{
					t_value++;
					Vector2 tr = vel.GetPerp().Normalized() * (t_param1 * cos(t_value*t_param2));
					aabb.p += tr;
					movement_occured = true;
					break;
				}
			case pttRandom:
				{
					Vector2 tr = vel.GetPerp().Normalized() * Random_Float(-t_param1, t_param1);
					Vector2 tr2 = vel.GetPerp().GetPerp().Normalized() * Random_Float(-t_param2, t_param2);
					movement_occured = true;
					aabb.p += tr;
				}
			default:
				{
					break;
				}
		}

	if ( lifetime > 0 )
	{
		if ( lifetime <= 1 )
		{
			lifetime = 0;
			if ( this->sprite && this->sprite->GetAnimation("die") )
			{
				this->activity = oatDying;
				this->SetAnimation("die", false);
			}
			else
				this->SetDead();
			return;
		}
		else
		{
			lifetime -= 1;
		}
	}

	if ( old_aabb != aabb )
	{
		old_aabb = aabb;
		movement_occured = true;
	}
	Vector2 old_v = vel;

	this->ClearOnPlane();
	if ( CheckSuspectedPlane() )
	{
		// Сдвигаем с объектом на котором стоим. Если стоим на склоне, сюда не попадаем.
		if (suspected_plane->IsDynamic()) 
		{
			this->aabb.p += ((ObjDynamic*)suspected_plane)->vel + 0.5f * ((ObjDynamic*)suspected_plane)->acc;
			this->aabb.p += ((ObjDynamic*)suspected_plane)->env->gravity_bonus;
			this->aabb.p.y = suspected_plane->aabb.Top() - aabb.H;	//Could be worse.
		}
		this->SetOnPlane();
		drop_from = NULL;
	}
	else 
	{
		//Если это склон, то мы можем быть внутри него, но не на нём, так что не трогаем
		if (suspected_plane && suspected_plane->slopeType == 0) 
		{
			// Не склон и не стоим
			this->suspected_plane = NULL;
		}
		//Самое время проверить, не зашли ли мы сразу на другую поверхность.
		CheckUnderlyngPlane();
	}

	// Гравитируем только если объект не на плоскости
	if (!IsOnPlane())
	{
		vel += acc + gravity;
		aabb.p += env->gravity_bonus;
	}
	else
	{
		vel += acc;
		vel.x += gravity.x;
		aabb.p.x += env->gravity_bonus.x;
	}

	//Если стояли на склоне, то скорость направлена по нему.
	if ( suspected_plane && SolveVSSlope(this->aabb, suspected_plane, Vector2(0,1)) && (!IsNear(old_v.x, 0, 0.5)) && !(suspected_plane->IsOneSide() && this->IsDropping()) )
	{
		//double angle = atan( suspected_plane->slopeType == 1 ? suspected_plane->aabb.H/suspected_plane->aabb.W : suspected_plane->aabb.W/suspected_plane->aabb.H ) + (vel.x < 0 ? 2*PI : 0);
		//vel = Vector2(vel.x*cos( angle ) + vel.y*sin(angle), -vel.x*sin(angle) + vel.x*cos(angle));
		//Но если не видно разницы, то зачем считать больше?
		float next_x = aabb.p.x+old_v.x+acc.x/2;
		float shift = GetSlopeTop(suspected_plane, next_x) - GetSlopeTop(suspected_plane, aabb.p.x);
		if (shift > 0) 
		{
			old_v.y += shift + 1;
		}
		if ( old_v.x > 0 && shift < 0 ) old_v.x += shift/2;
		if ( old_v.x < 0 && shift < 0 ) old_v.x -= shift/2;
		//vel.x -= 2*GetSlopeX(suspected_plane, aabb.p.y+vel.y) - GetSlopeX(suspected_plane, aabb.p.y);
	}

	if (vel.x > max_x_vel)
		vel.x = max_x_vel;
	else if (vel.x < -max_x_vel)
		vel.x = -max_x_vel;

	if (vel.y > max_y_vel)
		vel.y = max_y_vel;
	else if (vel.y < -max_y_vel)
		vel.y = -max_y_vel;

#ifdef USE_TIME_DEPENDENT_PHYS
	aabb.p += old_v + acc * 0.5f;
#else
	aabb.p += vel;
#endif // USE_TIME_DEPENDENT_PHYS
	
	if (IsOnPlane())
		vel.x *= 0.75f;
	else
		vel.x *= 0.9999f;

	//if (old_env != default_environment && env == default_environment)
	//	old_env->OnLeave(this);
	//old_env = env;
	//env = default_environment;

	// Скажи нет идеальным геометрическим фигурам
	assert(aabb.H != 0);
	assert(aabb.W != 0);

	
	if (aabb != old_aabb || movement_occured)
	{
		if ( this->childrenConnection )
			this->childrenConnection->Event( ObjectEventInfo( eventMoved, aabb.p - old_aabb.p ) );
		asap->UpdateObject(sap_handle, aabb.GetASAP_AABB());
	}

	// Проевряем, есть ли плоскость под объектом.
	//CheckUnderlyngPlane();
}

// Проверка наличия плоскости "под"
using namespace Opcode;

void ObjDynamic::CheckUnderlyngPlane()
{
	//if (!this->IsOnPlane())
	//	return;

	assert(asap);

	const ASAP_Box* boxes = asap->GetBoxes();
	const ASAP_EndPoint* epY = asap->GetEndPoints(Y_);
	assert(boxes);
	assert(epY);

	const ASAP_Box* object = boxes + this->sap_handle;

	const Opcode::ASAP_EndPoint* current = epY + (boxes + this->sap_handle)->mMax[Y_];
	const ValType limit = current->mValue + 1;

	if ( SolveVSSlope(this->aabb, this->suspected_plane, Vector2(0, 1)) && (!this->suspected_plane->IsOneSide() || !SolveVSSlope(this->aabb, this->suspected_plane, Vector2(0, 0) )) )
	{
		this->SetOnPlane();
		return;
	}

	const ASAP_AABB& asab_aabb = this->aabb.GetASAP_AABB();

	while ((++current)->mValue <= limit)
	{
		if (!current->IsMax())
		{
			if ( ((ObjDynamic*)(boxes[current->GetOwner()].mObject))->IsSolid() 
			&& ((ObjPhysic*)(boxes[current->GetOwner()].mObject))->slopeType == 0 )
			{
				const ASAP_Box* id1 = boxes + current->GetOwner();
				// Our max passed a min => start overlap
				if (object != id1 &&
					Intersect2D(*object, *id1, X_, Z_) &&
					Intersect1D_Min(asab_aabb, *id1, epY, Y_) &&
					!( this->IsDropping() && ((ObjPhysic*)boxes[current->GetOwner()].mObject)->IsOneSide() ) )
				{
					this->SetOnPlane();
					drop_from = NULL;
					this->suspected_plane = (ObjPhysic*)(boxes[current->GetOwner()].mObject);
					return;
				}
			}

		}
	}
	this->ClearOnPlane();
	
}

void ObjDynamic::DropShadow()
{
	//Если тени отключены - ничего не делать.
	if ( !cfg.shadows )
		return;

	//Спящие объекты не отбрасывают тени.
	if ( this->IsSleep() )
		return;

	//Объекты, не отбрасывающие тень нас не интересуют.
	if ( !this->drops_shadow )
		return;

	//Нас не интересуют тени, которые мы не можем увидеть.
	if ( this->aabb.Left() > CAMERA_RIGHT || this->aabb.Right() < CAMERA_LEFT ) 
		return;

	//Если мы стоим на поверхности - тень прямо под ногами.
	if ( this->IsOnPlane() )
	{
		float y = this->aabb.Bottom();
		float left = max(  aabb.Left(),  this->suspected_plane->aabb.Left() );
		float right = min( aabb.Right(), this->suspected_plane->aabb.Right());
		float w = 0.5f*(right-left);
		RenderEllipse( left+w, y, w, shadow_width*aabb.W , SHADOW_Z, RGBAf(SHADOW_COLOR), this->id, 90);
		return;
	}

	assert(asap);

	const ASAP_Box* boxes = asap->GetBoxes();
	const ASAP_EndPoint* epY = asap->GetEndPoints(Y_);
	assert(boxes);
	assert(epY);

	const ASAP_Box* object = boxes + this->sap_handle;

	const Opcode::ASAP_EndPoint* current = epY + (boxes + this->sap_handle)->mMax[Y_];
	const ValType limit = current->mValue + SHADOW_DISTANCE;

	if ( SolveVSSlope(this->aabb, this->suspected_plane, Vector2(0, 1)) && (!this->suspected_plane->IsOneSide() || !SolveVSSlope(this->aabb, this->suspected_plane, Vector2(0, 0) )) )
	{
		//TODO
		return;
	}

	const ASAP_AABB& asab_aabb = this->aabb.GetASAP_AABB();

	while ((++current)->mValue <= limit)
	{
		if (!current->IsMax())
		{
			if ( ((ObjDynamic*)(boxes[current->GetOwner()].mObject))->IsSolid() 
			&& ((ObjDynamic*)(boxes[current->GetOwner()].mObject))->slopeType == 0 )
			{
				const ASAP_Box* id1 = boxes + current->GetOwner();
				// Our max passed a min => start overlap
				if ((object != id1 &&
					Intersect2D(*object, *id1, X_, Z_) &&
					Intersect1D_Min(asab_aabb, *id1, epY, Y_)) )
				{
					GameObject* shadow_plane = ((GameObject*)boxes[current->GetOwner()].mObject);
					float w = aabb.W;
					float y = shadow_plane->aabb.Top();
					float dist = y - aabb.Bottom();
					w *= 1.0f - dist / (SHADOW_DISTANCE + 1.0f);
					float left = max(  aabb.p.x - w, shadow_plane->aabb.Left() );
					float right = min( aabb.p.x + w, shadow_plane->aabb.Right());
					w = 0.5f*(right-left);
					RenderEllipse( left + w, y, w, shadow_width*w, SHADOW_Z, RGBAf(SHADOW_COLOR), this->id, 90);
					return;
				}
			}

		}
	}
}


void ObjDynamic::SetEnv(ObjEnvironment* e)
{
	if (e != env)
	{
		if (env != default_environment)
			env->OnLeave(this);
		if (e != default_environment)
			e->OnEnter(this);

		env = e;
	}
}
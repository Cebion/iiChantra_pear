#include "StdAfx.h"
#include "object_effect.h"
#include "../phys/sap/OPC_ArraySAP.h"
#include "object_sprite.h"
#include "../proto.h"

#include "../../resource_mgr.h"
extern Opcode::ArraySAP *asap;

//////////////////////////////////////////////////////////////////////////

extern ResourceMgr<Proto> * protoMgr;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ObjEffect* CreateEffect(const char* proto_name, bool fixed, ObjDynamic* origin, USHORT origin_point, const Vector2& disp)
{
	const Proto* proto = protoMgr->GetByName(proto_name, "effects/");

	if (!proto)
		return NULL;

	if (!(origin && origin->sprite && origin_point < origin->sprite->mpCount))
	{
		sLog(DEFAULT_LOG_NAME, logLevelError, "Попытка создать эффект %s на несуществующей точке крепления %d у %s (id = %d)", 
			proto_name, origin_point, (origin && origin->sprite) ? origin->sprite->proto_name : "", origin ? origin->id : -1);
		return NULL;
	}

	ObjEffect* object = new ObjEffect(origin, origin_point, disp);
	object->type = objEffect;

	if (LoadObjectFromProto(proto, (GameObject*)object))
	{
		DELETESINGLE(object);
		return NULL;
	}

	if ( proto->mpCount )
	{
		object->sprite->mpCount = proto->mpCount;
		object->sprite->mp = new Vector2[ object->sprite->mpCount ];
		int i;
		for ( i = 0; i < object->sprite->mpCount; i++ )
			object->sprite->mp[i] = Vector2(0, 0);
	}

	if (fixed) object->sprite->SetFixed();

	object->ClearPhysic();
	object->SetAnimation("idle", true);
	object->flash_target = Vector4(1.0, 1.0, 1.0, 1.0);

//#ifdef COORD_LEFT_UP_CORNER
//	object->aabb.p.x += object->sprite->frameWidth * 0.5f;
//	object->aabb.p.y += object->sprite->frameHeight * 0.5f;
//#endif // COORD_LEFT_UP_CORNER

	Vector2 disp2 = Vector2(disp);
	Vector2 mp = origin->sprite->mp[ origin_point ];
	if ( origin->sprite->IsMirrored() )
	{
		disp2.x *= -1;
		mp.x *= -1;
		object->SetFacing( true );
	}
	object->aabb.p = origin->aabb.p + disp2 + mp;

	object->SetPhysic();
	object->ghostlike = true;
	if (proto->phys_bullet_collidable) object->SetBulletCollidable();
	object->SetDynamic();
	object->flash_stage = 0;

	AddObject(object);

	return object;
}

void ObjEffect::ReceiveDamage( UINT ammount )
{
	this->health -= ammount;
	if ( this->health <= 0 )
		this->SetAnimation("die", false);
	else if ( flash_stage <= 0 )
	{
		this->flash_target = RGBAf(1.0f, 0.5f, 0.0f, 1.0f);
		this->flash_stage = 1;
		this->flash_speed = 2;
	}
}

void ObjEffect::Process()
{
	if (IsSleep())
		return;

	if ( this->sprite && this->sprite->overlayCount > 0 )
	{
		if ( flash_stage != 0 )
		{
			if (!this->sprite->ocolor)
				sLog(DEFAULT_LOG_NAME, logLevelError, "ObjEffect::Process() ocolor == NULL, прототип %s", this->sprite->proto_name);
			else
				this->sprite->ocolor[0] = RGBAf(1.0, 1.0, 1.0, 0.0) + (flash_target - RGBAf(1.0, 1.0, 1.0, 0.0))*(abs((float)flash_stage)/flash_speed);
			if ( flash_stage < 0 && flash_stage > -1 ) flash_stage = 0;
			else flash_stage++;
			if ( flash_stage > flash_speed ) flash_stage = -flash_speed; 
		}
		else
		{
			if (this->sprite->ocolor[0].a != 0)
				this->sprite->ocolor[0].a = 0;
		}
	}
}

void ObjEffect::ParentEvent( ObjectEventInfo info )
{
	switch ( info.type )
	{
		case eventDead:
			{
				this->SetDead();
				break;
			}
		case eventMPChanged:
			{
				if ( info.num == this->origin_point )
					this->aabb.p += info.coord;
				break;
			}
		case eventMoved:
		case eventCollisionPushed:
			{
				this->aabb.p += info.coord;
				break;
			}
		case eventFacingChange:
			{
				this->disp = this->aabb.p - this->parentConnection->getParent()->aabb.p;
				this->disp.x *= -1;
				this->vel.x *= -1;
				this->acc.x *= -1;
				this->SetFacing( !this->GetFacing() );
				this->aabb.p = this->parentConnection->getParent()->aabb.p + this->disp;
				break;
			}
		default: break;
	}
}


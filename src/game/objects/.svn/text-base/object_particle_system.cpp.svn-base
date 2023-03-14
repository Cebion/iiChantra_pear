#include "StdAfx.h"
#include "object_particle_system.h"
#include "../../misc.h"
#include "../phys/phys_misc.h"
#include "object.h"
#include "../../resource_mgr.h"

//////////////////////////////////////////////////////////////////////////

extern ResourceMgr<Proto> * protoMgr;
extern ResourceMgr<Texture> * textureMgr;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

GameObject* CreateParticleSystem( const char * proto_name, Vector2 coord, UINT emitter, int mode )
{	
	const Proto* proto = protoMgr->GetByName( proto_name, "particles/" );
	if (proto == NULL)
	{
		return NULL;
	}

	GameObject* obj = new GameObject();
	obj->ParticleSystem = new CParticleSystem();

	obj->ParticleSystem->Init();

	obj->ParticleSystem->Info.sc = proto->start_color;
	obj->ParticleSystem->Info.ec = proto->end_color;
	obj->ParticleSystem->Info.vc = proto->var_color;
	obj->ParticleSystem->texture = textureMgr->GetByName(proto->texture);
	obj->ParticleSystem->Info.emission = proto->emission;
	obj->ParticleSystem->Info.MaxParticles = proto->max_particles;
	obj->ParticleSystem->Info.lifemin = (int)proto->particle_life_min;
	obj->ParticleSystem->Info.lifemax = (int)proto->particle_life_max;

	obj->ParticleSystem->Info.min_speed = proto->min_speed;
	obj->ParticleSystem->Info.max_speed = proto->max_speed;
	obj->ParticleSystem->Info.min_angle = proto->min_angle;
	obj->ParticleSystem->Info.max_angle = proto->max_angle;
	obj->ParticleSystem->Info.min_param = proto->min_param;
	obj->ParticleSystem->Info.max_param = proto->max_param;
	obj->ParticleSystem->Info.min_trace = proto->min_trace;
	obj->ParticleSystem->Info.max_trace = proto->max_trace;
	obj->ParticleSystem->Info.trajectory = (TrajectoryType)proto->trajectory;
	obj->ParticleSystem->Info.t_param1 = proto->trajectory_param1;
	obj->ParticleSystem->Info.t_param2 = proto->trajectory_param2;
	obj->ParticleSystem->Info.wind_affected = proto->affected_by_wind == 1;
	obj->ParticleSystem->Info.gravity = Vector2( proto->gravity_x, proto->gravity_y );

	obj->ParticleSystem->Info.startsize = proto->start_size;
	obj->ParticleSystem->Info.sizevar = proto->size_variability;
	obj->ParticleSystem->Info.endsize = proto->end_size;

	obj->ParticleSystem->Info.plife = proto->particle_life;
	obj->ParticleSystem->Info.plifevar = (int)proto->particle_life_var;

	obj->ParticleSystem->Info.life = proto->system_life;

	obj->ParticleSystem->Info.p = coord;
	obj->ParticleSystem->z = proto->z;

	obj->ParticleSystem->emitter = emitter;
	obj->ParticleSystem->mode = mode;
	if ( emitter && (mode & 4) )
	{
		GameObject* emit_obj = GetGameObject( emitter );
		if ( emit_obj )
		{
			CAABB* emit_aabb = &emit_obj->aabb;
			Vector2 * pts = new Vector2[2];
			pts[1] = Vector2(emit_aabb->p.x-emit_aabb->W/*/2*/,emit_aabb->p.y);
			pts[0] = Vector2(emit_aabb->p.x+emit_aabb->W/*/2*/,emit_aabb->p.y);
			obj->ParticleSystem->SetGeometry(pts, 2);
		}
	}
	obj->type = objParticleSystem;

	if (LoadObjectFromProto(proto, obj))
	{
		DELETESINGLE(obj);
		return NULL;
	}

	AddObject(obj);
	return obj;

}

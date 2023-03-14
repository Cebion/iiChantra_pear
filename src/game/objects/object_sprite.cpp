#include "StdAfx.h"

//#include "../../render/texture.h"

//#include "../phys/phys_misc.h"
//#include "../animation.h"
//#include "../sprite.h"
//#include "../proto.h"

#include "object_sprite.h"
#include "object_effect.h"
#include "../sprite.h"

#include "../../resource_mgr.h"

//////////////////////////////////////////////////////////////////////////

extern ResourceMgr<Proto> * protoMgr;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

GameObject* CreateColorBox(const CAABB& aabb, float z, const RGBAf& color)
{
#ifdef MAP_EDITOR
	ObjPhysic* obj = new ObjPhysic;
#else
	GameObject* obj = new GameObject;
#endif // MAP_EDITOR
	obj->aabb = aabb;

	obj->sprite = new Sprite();
	obj->sprite->color = color;
	obj->sprite->render_without_texture = true;
	obj->sprite->SetVisible();
	obj->sprite->z = z;
	obj->sprite->frameWidth = (USHORT)aabb.W * 2;
	obj->sprite->frameHeight = (USHORT)aabb.H * 2;
	obj->type = objSprite;
#ifdef MAP_EDITOR
	obj->SetPhysic();
#endif // MAP_EDITOR
	AddObject(obj);
	return obj;
}

GameObject* CreateSprite(const Proto* proto, Vector2 coord, bool fixed, const char* start_anim, bool resize, CAABB size)
{
	if (!proto)
		return NULL;

	GameObject* object = NULL;
	if (proto->physic)
	{
		ObjPhysic* o = new ObjPhysic;
		o->sap_handle = 0;
		o->objFlags = 0;
		o->physFlags = 0;
		o->SetPhysic();
		object = (GameObject*)o;
	}
	else if (proto->effect)
	{
		ObjEffect* o = new ObjEffect;
		o->type = objEffect;
		object = (GameObject*)o;
	}
	else
	{
#ifdef MAP_EDITOR
		object = new ObjPhysic;
#else
		object = new GameObject;
#endif // MAP_EDITOR
		object->objFlags = 0;
	}

	if (LoadObjectFromProto(proto, object))
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
	if (object->type != objEffect) object->type = objSprite;  //Ерунда какая-то. Надо поискать, где создаются не спрайты.

	if (fixed) object->sprite->SetFixed();
	object->aabb.p = coord;

	// TODO: Кривой хак. Хоя вероятно и перерастет в постояный. Но лучше ничего не придумал.
	// В этот момент объектв в SAP еще не добавлен, поэтому в SetAnimation нельзя еще обновлять
	// его состояние в SAP.
	object->ClearPhysic();
	object->SetAnimation(start_anim ? start_anim : "idle", true);

#ifdef COORD_LEFT_UP_CORNER
	//	object->aabb.p.x += object->sprite->frameWidth * 0.5f;
	//	object->aabb.p.y += object->sprite->frameHeight * 0.5f;
	if (object->sprite->tex)
	{
		object->aabb.p.x += object->sprite->tex->frame->size.x * 0.5f;
		object->aabb.p.y += object->sprite->tex->frame->size.y * 0.5f;
	}
	else
	{
		object->aabb.p.x += object->sprite->frameWidth * 0.5f;
		object->aabb.p.y += object->sprite->frameHeight * 0.5f;
	}
#endif // COORD_LEFT_UP_CORNER
	if (resize) 
	{
		object->aabb = size;
#ifdef MAP_EDITOR
		object->grouped = true;
#endif //MAP_EDITOR
	}

	if (proto->physic) object->SetPhysic();

#ifdef MAP_EDITOR
	object->SetPhysic();
#endif // MAP_EDITOR

	AddObject(object);

	return object;
}

GameObject* CreateSprite(const char* proto_name, Vector2 coord, bool fixed, const char* start_anim, CAABB size)
{
	if (!proto_name)
		return NULL;
	GameObject* obj = CreateSprite(protoMgr->GetByName(proto_name, "sprites/"), coord, fixed, start_anim, true, size);
	obj->sprite->renderMethod = rsmRepeatXY;
	//obj->aabb = size;
#ifdef MAP_EDITOR
	obj->proto_name = new char[ strlen(proto_name) + 1 ];
	strcpy( obj->proto_name, proto_name );
	obj->creation_shift = obj->aabb.p - coord;
#endif //MAP_EDITOR
	return obj;
}

GameObject* CreateSprite(const char* proto_name, Vector2 coord, bool fixed, const char* start_anim)
{
	if (!proto_name)
		return NULL;

	GameObject* go = CreateSprite(protoMgr->GetByName(proto_name, "sprites/"), coord, fixed, start_anim, false, CAABB());
#ifdef MAP_EDITOR
	go->proto_name = new char[ strlen(proto_name) + 1 ];
	strcpy( go->proto_name, proto_name );
	go->creation_shift = go->aabb.p - coord;
#endif //MAP_EDITOR
	return go;
}

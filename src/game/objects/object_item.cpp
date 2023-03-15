#include "StdAfx.h"
#include "object_item.h"
#include "object.h"
#include "../../misc.h"

#include "../../resource_mgr.h"

//////////////////////////////////////////////////////////////////////////

extern ResourceMgr<Proto> * protoMgr;

//////////////////////////////////////////////////////////////////////////

void ObjItem::Touch( ObjDynamic* obj )
{
	//Касание предметом предмета ничего не даёт, для пули своя обработка, мёртвые предметов не берут.
	if ( obj->type == objItem || obj->IsBullet() || obj->activity == oatDying || obj->IsDead() ) return;
	this->sprite->stack->Push( obj->id );
	this->SetAnimation( "touch", false );
}

void ObjItem::Process()
{
	this->aabb.p += this->vel;
}

ObjItem* CreateItem(const char* proto_name, Vector2 coord, const char* start_anim)
{
	const Proto* proto = protoMgr->GetByName( proto_name, "items/" );
	if (!proto)
		return NULL;

	ObjItem* item = new ObjItem();

	if (LoadObjectFromProto(proto, (GameObject*)item))
	{
		DELETESINGLE(item);
		return NULL;
	}

	item->aabb.p = coord;


	// TODO: Кривой хак. Хоя вероятно и перерастет в постояный. Но лучше ничего не придумал.
	// В этот момент объектв в SAP еще не добавлен, поэтому в SetAnimation нельзя еще обновлять
	// его состояние в SAP.
	item->type = objItem;
	item->ClearPhysic();
	item->SetAnimation(start_anim ? start_anim : "init", true);
	item->SetPhysic();
	item->SetDynamic();

#ifdef COORD_LEFT_UP_CORNER
	item->aabb.p.x += item->sprite->frameWidth * 0.5f;
	item->aabb.p.y += item->sprite->frameHeight * 0.5f;
#endif // COORD_LEFT_UP_CORNER

	AddObject(item);

#ifdef MAP_EDITOR
	item->proto_name = new char[ strlen(proto_name) + 1 ];
	strcpy( item->proto_name, proto_name );
	item->creation_shift = item->aabb.p - coord;
#endif //MAP_EDITOR

	return item;
}

ObjItem* CreateItem(const char* proto_name, Vector2 coord, const char* start_anim, CAABB size)
{
	ObjItem* item = CreateItem(proto_name, coord, start_anim);
	if ( item )
	{
		item->old_aabb = item->aabb;
		item->aabb = size;
		if ( item->sprite )
		{
			item->sprite->renderMethod = rsmRepeatXY;
		}
#ifdef MAP_EDITOR
		item->grouped = true;
#endif //MAP_EDITOR
	}
	return item;
}
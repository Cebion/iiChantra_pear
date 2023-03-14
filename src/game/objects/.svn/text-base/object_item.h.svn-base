#ifndef __OBJECT_ITEM_H_
#define __OBJECT_ITEM_H_

#include "object_dynamic.h"
#include "object.h"

class ObjItem : public ObjDynamic
{
public:
	void Process();
	void Touch( ObjDynamic* obj );
};
ObjItem* CreateItem(const char* proto_name, Vector2 coord, const char* start_anim);
ObjItem* CreateItem(const char* proto_name, Vector2 coord, const char* start_anim, CAABB size);
#endif //__OBJECT_ITEM_H_


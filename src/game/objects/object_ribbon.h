#ifndef __OBJECT_RIBBON_H_
#define __OBJECT_RIBBON_H_

#include "object.h"
#include "object_physic.h"

class ObjRibbon: 
#ifdef MAP_EDITOR
	public ObjPhysic
#else
	public GameObject
#endif //MAP_EDITOR
{
public:
	float bl, br, bt, bb;
	bool ubl, ubr, ubt, ubb;		// bounds usage
	bool repeat_x, repeat_y;		// направления повторения
	Vector2 k;
#ifdef MAP_EDITOR
	bool from_proto;
#endif //MAP_EDITOR

	enum ORBoundSide { orBoundLeft, orBoundRight, orBoundTop, orBoundBottom };

	float getBound( ORBoundSide side );

	void setBounds( float x1, float y1, float x2, float y2 );
	void setBoundsUsage( bool x1, bool y1, bool x2, bool y2 );
	void setRepitition( bool x, bool y );

	virtual void Draw();

	ObjRibbon()
	{
		type = objRibbon;
		k = Vector2(1,1);
		ubl = ubr = ubt = ubb = false;
		repeat_x = repeat_y = true;
	}
};

ObjRibbon* CreateRibbon( const char* texture, Vector2 k, Vector2 coord, float z, bool from_proto );

#endif //__OBJECT_RIBBON_H_
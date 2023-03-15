#include "StdAfx.h"
#include "object_ribbon.h"
#include "../camera.h"
#include "../sprite.h"
#include "../../render/texture.h"
#include "../../resource_mgr.h"
#include "../../config.h"

extern float CAMERA_BOTTOM;
extern float CAMERA_TOP;
extern float CAMERA_LEFT;
extern float CAMERA_RIGHT;
extern float CAMERA_X;
extern float CAMERA_Y;
extern ResourceMgr<Proto> * protoMgr;
extern ResourceMgr<Texture> * textureMgr;

float ObjRibbon::getBound( ORBoundSide side )
{
	switch (side)
	{
	case orBoundLeft:
		return ubl ? max( bl + k.x*CAMERA_X, CAMERA_LEFT ) : CAMERA_LEFT;
	case orBoundRight:
		return ubr ? min( br + k.x*CAMERA_X, CAMERA_RIGHT ) : CAMERA_RIGHT;
	case orBoundTop:
		return ubt ? max( bt + k.y*CAMERA_Y, CAMERA_TOP ) : CAMERA_TOP;
	case orBoundBottom:
		return ubb ? min( bb + k.y*CAMERA_Y, CAMERA_BOTTOM ) : CAMERA_BOTTOM;
	}
}

extern config cfg;

void ObjRibbon::Draw()
{
	//Такого быть не должно, но если вдруг...
	if ( this->sprite == NULL )
		return;

#ifdef MAP_EDITOR
	RenderSprite( aabb.p.x-aabb.W, aabb.p.y-aabb.H, 1, 0, 0, 2*aabb.W, 2*aabb.H, NULL, false, RGBAf(DEBUG_WAYPOINT_COLOR) );
	if ( this->show_border )
	{
		RenderLine( CAMERA_LEFT, getBound(orBoundTop), CAMERA_RIGHT, getBound(orBoundTop), 1, RGBAf(1, 0, 0, 1));
		RenderLine( CAMERA_LEFT, getBound(orBoundBottom), CAMERA_RIGHT, getBound(orBoundBottom), 1, RGBAf(0, 1, 0, 1));
		RenderLine( getBound(orBoundLeft), CAMERA_TOP, getBound(orBoundLeft), CAMERA_BOTTOM, 1, RGBAf(0, 0, 1, 1));
		RenderLine( getBound(orBoundRight), CAMERA_TOP, getBound(orBoundRight), CAMERA_BOTTOM, 1, RGBAf(1, 1, 0, 1));
	}
#endif //MAP_EDITOR

	UINT framenum = this->sprite->currentFrame;
	FrameInfo* f = (this->sprite->tex->frame + framenum);
	this->sprite->frameWidth = (USHORT)f->size.x;
	this->sprite->frameHeight = (USHORT)f->size.y;

	//Проверяем, попали ли границы на экран в принипе.
	if ( ubr && getBound(orBoundRight) < CAMERA_LEFT )
		return;
	if ( ubl && getBound(orBoundLeft) > CAMERA_RIGHT )
		return;
	if ( ubb && getBound(orBoundBottom) < CAMERA_TOP )
		return;
	if ( ubt && getBound(orBoundTop) > CAMERA_BOTTOM )
		return;

	Vector2 coord = aabb.p + Vector2( k.x*CAMERA_X, k.y*CAMERA_Y ) - Vector2( aabb.W, aabb.H );
	coord -= Vector2( this->sprite->realX, this->sprite->realY );
	if ( !repeat_x && (coord.x + sprite->frameWidth < getBound(orBoundLeft) || coord.x > getBound(orBoundRight)) )
		return;
	if ( !repeat_y && (coord.y + sprite->frameHeight < getBound(orBoundTop) || coord.y > getBound(orBoundBottom)) )
		return;
	//Даже если попали по границе - могли оказаться за кадром.
	if ( !repeat_x && (coord.x + this->sprite->frameWidth < CAMERA_LEFT || coord.x - this->sprite->frameWidth > CAMERA_RIGHT) )
		return;
	if ( !repeat_y && (coord.y + this->sprite->frameHeight < CAMERA_TOP || coord.y - this->sprite->frameHeight > CAMERA_BOTTOM) )
		return;

	//"Отматываем" на ближайшее повторение по координатам, по которым повторяем.
	if ( repeat_x )
	{
		while ( coord.x + this->sprite->frameWidth < getBound(orBoundRight) )
			coord.x += this->sprite->frameWidth;
		while ( coord.x + this->sprite->frameWidth > getBound(orBoundLeft) )
			coord.x -= this->sprite->frameWidth;
	}
	if ( repeat_y  )
	{
		while ( coord.y + this->sprite->frameHeight < getBound(orBoundBottom) )
			coord.y += this->sprite->frameHeight;
		while ( coord.y + this->sprite->frameHeight > getBound(orBoundTop) )
			coord.y -= this->sprite->frameHeight;
	}
	Vector2 bs = Vector2( max(getBound(orBoundLeft) - coord.x - this->sprite->frameWidth, 0.0f), max(getBound(orBoundTop) - coord.y - this->sprite->frameHeight, 0.0f) );
	if ( repeat_x ) coord.x = getBound(orBoundLeft);
	if ( repeat_y ) coord.y = getBound(orBoundTop);
	if ( !repeat_x && coord.x < getBound(orBoundLeft) )
	{
		bs.x += getBound(orBoundLeft) - coord.x;
		coord.x = getBound(orBoundLeft);
	}

	Vector2 edge = Vector2( getBound(orBoundRight), getBound(orBoundBottom) );

	if (coord.x != edge.x && coord.y != edge.y)
	{
		ASSERT(coord.x < edge.x);
		ASSERT(coord.y < edge.y);
	
		RenderSpriteCyclic(coord, this->sprite->z, edge, bs, f, this->sprite->tex, this->sprite->color, this->sprite->IsMirrored(), repeat_x, repeat_y);
	}
}

void ObjRibbon::setBounds( float x1, float y1, float x2, float y2 )
{
	bl = x1;
	bt = y1;
	br = x2;
	bb = y2;
}

void ObjRibbon::setBoundsUsage( bool x1, bool y1, bool x2, bool y2 )
{
	ubl = x1;
	ubr = x2;
	ubt = y1;
	ubb = y2;
}

void ObjRibbon::setRepitition(bool x, bool y)
{
	repeat_x = x;
	repeat_y = y;
}

ObjRibbon* CreateRibbon( const char* texture, Vector2 k, Vector2 coord, float z, bool from_proto )
{
	ObjRibbon* ribbon = new ObjRibbon();
	ribbon->k = k;
	ribbon->aabb.p = coord;
	ribbon->aabb.W = 16;
	ribbon->aabb.H = 16;
	if ( from_proto )
	{
		Proto* proto = protoMgr->GetByName(texture, "sprites/");
		ribbon->sprite = new Sprite( proto );
		ribbon->SetAnimation( "idle", false );
	}
	else
	{
		ribbon->sprite = new Sprite();
		ribbon->sprite->tex = textureMgr->GetByName( texture );
		ribbon->sprite->frameWidth = (USHORT)ribbon->sprite->tex->frame->size.x;
		ribbon->sprite->frameHeight = (USHORT)ribbon->sprite->tex->frame->size.y;
	}
	ribbon->sprite->z = z;

#ifdef MAP_EDITOR
	ribbon->from_proto = from_proto;
	ribbon->proto_name = StrDupl(texture);
	ribbon->creation_shift = ribbon->aabb.p - coord;
#endif //MAP_EDITOR

	AddObject( ribbon );

	return ribbon;
}

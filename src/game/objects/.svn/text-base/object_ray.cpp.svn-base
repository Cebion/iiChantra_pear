#include "StdAfx.h"

#include "object_ray.h"

#include "object.h"
#include "object_character.h"
#include "object_bullet.h"
#include "object_sprite.h"

#include "../../resource_mgr.h"

//////////////////////////////////////////////////////////////////////////

extern ResourceMgr<Proto> * protoMgr;

ObjRay::ObjRay()
{
	type = objRay;
	debug_color = RGBAf(1.0f, 1.0f, 1.0f, 1.0f);
	collisionsCount = 1;
	rayFlags = 0;
	//SetUseCollisionCounter();
	timeToLive = 0;
	//SetUseTTLCounter();
	searchDistance = 0.0f;

	damage = 0;
	damage_type = 0;
	push_force = 0.0f;
	multiple_targets = hurts_same_type = false;

	next_shift_y = 0;

	parentConnection = new ObjectConnection( NULL );
}

ObjRay::ObjRay( GameObject* shooter )
{
	type = objRay;
	debug_color = RGBAf(1.0f, 1.0f, 1.0f, 1.0f);
	collisionsCount = 1;
	rayFlags = 0;
	//SetUseCollisionCounter();
	timeToLive = 0;
	//SetUseTTLCounter();
	searchDistance = 0.0f;

	damage = 0;
	damage_type = 0;
	push_force = 0.0f;
	multiple_targets = hurts_same_type = false;

	next_shift_y = 0;

	if (shooter)
	{
		ObjectConnection::addChild( shooter, this );
		this->shooter_type = shooter->type;
	}
}

ObjRay::~ObjRay()
{
	DELETEARRAY(this->end_effect);
}

BOOL LoadRayFromProto(const Proto* proto, ObjRay* ray)
{
	if (!proto || !ray)
		return TRUE;

	ray->damage_type = proto->damage_type;
	ray->damage = proto->bullet_damage;
	if (proto->multiple_targets > 0)
	{
		ray->SetUseCollisionCounter();
		ray->collisionsCount = proto->multiple_targets;
	}
	ray->push_force = proto->push_force;
	ray->hurts_same_type = proto->hurts_same_type != 0;
	if (proto->time_to_live > 0)
	{
		ray->SetUseTTLCounter();
		ray->timeToLive = proto->time_to_live;
	}

	ray->sprite = new Sprite(proto);

	ray->next_shift_y = proto->next_shift_y;

	ray->end_effect = StrDupl(proto->end_effect);

	return FALSE;
}


ObjRay* CreateRay(float x1, float y1, float x2, float y2)
{
	ObjRay* ray = new ObjRay;
	
	ray->ray = CRay(x1, y1, x2, y2);

	AddObject(ray);

	return ray;
}

ObjRay* CreateRay(const Proto* proto, Vector2 coord, ObjCharacter* shooter, WeaponDirection wd)
{
	if (!proto)
		return NULL;
	
	float x1 = coord.x;
	float y1 = coord.y;

	switch (wd)
	{
	case wdLeft: 
		x1 -= 1.0f;
		break;
	case wdRight:
		x1 += 1.0f;
		break;
	case wdUpLeft:
		x1 -= 1.0f;
		y1 -= 1.0f;
		break;
	case wdUpRight:
		x1 += 1.0f;
		y1 -= 1.0f;
		break;
	case wdDownLeft:
		x1 -= 1.0f;
		y1 += 1.0f;
		break;
	case wdDownRight:
		x1 += 1.0f;
		y1 += 1.0f;
		break;
	}

	ObjRay* ray = new ObjRay( shooter );

	ray->ray = CRay(coord.x, coord.y, x1, y1);

	ray->aabb.p = coord;
	
	if (/*LoadObjectFromProto(proto, (GameObject*)ray) ||*/
		LoadRayFromProto(proto, ray))
	{
		DELETESINGLE(ray);
		return NULL;
	}

	if (ray->sprite)
	{
		switch (wd)
		{
		case wdLeft:
			ray->sprite->SetMirrored();
		case wdRight:
			ray->SetAnimation("straight", true);
			break;
		
		
		case wdUpLeft:
			ray->sprite->SetMirrored();
		case wdUpRight:
			ray->SetAnimation("diagup", true);
			break;

		
		case wdDownLeft:
			ray->sprite->SetMirrored();
		case wdDownRight:
			ray->SetAnimation("diagdown", true);
			break;
		}
	}

	AddObject(ray);
	
	return ray;
}

ObjRay* CreateRay(const char* proto_name, Vector2 coord, ObjCharacter* shooter, WeaponDirection wd)
{
	if (!proto_name)
		return NULL;

	return CreateRay(protoMgr->GetByName(proto_name, "projectiles/"), coord, shooter, wd);
}

void ObjRay::Process()
{
	if (IsUseCollisionCounter() && collisionsCount == 0)
	{
		//this->SetDead();
		this->activity = oatDying;
	}

	if (IsUseTTLCounter())
	{
		if (timeToLive > 0)
			timeToLive--;
		else
			//this->SetDead();
			this->activity = oatDying;
	}

	if (this->activity == oatDying)
	{
		if (!this->sprite || this->sprite->IsAnimDone())
		{
			this->SetDead();
		}
	}
}

// Служит для отсеивания объектов, с которыми луч геометрически пересекается, 
// но согласно игровой логике они не пересекаются
bool ObjRay::CheckIntersection(ObjPhysic* obj)
{
	//Пули никогда не пересекаются с КОНТРАЛАЙК-платформами. Гранаты и прочее - отскакивают, но это в решении столкновений.
	if (obj->IsOneSide())
		return false;

	if (!obj->IsBulletCollidable())
		return false;

	GameObject* shooter = this->parentConnection->getParent();
	if ( shooter && shooter == obj )
		return false;

	if (this->shooter_type == obj->type && !this->hurts_same_type)
		return false;

	return true;
}


bool ObjRay::Hit(ObjPhysic* obj)
{
	if (this->IsDead() || (activity == oatDying/* && !this->IsUseCollisionCounter()*/) )
		return false;

	// Эта проверка как бы и не нужна, все равно она уже вызывалась
	//if (!this->CheckIntersection(obj))
	//	return false;
	assert(this->CheckIntersection(obj));

	if (obj->IsDynamic())
	{
		if (  obj->sprite->GetAnimation("pain") )
		{
			// TODO: приложить силу
			//long double ang = atan( fabs(this->aabb.p.y - obj->aabb.p.y)/fabs(this->aabb.p.x - obj->aabb.p.x) );
			//Vector2 push = Vector2( push_force*cos(ang), push_force*sin(ang) );
			//if ( (push.x < 0 && this->aabb.p.x < obj->aabb.p.x) || (push.x > 0 && this->aabb.p.x > obj->aabb.p.x) ) push.x *= -1;
			//if ( (push.y < 0 && this->aabb.p.y < obj->aabb.p.y) || (push.y > 0 && this->aabb.p.y > obj->aabb.p.y) ) push.y *= -1;
			//((ObjDynamic*)obj)->vel += push;
		}
		switch (obj->type)
		{
		case objItem:
			{
				//Стоит добавить здоровье, но пока - сразу уничтожаем
				if ( obj->sprite->cur_anim != "die" )
					obj->SetAnimation("die", true);
			}
			break;
		case objPlayer:
		case objEnemy:
			{
				((ObjCharacter*)obj)->ReceiveDamage(this->damage, this->damage_type);
				break;
			}
		case objBullet:
			{
				if (obj->parentConnection == this->parentConnection) return false;
				ObjBullet* ob = (ObjBullet*)obj;
				if (!ob->multiple_targets) ob->activity = oatDying;
				else ob->activity = oatUndead;
				break;
			}
		default: break;
		}

		//if (!multiple_targets) this->activity = oatDying;
		//else this->activity = oatUndead;
	}
	else
	{
		this->collisionsCount = 0;
		this->activity = oatDying;
	}

	Vector2 c;
	bool got_c_point = false;
	if (this->end_effect && this->activity != oatDying)
	{
		this->ray.GetIntersectionPoint(obj->aabb, c);
		got_c_point = true;

		GameObject* spr = CreateSprite(this->end_effect, c, false, "idle");
#ifdef COORD_LEFT_UP_CORNER
		// Костыль, исправляющий эффект другого костыля
		// Нужно, чтобы вспышка от попадания лазера была центром
		// именно в точке пересечения.
		if (spr->sprite->tex)
		{
			spr->aabb.p.x -= spr->sprite->tex->frame->size.x * 0.5f;
			spr->aabb.p.y -= spr->sprite->tex->frame->size.y * 0.5f;
		}
		else
		{
			spr->aabb.p.x -= spr->sprite->frameWidth * 0.5f;
			spr->aabb.p.y -= spr->sprite->frameHeight * 0.5f;
		}
#endif // COORD_LEFT_UP_CORNER

	}

	if (this->IsUseCollisionCounter() && this->collisionsCount > 0)
	{
		this->collisionsCount--;		
	}

	if (this->collisionsCount == 0)
	{
		this->activity = oatDying;

		if (!got_c_point)
			this->ray.GetIntersectionPoint(obj->aabb, c);

		this->ray_end_point = c;
		this->SetDrawRayEndPoint();
	}

	

	return true;
}



#include "../object_manager.h"

extern LSQ_HandleT RayTree;


void ObjRay::Draw()
{
	CRay& r = this->ray;

	extern float CAMERA_LEFT;
	extern float CAMERA_RIGHT;
	extern float CAMERA_TOP;
	extern float CAMERA_BOTTOM;

	Sprite* s = this->sprite;
	RGBAf color = RGBAf(1.0f, 1.0f, 1.0f, 1.0f);
	float z = 1.0f;
	if (s)
	{
		if (!s->IsVisible())
			return;

		color = s->color;
		z = s->z;		
	}

	if (r.IsIntersecting(CAMERA_LEFT, CAMERA_RIGHT, CAMERA_TOP, CAMERA_BOTTOM))
	{
		float x1 = r.p.x;
		float y1 = r.p.y;
		float x2;
		float y2;

		if (this->IsDrawRayEndPoint())
		{
			x2 = ray_end_point.x;
			y2 = ray_end_point.y;
		}
		else
		{
			// TODO: Вычислять x1/y1, если начало луча в кадр не попадает.
			if (r.IsVertical())
			{
				x2 = r.p.x;
				if (r.k > 0)
					y2 = CAMERA_BOTTOM;
				else
					y2 = CAMERA_TOP;
			}
			else if(r.IsHorizontal())
			{
				y2 = r.p.y;
				if (r.dir)
					x2 = CAMERA_RIGHT;
				else
					x2 = CAMERA_LEFT;
			}
			else
			{
				if (r.dir)
				{
					if (!r.GetIntersectionPoint(CAMERA_RIGHT, CAMERA_TOP, CAMERA_RIGHT, CAMERA_BOTTOM, x2, y2))
						if (!r.GetIntersectionPoint(CAMERA_LEFT, CAMERA_BOTTOM, CAMERA_RIGHT, CAMERA_BOTTOM, x2, y2))
							if (!r.GetIntersectionPoint(CAMERA_LEFT, CAMERA_TOP, CAMERA_RIGHT, CAMERA_TOP, x2, y2))
								return;
				}
				else
				{
					if (!r.GetIntersectionPoint(CAMERA_LEFT, CAMERA_TOP, CAMERA_LEFT, CAMERA_BOTTOM, x2, y2))
						if (!r.GetIntersectionPoint(CAMERA_LEFT, CAMERA_BOTTOM, CAMERA_RIGHT, CAMERA_BOTTOM, x2, y2))
							if (!r.GetIntersectionPoint(CAMERA_LEFT, CAMERA_TOP, CAMERA_RIGHT, CAMERA_TOP, x2, y2))
								return;
				}
			}	

		}


		//RenderLine(x1, y1, 
		//	x2, y2, 
		//	z, 
		//	color);

		if (s && !s->render_without_texture && s->tex)
		{	
			coord2f_t* frame_coord = NULL;
			FrameInfo* f = NULL;
			f = s->tex->frame + s->currentFrame;


			if (s->IsMirrored())
			{
				x1 -= 2*f->size.x - s->realWidth - s->realX;
				y1 -= s->realY;
				frame_coord = f->coordMir;
			}
			else
			{
				x1 -= s->realX;
				y1 -= s->realY;
				frame_coord = f->coord;
			}

			// Условие (ray.k > 0) == (bool)s->IsMirrored() сработает для UpRight и UpLeft
			if (!ray.IsHorizontal() &&  (ray.k > 0) == s->IsMirrored())
			{
				y1 -= f->size.y;
			}


			float shift_y = (float) (ray.IsHorizontal() ? 0 : ( ray.k > 0 ? (int)s->realHeight - next_shift_y : next_shift_y - s->realHeight ) );
			float shift_x = (float) (ray.dir ? s->realWidth : -s->realWidth);
			if (!ray.dir)
				shift_y = -shift_y;

			int i = 0;
			do 
			{
				if (ray.dir)
				{
					if (x1 + shift_x >= x2)
					{
						coord2f_t c = f->size;
						c.x = x2 - x1;
						RenderSpritePart(x1, y1, z, &f->size, &c, frame_coord, (bool)s->IsMirrored(), s->tex, color);
						break;
					}


				}
				else
				{
					if (x1 <= x2)
					{
						coord2f_t c = f->size;
						c.x -= x2 - x1;
						RenderSpritePart(x2, y1, z, &f->size, &c, frame_coord, (bool)s->IsMirrored(), s->tex, color);
						break;
					}
				}


				RenderSprite(x1, y1, z, &f->size, frame_coord, s->tex, color);

				x1 += shift_x;
				y1 += shift_y;

				i++;

				// TODO: Эти ray.dir и k > 0 в условии - костыль. Првильно сделать - вычислить x1/y1, если начало луча не видно.
			} while((x1 + s->frameWidth > CAMERA_LEFT || ray.dir) && (x1 < CAMERA_RIGHT || !ray.dir) && 
					(y1 + s->frameHeight > CAMERA_TOP || ray.k>0) && (y1 < CAMERA_BOTTOM || ray.k<0) );

		}
		else
		{
			RenderLine(x1, y1, 
				x2, y2, 
				z, 
				color);
		}
	}
}



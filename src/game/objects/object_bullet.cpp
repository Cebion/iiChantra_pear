#include "StdAfx.h"

#include "object_character.h"

#include "object_bullet.h"
#include "object_enemy.h"
#include "object_effect.h"
#include "object_player.h"
#include "math.h"

#include "../../resource_mgr.h"

//////////////////////////////////////////////////////////////////////////

extern ResourceMgr<Proto> * protoMgr;

//////////////////////////////////////////////////////////////////////////

//#define SHOOTER_SPEED_ADDS_TO_BULLETS

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Загрузка пули из прототипа. Рассчет скорости пули.
BOOL LoadBulletFromProto(const Proto* proto, ObjBullet* bullet, WeaponDirection wd)
{
	if (!proto || !bullet)
		return TRUE;

	switch (wd)
	{
	case wdLeft:
	case wdRight:
		bullet->acc.x = proto->bullet_vel;
		bullet->acc.y = 0;
		break;
	case wdUpLeft:
	case wdUpRight:
		bullet->acc.x = proto->bullet_vel * ((scalar)M_SQRT2 * 0.5f);	// cos 45
		bullet->acc.y = -proto->bullet_vel * ((scalar)M_SQRT2 * 0.5f);	// sin 45
		break;
	case wdDownLeft:
	case wdDownRight:
		bullet->acc.x = proto->bullet_vel * ((scalar)M_SQRT2 * 0.5f);	// cos 45
		bullet->acc.y = proto->bullet_vel * ((scalar)M_SQRT2 * 0.5f);	// sin 45
		break;
	}

	switch (wd)
	{
	case wdLeft:
	case wdUpLeft:
	case wdDownLeft:
		bullet->SetFacing( true );
		break;
	default: break;
	}

	if (bullet->GetFacing())
		bullet->acc.x = -bullet->acc.x;

	bullet->damage_type = proto->damage_type;
	bullet->max_x_vel = fabs(bullet->acc.x);
	bullet->max_y_vel = fabs(bullet->acc.y);
	bullet->vel = bullet->acc;	// БОЛЬШИЕ СОМНЕНИЯ. Почему изначально присваивалось ускорение, а не скорость?

	bullet->damage = proto->bullet_damage;

	bullet->multiple_targets = proto->multiple_targets != 0;
	bullet->push_force = proto->push_force;

	bullet->hurts_same_type = (proto->hurts_same_type & 1) != 0;
	bullet->hurts_same_faction = (proto->hurts_same_type & 2) != 0;

	bullet->t_value = Random_Float( proto->min_param, proto->max_param );
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////

ObjBullet* CreateBullet(const Proto* proto, Vector2 coord, ObjCharacter* shooter, WeaponDirection wd)
{
	if (!proto)
		return NULL;

	ObjBullet* bullet = new ObjBullet(shooter);



	if (LoadObjectFromProto(proto, (GameObject*)bullet) ||
		LoadBulletFromProto(proto, bullet, wd))
	{
		DELETESINGLE(bullet);
		return NULL;
	}

	// TODO: Кривой хак. Хоя вероятно и перерастет в постояный. Но лучше ничего не придумал.
	// В этот момент объектв в SAP еще не добавлен, поэтому в SetAnimation нельзя еще обновлять
	// его состояние в SAP.
	bullet->ClearPhysic();

	switch (wd)
	{
	case wdLeft:
	case wdRight:
		bullet->SetAnimation("straight", true);
		break;
	case wdUpLeft:
	case wdUpRight:
		bullet->SetAnimation("diagup", true);
		break;
	case wdDownLeft:
	case wdDownRight:
		bullet->SetAnimation("diagdown", true);
		break;
	}

	bullet->SetPhysic();

	bullet->aabb.p = coord;
	bullet->trajectory = (TrajectoryType)proto->trajectory;
	bullet->t_param1 = proto->trajectory_param1;
	bullet->t_param2 = proto->trajectory_param2;

	AddObject(bullet);

	return bullet;
}

ObjBullet* CreateBullet(const Proto* proto, Vector2 coord, ObjCharacter* shooter, Vector2 aim, int angle)
{
	// TODO: Если угол совсем не используется, то тогда его и передават не надо.
	UNUSED_ARG(angle);
	if (!proto)
		return NULL;

	ObjBullet* bullet = CreateBullet(proto, coord, shooter, wdLeft);
		
	bullet->ClearPhysic();
	bullet->SetAnimation("straight", true);
	float ang = atan2( aim.y - coord.y, aim.x - coord.x );
	bullet->vel = Vector2(bullet->max_x_vel*cos(ang), bullet->max_x_vel*sin(ang) );
	if ( aim.x > coord.x ) 
		bullet->sprite->SetMirrored();
	if ( aim.x < coord.x && bullet->vel.x > 0) bullet->vel.x *= -1;
	bullet->acc = bullet->vel;
	bullet->max_y_vel = fabs(bullet->vel.y);
	bullet->max_x_vel = fabs(bullet->vel.x);
	bullet->SetPhysic();
	bullet->SetDynamic();
	bullet->SetBulletCollidable();

	return bullet;
}

ObjBullet* CreateBullet(const char* proto_name, Vector2 coord, ObjCharacter* shooter, WeaponDirection wd)
{
	if (!proto_name)
		return NULL;

	return CreateBullet(protoMgr->GetByName(proto_name, "projectiles/"), coord, shooter, wd);
}

ObjBullet* CreateAngledBullet(const char* proto_name, Vector2 coord, GameObject* shooter, bool mirror, int angle, int dir)
{
	if (!proto_name)
		return NULL;

	ObjBullet * bul = CreateBullet(protoMgr->GetByName(proto_name, "projectiles/"), coord, (ObjCharacter*)shooter, wdRight);
	if (!bul) return NULL;
	float ang = (float)PI*angle/180.0f;
	bul->vel = Vector2( bul->vel.x*cos(ang), bul->vel.x*sin(ang) );
	if ( mirror )
	{
		bul->sprite->SetMirrored();
		bul->vel *= -1;
	}
	if ( dir > 4 || dir < -4 )
	{
		if ( mirror ) bul->sprite->ClearMirrored();
		else bul->sprite->SetMirrored();
		if ( dir == 9 || dir == -9 ) dir = 0;
		else if ( dir < 0 ) dir += 5;
		else dir -= 5;
	}
	bul->acc = bul->vel;
	bul->max_x_vel = fabs(bul->vel.x);
	bul->max_y_vel = fabs(bul->vel.y);
	//Это должен быть параметр-строка, но это точно породит уточек, так что пока такой трюк.
	if ( dir == 4 )
		bul->SetAnimation("straightup", false);
	if ( dir == -4 )
		bul->SetAnimation("straightdown", false);
	if ( dir == 3 )
		bul->SetAnimation("highup", false);
	if ( dir == -3 )
		bul->SetAnimation("highdown", false);
	if ( dir == 2 )
		bul->SetAnimation("diagup", false);
	if ( dir == -2 )
		bul->SetAnimation("diagdown", false);
	if ( dir == 1 )
		bul->SetAnimation("slightlyup", false);
	if ( dir == -1 )
		bul->SetAnimation("slightlydown", false);
	return bul;
}

ObjBullet* CreateBullet(const char* proto, Vector2 coord, ObjCharacter* shooter, Vector2 aim, int angle)
{
	if (!proto)
		return NULL;
	return CreateBullet(protoMgr->GetByName(proto, "projectiles/"), coord, shooter, aim, angle);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ObjBullet::Process()
{
	if (activity == oatUndead)
		activity = oatDying;
	if (activity == oatDying)
	{
		this->SetAnimation("die", false);
		if ( this->sprite->IsAnimDone() ) this->SetDead();
	}
	if ( this->vel.x < 0 && this->sprite ) this->sprite->SetMirrored();
}

void ObjBullet::PhysProcess()
{
	switch ( trajectory )
		{
			case pttOrbit:
				{
					GameObject* shooter = (ObjDynamic*)this->parentConnection->getParent();
					if ( shooter )
					{
						Vector2 center_point = shooter->aabb.p;
						t_value += t_param1;
						Vector2 tr = center_point + t_param2 * Vector2( cos( t_value ), sin( t_value) );
						if ( (tr - aabb.p).x >= 0 ) this->SetFacing( false ); 
						else this->SetFacing( true );
						vel = tr - aabb.p;
					}
					break;
				}
			default:
				{
					break;
				}
		}
	ObjDynamic::PhysProcess();
}

// Попадание пули во что-то (пересечение ее с физическим объектом)
void ObjBullet::Hit(ObjPhysic* obj)
{
	if (this->IsDead() || (activity == oatDying /*&& !multiple_targets*/) )
		return;

	//Пули никогда не пересекаются с КОНТРАЛАЙК-платформами. Гранаты и прочее - отскакивают, но это в решении столкновений.
	if (obj->IsOneSide())
		return;

	if (!obj->IsBulletCollidable())
		return;

	GameObject* shooter = this->parentConnection->getParent();
	if ( shooter && shooter == obj )
		return;

	if (this->shooter_type == obj->type && !this->hurts_same_type)
		return;

	if (!this->hurts_same_faction && shooter && (shooter->type == objEnemy || shooter->type == objPlayer) && !((ObjCharacter*)shooter)->IsEnemy(obj))
		return;

	DELETEARRAY(this->mem_anim);
	this->mem_anim = StrDupl(this->sprite->cur_anim.c_str());
	this->mem_frame = this->sprite->GetAnimation(this->sprite->cur_anim_num)->current;
	this->mem_tick = internal_time - this->sprite->prevAnimTick;

	if (obj->IsDynamic())
	{
		//Не отталкиваем прикреплённые объекты, а также объекты с бесконечной (отрицательной) массой.
		if ( obj->type != objEffect && !(obj->type == objEnemy && ((ObjEnemy*)obj)->attached) && ((ObjDynamic*)obj)->mass >= 0)
		{
			//Вот такое решение, чтобы взрывы отталкивали от центра, а обычные выстрелы по скорости.
			//Возможно стоит проверять multiple_targets
			float ang = 0; 
			if ( this->vel.x != 0 || this->vel.y != 0 )
				ang = atan2( this->vel.y, this->vel.x );
			else
				ang = atan2( obj->aabb.p.y - this->aabb.p.y, obj->aabb.p.x - this->aabb.p.x );
			Vector2 push = push_force*Vector2( cos(ang), sin(ang) );
			ObjDynamic* od = (ObjDynamic*)obj;
			if ( od->mass == 0 )
				od->acc += push;
			else
			{
				od->vel += push * (1/od->mass);
				od->aabb.p += push * (1/od->mass);
			}
		}
		switch (obj->type)
		{
		case objItem:
			{
				//Стоит добавить здоровье, но пока - сразу уничтожаем
				obj->SetAnimation("die", false);
			}
			break;
		case objPlayer:
			{
				((ObjPlayer*)obj)->ReceiveDamage(this->damage, this->damage_type);
				break;
			}
		case objEnemy:
			{
				((ObjCharacter*)obj)->ReceiveDamage(this->damage, this->damage_type);
				break;
			}
		case objEffect:
			{
				((ObjEffect*)obj)->ReceiveDamage(this->damage);
				break;
			}
		case objBullet:
			{
				//Поскольку мы сохраняем единственный экземпляр соединения для всех детей достаточно проверить само соединение, а не родителя
				if (obj->parentConnection && obj->parentConnection == this->parentConnection) break;
				ObjBullet* ob = (ObjBullet*)obj;
				if ( !ob->IsShielding() ) //Только если другой выстрел не поглощающий.
				{
					if (!ob->multiple_targets) ob->activity = oatDying;
					else ob->activity = oatUndead;
				}
				if ( this->IsShielding() ) return; //Если поглощаем выстрелы, то не начинаем анимацию попадания.
				break;
			}
		default: break;
		}

		if (!multiple_targets) this->activity = oatDying;
		else if (this->activity != oatDying) this->activity = oatUndead;
	}
	else
	{
		// Проверка здесь - сомнительный способ сделать так, что бы взрывы
		// не зацикливали анимации, из-за чего они не удалялись
		//if (activity != oatDying && activity != oatUndead)
		if (activity != oatDying)
		{
			this->SetAnimation("miss", false);
		}
	}
}

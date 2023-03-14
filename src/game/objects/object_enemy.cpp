#include "StdAfx.h"

#include "../../misc.h"
#include "object_enemy.h"
#include "object_spawner.h"
#include "object_dynamic.h"
#include "object_bullet.h"
#include "object_waypoint.h"
#include "../player.h"
#include "../phys/sap/SweepAndPrune.h"

#include "../../resource_mgr.h"

//////////////////////////////////////////////////////////////////////////

extern ResourceMgr<Proto> * protoMgr;
extern map<int, GameObject*> factions;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ObjEnemy* CreateEnemy(const char* protoname, Vector2 coord, const char* start_anim)
{
	const Proto* proto = protoMgr->GetByName(protoname, "enemies/");
	if (!proto)
		return NULL;

	ObjEnemy* enemy = new ObjEnemy;
	if (LoadObjectFromProto(proto, (GameObject*)enemy))
	{
		DELETESINGLE(enemy);
		return NULL;
	}

	enemy->LoadFactionInfo(proto);
	enemy->aabb.p = coord;


	// TODO: Кривой хак. Хоя вероятно и перерастет в постояный. Но лучше ничего не придумал.
	// В этот момент объектв в SAP еще не добавлен, поэтому в SetAnimation нельзя еще обновлять
	// его состояние в SAP.
	enemy->ClearPhysic();
	enemy->SetAnimation(start_anim ? start_anim : "init", true);
	enemy->SetPhysic();
	enemy->gravity = Vector2(proto->gravity_x, proto->gravity_y);

#ifdef COORD_LEFT_UP_CORNER
	enemy->aabb.p.x += enemy->sprite->frameWidth * 0.5f;
	enemy->aabb.p.y += enemy->sprite->frameHeight * 0.5f;
#endif // COORD_LEFT_UP_CORNER

	if ( proto->mpCount )
	{
		enemy->sprite->mpCount = proto->mpCount;
		enemy->sprite->mp = new Vector2[ enemy->sprite->mpCount ];
		int i;
		for ( i = 0; i < enemy->sprite->mpCount; i++ )
			enemy->sprite->mp[i] = Vector2(0, 0);
	}

	enemy->offscreen_behavior = (CharacterOffscreenBeh)proto->offscreen_behavior;
	enemy->offscreen_distance = proto->offscreen_distance;

	enemy->SetPhysic();
	enemy->SetDynamic();

	AddObject(enemy);
#ifdef MAP_EDITOR
	enemy->proto_name = new char[ strlen(protoname) + 1 ];
	strcpy( enemy->proto_name, protoname );
	enemy->creation_shift = enemy->aabb.p - coord;
#endif //MAP_EDITOR

	enemy->InitFaction();

	return enemy;
}

extern Player* playerControl;
extern float CAMERA_LEFT;
extern float CAMERA_RIGHT;
extern float CAMERA_BOTTOM;
extern float CAMERA_TOP;

void ObjEnemy::Process()
{
	if ( this->offscreen_behavior != cobNone && (
		 this->aabb.GetMax(0) + this->offscreen_distance < CAMERA_LEFT || 
		 this->aabb.GetMin(0) - this->offscreen_distance > CAMERA_RIGHT ||
		 this->aabb.GetMax(1) + this->offscreen_distance < CAMERA_TOP ||
		 this->aabb.GetMin(1) - this->offscreen_distance > CAMERA_BOTTOM ) )
	{
		switch ( offscreen_behavior )
		{
			case cobSleep:
				{
					this->SetSleep();
					return;
				}
			case cobDie:
				{
					this->SetDead();
					if ( this->parentConnection && this->parentConnection->getParent()->type == objSpawner && this->activity != oatDying )
					{
						((ObjSpawner*)this->parentConnection->getParent())->charges++;
					}
					break;
				}
			case cobAnim:
				{
					this->SetAnimation("offscreen", false);
					break;
				}
			default: break;
		}
	}
	else if ( offscreen_behavior == cobSleep && this->IsSleep() )
		this->ClearSleep();

	if (IsSleep())
		return;

	if ( target )
	{
		GameObject* otarget = target->getParent();
		if ( !otarget || ((ObjCharacter*)otarget)->activity == oatDying || !otarget->sprite->IsVisible() || otarget->IsSleep() )
		{
			target->childDead( id );
			target = NULL;
			if ( this->activity != oatDying )
			{
				if ( this->sprite->GetAnimation("target_dead") )
					SetAnimation("target_dead", true);
				else
					SetAnimation("idle", true);
			}
		}
	}

	if ( this->movement == omtMovingToWaypoint && this->current_waypoint != 0 && this->activity != oatDying && this->waypoint_speed > 0 )
	{
		ObjWaypoint* wp = NULL;

		if ( waypoint_mode & 1 ) //Берём точку с абсолютным номером
			wp = GetWaypoint( waypoint_global );
		else
			wp = (ObjWaypoint*)GetGameObject( current_waypoint );

		if (!wp)
		{
			vel = Vector2(0, 0);
			acc = Vector2(0, 0);
			sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, 	"Waypoint id=%d doesn't exists (enemy id=%d)", current_waypoint, this->id);
			movement = omtIdling;
			current_waypoint = 0;
			return;
		}

		float accuracy_x = wp->aabb.W;
		float accuracy_y = wp->aabb.H;
		if ( waypoint_mode & 64 ) //Достаточно коснуться.
		{
				accuracy_x += aabb.W;
				accuracy_y += aabb.H;
		}
		float speed_limit = waypoint_speed_limit;
		if (   abs(aabb.p.x - wp->aabb.p.x) < accuracy_x
			&& abs(aabb.p.y - wp->aabb.p.y) < accuracy_y )
		{
			current_waypoint = wp->pointReached( this );
			waypoint_start = Vector2(this->aabb.p.x, this->aabb.p.y);
		}
		else
		{
			float speed = waypoint_speed; 
			if ( waypoint_mode & 16 && (wp->aabb.p - aabb.p).Length() < vel.Length() + acc.Length()/2 )
				speed = -speed;
			if ( waypoint_mode & 128 ) //Тем более замедляемся, чем ближе мы к цели.
			{
				{
					float path = abs( (wp->aabb.p - waypoint_start).Length() );
					float dist = abs( (wp->aabb.p - aabb.p).Length() );
					if ( dist < path/4 )
					{
						float k = 4*dist/path;
						if ( waypoint_mode & 8 ) //Если скорость, то не можем позволить падать до нуля.
							k = max( k, 0.25f );
						speed *= k;
					}
					else if ( dist > 3*path/4 && dist < path )
					{
						float k = 4*( path/4 - ( dist - 3*path/4 ))/path;
						if ( waypoint_mode & 8 ) //Если скорость, то не можем позволить падать до нуля.
							k = max( k, 0.25f );
						speed *= k;
					}
				}
			}
			float angle = atan2( wp->aabb.p.y - aabb.p.y, wp->aabb.p.x - aabb.p.x );
			Vector2 speed_bonus = Vector2();
			speed_bonus.x = cos(angle) * speed;
			speed_bonus.y = sin(angle) * speed;
			if ( waypoint_mode & 32 ) //Не пролетаем мимо.
			{
				Vector2 approx_point = aabb.p + vel + 0.5f*speed_bonus;
				if ( waypoint_mode & 8 )
				{
					approx_point = aabb.p + speed_bonus + 0.5f*acc;
					if ( (approx_point.x + aabb.W > wp->aabb.Right() && aabb.p.x < wp->aabb.Right()) ||
						 (approx_point.x - aabb.W < wp->aabb.Left() && aabb.p.x > wp->aabb.Left()) )
						 speed_bonus.x = wp->aabb.p.x - aabb.p.x - 0.5f*acc.x;
					if ( (approx_point.y + aabb.H > wp->aabb.Bottom() && aabb.p.y < wp->aabb.Bottom()) ||
						 (approx_point.y - aabb.H < wp->aabb.Top() && aabb.p.y > wp->aabb.Top()) )
						 speed_bonus.y = wp->aabb.p.y - aabb.p.y - 0.5f*acc.y;
				}
				else
				{
					if ( (approx_point.x + aabb.W > wp->aabb.Right() && aabb.p.x < wp->aabb.Right()) ||
						 (approx_point.x - aabb.W < wp->aabb.Left() && aabb.p.x > wp->aabb.Left()) )
						 speed_bonus.x = 2*(wp->aabb.p.x - aabb.p.x - vel.x);
					if ( (approx_point.y + aabb.H > wp->aabb.Bottom() && aabb.p.y < wp->aabb.Bottom()) ||
						 (approx_point.y - aabb.H < wp->aabb.Top() && aabb.p.y > wp->aabb.Top()) )
						 speed_bonus.y = 2*(wp->aabb.p.y - aabb.p.y - vel.y);
				}
			}
			if ( abs( wp->aabb.p.x - aabb.p.x ) < wp->aabb.W && speed_bonus.y != 0 ) speed_bonus.x = 0;
			if ( abs( wp->aabb.p.y - aabb.p.y ) < wp->aabb.H && speed_bonus.x != 0 ) speed_bonus.y = 0;
			if ( waypoint_speed_limit == 0 )
				speed_limit = max( abs(vel.x), abs(vel.y) );
			//oe->waypoint_speed = speed_bonus;
			if ( (speed_bonus.x > 0 && vel.x > speed_limit) || (speed_bonus.x < 0 && vel.x < -speed_limit) )
				speed_bonus.x = 0;
			if ( (speed_bonus.y > 0 && vel.y > speed_limit) || (speed_bonus.y < 0 && vel.y < -speed_limit) )
				speed_bonus.y = 0;
			if ( waypoint_mode & 8 )	//Скорость, а не ускорение
			{
				if ( gravity.y != 0 )
					vel.x = speed_bonus.x;
				else
					vel = speed_bonus;
			}
			else
			{
				if ( gravity.y != 0 )
					acc.x = speed_bonus.x;
				else
					acc = speed_bonus;
			}
		}
	}

	Sprite* s = this->sprite;
	ASSERT(s);

	ObjectMovementType old_movement = this->movement;

	if ( activity == oatDying )
	{
		movement = omtIdling;
		if ( health > 0 ) 
		{
			if ( target) target->children.push_back( this );
			movement = old_movement;
			activity = oatIdling;
			this->SetAnimation( "respawn", false );
		}
		else 
		{
			if (s->IsAnimDone())
			{
				this->SetDead();
			}
			return;
		}
	}

	if ( !IsOnPlane() && gravity.y != 0 )
	{
		if ( movement != omtMovingToWaypoint ) movement = omtJumping;
	}
	else if ( movement != omtMovingToWaypoint )
	{
		movement = omtWalking;
	}

	// Запуск анимаций
	if ( this->gravity.y != 0.0f && this->activity != oatDying )
	{
		if ( this->movement == omtJumping && old_movement != omtJumping  )
		{
			if ( this->activity != oatDying && this->sprite->cur_anim != "touch" ) 
				this->SetAnimationIfPossible("jump", true);
		}
		if ( this->movement != omtJumping && old_movement == omtJumping )
		{
			if ( this->activity != oatDying && this->sprite->cur_anim != "touch" )
				this->SetAnimationIfPossible("land", true);
		}
	}

	if ( this->vel.x > 0 )
		this->SetFacing( false );
	else if ( this->vel.x < 0 )
		this->SetFacing( true );

}

extern UINT internal_time;

bool ObjEnemy::ReduceHealth( UINT damage )
{
	bool ret = ObjCharacter::ReduceHealth( damage );
	//if ( ret && target )
	//	target->childDead(id);
	return ret;
}

void ObjEnemy::Touch(ObjPhysic *obj)
{
	if ( !this->IsTouchable() ) return;
	if ( obj->type == objBullet && obj->parentConnection->getParent() && obj->parentConnection->getParent() == this) return;
	Animation* a = this->sprite->GetAnimation("touch");
	if (a)
	{
		this->ClearTouchable();
		DELETEARRAY(mem_anim);
		mem_anim = StrDupl(this->sprite->cur_anim.c_str());
		mem_frame = sprite->GetAnimation(this->sprite->cur_anim_num )->current;
		if ( mem_tick == 0 ) mem_tick = internal_time;
		this->sprite->stack->Push(obj->id);
		this->SetAnimation("touch", false);
	}
}

void ObjEnemy::ParentEvent( ObjectEventInfo info )
{
	if ( !attached ) return;
	if ( !parentConnection->getParent() ) return;
	if ( activity == oatDying ) return;
	vel.x = vel.y = 0;
	switch ( info.type )
	{
		case eventDead:
			{
				this->SetAnimation( "die", false );
				break;
			}
		case eventMoved:
		case eventCollisionPushed:
			{
				aabb.p = parentConnection->getParent()->aabb.p;
				break;
			}
		case eventFacingChange:
			{
				this->vel.x *= -1;
				this->acc.x *= -1;
				this->SetFacing( !this->GetFacing() );
				break;
			}
		default: break;
	}
}

void ObjEnemy::UpdateTarget(ObjCharacter* target)
{
	if (!target) return;
	if ( this->target )
		this->target->childDead(id);
	target->targeted_by->children.push_back( this );
	this->target = target->targeted_by;
}

ObjCharacter* ObjEnemy::GetTarget()
{
	ObjCharacter* ret = NULL;
	if ( target ) ret = (ObjCharacter*)target->getParent();
	return ret;
}

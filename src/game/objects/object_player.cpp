#include "StdAfx.h"

#include "../../misc.h"

#include "object_player.h"
#include "object_environment.h"
#include "../../config.h"
#include "../../input_mgr.h"
#include "../phys/sap/SweepAndPrune.h"

#include "../player.h"

#include "../../resource_mgr.h"

//////////////////////////////////////////////////////////////////////////

extern ResourceMgr<Proto> * protoMgr;

extern Player* playerControl;

//////////////////////////////////////////////////////////////////////////

BOOL LoadPlayerFromProto(const Proto* proto, ObjPlayer* player)
{
	player->weapon = CreateWeapon(proto->main_weapon);
	player->alt_weapon = CreateWeapon(proto->alt_weapon);

	player->cur_weapon = player->weapon;

	if (proto->health > 0)
		player->health = (short)proto->health;

	return FALSE;
}

ObjPlayer* CreatePlayer(const Proto* proto, Vector2 coord, const char* start_anim)
{
	if (!proto)
		return NULL;

	ObjPlayer* player = new ObjPlayer;
	if (LoadObjectFromProto(proto, (GameObject*)player) ||
		LoadPlayerFromProto(proto, player))
	{
		DELETESINGLE(player);
		return NULL;
	}

	player->LoadFactionInfo( proto );
	player->aabb.p = coord;

	// TODO: Кривой хак. Хоя вероятно и перерастет в постояный. Но лучше ничего не придумал.
	// В этот момент объектв в SAP еще не добавлен, поэтому в SetAnimation нельзя еще обновлять
	// его состояние в SAP.
	player->ClearPhysic();
	player->SetAnimation(start_anim ? start_anim : "idle", true);
	player->SetPhysic();

#ifdef COORD_LEFT_UP_CORNER
	player->aabb.p.x += player->sprite->frameWidth * 0.5f;
	player->aabb.p.y += player->sprite->frameHeight * 0.5f;
#endif // COORD_LEFT_UP_CORNER

#ifdef MAP_EDITOR
	player->creation_shift = player->aabb.p - coord;
#endif //MAP_EDITOR

	AddObject(player);
	
	player->gravity = Vector2( proto->gravity_x, proto->gravity_y );

	if ( proto->mpCount )
	{
		player->sprite->mpCount = proto->mpCount;
		player->sprite->mp = new Vector2[ player->sprite->mpCount ];
		int i;
		for ( i = 0; i < player->sprite->mpCount; i++ )
			player->sprite->mp[i] = Vector2(0, 0);
	}

	player->health_max = player->health * 2; //TODO: Заменить на что-то разумное.
	player->InitFaction();

	return player;
}

void ObjPlayer::SetAmmo( int amount )
{
	this->ammo = amount;
}

void ObjPlayer::RecieveAmmo( int amount )
{
	this->ammo += amount;
}

ObjPlayer* CreatePlayer(const char* proto_name, Vector2 coord, const char* start_anim)
{
	if (!proto_name)
		return NULL;

	ObjPlayer* op = CreatePlayer(protoMgr->GetByName( proto_name, "characters/" ), coord, start_anim);
#ifdef MAP_EDITOR
	op->proto_name = new char[ strlen(proto_name) + 1 ];
	strcpy( op->proto_name, proto_name );
#endif //MAP_EDITOR
	return op;
}

void ObjPlayer::ReceiveDamage( UINT damage, UINT damage_type )
{
#ifdef GOD_MODE
	return;
#endif //GOD_MODE
	if ( !playerControl->current->controlEnabled ) return; //Нечего получать урон, когда мы не можем ответить. Для этого есть ReduceHealth.
	((ObjCharacter*)this)->ReceiveDamage( damage, damage_type );
	if ( !this->is_invincible )
	{
		this->is_invincible = true;
		this->recovery_time = 50;
	}
}

//////////////////////////////////////////////////////////////////////////

ObjPlayer::~ObjPlayer()
{
	playerControl->OnDying(this);

	DELETESINGLE(alt_weapon);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

extern config cfg;

//#define KEYDOWN(vk_code)((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)




extern InputMgr inpmgr;

#define KEYHOLDED(key) inpmgr.IsHolded(key)



#define HOLDED_FIRE KEYHOLDED(cakFire)
#define HOLDED_LEFT KEYHOLDED(cakLeft)
#define HOLDED_RIGHT KEYHOLDED(cakRight)
#define HOLDED_JUMP KEYHOLDED(cakJump)
#define HOLDED_SIT KEYHOLDED(cakSit)
#define HOLDED_DOWN KEYHOLDED(cakDown)
#define HOLDED_UP KEYHOLDED(cakUp)



#define RELEASED_CHANGE_WEAPON	inpmgr.IsProlongedReleased(cakChangeWeapon)
#define RELEASED_USE			inpmgr.IsProlongedReleased(cakPlayerUse)


#define PLAYER_STILL_WALKING_LEFT (this->movement == omtWalking && !this->movementDirectionX)
#define PLAYER_STILL_WALKING_RIGHT (this->movement == omtWalking && this->movementDirectionX)
#define PLAYER_STILL_AIMING_UP (this->movement == omtAimingUp || this->movement == omtWalkingAimingUp)

extern UINT current_time;

void ObjPlayer::Process()
{
	if (IsSleep())
		return;

	//Если управления нет - камеру не двигаем и сами движения не начинаем
	//Мигать при неуязвимости тоже не стоит.
	if ( playerControl->current->controlEnabled )
	{
	
		if ( (HOLDED_DOWN && camera_offset < 100)||
		 (!HOLDED_DOWN && !HOLDED_UP && camera_offset < 0) )
			camera_offset+=10;
		if ( (HOLDED_UP && camera_offset > -100) ||
		 (!HOLDED_DOWN && !HOLDED_UP && camera_offset > 0) )
			camera_offset-=10;
#ifndef GOD_MODE
		if (this->is_invincible)
		{
				/*UINT off = abs(((int)current_time % 10)-5);
				s->color = RGBAf(0.8-off*0.05, 0.8-off*0.05, 1.0-off*0.1, 1.0);*/
				if ( current_time % 100 > 50 )
					this->sprite->color = RGBAf(1.0f, 1.0f, 1.0f, 0.0f);
				else
					this->sprite->color = RGBAf(0.9f, 0.9f, 1.0f, 1.0f);
		}
		else
		{
			this->sprite->color = RGBAf(1.0f, 1.0f, 1.0f, 1.0f);
		}
#endif //!GOD_MODE
	}
	else
	{
		if ( this->activity != oatMorphing )
			this->activity = oatIdling;
		this->sprite->color = RGBAf(1.0f, 1.0f, 1.0f, 1.0f);
		if ( !IsNear(vel.x, 0.0f, 0.01f) )
			movementDirectionX = this->vel.x > 0;
		if ( this->IsOnPlane() ) //TODO: нормальная обработка неуправляемого движения.
		{
			if ( abs(vel.x) > 0.01 )
				this->movement = omtWalking;
			else
				this->movement = omtIdling;
		}
		else
		{
			this->movement = omtJumping;
		}
	}

	if ( this->recovery_time > 0 )
	{
		this->recovery_time--;
		if ( this->recovery_time <= 0 ) this->is_invincible = false;
	}

	Sprite* s = this->sprite;
	ASSERT(s);

	if ( activity == oatMorphing )
		return;

	ObjectMovementType last_movement = this->movement;
	bool last_direction = (this->sprite->IsMirrored() == false);
	CharacterGunDirection oldGunDirection = gunDirection;

	gunDirection = cgdNone;

	if (this->health <= 0)
	{
		activity = oatDying;
		//player_change_blocked = true;
	}
	if (activity == oatDying)
	{
		//if (!player_change_blocked) player_change_blocked = true;
		movement = omtIdling;
		this->SetAnimation("die", false);
		if (s->IsAnimDone())
		{
			this->SetDead();
		}
		return;
	}

	// Обработка нажатий на клавиши
	// TODO: Обернуть потом в проверку на включеность клавиатуры/ввода игрока
	float dvx = 0.0f;	// Изменение скорости, применится в конце в зависимости от типа движения

	if (movement == omtJumping && IsOnPlane())
	{
		//	movement = omtLanding;
		movement = omtIdling;
	}

	if (this->controlEnabled)
	{
		if (HOLDED_FIRE)
			this->activity = oatShooting;
		else if (this->activity == oatShooting)
			this->activity = oatIdling;

		if (HOLDED_SIT && movement != omtJumping && movement != omtDropSitting)
			movement = omtSitting;

		if (HOLDED_UP && !HOLDED_DOWN && weapon != NULL)
			gunDirection = cgdUp;

		if (HOLDED_DOWN && !HOLDED_UP && weapon != NULL)
			gunDirection = cgdDown;

		if ( IsOnPlane() && !HOLDED_JUMP )
			jump_complete = true;

		this->ClearDropping();
		if (HOLDED_JUMP && HOLDED_SIT )
		{
			if ( movement != omtDropSitting )
			{
				this->SetDropping();
				//if (!keys_last[cfg.jump] || !keys_last[cfg.sit])
				//this->ClearOnPlane();
				//this->suspected_plane = NULL;
			}
		} else
		if (HOLDED_JUMP && /*!HOLDED_DOWN &&*/ IsOnPlane() && movement != omtLanding && jump_complete )
		{
			movement = omtJumping;
			this->vel.y -= this->jump_vel*this->env->jump_vel_multiplier/*/10.0*/;
			this->ClearOnPlane();
			jump_complete = false;
		}

		//[Резерв прыжка, плохо, надо придумать получше это всё]
		/*if ( !IsOnPlane() )
		{
			if ( HOLDED_JUMP && this->jump_reserve > 0 )
			{
				this->vel.y -= jump_vel/10.0;
				jump_reserve--;
			}
			if ( !HOLDED_JUMP ) jump_reserve = 0;
		}
		else jump_reserve = 10;*/
		//[/Резерв прыжка]
		//Увеличенная гравитация, если летим вверх, но прыжок отпущен. Могут быть проблемы.
		if ( !IsOnPlane() && this->vel.y < 0 && !HOLDED_JUMP ) this->vel.y += this->gravity.y * 0.5f /** time*/;

		if (!HOLDED_SIT && (movement == omtSitting || movement == omtDropSitting))
			movement = omtIdling;


		if (HOLDED_RIGHT && !HOLDED_LEFT)
		{
			dvx += walk_acc * (IsOnPlane() ? 0.5f : 0.1f); //Движение в прыжке должно быть МЕНЕЕ управляемо, не более.
			movementDirectionX = true;
		}
		if (HOLDED_LEFT && !HOLDED_RIGHT)
		{
			dvx -= walk_acc * (IsOnPlane() ? 0.5f : 0.1f);
			movementDirectionX = false;
		}

		if (RELEASED_CHANGE_WEAPON)
		{
			this->ChangeWeapon();
			if (activity == oatShooting)
				activity = oatIdling;
		}

		if (RELEASED_USE)
		{
			if (this->env)
			{
				this->env->OnUse(this);
			}
		}
	}


	// Различные проверки для определения типа передвижения
	if (movement != omtSitting && movement != omtJumping &&
		movement != omtLanding && movement != omtDropSitting &&
		!IsNear(vel.x, 0, 0.1f))
	{
		movement = omtWalking;
	}

	if(this->controlEnabled &&
		((!HOLDED_LEFT && PLAYER_STILL_WALKING_LEFT) ||
		(!HOLDED_RIGHT && PLAYER_STILL_WALKING_RIGHT) ||
		(HOLDED_LEFT && HOLDED_RIGHT)/* ||
		(!HOLDED_UP && PLAYER_STILL_AIMING_UP))*/))
	{
		movement = omtIdling;
	}

	if (!IsOnPlane() && movement != omtJumping)
		movement = omtJumping;

	// Запуск анимаций
	if (this->movement == omtWalking)
	{
		if (gunDirection == cgdUp)
		{
			this->SetAnimation("walkgunaimup", false);
		}
		else if (gunDirection == cgdDown)
		{
			this->SetAnimation("walkgunaimdown", false);
		}
		else
		{
			this->SetAnimation("walk", false);
		}
	}
	else if (this->movement == omtJumping)
	{
		if (gunDirection == cgdDown)
		{
			if (oldGunDirection != gunDirection)
				this->SetAnimation("jumpgunliftaimdown", false);

			if (last_movement != omtJumping)
				this->SetAnimation("jumpgunaimdown", false);
		}
		else if (gunDirection == cgdUp)
		{
			if (oldGunDirection != gunDirection)
				this->SetAnimation("jumpgunliftaimup", false);

			if (last_movement != omtJumping)
				this->SetAnimation("jumpgunaimup", false);
		}
		else
		{
			if (last_movement != omtJumping )
			{
				this->SetAnimation("jump", false);
			}
			else if ( this->sprite->IsAnimDone() )
				this->SetAnimation("fly", false);
			if ( this->sprite->cur_anim == "jump" && IsOnPlane() )
				this->sprite->SetCurrentFrame(0);
		}
	}
	/*else if (this->movement == omtLanding)
	{
		this->SetAnimation("land", false);
		if (s->IsAnimDone())
		{
			// Анимация закончилась - мы приземлились
			movement = omtIdling;
		}

	}*/
	else if ((this->movement == omtSitting || this->movement == omtDropSitting ) && this->activity != oatShooting)
	{
		if (gunDirection == cgdUp)
		{
			this->SetAnimation("sitaimup", false);
		} else
		if (gunDirection == cgdDown)
		{
			this->SetAnimation("sitaimdown", false);
		} else this->SetAnimation("sit", false);
	}

	if (this->movement == omtIdling /*&& !HOLDED_FIRE*//*this->activity != oatShooting*/)
	{
		if (gunDirection == cgdUp)
		{
			if (oldGunDirection != gunDirection)
				this->SetAnimation("gunliftaimup", false);

			if (last_movement != omtIdling)
				this->SetAnimation("gunaimup", false);
		}
		else if (gunDirection == cgdDown)
		{
			if (oldGunDirection != gunDirection)
				this->SetAnimation("gunliftaimdown", false);

			if (last_movement != omtIdling)
				this->SetAnimation("gunaimdown", false);
		}
		else if (this->activity == oatShooting)
		{
			//if (!(this->sprite->cur_anim == "shoot" && !this->sprite->IsAnimDone()))
				//this->SetAnimation("aim", false);
		}
		else if (this->sprite->cur_anim != "land" && this->sprite->cur_anim != "stop" && this->sprite->cur_anim != "pain")
		{
			this->SetAnimation("idle", false);
		}
	}

	//Переходные анимации
	if ( last_movement == omtJumping && this->movement != omtJumping && this->IsOnPlane() )
	{
		this->SetAnimation("land", false);
	}
	if ( last_movement == omtWalking && (this->movement == omtIdling || (movementDirectionX ^ last_direction)) )
		this->SetAnimation("stop", false);


	if (cur_weapon && this->activity == oatShooting && cur_weapon->ClipReloaded(HOLDED_FIRE) ) cur_weapon->clip = cur_weapon->shots_per_clip;
	if (cur_weapon && this->activity == oatShooting && this->cur_weapon->IsReady())
	{
		if (/*&& cur_weapon->IsReady() &&*/
			!((this->sprite->cur_anim == "gunliftaimup" || this->sprite->cur_anim == "gunliftaimdown")/* && !this->sprite->IsAnimDone()*/))
		{
			if (movement == omtSitting || movement == omtDropSitting)
			{
				if (this->gunDirection == cgdDown)
				{
					this->SetAnimation("sitdownshoot", true);
				}
				else if (this->gunDirection == cgdUp)
				{
					this->SetAnimation("situpshoot", true);
				}
				else
				{
					this->SetAnimation("sitshoot", true);
				}
			}
			//else if (movement == omtWalking)
			//{

			//}
			else if (movement == omtJumping)
			{
				if (this->gunDirection == cgdDown)
				{
					this->SetAnimation("jumpgunaimdownshoot", true);
				}
				else if (this->gunDirection == cgdUp)
				{
					this->SetAnimation("jumpgunaimupshoot", true);
				}
				else
				{
					this->SetAnimation("jumpshoot", true);
				}
			}
			else if (movement == omtIdling)
			{
				if (this->gunDirection == cgdUp)
				{
					this->SetAnimation("gunaimupshoot", true);
				}
				else if (this->gunDirection == cgdDown)
				{
					this->SetAnimation("gunaimdownshoot", true);
				}
				else
				{
					this->SetAnimation("shoot", true);
				}
			}
		}
	}


	if (movement != omtSitting && movement != omtLanding && movement != omtDropSitting)
	{
		vel.x += dvx*this->env->walk_vel_multiplier;
	}


	// Разворачиваем спрайт по направлению
	if ( (abs(vel.x) > 0.01f) || this->movement == omtSitting ) //Плохо, но жизнь такая
	{
		if (movementDirectionX )
			SetFacing( false );
		else
			SetFacing( true );
	}
}

//Проверяем, можем ли выстрелить
bool ObjPlayer::HasAmmo()
{
	//if ( this != _PLAYER ) return false;
	return (cur_weapon && ( this->cur_weapon->is_infinite || (this->ammo >= this->cur_weapon->bullets_count) ) );
}

//Тратим патроны на выстрел.
void ObjPlayer::SpendAmmo()
{
	if ( /*this != _PLAYER ||*/ !this->cur_weapon->IsReady() ) return;
	//this->ammo -= this->cur_weapon->bullets_count;
	//if ( this->ammo < 0 ) this->ammo = 0;
	if (this->ammo > this->cur_weapon->bullets_count)
		this->ammo -= this->cur_weapon->bullets_count;
	else
		this->ammo = 0;
}

// Переключает основное и альтернативное оружие
void ObjPlayer::ChangeWeapon()
{
	if (alt_weapon && cur_weapon == this->weapon)
		cur_weapon = alt_weapon;
	else
		cur_weapon = weapon;
}


void ObjPlayer::GiveWeapon(const char* weapon_name, bool primary)
{
	if (!weapon_name)
		return;

	if ( !primary )
	{
		if (this->alt_weapon) DELETESINGLE(this->alt_weapon);
		this->alt_weapon = CreateWeapon(weapon_name);
		if (this->cur_weapon != this->weapon) this->cur_weapon = this->alt_weapon;
	}
	else
	{
		if (this->weapon) DELETESINGLE(this->weapon);
		this->weapon = CreateWeapon(weapon_name);
		if (this->cur_weapon != this->alt_weapon) this->cur_weapon = this->weapon;
	}
}

#include "StdAfx.h"

#include "player.h"
#include "camera.h"
#include "../config.h"
#include "../script/api.h"

#include "../input_mgr.h"

#include "../resource_mgr.h"

//////////////////////////////////////////////////////////////////////////


extern ResourceMgr<Proto> * protoMgr;

extern InputMgr inpmgr;

extern config cfg;

extern lua_State* lua;

extern game::GameStates game_state;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Вызывается для создания игрока. Сохраняет ссылки на объект игрока.
// Сам объект создается функцией CreatePlayer из object_player.cpp
ObjPlayer* Player::Create(const char* proto_name, Vector2 coord, const char* start_anim)
{
	if (this->first && this->second)		// Если оба игрока созданы
		return NULL;

	ObjPlayer* plr = CreatePlayer(proto_name, coord, start_anim);
	if (plr)
	{
		if (!this->first)
		{
			this->first = plr;
			this->current = plr;

			first_proto = new char[strlen(proto_name) + 1];
			memset(first_proto, 0, strlen(proto_name) + 1);
			strcpy(first_proto, proto_name);

			revivePoint = coord;		// По умолчанию, буем оживлять в месте создания первого игрока
		}
		else
		{
			this->second = plr;
			second_proto = new char[strlen(proto_name) + 1];
			memset(second_proto, 0, strlen(proto_name) + 1);
			strcpy(second_proto, proto_name);

			// Второй игрок пока что не доступен для игрового мира и ничего не делает
			plr->sprite->ClearVisible();
			plr->SetSleep();
			plr->ClearBulletCollidable();
		}
	}
	
	return plr;
}

// Оживляет игрока номер num, задавая тартовую анимацию start_anim
ObjPlayer* Player::Revive(int num, const char* start_anim)
{
	char* pname = NULL;

	if (num == 1) 
	{
		pname = this->first_proto;
		if (this->first)
			return NULL;	// Сложно оживлять живого
	}
	else if (num == 2) 
	{
		pname = this->second_proto;
		if (this->second)
			return NULL;	// Сложно оживлять живого
	}
	else
		ASSERT(false);		// Сюда попадать нельзя

	if (!pname)
		return NULL;		// Неизвестно, кого оживлять

	ObjPlayer* plr = CreatePlayer(pname, revivePoint, start_anim);

	ObjPlayer* another_plr = NULL;
	const Proto* aproto = NULL;

	if (num == 1) 
	{
		this->first = plr;
		another_plr = second;
		aproto = protoMgr->GetByName(second_proto);
	}
	else if (num == 2) 
	{
		this->second = plr;
		another_plr = first;
		aproto = protoMgr->GetByName(first_proto);
	}

	// Восстанавливаем второму игроку хейлы тоже
	if (another_plr && aproto)
	{
		if (another_plr->health < (short)aproto->health)
			another_plr->health = aproto->health;
	}

	if (!this->current) this->current = plr;
	this->current->recovery_time = 100;
	this->current->is_invincible = true;

	return plr;
}

ObjPlayer* Player::GetByNum(int num)
{
	ObjPlayer* plr = NULL;
	if (num == 1)
		plr = this->first;
	else if (num == 2)
		plr = this->second;

	return plr;
}

// Вызывается на каждом ходу. Обработка смены игроков вызывается здесь
void Player::Update()
{
	if (!this->first && !this->second)
		return;

	//if (!this->current)
	//	this->current = this->first;


	Changer( false, false );

	// Вот тут можно сделать прием команд управления и передача их объекту
}


// Обработка смены игроков
void Player::Changer( bool forced, bool immediate )
{
	if (!(this->current && this->first && this->second))
		return;		// Некого менять

	if ( !forced )
	{
		if (this->change_blocked )
			return;		// Нельзя менять

		if ( this->current->activity == oatDying || !this->current->IsOnPlane() )
			return;		// Игрок умирает или не стоит на твёрдой поверхности, нельзя менять

	}
	if ( !immediate && (forced || inpmgr.IsProlongedReleased(cakChangePlayer)) )
	{
		this->current->vel = Vector2(0,0);
		this->current->activity = oatMorphing;
		this->current->SetAnimation("morph_out", false);
	}
	if ( (this->current->activity == oatMorphing && this->current->sprite->IsAnimDone()) || immediate )
	{
		ObjPlayer* old = this->current;
		if (this->current == this->first)
			this->current = this->second;
		else
			this->current = this->first;

		old->SetAnimation("idle", true);

		this->current->aabb.p = old->aabb.p;
		this->current->old_aabb = old->old_aabb;
		this->current->vel = old->vel;
		this->current->acc = old->acc;

		this->current->controlEnabled = old->controlEnabled;
		this->current->gravity = old->gravity;
		this->current->movement = old->movement;
		if ( old->sprite->IsMirrored() ) this->current->sprite->SetMirrored();
		else this->current->sprite->ClearMirrored();
		if (!immediate) 
		{
			this->current->SetAnimation("morph_in", false);
			this->current->activity = oatMorphing;
		}
		if (this->current->aabb.H != old->aabb.H)
			this->current->aabb.p.y = old->aabb.Bottom() - this->current->aabb.H;

		old->IsOnPlane() ? this->current->SetOnPlane() : this->current->ClearOnPlane();
		this->current->suspected_plane = old->suspected_plane;
		this->current->drop_from = old->drop_from;

		old->IsBulletCollidable() ?
			this->current->SetBulletCollidable() :
			this->current->ClearBulletCollidable();

		old->ClearBulletCollidable();

		this->current->movementDirectionX = old->movementDirectionX;
		/*old->sprite->IsMirrored() ?
			_PLAYER->sprite->SetMirrored() :
			_PLAYER->sprite->ClearMirrored();*/

		this->current->sprite->SetVisible();
		old->sprite->ClearVisible();

		this->current->ClearSleep();
		old->SetSleep();

		if (GetCameraAttachedObject() == old)
			CameraAttachToObject(this->current);

		if (onChangePlayerProcessor >= 0)
		{
			lua_pushinteger(lua, old->id);
			lua_pushinteger(lua, current->id);
			if (SCRIPT::ExecChunkFromReg(onChangePlayerProcessor, 2))
			{
				// В скрипте произошла какая-то ошибка. Будем испльзовать стандартный в следующий раз.
				SCRIPT::RemoveFromRegistry(onChangePlayerProcessor);
			}
		}
	}
}

// Блокировать смену игроков
void Player::BlockChange()
{
	this->change_blocked = true;
}

// Разблокировать смену игроков
void Player::UnblockChange()
{
	this->change_blocked = false;
}

// Вызывается из деструктора объекта игрока
void Player::OnDying(ObjPlayer* plr)
{
	if (game_state == game::GAME_DESTROYING)
		return;			// При уничтожении игры уже не надо реагировать на уничтожение объекта

	if ( plr->IsSleep() )
	{
		if ( plr == this->first )
		{
			if ( this->current == this->first )
			{ 
				this->first = this->second;
				this->second = NULL;
			}
			else this->first = NULL;
		}
		if ( plr == this->second )
			this->second = NULL;
		return;			// Если удаляется неактивный персонаж - то туда ему и дорога. Это не смерть.
	}

	if (plr == this->current)
		current = NULL;

	int num = 0;

	if (plr == this->second)
	{
		num = 2;
		this->second = NULL;
		//DELETEARRAY(this->second_proto);
	}
	else if (plr == this->first)
	{
		num = 1;
		this->first = NULL;
		//DELETEARRAY(this->first_proto);

		//if (this->second)
		//{
		//	this->first = this->second;
		//	this->first_proto = this->second_proto;
		//	this->second = NULL;
		//	this->second_proto = NULL;
		//}
	}
	else
		ASSERT(false);		// Не должно сюда попадать

	if (onPlayerDeathProcessor >= 0)
	{
		PushObject(lua, plr);			// Передаем в функцию таблицу с информацией об объекте уничтожаемого игрока
		lua_pushinteger(lua, num);		// Номер уничтожаемого игрока

		if (SCRIPT::ExecChunkFromReg(onPlayerDeathProcessor, 2))
		{
			// В скрипте произошла какая-то ошибка. Будем испльзовать стандартный в следующий раз.
			SCRIPT::RemoveFromRegistry(onPlayerDeathProcessor);
		}
	}
}

#ifndef __OBJECT_CHARACTER_H_
#define __OBJECT_CHARACTER_H_

#include "object_dynamic.h"
#include "weapon.h"
#include <vector>

#include "../../render/font.h"

#include "../../resource_mgr.h"

enum CharacterGunDirection { cgdNone, cgdUp, cgdDown };
enum CharacterShootingBeh { csbNoShooting, csbFreeShooting, csbOnAnimCommand };
enum CharacterOffscreenBeh { cobNone, cobDie, cobAnim, cobSleep };
extern UINT internal_time;
extern UINT current_time;


// Если наследуетесь от этого класса, не забудеьте добавить соответсвующую проверку в
// GameObject::ProcessSprite(), там где определяется ObjCharacter* ch.
class ObjCharacter : public ObjDynamic
{
public:
	bool is_invincible;
	short health;
	short health_max;
	char* message;
	bool message_new;
	UINT message_time;

	CharacterOffscreenBeh offscreen_behavior;
	float offscreen_distance;

	//Был добавлен для босса новогоднего билда, но вообще не такая и плохая идея.
	float damage_mult;

	ObjectConnection* faction;
	ObjectConnection* targeted_by;
	int faction_id;
	vector<int> faction_hates;
	vector<int> faction_follows;

	CharacterGunDirection gunDirection;

	UINT gunReloadTick;
	UINT gunReloadTime;

	Weapon* weapon;
	Weapon* cur_weapon;

	CharacterShootingBeh shootingBeh;

	ObjCharacter()
	{
		is_invincible = false;
		message = NULL;
		message_new = false;
		health = 0;
		gunDirection = cgdNone;
		mem_anim = NULL;
		mem_frame = mem_tick = 0;
		offscreen_behavior = cobNone;
		offscreen_distance = 0.0f;

		weapon = NULL;
		cur_weapon = NULL;

		shootingBeh = csbNoShooting;

		gunReloadTick = 0;
		gunReloadTime = 0;

		damage_mult = 1.0;

		faction_hates = faction_follows = vector<int>();
		targeted_by = new ObjectConnection( this );
	}

	virtual ~ObjCharacter()
	{
		DELETESINGLE(weapon);
		DELETEARRAY(mem_anim);
		if (faction)
			faction->childDead(this->id);
		//if ( activity != oatDying )  //Если есть активность, то нас не удалили и мы уже отметились в начале смерти.
			targeted_by->parentDead();
		if ( message ) DELETEARRAY(message);
	}
	
	void ProcessShooting();

	void LoadFactionInfo( const Proto* proto )
	{
		faction_id = proto->faction_id;
		vector<int>::const_iterator it;
		for (it = proto->faction_hates.begin(); it != proto->faction_hates.end(); it++)
			faction_hates.push_back( *it );
		for (it = proto->faction_follows.begin(); it != proto->faction_follows.end(); it++)
			faction_follows.push_back( *it );
	}

	void ReceiveDamage(UINT damage, UINT damage_type)
	{
		if ( is_invincible || activity == oatDying ) return;

		//Если анимацию получения урона не предусмотрели, то обходимся без неё. Также не прерываем важные анимации.
		//TODO: добавить флаг важной анимации
		//Поскольку нанесения реального урона переложено на саму анимацию без неё персонаж был бы бессмертен.
		if ( sprite->GetAnimation("pain") == NULL || this->activity == oatMorphing )
		{
			ReduceHealth((UINT)(damage*damage_mult));
			return;
		}

		if ( sprite->cur_anim != "pain" &&  sprite->cur_anim != "die" && damage > 0 )
		{
			DELETEARRAY(mem_anim);
			mem_anim = StrDupl(this->sprite->cur_anim.c_str());
			mem_frame = sprite->GetAnimation(this->sprite->cur_anim_num)->current;
			if ( mem_tick == 0 ) mem_tick = internal_time;
			sprite->stack->Push( (int)(damage*damage_mult) );
			sprite->stack->Push( damage_type );
			SetAnimation("pain", true );
		}

		if (health <= 0 && activity != oatDying)
		{
			SetAnimation("die", false);
			activity = oatDying;
		}
	}

	virtual bool ReduceHealth(UINT damage)
	{
		health -= (short)damage;
		if (health <= 0 && activity != oatDying)
		{
			SetAnimation("die", false);
			//targeted_by->parentDead();
			activity = oatDying;
			return true;
		}
		return false;
	}

	void ReceiveHealing(UINT amount)
	{
		health += (short)amount;
		if ( health > health_max ) health = health_max;
	}

	void Draw()
	{
		if (IsSleep())
			return;

		if ( message )
		{
			Font* fnt = FontByName( "dialogue" );
			if (fnt)
			{
				float w = (float)fnt->GetStringWidth( message );
				fnt->p = Vector2( aabb.p.x-(w/2), aabb.Top()-15 );
				fnt->z = 1.0f;
				fnt->tClr = RGBAf( 1.0f, 1.0f, 1.0f, 1.0f*(1.0f-(current_time - message_time)/5000.0f) );
				fnt->Print( message );
			}
		}

		GameObject::Draw();
	}

	void InitFaction();
	GameObject* GetNearestCharacter( vector<int> factions_list );
	bool IsEnemy( GameObject* obj );
};

void ClearFactions();

#endif // __OBJECT_CHARACTER_H_

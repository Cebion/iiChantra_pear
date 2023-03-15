#include "StdAfx.h"
#include "object_character.h"
#include "object_player.h"
#include "../../misc.h"
#include "object_dynamic.h"
#include "../phys/phys_collisionsolver.h"
#include "../phys/sap/OPC_ArraySAP.h"

extern Opcode::ArraySAP *asap;
map<int, GameObject*> factions;

void ObjCharacter::ProcessShooting()
{
	if (this->IsSleep())
		return;

	if (!this->cur_weapon)
		return;

	if ( this->sprite->mpCount < 1 && this->activity == oatShooting )
	{
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Попытка выстрела объекта без точек крепления");
		return;
	}

	switch (this->shootingBeh)
	{
	case csbFreeShooting:
		if (this->activity == oatShooting && (this->type != objPlayer || ((ObjPlayer*)this)->HasAmmo() ))
		{
			if (this->type == objPlayer) ((ObjPlayer*)this)->SpendAmmo();
			if (this->sprite->IsMirrored()) this->cur_weapon->Fire(this, this->aabb.p+Vector2( -(this->sprite->mp[0].x), this->sprite->mp[0].y ));
			else this->cur_weapon->Fire(this, this->aabb.p+this->sprite->mp[0]);
		}
		break;
	case csbOnAnimCommand:
		if (this->type != objPlayer || ((ObjPlayer*)this)->HasAmmo() )
		{
			((ObjPlayer*)this)->SpendAmmo();
			if (this->sprite->IsMirrored()) this->cur_weapon->Fire(this, this->aabb.p+Vector2( -(this->sprite->mp[0].x), this->sprite->mp[0].y ));
			else this->cur_weapon->Fire(this, this->aabb.p+this->sprite->mp[0]);
		}
		this->shootingBeh = csbNoShooting;
		break;
	default: break;
	}
}

void ObjCharacter::InitFaction()
{
	map<int, GameObject*>::iterator it = factions.find( this->faction_id );
	if ( it != factions.end() )
	{
		it->second->childrenConnection->children.push_back(this);
		this->faction = it->second->childrenConnection;
	}
	else
	{
		GameObject* fact = new GameObject();
		//AddObject(fact);
		factions[this->faction_id] = fact;
		fact->childrenConnection = new ObjectConnection( fact );
		fact->childrenConnection->children.push_back( this );
		this->faction = fact->childrenConnection;
	}
}

GameObject* ObjCharacter::GetNearestCharacter( vector<int> factions_list )
{
	bool first = true;
	Vector2 dist;
	float mn = 0;
	GameObject* mobj = NULL;
	GameObject* obj;
	bool dying;
	map<int, GameObject*>::iterator foit;
	for (vector<int>::iterator fit = factions_list.begin(); fit != factions_list.end(); fit++)
	{
		foit = factions.find( (*fit) );
		if ( foit != factions.end() )
		{
			for (vector<GameObject*>::iterator oit = foit->second->childrenConnection->children.begin(); oit != foit->second->childrenConnection->children.end(); oit++)
			{
				obj = (*oit);
				dist = obj->aabb.p - this->aabb.p;
				dying = false;
				if ( obj->type == objPlayer || obj->type == objEnemy )
					dying = ((ObjCharacter*)obj)->activity == oatDying;
				if ( !dying && obj->sprite->IsVisible() && (first || dist.Length() < mn) )
				{
					first = false;
					mn = dist.Length();
					mobj = obj;
				}
			}
		}
	}
	return mobj;
}

bool ObjCharacter::IsEnemy( GameObject* obj )
{
	if ( obj->type != objEnemy && obj->type != objPlayer )
		return true; //Окружение есть враг всему живому.
	if ( this->type == objPlayer ) //Игроку враги все, с номером стороны больше 0 и не той же стороны.
	{
		ObjCharacter* oen = (ObjCharacter*)obj;
		return ( oen->faction_id != faction_id && oen->faction_id > 0 );
	}
	vector<int>::iterator fit = find(  faction_hates.begin(), faction_hates.end(), ((ObjCharacter*)obj)->faction_id );
	if ( fit == faction_hates.end() )
		return false;
	return true;
}

void ClearFactions()
{
	for (map<int, GameObject*>::iterator it = factions.begin(); it != factions.end(); it++)
	{
		it->second->childrenConnection->parentDead();
		DELETESINGLE( it->second );
	}
	factions.clear();
}

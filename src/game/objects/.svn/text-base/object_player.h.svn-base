#ifndef __OBJECT_PLAYER_H_
#define __OBJECT_PLAYER_H_

#include "object_character.h"

//#define GOD_MODE

class ObjPlayer : public ObjCharacter
{
public:
	//USHORT health;
	bool controlEnabled;			// Может ли игрок управлять	
	Weapon* alt_weapon;				// Альтернативное оружие
	UINT ammo;
	int camera_offset;			//Позволяет смотреть вверх-вниз
	//USHORT jump_reserve;
	bool jump_complete;				//Самый тупой способ
	float recovery_time;
	
	ObjPlayer()
	{
		camera_offset = 0;
		this->controlEnabled = true;
		alt_weapon = NULL;

		this->env = default_environment;

#ifdef GOD_MODE
		is_invincible = true;
#endif // GOD_MODE

		this->type = objPlayer;
		this->gravity = Vector2(0.0f, 0.8f);
		this->health = 50;
		this->ammo = 100;
		jump_complete = true;
		//jump_reserve = 10; //Хранить не "куски", а сразу значения ускорения?
	}

	~ObjPlayer();

	virtual void Process();
	void ChangeWeapon();
	void SetAmmo( int amount );
	void RecieveAmmo( int amount );
	bool HasAmmo();
	void SpendAmmo();
	void GiveWeapon(const char* weapon_name, bool primary);
	void ReceiveDamage( UINT damage, UINT damage_type );
};

ObjPlayer* CreatePlayer(const char* proto_name, Vector2 coord, const char* start_anim);


#endif // __OBJECT_PLAYER_H_
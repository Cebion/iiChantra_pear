#ifndef __OBJECT_ENVIRONMENT_H_
#define __OBJECT_ENVIRONMENT_H_

#include "object_physic.h"

class ObjEnvironment : public ObjPhysic
{
	public:
		char* script_on_enter;
		char* script_on_leave;
		char* script_on_stay;
		char* script_on_use;
		float walk_vel_multiplier;
		float jump_vel_multiplier;
		float bounce_bonus;
		char** sounds;
		char** sprites;
		int material;
		Vector2 gravity_bonus;

		ObjEnvironment()
		{
			walk_vel_multiplier = jump_vel_multiplier = 1;
			bounce_bonus = 0;
			gravity_bonus = Vector2(0, 0);
			material = 0;
			sounds = new char*[32];
			sprites = new char*[32];
			script_on_enter = script_on_leave = script_on_stay = script_on_use = NULL;
			for ( int i = 0; i < 32; i++ )
			{
				sounds[i] = NULL;
				sprites[i] = NULL;
			}
		}

		~ObjEnvironment()
		{
			DELETEARRAY( script_on_enter );
			DELETEARRAY( script_on_leave );
			DELETEARRAY( script_on_stay );
			DELETEARRAY(script_on_use);
			for ( int i = 0; i < 32; i++ )
			{
				DELETEARRAY(sounds[i]);
				DELETEARRAY(sprites[i]);
			}
			DELETEARRAY(sounds);
			DELETEARRAY(sprites);
		}

		char* GetSound( int index );
		char* GetSprite( int index );
		void OnEnter( GameObject* obj );
		void OnLeave( GameObject* obj );
		void OnStay( GameObject* obj );
		void OnUse( GameObject* obj );
};

ObjEnvironment* CreateEnvironment(const char* proto_name, Vector2 coord);
void DefaultEnvDelete();
void DefaultEnvSet( ObjEnvironment* env );

#endif
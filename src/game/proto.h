#ifndef __PROTO_H_
#define __PROTO_H_


#include "../render/types.h"
#include "phys/phys_misc.h"

#include "../resource.h"

enum PROTO_BEH { protoNullBeh, protoPlayer, protoEnemy, protoPowerup, protoSprite };

#include "animation_frame.h"
#include "animation.h"

class Proto : public Resource
{
public:
	PROTO_BEH behaviour;
	char* texture;
	UINT overlayCount;
	UINT* overlayUsage;
	RGBAf* ocolor;
	float z;
	UINT drops_shadow;
	float shadow_width;
	UINT frame_widht;
	UINT frame_height;
	int next_shift_y;
	UINT hurts_same_type;
	float push_force;
	float mass;
	RGBAf color;
#ifdef MAP_EDITOR
	RGBAf editor_color;
#endif //MAP_EDITOR
	float walk_vel_multiplier;
	float jump_vel_multiplier;
	float bounce_bonus;
	vector<char*> sounds;
	vector<char*> sprites;

	int faction_id;
	vector<int> faction_hates;
	vector<int> faction_follows;

	Vector2 gravity_bonus;
	UINT ghost_to;

	//Поведение за границами экрана для ObjCharacter
	float offscreen_distance;
	int offscreen_behavior;

	//Обработка отражения спрайта противников и персонажей
	int facing;

	//Количество "точек крепления" спрайта
	USHORT mpCount;
	
	//vector<AnimProto*> animations;
	Animation* animations;
	size_t animationsCount;
	map<string, size_t> animNames;

	float phys_walk_acc;
	float phys_jump_vel;
	float phys_max_x_vel;
	float phys_max_y_vel;
	float bounce;
	UINT phys_solid;
	UINT phys_one_sided;
	UINT phys_ghostlike;
	UINT phys_bullet_collidable;
	UINT physic;
	UINT slopeType;
	UINT touch_detection;
	

	UINT health;
	char* main_weapon;
	char* alt_weapon;

	float bullet_vel;
	UINT bullet_damage;

	UINT reload_time;
	int bullets_count;
	UINT multiple_targets;
	UINT shots_per_clip;
	UINT clip_reload_time;
	UINT is_ray_weapon;
	UINT time_to_live; 
	char* end_effect;

	UINT effect;

	//////////////////////////////////////////////////////////////////////////
	//PSystem flds

	RGBAf start_color;
	RGBAf end_color;
	RGBAf var_color;
	
	UINT damage_type;

	int max_particles;
	float particle_life_min;
	float particle_life_max;

	float start_size;
	float size_variability;
	float end_size;

	float particle_life;
	float particle_life_var;

	float system_life;
	int emission;

	float min_speed;
	float max_speed;
	float min_angle;
	float max_angle;
	float min_param;
	float max_param;
	float min_trace;
	float max_trace;
	int trajectory;
	float trajectory_param1;
	float trajectory_param2;
	int affected_by_wind;
	float gravity_x;	//Пока только для частиц.
	float gravity_y;

	UINT env_material;
	char* env_onenter;
	char* env_onleave;
	char* env_onstay;
	char* env_onuse;

	//////////////////////////////////////////////////////////////////////////

	Proto(string file_name, string name) : Resource(file_name, name)
	{
		env_onenter = env_onleave = env_onstay = env_onuse = NULL;
		env_material = 0;
		behaviour = protoNullBeh;
		texture = NULL;

		frame_widht = frame_height = next_shift_y =
			phys_solid = phys_ghostlike = phys_bullet_collidable = physic = 
			effect = phys_one_sided = hurts_same_type = slopeType = 
			touch_detection = ghost_to = drops_shadow = facing = 0;

		z = push_force = 0.0f;

		overlayCount = 0;
		overlayUsage = NULL;
		ocolor = NULL;

		color = RGBAf(1.0f, 1.0f, 1.0f, 1.0f);

		phys_walk_acc = phys_jump_vel = phys_max_x_vel = phys_max_y_vel = 0.0f;
		shadow_width = 0.25f;

		health = 0;
		main_weapon = NULL;
		alt_weapon = NULL;

		offscreen_distance = 0.0f;
		offscreen_behavior = 0;


		bullet_damage = 0;
		bullet_vel = 0;
		reload_time = 0;
		bullets_count = 0;
		shots_per_clip = 0;
		clip_reload_time = 0;
		multiple_targets = 0;
		is_ray_weapon = 0;
		time_to_live = 0;
		bounce = 0;
		mpCount = 0;
		gravity_bonus = Vector2(0,0);

		end_effect = NULL;

		animations = NULL;
		animationsCount = 0;

		mass = 1.0f;

		min_speed = max_speed = min_angle = max_angle = min_param = max_param = gravity_x = gravity_y = 
			min_trace = max_trace = trajectory_param1 = trajectory_param2 = 0.0f;
		trajectory = affected_by_wind = 0;

		faction_id = 0;
		faction_hates = faction_follows = vector<int>();

#ifdef MAP_EDITOR
		editor_color = RGBAf( 0.0f, 0.5f, 0.5f, 0.5f );
#endif //MAP_EDITOR
	}

	~Proto()
	{
		if (env_onenter) DELETEARRAY(env_onenter);
		//for(vector<AnimProto*>::iterator ait = this->animations.begin(); 
		//	ait != this->animations.end();
		//	ait++)
		//{
		//	AnimProto* ap = *ait;
		//	DELETESINGLE(ap);
		//}
		//this->animations.clear();
		DELETEARRAY(this->animations);
		this->animNames.clear();

		DELETEARRAY(this->texture);
		DELETEARRAY(this->main_weapon);
		DELETEARRAY(this->alt_weapon);
		for(vector<char*>::iterator ait = this->sounds.begin(); 
			ait != this->sounds.end();
			ait++)
		{
			DELETEARRAY(*ait);
		}
		this->sounds.clear();
		for(vector<char*>::iterator ait = this->sprites.begin(); 
			ait != this->sprites.end();
			ait++)
		{
			DELETEARRAY(*ait);
		}
		this->sprites.clear();
		DELETEARRAY(env_onenter);
		DELETEARRAY(env_onleave);
		DELETEARRAY(env_onstay);
		DELETEARRAY(env_onuse);

		DELETEARRAY(this->end_effect);

		DELETEARRAY(overlayUsage);
		DELETEARRAY(ocolor);
		faction_hates.clear();
		faction_follows.clear();
	}

	virtual bool Load();

	//virtual bool Recover();

	private:
		void LoadAnimProtos();
		void Copy(const Proto& src);

};


#endif // __PROTO_H_


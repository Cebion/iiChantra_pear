#include "StdAfx.h"
#include "proto.h"
#include "../misc.h"

#include "../script/script.h"

#include "../resource_mgr.h"

//////////////////////////////////////////////////////////////////////////

extern lua_State* lua;
extern ResourceMgr<Proto> * protoMgr;

//////////////////////////////////////////////////////////////////////////





bool Proto::Load()
{
	STACK_CHECK_INIT(lua);

	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Загружаем прототип: %s", this->name.c_str());

	// Файл протоипа будет выполнен в защищенном окружении, чтобы не запороть что-нить глобальное.
	// Окружение создается как здесь: http://community.livejournal.com/ru_lua/402.html
	lua_newtable(lua);				// Стек: env
	lua_newtable(lua);				// Стек: env meta
	lua_getglobal(lua, "_G");			// Стек: env meta _G
	lua_setfield(lua, -2, "__index");	// Стек: env meta
	lua_setmetatable(lua, -2);		// Стек: env

	if( luaL_loadfile(lua, this->file_name.c_str()) )
	{
		// Стек: env err
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "%s", lua_tostring(lua, -1));
		lua_pop(lua, 2);	// Стек: 
		STACK_CHECK(lua);
		return false;
	}
	// Стек: env loadfile
	lua_pushvalue(lua, -2);			// Стек: env loadfile env
	lua_setfenv(lua, -2);				// Стек: env loadfile

	if(lua_pcall(lua, 0, 0, 0))
	{
		// Какая-то ошибка выполнения файла
		const char* err = lua_tostring(lua, -1);	// Стек: env err
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_ERROR_EV, err );
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Не удалось загрузить прототип %s", file_name.c_str() );
		lua_pop(lua, 2);	// Стек: 

		STACK_CHECK(lua);

		return false;
	}
	else
	{
		//char* proto_name = NULL;
		//SCRIPT::GetStringFieldByName(lua, "name", &proto_name);
		//sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "В прото %s записано имя %s", this->name.c_str(), proto_name);
		//if (this->name != string(proto_name))
		//{
		//	sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "!!!!!!!!!!!!!!!!!! имена не совпадают");
		//}
		//DELETE_ARRAY(proto_name);
		
		char* temp_str = NULL;
		SCRIPT::GetStringFieldByName(lua, "parent", &temp_str);
		if (temp_str)
		{
			sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Пытаемся получить прототип-родитель %s", temp_str);
			const Proto* parent = protoMgr->GetByName(temp_str);
			if (parent)
			{
				this->Copy(*parent);
			}
			else
			{
				sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Невозможно получить прототип-родитель %s", temp_str);
			}
		}
		DELETEARRAY(temp_str);

#define GET_STR(a, b) SCRIPT::GetStringFieldByName(lua, a, &temp_str); if (temp_str) { DELETEARRAY(b); b = temp_str; temp_str = NULL; }

		// Стек: env
		SCRIPT::GetUIntFieldByName(lua, "damage_type", &this->damage_type);
		SCRIPT::GetUIntFieldByName(lua, "behaviour", (UINT*)&this->behaviour);
		
		//SCRIPT::GetStringFieldByName(lua, "texture", temp_str); if (temp_srt) { DELETEARRAY(texture); texture = temp_str; }
		GET_STR("texture", texture)
		SCRIPT::GetFloatFieldByName(lua, "z", &this->z);
		SCRIPT::GetUIntFieldByName(lua, "ghost_to", &this->ghost_to);
		SCRIPT::GetUIntFieldByName(lua, "frame_width", &this->frame_widht);
		SCRIPT::GetUIntFieldByName(lua, "frame_height", &this->frame_height);
		SCRIPT::GetUIntFieldByName(lua, "drops_shadow", &this->drops_shadow);
		SCRIPT::GetFloatFieldByName(lua, "shadow_width", &this->shadow_width);
		SCRIPT::GetIntFieldByName(lua, "next_shift_y", &this->next_shift_y);
		SCRIPT::GetColorFieldByName(lua, "color", this->color);
		SCRIPT::GetFloatFieldByName(lua, "phys_max_x_vel", &this->phys_max_x_vel);
		SCRIPT::GetUIntFieldByName(lua, "material", &this->env_material);
		//SCRIPT::GetStringFieldByName(lua, "script_on_enter", &this->env_onenter);
		GET_STR("script_on_enter", env_onenter)
		//SCRIPT::GetStringFieldByName(lua, "script_on_leave", &this->env_onleave);
		GET_STR("script_on_leave", env_onleave)
		//SCRIPT::GetStringFieldByName(lua, "script_on_stay", &this->env_onstay);
		GET_STR("script_on_stay", env_onstay)
		GET_STR("script_on_use", env_onuse)
		SCRIPT::GetFloatFieldByName(lua, "gravity_bonus_x", &this->gravity_bonus.x);
		SCRIPT::GetFloatFieldByName(lua, "gravity_bonus_y", &this->gravity_bonus.y);
		SCRIPT::GetFloatFieldByName(lua, "phys_max_y_vel", &this->phys_max_y_vel);
		SCRIPT::GetFloatFieldByName(lua, "phys_jump_vel", &this->phys_jump_vel);
		SCRIPT::GetFloatFieldByName(lua, "phys_walk_acc", &this->phys_walk_acc);
		SCRIPT::GetFloatFieldByName(lua, "walk_vel_multiplier", &this->walk_vel_multiplier);
		SCRIPT::GetFloatFieldByName(lua, "jump_vel_multiplier", &this->jump_vel_multiplier);
		SCRIPT::GetFloatFieldByName(lua, "bounce_bonus", &this->bounce_bonus);
		SCRIPT::GetFloatFieldByName(lua, "bounce", &this->bounce);
		SCRIPT::GetFloatFieldByName(lua, "push_force", &this->push_force);
		SCRIPT::GetFloatFieldByName(lua, "mass", &this->mass);
		SCRIPT::GetFloatFieldByName(lua, "offscreen_distance", &this->offscreen_distance);
		SCRIPT::GetIntFieldByName(lua, "offscreen_behavior", &this->offscreen_behavior);
		SCRIPT::GetIntFieldByName(lua, "facing", &this->facing);
		SCRIPT::GetUIntFieldByName(lua, "phys_slope", &this->slopeType);
		SCRIPT::GetUIntFieldByName(lua, "physic", &this->physic);
		SCRIPT::GetUIntFieldByName(lua, "effect", &this->effect);
		SCRIPT::GetUIntFieldByName(lua, "phys_solid", &this->phys_solid);
		SCRIPT::GetUIntFieldByName(lua, "phys_one_sided", &this->phys_one_sided);
		SCRIPT::GetUIntFieldByName(lua, "phys_ghostlike", &this->phys_ghostlike);
		SCRIPT::GetUIntFieldByName(lua, "phys_bullet_collidable", &this->phys_bullet_collidable);
		SCRIPT::GetUIntFieldByName(lua, "touch_detection", &this->touch_detection);
		SCRIPT::GetIntFieldByName(lua, "bullets_per_shot", &this->bullets_count);
		SCRIPT::GetUIntFieldByName(lua, "reload_time", &this->reload_time);
		SCRIPT::GetUIntFieldByName(lua, "clip_reload_time", &this->clip_reload_time);
		SCRIPT::GetUIntFieldByName(lua, "shots_per_clip", &this->shots_per_clip);
		SCRIPT::GetUIntFieldByName(lua, "is_ray_weapon", &this->is_ray_weapon);
		SCRIPT::GetUIntFieldByName(lua, "time_to_live", &this->time_to_live);
		//SCRIPT::GetStringFieldByName(lua, "end_effect", &this->end_effect);
		GET_STR("end_effect", end_effect)
		SCRIPT::GetUIntFieldByName(lua, "bullet_damage", &this->bullet_damage);
		SCRIPT::GetFloatFieldByName(lua, "bullet_vel", &this->bullet_vel);
		SCRIPT::GetUIntFieldByName(lua, "multiple_targets", &this->multiple_targets);
		SCRIPT::GetUIntFieldByName(lua, "hurts_same_type", &this->hurts_same_type);
		//SCRIPT::GetStringFieldByName(lua, "main_weapon", &this->main_weapon);
		GET_STR("main_weapon", main_weapon)
		//SCRIPT::GetStringFieldByName(lua, "alt_weapon", &this->alt_weapon);
		GET_STR("alt_weapon", alt_weapon)
		SCRIPT::GetUIntFieldByName(lua, "health", &this->health);
		SCRIPT::GetUIntFieldByName(lua, "mp_count", (UINT*)&this->mpCount);

		SCRIPT::GetFloatFieldByName(lua, "particle_min_speed", &this->min_speed);
		SCRIPT::GetFloatFieldByName(lua, "particle_max_speed", &this->max_speed);
		SCRIPT::GetFloatFieldByName(lua, "particle_min_angle", &this->min_angle);
		SCRIPT::GetFloatFieldByName(lua, "particle_max_angle", &this->max_angle);
		SCRIPT::GetFloatFieldByName(lua, "particle_min_param", &this->min_param);
		SCRIPT::GetFloatFieldByName(lua, "particle_max_param", &this->max_param);
		SCRIPT::GetFloatFieldByName(lua, "particle_min_trace", &this->min_trace);
		SCRIPT::GetFloatFieldByName(lua, "particle_max_trace", &this->max_trace);
		SCRIPT::GetIntFieldByName(lua, "trajectory_type", &this->trajectory);
		SCRIPT::GetFloatFieldByName(lua, "trajectory_param1", &this->trajectory_param1);
		SCRIPT::GetFloatFieldByName(lua, "trajectory_param2", &this->trajectory_param2);
		SCRIPT::GetIntFieldByName(lua, "affected_by_wind", &this->affected_by_wind);
		SCRIPT::GetFloatFieldByName(lua, "gravity_x", &this->gravity_x);
		SCRIPT::GetFloatFieldByName(lua, "gravity_y", &this->gravity_y);

		SCRIPT::GetColorFieldByName(lua, "start_color", this->start_color);
		SCRIPT::GetColorFieldByName(lua, "end_color", this->end_color);
		SCRIPT::GetColorFieldByName(lua, "var_color", this->var_color);

		SCRIPT::GetIntFieldByName(lua, "max_particles", &this->max_particles);

		SCRIPT::GetFloatFieldByName(lua, "particle_life_min", &this->particle_life_min);

		SCRIPT::GetFloatFieldByName(lua, "particle_life_max", &this->particle_life_max);
		SCRIPT::GetFloatFieldByName(lua, "start_size", &this->start_size);
		SCRIPT::GetFloatFieldByName(lua, "size_variability", &this->size_variability);
		SCRIPT::GetFloatFieldByName(lua, "end_size", &this->end_size);
		SCRIPT::GetFloatFieldByName(lua, "particle_life", &this->particle_life);

		SCRIPT::GetFloatFieldByName(lua, "particle_life_var", &this->particle_life_var);
		SCRIPT::GetFloatFieldByName(lua, "system_life", &this->system_life);

		SCRIPT::GetIntFieldByName(lua, "emission", &this->emission);

#ifdef MAP_EDITOR
		SCRIPT::GetColorFieldByName(lua, "editor_color", this->editor_color );
#endif //MAP_EDITOR

		SCRIPT::GetIntFieldByName(lua, "faction_id", &this->faction_id);
		SCRIPT::GetIntVectorFieldByName( lua, "faction_hates", this->faction_hates, false );
		SCRIPT::GetIntVectorFieldByName( lua, "faction_follows", this->faction_follows, false );

		#undef GET_STR

		// overlay
		// Стек: env
		lua_getfield(lua, -1, "overlay");	// Стек: env overlayCount
		if (lua_istable(lua, -1))
		{
			this->overlayCount = (UINT)lua_objlen(lua, -1);
			if (overlayCount)
			{
				this->overlayUsage = new UINT[this->overlayCount];
				UINT i = 0;
				for(lua_pushnil(lua); lua_next(lua, -2); i++, lua_pop(lua, 1))
				{
					this->overlayUsage[i] = (UINT)lua_tointeger(lua, -1);
				}

				i = 0;
				this->ocolor = new RGBAf[this->overlayCount];
					
				lua_getfield(lua, -2, "ocolor");
				if (lua_istable(lua, -1))
				{
					int top = lua_gettop(lua);
					for(lua_pushnil(lua); i < this->overlayCount && lua_next(lua, -2); i++, lua_pop(lua, 1))
					{
						SCRIPT::GetColorFromTable(lua, -1, this->ocolor[i]);
					}
					// Этот settop нужен на случай, если мы обхожим не весь ocolor - lua_next тогда не удаляет последний элемент со стека.
					lua_settop(lua, top);
				}
				lua_pop(lua, 1);						// Стек: env
				
				if (i < this->overlayCount)
				{
					sLog(DEFAULT_LOG_NAME, logLevelWarning, "В прототипе %s не указаны цвета всех оверлеев. Дополняем стандартными", this->name.c_str());
					for (; i < this->overlayCount; i++)
						this->ocolor[i] = RGBAf(1.0f, 1.0f, 1.0f, 1.0f);
				}
			}
			else
			{
				sLog(DEFAULT_LOG_NAME, logLevelError, "В прототипе %s таблица overlayCount - пуста", this->name.c_str());
			}
		}
		lua_pop(lua, 1);						// Стек: env

		// Стек: env
		lua_getfield(lua, -1, "sprites");		// Стек: env sprites
		if (lua_istable(lua, -1))
		{
			lua_pushnil(lua);
			while (lua_next(lua, -2) != 0) 
			{
				const char* str = lua_tostring(lua, -1);
				char* val = new char[strlen(str) + 1];
				strcpy(val, str);
				this->sprites.push_back(val);
				lua_pop(lua, 1);
			}
		}
		lua_pop(lua, 1);						// Стек: env

		lua_getfield(lua, -1, "sounds");		// Стек: env sounds
		if (lua_istable(lua, -1))
		{
			lua_pushnil(lua);
			while (lua_next(lua, -2) != 0) 
			{
				const char* str = lua_tostring(lua, -1);
				char* val = new char[strlen(str) + 1];
				strcpy(val, str);
				this->sounds.push_back(val);
				lua_pop(lua, 1);
			}
		}
		lua_pop(lua, 1);						// Стек: env

		// Загрузка массива прототипов анимаций
		LoadAnimProtos();

	} // if lua_pcall

	lua_pop(lua, 1);						// Стек: 

	STACK_CHECK(lua);

	return true;
}


void Proto::LoadAnimProtos() 
{
	STACK_CHECK_INIT(lua);
	// Стек: env
	lua_getfield(lua, -1, "animations");	// Стек: env animations
	if (lua_istable(lua, -1))
	{
		const size_t count = lua_objlen(lua, -1);
		if (count == 0)
		{
			lua_pop(lua, 1);						// Стек: env
			return;
		}

		Animation* a = new Animation[count];
		char* name_buf = NULL;
		size_t nbFrames = 0;
		AnimationFrame* af = NULL;
		map<string, size_t> names;

		// Цикл по массиву анимаций
		size_t i = 0;
		// Если в массиве было что-то, отличное от таблиц, часть нормальных таблиц может не прочитаться
		//for (UINT i = 1; i <= count; i++)
		// Стек: env animations nil
		for(lua_pushnil(lua); lua_next(lua, -2); lua_pop(lua, 1), i++)
		{
			// Стек: env animations key animations[key]
			if (lua_istable(lua, -1))
			{
				// Получили таблицу прототипа анимации, читаем ее
				
				// name_buf будет храниться в анимации, если все будет ОК. Только иначе освобождаем его в этой функции.
				SCRIPT::GetStringFieldByName(lua, "name", &name_buf);

				lua_getfield(lua, -1, "frames");	// Стек: env animations key animations[key] frames
				if (lua_istable(lua, -1))
				{
					nbFrames = lua_objlen(lua, -1);
					if (nbFrames > 0 && nbFrames < MAX_ANIMFRAMES_COUNT)
					{
						af = new AnimationFrame[nbFrames];

						size_t j = 0;
						// Стек: env animations key animations[key] frames nil
						for(lua_pushnil(lua); lua_next(lua, -2); lua_pop(lua, 1), j++)
						{
							// Стек: env animations key animations[key] frames key frame
							assert(j < nbFrames);
							if (lua_istable(lua, -1))
							{
								SCRIPT::GetUIntFieldByName(lua, "num", &af[j].num);
								SCRIPT::GetUIntFieldByName(lua, "dur", &af[j].duration);
								SCRIPT::GetUIntFieldByName(lua, "com", (UINT*)&(af[j].command) );		// TODO: хз, возможно опасно
								SCRIPT::GetIntFieldByName(lua, "param", &af[j].param);
								SCRIPT::GetStringFieldByName(lua, "txt", &af[j].txt_param);
							}
							else
							{
								sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "В прототипе %s в анимации %s кадр %d - %s. Отменяем загрузку кадров.", 
									this->name.c_str(), name_buf, j, lua_typename(lua, lua_type(lua, -1)) );

								DELETEARRAY(af);
								nbFrames = 0;
							}
							//lua_pop(lua, 1);	// Стек: env animations key animations[key] frames key
						}
					}
					else
					{						
						sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "В анимации %s %d кадров. Пропускаем.", name_buf, nbFrames);
						nbFrames = 0;
						DELETEARRAY(name_buf);
					}

					// Стек: env animations key animations[key] frames	

				} // if (lua_istable(lua, -1))	frames
				else
				{						
					sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "В анимации %s нет таблицы frames. Пропускаем.", name_buf);
					nbFrames = 0;
					DELETEARRAY(name_buf);
				}

				if (nbFrames > 0 && nbFrames < MAX_ANIMFRAMES_COUNT)
				{
					//this->animations.push_back(ap);
					a[i].frames = af;
					a[i].frameCount = (USHORT)nbFrames;
					a[i].name = name_buf;

					if (name_buf)
					{
						// TODO: проверка на повторения
						names[string(name_buf)] = i;
					}
				}

				// Стек: env animations key animations[key] frames
				lua_pop(lua, 1);	// Стек: env animations key animations[key]
			}
			else
			{
				sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "В прототипе %s animations[%d] - %s", 
					this->name.c_str(), i, lua_typename(lua, lua_type(lua, -1)) );
			}

		} // for 

		if (this->animations)
		{
			bool* copy_anim = (bool*)malloc(count);
			memset(copy_anim, true, count);
			size_t nbCopy = count;

			map<string, size_t>::iterator fit;
			ASSERT(names.size() == count);
			// Тут анимации родителя заменяются на те, что объявлены в ребенке.
			for(map<string, size_t>::iterator it = names.begin(); 
				it != names.end(); )
			{
				fit = this->animNames.find(it->first);
				if (fit != this->animNames.end())
				{
					// Тут происходит копирование новой анимации в старую, так что новая замещает старую.
					this->animations[fit->second] = a[it->second];
					copy_anim[it->second] = false;
					nbCopy--;

					names.erase(it++);
					continue;
				}

				it++;
			}

			if (nbCopy > 0)
			{
				// В ребенке объявлены совсем новые анимации
				// Тут создается массив большео размера и в него копируются и старые, и совсем новые.
				size_t new_size =  this->animationsCount + nbCopy;
				Animation* new_arr = new Animation[new_size];
				size_t i = 0;
				// Копируем в новый массив старые и новые, заместившие старые.
				for (; i < this->animationsCount; i++)
				{
					new_arr[i] = this->animations[i];
				}

				// Копируем совсем новые.
				for(size_t j = 0; j < count && nbCopy > 0; j++)
				{
					if (copy_anim[j])
					{
						new_arr[i] = a[j];
						names[a[j].name] = i;
						i++;
						nbCopy--;
					}
				}

				this->animationsCount = new_size;
				// Добавляем к старым именам совсем новые.
				this->animNames.insert(names.begin(), names.end());
				DELETEARRAY(this->animations);
				this->animations = new_arr;
			}
			DELETEARRAY(a);
			free(copy_anim);
		}
		else
		{
			this->animations = a;
			this->animationsCount = count;
			this->animNames = names;
		}

		

		// Стек: env animations

	} // if (lua_istable(lua, -1))   animations	

	lua_pop(lua, 1);						// Стек: env
	STACK_CHECK(lua);
}


void Proto::Copy(const Proto& src) 
{
	behaviour = src.behaviour;
	DELETEARRAY(texture);
	texture = StrDupl(src.texture);
	z = src.z;
	drops_shadow = src.drops_shadow;
	frame_widht = src.frame_widht;
	frame_height = src.frame_height;
	next_shift_y = src.next_shift_y;
	hurts_same_type = src.hurts_same_type;
	push_force = src.push_force;
	mass = src.mass;
	color = src.color;
	ocolor = src.ocolor;
	walk_vel_multiplier = src.walk_vel_multiplier;
	jump_vel_multiplier = src.jump_vel_multiplier;
	bounce_bonus = src.bounce_bonus;	
	gravity_bonus = src.gravity_bonus;
	ghost_to = src.ghost_to;
	//Поведение за границами экрана для ObjCharacter
	offscreen_distance = src.offscreen_distance;
	offscreen_behavior = src.offscreen_behavior;
	facing = src.facing;
	//Количество "точек крепления" спрайта
	mpCount = src.mpCount;
	phys_walk_acc = src.phys_walk_acc;
	phys_jump_vel = src.phys_jump_vel;
	phys_max_x_vel = src.phys_max_x_vel;
	phys_max_y_vel = src.phys_max_y_vel;
	bounce = src.bounce;
	phys_solid = src.phys_solid;
	phys_one_sided = src.phys_one_sided;
	phys_ghostlike = src.phys_ghostlike;
	phys_bullet_collidable = src.phys_bullet_collidable;
	physic = src.physic;
	slopeType = src.slopeType;
	touch_detection = src.touch_detection;
	health = src.health;
	DELETEARRAY(main_weapon);
	main_weapon = StrDupl(src.main_weapon);
	DELETEARRAY(alt_weapon);
	alt_weapon = StrDupl(src.alt_weapon);
	bullet_vel = src.bullet_vel;
	bullet_damage = src.bullet_damage;
	reload_time = src.reload_time;
	bullets_count = src.bullets_count;
	multiple_targets = src.multiple_targets;
	shots_per_clip = src.shots_per_clip;
	clip_reload_time = src.clip_reload_time;
	is_ray_weapon = src.is_ray_weapon;
	time_to_live = src.time_to_live; 
	DELETEARRAY(end_effect);
	end_effect = StrDupl(src.end_effect);
	effect = src.effect;
	//////////////////////////////////////////////////////////////////////////
	//PSystem flds
	start_color = src.start_color;
	end_color = src.end_color;
	var_color = src.var_color;
	//texture_name = StrDupl(src.texture_name);
	damage_type = src.damage_type;
	max_particles = src.max_particles;
	particle_life_min = src.particle_life_min;
	particle_life_max = src.particle_life_max;
	start_size = src.start_size;
	size_variability = src.size_variability;
	end_size = src.end_size;
	particle_life = src.particle_life;
	particle_life_var = src.particle_life_var;
	system_life = src.system_life;
	emission = src.emission;
	min_speed = src.min_speed;
	max_speed = src.max_speed;
	min_angle = src.min_angle;
	max_angle = src.max_angle;
	min_param = src.min_param;
	max_param = src.max_param;
	min_trace = src.min_trace;
	max_trace = src.max_trace;
	trajectory = src.trajectory;
	trajectory_param1 = src.trajectory_param1;
	trajectory_param2 = src.trajectory_param2;
	affected_by_wind = src.affected_by_wind;
	gravity_x = src.gravity_x;	//Пока только для частиц.
	gravity_y = src.gravity_y;
	env_material = src.env_material;
	DELETEARRAY(env_onenter);
	env_onenter = StrDupl(src.env_onenter);
	DELETEARRAY(env_onleave);
	env_onleave = StrDupl(src.env_onleave);
	DELETEARRAY(env_onstay);
	env_onstay = StrDupl(src.env_onstay);
	DELETEARRAY(env_onuse);
	env_onstay = StrDupl(src.env_onuse);

	//////////////////////////////////////////////////////////////////////////

	overlayCount = src.overlayCount;
	if (src.overlayUsage)
	{
		overlayUsage = new UINT[overlayCount];
		memcpy(overlayUsage, src.overlayUsage, sizeof(overlayUsage[0])*overlayCount);
	}
	if (src.ocolor)
	{
		ocolor = new RGBAf[overlayCount];
		memcpy(ocolor, src.ocolor, sizeof(ocolor[0])*overlayCount);
	}

	//////////////////////////////////////////////////////////////////////////

	vector<char*>::const_iterator it;
	for (it = src.sounds.begin(); it != src.sounds.end(); it++)
		this->sounds.push_back(StrDupl(*it));

	for (it = src.sprites.begin(); it != src.sprites.end(); it++)
		this->sprites.push_back(StrDupl(*it));

//	vector<char*> sounds;
//	vector<char*> sprites;

	//////////////////////////////////////////////////////////////////////////

	this->faction_id = src.faction_id;
	vector<int>::const_iterator iit;
	for (iit = src.faction_hates.begin(); iit != src.faction_hates.end(); iit++)
		this->faction_hates.push_back(*iit);
	for (iit = src.faction_follows.begin(); iit != src.faction_follows.end(); iit++)
		this->faction_follows.push_back(*iit);

	//////////////////////////////////////////////////////////////////////////

	this->animationsCount = src.animationsCount;
	//Animation* animations;
	this->animations = new Animation[this->animationsCount];
	for (size_t i = 0; i < animationsCount; i++)
	{
		this->animations[i] = src.animations[i];
	}
	animNames = src.animNames;
	
	//map<string, size_t> animNames;


}

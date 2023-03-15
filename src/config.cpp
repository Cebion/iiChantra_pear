#include "StdAfx.h"

#include "config.h"
#include "script/script.h"

#include "scene.h"

extern lua_State* lua;
extern config cfg;
extern char path_config[MAX_PATH];


// Это названия полей в таблице в файле. Порядок тот же, что и в ConfigActionKeys.
const char* configKeysNames[] = 
{	
	"left", "right", "down", "up", "jump", "sit", "fire",
	"change_weapon", "change_player", 
	"gui_nav_accept", "gui_nav_decline",
	"gui_nav_prev", "gui_nav_next",
	"gui_nav_menu",
	"gui_nav_screenshot",
	"player_use"
};

const char* GetConfigKeyName(ConfigActionKeys key)
{
	return configKeysNames[key];
}


// Это названия кнопок для стандартной раскладки управления. Порядок тот же, что и в ConfigActionKeys.
const char* standardKeys[KEY_SETS_NUMBER][cakLastKey] = 
{
	{
		"left", "right", "down", "up", "x", "z", "c",
		"lshift", "a", 
		"enter", "x",
		"up", "down",
		"esc",
		"f11",
		"d"
	},
	{
		"joystick_hat_left", "joystick_hat_right", "joystick_hat_down", "joystick_hat_up",
		"joystick_3", "joystick_4", "joystick_1", "joystick_2", "joystick_6", "joystick_1",
		"joystick_3", "joystick_hat_up", "joystick_hat_down", "joystick_10", "f11", ""
	},
	{
		"", "", "", "", "", "", "",
		"", "",
		"c", "", "", "", "", "", ""
	},
	{
		"", "", "", "", "", "", "",
		"", "",
		"", "", "", "", "", "", ""
	}
};


void CheckConfigKeySanity( size_t s, ConfigActionKeys key )
{
	if (cfg.cfg_keys[s][key] == 0)
	{
		UINT val = GetNumByVKey(standardKeys[s][key]);
		cfg.cfg_keys[s][key] = val;

		STACK_CHECK_INIT(lua);

		lua_getglobal(lua, "CONFIG");		// Стек: config
		if (lua_istable(lua, -1))
		{
			lua_getfield(lua, -1, "key_conf");		// Стек: config key_conf
			if (lua_istable(lua, -1))
			{
				lua_rawgeti(lua, -1, s+1);			// Стек: config key_conf key_conf[s]
				if (lua_istable(lua, -1))
				{
					lua_pushinteger(lua, val);
					lua_setfield(lua, -2, configKeysNames[key]);
				}
				lua_pop(lua, 1);
			}
			lua_pop(lua, 1);
		}
		lua_pop(lua, 1);

		STACK_CHECK(lua);

	}
}

void LoadKeysConfig() 
{
	STACK_CHECK_INIT(lua);
	lua_getfield(lua, -1, "key_conf");		// Стек: config
	if (lua_istable(lua, -1))
	{
		size_t i = 0;
		lua_pushnil(lua);
		while (lua_next(lua, -2) != 0 && i < KEY_SETS_NUMBER) 
		{
			luaL_argcheck(lua, lua_istable(lua, -1), -1, "В таблице key_conf должны быть таблицы");

			for (size_t j = 0; j < cakLastKey; j++)
			{
				SCRIPT::GetUIntFieldByName(lua, configKeysNames[j], &cfg.cfg_keys[i][j] );
			}

			i++;

			lua_pop(lua, 1);

		}
	}
	lua_pop(lua, 1);						// Стек: config

	STACK_CHECK(lua);
}
bool LoadConfig()
{
	bool res = false;

	lua_getglobal(lua, "CONFIG");		// Стек: config

	if (lua_istable(lua, -1))
	{
		bool videomode_changed = false;
		bool backcolor_changed = false;
		float tmpf = 0;
		UINT tmpu = 0;
		BOOL tmpb = 0;

		tmpu = cfg.scr_width;	SCRIPT::GetUIntFieldByName(lua, "scr_width", &cfg.scr_width );		videomode_changed = ( (tmpu != cfg.scr_width) || videomode_changed);
		tmpu = cfg.scr_height;	SCRIPT::GetUIntFieldByName(lua, "scr_height", &cfg.scr_height );	videomode_changed = ( (tmpu != cfg.scr_height) || videomode_changed);
		tmpu = cfg.scr_bpp;		SCRIPT::GetUIntFieldByName(lua, "bpp", &cfg.scr_bpp );				videomode_changed = ( (tmpu != cfg.scr_bpp) || videomode_changed);

		tmpf = cfg.near_z;	SCRIPT::GetFloatFieldByName(lua, "near_z", &cfg.near_z);	videomode_changed = ( (tmpf != cfg.near_z) || videomode_changed);
		tmpf = cfg.far_z;	SCRIPT::GetFloatFieldByName(lua, "far_z", &cfg.far_z);		videomode_changed = ( (tmpf != cfg.far_z) || videomode_changed);

		tmpu = cfg.window_width;	SCRIPT::GetUIntFieldByName(lua, "window_width", &cfg.window_width );	videomode_changed = ( (tmpu != cfg.window_width) || videomode_changed);
		tmpu = cfg.window_height;	SCRIPT::GetUIntFieldByName(lua, "window_height", &cfg.window_height );	videomode_changed = ( (tmpu != cfg.window_height) || videomode_changed);

		tmpb = cfg.fullscreen;	SCRIPT::GetUIntFieldByName(lua, "fullscreen", (UINT*)&cfg.fullscreen );	videomode_changed = ( (tmpb != cfg.fullscreen) || videomode_changed);
		tmpb = cfg.vert_sync; 	SCRIPT::GetUIntFieldByName(lua, "vert_sync", (UINT*)&cfg.vert_sync );	videomode_changed = ( (tmpb != cfg.vert_sync) || videomode_changed);
		
		SCRIPT::GetUIntFieldByName(lua, "debug", (UINT*)&cfg.debug );
		SCRIPT::GetUIntFieldByName(lua, "show_fps", (UINT*)&cfg.show_fps );

		tmpf = cfg.backcolor_r; SCRIPT::GetFloatFieldByName(lua, "backcolor_r", &cfg.backcolor_r); backcolor_changed = ( (tmpf != cfg.backcolor_r) || backcolor_changed);
		tmpf = cfg.backcolor_g; SCRIPT::GetFloatFieldByName(lua, "backcolor_g", &cfg.backcolor_g); backcolor_changed = ( (tmpf != cfg.backcolor_g) || backcolor_changed);
		tmpf = cfg.backcolor_b; SCRIPT::GetFloatFieldByName(lua, "backcolor_b", &cfg.backcolor_b); backcolor_changed = ( (tmpf != cfg.backcolor_b) || backcolor_changed);

		SCRIPT::GetFloatFieldByName(lua, "volume", &cfg.volume);
		SCRIPT::GetFloatFieldByName(lua, "volume_music", &cfg.volume_music);
		SCRIPT::GetFloatFieldByName(lua, "volume_sound", &cfg.volume_sound);

#ifdef GAMETICK_FROM_CONFIG
		SCRIPT::GetUIntFieldByName(lua, "gametick", (UINT*)&cfg.gametick );
#endif // GAMETICK_FROM_CONFIG

		LoadKeysConfig();

		SCRIPT::GetUIntFieldByName(lua, "gui_nav_mode", &cfg.gui_nav_mode );
		SCRIPT::GetUIntFieldByName(lua, "gui_nav_cycled", &cfg.gui_nav_cycled );

		SCRIPT::GetUIntFieldByName(lua, "shadows", &cfg.shadows );

		SCRIPT::GetUIntFieldByName(lua, "joystick_sensivity", &cfg.joystick_sensivity );

		SCRIPT::GetUIntFieldByName(lua, "log_level", (UINT*)&cfg.log_level );

		DELETEARRAY(cfg.language);
		SCRIPT::GetStringFieldByName(lua, "language", &cfg.language );

		SCRIPT::GetUIntFieldByName(lua, "weather", (UINT*)&cfg.weather );

		//Checking for config keys sanity
		CheckConfigKeySanity(0, cakGuiNavAccept);
		CheckConfigKeySanity(0, cakGuiNavDecline);
		CheckConfigKeySanity(0, cakGuiNavNext);
		CheckConfigKeySanity(0, cakGuiNavPrev);

		scene::ApplyConfig(videomode_changed, backcolor_changed);
		res = true;

	}
	else
		luaL_error(lua, "Ошибка. Таблица CONFIG не существует.");

	lua_pop(lua, 1); // Стек:

	return res;
}

void PrintControls(FILE* f)
{
	fputs("\tkey_conf = {", f);

	char ch[MAX_VKEY_NAME_LEN] = "";

	for (size_t i = 0; i < KEY_SETS_NUMBER; i++)
	{
		if (i > 0)
			fputs(",", f);
		fputs("\n\t\t{\n", f);

		for (size_t j = 0; j < cakLastKey; j++)
		{
			fprintf(f, "\t\t\t%s = ", configKeysNames[j]);
			if (GetVKeyByNum(cfg.cfg_keys[i][j], ch)) 
				fprintf(f, "keys[\"%s\"];\n", ch); 
			else 
				fprintf(f, "%s;\n", ch);
		}
		fprintf(f, "\t\t}");
	}
	fputs("\n\t};\n", f);
}

bool SaveConfig()
{
	bool res = false;
	char path_to_cfg[MAX_PATH];
	sprintf(path_to_cfg, "%s%s", path_config, DEFAULT_CONFIG_NAME);

	FILE* f;
	f = fopen(path_to_cfg, "w");

	if(f)
	{
		fprintf(f, "CONFIG = \n");
		fprintf(f, "{");
		fprintf(f, "\twindow_width = %d;\n", cfg.window_width);
		fprintf(f, "\twindow_height = %d;\n", cfg.window_height);
		fprintf(f, "\tscr_width = %d;\n", cfg.scr_width);
		fprintf(f, "\tscr_height = %d;\n", cfg.scr_height);
		fprintf(f, "\tnear_z = %f;\n", cfg.near_z);
		fprintf(f, "\tfar_z = %f;\n", cfg.far_z);
		fprintf(f, "\tbpp = %d;\n", cfg.scr_bpp);
		fprintf(f, "\tfullscreen = %d;\n", cfg.fullscreen);
		fprintf(f, "\tvert_sync = %d;\n", cfg.vert_sync);
		fprintf(f, "\tdebug = %d;\n", cfg.debug);
		fprintf(f, "\tshow_fps = %d;\n\n", cfg.show_fps);
		fprintf(f, "\tlog_level = constants.%s;\n", LogLevelGetName(cfg.log_level));
		fprintf(f, "\t\n");
#ifdef GAMETICK_FROM_CONFIG
		fprintf(f, "\tgametick = %d;\n\n", cfg.gametick);
		fprintf(f, "\t\n");
#endif // GAMETICK_FROM_CONFIG
		fprintf(f, "\tbackcolor_r = %f;\n", cfg.backcolor_r);
		fprintf(f, "\tbackcolor_g = %f;\n", cfg.backcolor_g);
		fprintf(f, "\tbackcolor_b = %f;\n", cfg.backcolor_b);
		fprintf(f, "\t\n");
		fprintf(f, "\tvolume = %f;\n", cfg.volume);
		fprintf(f, "\tvolume_music = %f;\n", cfg.volume_music);
		fprintf(f, "\tvolume_sound = %f;\n", cfg.volume_sound);
		fprintf(f, "\t\n");
		fprintf(f, "\t-- controls\n\n");
		PrintControls(f);

		fprintf(f, "\tjoystick_sensivity = %i;\n", cfg.joystick_sensivity);

		fprintf(f, "\t\n");
		fprintf(f, "\t-- gui settings\n\n");
		fprintf(f, "\tgui_nav_mode = %d;\n", cfg.gui_nav_mode);
		fprintf(f, "\tgui_nav_cycled = %d;\n", cfg.gui_nav_cycled);

		fprintf(f, "\t\n");
		fprintf(f, "\t-- game\n\n");
		fprintf(f, "\tlanguage = \"%s\";\n", cfg.language);
		fprintf(f, "\tshadows = %i;\n", cfg.shadows);
		fprintf(f, "\tweather = %i;\n", cfg.weather);

		fprintf(f, "}");
		fprintf(f, "\n");
		fprintf(f, "LoadConfig();\n");
		fclose(f);
		sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "%s succesfully saved", path_to_cfg);
		res = true;

		SCRIPT::ExecFile(path_to_cfg);
	}
	else
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Failed to save %s", path_to_cfg);

	return res;
}

bool RecreateConfig()
{
	sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Recreating config...");

	cfg.window_width = 640;
	cfg.window_height = 480;
	cfg.scr_width = 640;
	cfg.scr_height = 480;
	cfg.near_z = -1;
	cfg.far_z = 1;
	cfg.scr_bpp = 24;
	cfg.fullscreen = 0;
	cfg.vert_sync = 0;
	cfg.debug = 0;
	cfg.show_fps = 0;
#ifdef GAMETICK_FROM_CONFIG
	cfg.gametick = 10;
#endif // GAMETICK_FROM_CONFIG
	cfg.backcolor_r = 0.0f;
	cfg.backcolor_g = 0.0f;
	cfg.backcolor_b = 0.0f;
	cfg.volume = 1.0f;
	cfg.volume_music = 1.0f;
	cfg.volume_sound = 1.0f;

	for (size_t s = 0; s < KEY_SETS_NUMBER; s++)
		for (size_t i = 0; i < cakLastKey; i++)
			cfg.cfg_keys[s][i] = GetNumByVKey(standardKeys[s][i]);

	cfg.gui_nav_mode = 1;
	cfg.gui_nav_cycled = 1;
	cfg.language = StrDupl("english");
	cfg.joystick_sensivity = 1000;
	cfg.log_level = logLevelError;
	cfg.shadows = 1;
	cfg.weather = 1;

	return SaveConfig();
}

//////////////////////////////////////////////////////////////////////////

map<UINT, string> num_VKey_array;
map<string, UINT> VKey_num_array;

#define MAPVK(vk, name) ASSERT((strlen(name) + 1) < MAX_VKEY_NAME_LEN);	str = string(name); num_VKey_array[vk] = str; VKey_num_array[str] = vk; 

void InitVKeysArrays()
{
	string str;
	MAPVK(SDLK_BACKSPACE, "backspace");
	MAPVK(SDLK_TAB, "tab");
	MAPVK(SDLK_RETURN, "enter");
	MAPVK(SDLK_LSHIFT, "lshift");
	MAPVK(SDLK_LCTRL, "lctrl");
	MAPVK(SDLK_LALT, "lalt");
	MAPVK(SDLK_RSHIFT, "rshift");
	MAPVK(SDLK_RCTRL, "rctrl");
	MAPVK(SDLK_RALT, "ralt");
	MAPVK(SDLK_LSUPER, "lwin");
	MAPVK(SDLK_RSUPER, "rwin");
	MAPVK(SDLK_MENU, "menu");
	MAPVK(SDLK_PAUSE, "pause");
	MAPVK(SDLK_CAPSLOCK, "capslock");
	MAPVK(SDLK_SCROLLOCK, "scrollock");
	MAPVK(SDLK_NUMLOCK, "numlock");
	MAPVK(SDLK_ESCAPE, "esc");
	MAPVK(SDLK_SPACE, "space");
	MAPVK(SDLK_PAGEUP, "pgup");
	MAPVK(SDLK_PAGEDOWN, "pgdown");
	MAPVK(SDLK_END, "end");
	MAPVK(SDLK_HOME, "home");

	MAPVK(SDLK_LEFT, "left");
	MAPVK(SDLK_UP, "up");
	MAPVK(SDLK_RIGHT, "right");
	MAPVK(SDLK_DOWN, "down");

	MAPVK(SDLK_PRINT, "printscreen");
	MAPVK(SDLK_INSERT, "insert");
	MAPVK(SDLK_DELETE, "delete");

	MAPVK(SDLK_0, "0");
	MAPVK(SDLK_1, "1");
	MAPVK(SDLK_2, "2");
	MAPVK(SDLK_3, "3");
	MAPVK(SDLK_4, "4");
	MAPVK(SDLK_5, "5");
	MAPVK(SDLK_6, "6");
	MAPVK(SDLK_7, "7");
	MAPVK(SDLK_8, "8");
	MAPVK(SDLK_9, "9");

	MAPVK(SDLK_a, "a");
	MAPVK(SDLK_b, "b");
	MAPVK(SDLK_c, "c");
	MAPVK(SDLK_d, "d");
	MAPVK(SDLK_e, "e");
	MAPVK(SDLK_f, "f");
	MAPVK(SDLK_g, "g");
	MAPVK(SDLK_h, "h");
	MAPVK(SDLK_i, "i");
	MAPVK(SDLK_j, "j");
	MAPVK(SDLK_k, "k");
	MAPVK(SDLK_l, "l");
	MAPVK(SDLK_m, "m");
	MAPVK(SDLK_n, "n");
	MAPVK(SDLK_o, "o");
	MAPVK(SDLK_p, "p");
	MAPVK(SDLK_q, "q");
	MAPVK(SDLK_r, "r");
	MAPVK(SDLK_s, "s");
	MAPVK(SDLK_t, "t");
	MAPVK(SDLK_u, "u");
	MAPVK(SDLK_v, "v");
	MAPVK(SDLK_w, "w");
	MAPVK(SDLK_x, "x");
	MAPVK(SDLK_y, "y");
	MAPVK(SDLK_z, "z");

	MAPVK(SDLK_KP0, "num0");
	MAPVK(SDLK_KP1, "num1");
	MAPVK(SDLK_KP2, "num2");
	MAPVK(SDLK_KP3, "num3");
	MAPVK(SDLK_KP4, "num4");
	MAPVK(SDLK_KP5, "num5");
	MAPVK(SDLK_KP6, "num6");
	MAPVK(SDLK_KP7, "num7");
	MAPVK(SDLK_KP8, "num8");
	MAPVK(SDLK_KP9, "num9");
	MAPVK(SDLK_KP_MULTIPLY, "multiply");
	MAPVK(SDLK_KP_PLUS, "add");
	MAPVK(SDLK_KP_ENTER, "numenter");
	MAPVK(SDLK_KP_MINUS, "substract");
	MAPVK(SDLK_KP_PERIOD, "decimal");
	MAPVK(SDLK_KP_DIVIDE, "divide");

	MAPVK(SDLK_F1, "f1");
	MAPVK(SDLK_F2, "f2");
	MAPVK(SDLK_F3, "f3");
	MAPVK(SDLK_F4, "f4");
	MAPVK(SDLK_F5, "f5");
	MAPVK(SDLK_F6, "f6");
	MAPVK(SDLK_F7, "f7");
	MAPVK(SDLK_F8, "f8");
	MAPVK(SDLK_F9, "f9");
	MAPVK(SDLK_F10, "f10");
	MAPVK(SDLK_F11, "f11");
	MAPVK(SDLK_F12, "f12");

	MAPVK(SDLK_SEMICOLON, "semicolon");
	MAPVK(SDLK_PLUS, "plus");
	MAPVK(SDLK_COMMA, "comma");
	MAPVK(SDLK_MINUS, "minus");
	MAPVK(SDLK_PERIOD, "period");
	MAPVK(SDLK_SLASH, "slash");
	MAPVK(SDLK_EQUALS, "equals");
	MAPVK(SDLK_BACKQUOTE, "tilde");
	MAPVK(SDLK_LEFTBRACKET, "lbracket");
	MAPVK(SDLK_BACKSLASH, "backslash");
	MAPVK(SDLK_RIGHTBRACKET, "rbracket");
	MAPVK(SDLK_QUOTE, "quote");

	MAPVK(SDLK_LAST+1, "axis0+");
	MAPVK(SDLK_LAST+2, "axis0-");
	MAPVK(SDLK_LAST+3, "axis1+");
	MAPVK(SDLK_LAST+4, "axis1-");
	MAPVK(SDLK_LAST+5, "axis2+");
	MAPVK(SDLK_LAST+6, "axis2-");
	MAPVK(SDLK_LAST+7, "joystick_1");
	MAPVK(SDLK_LAST+8, "joystick_2");
	MAPVK(SDLK_LAST+9, "joystick_3");
	MAPVK(SDLK_LAST+10, "joystick_4");
	MAPVK(SDLK_LAST+11, "joystick_5");
	MAPVK(SDLK_LAST+12, "joystick_6");
	MAPVK(SDLK_LAST+13, "joystick_7");
	MAPVK(SDLK_LAST+14, "joystick_8");
	MAPVK(SDLK_LAST+15, "joystick_9");
	MAPVK(SDLK_LAST+16, "joystick_10");
	MAPVK(SDLK_LAST+17, "joystick_11");
	MAPVK(SDLK_LAST+18, "joystick_12");
	MAPVK(SDLK_LAST+19, "joystick_13");

	MAPVK(SDLK_LAST+20, "joystick_hat_up");
	MAPVK(SDLK_LAST+21, "joystick_hat_down");
	MAPVK(SDLK_LAST+22, "joystick_hat_left");
	MAPVK(SDLK_LAST+23, "joystick_hat_right");
}

void ClearVKeysArrays()
{
	num_VKey_array.clear();
	VKey_num_array.clear();;
}


BOOL GetVKeyByNum(UINT num, char* vk)
{
	map<UINT, string>::iterator it = num_VKey_array.find(num);
	if (it != num_VKey_array.end())
	{
		strcpy(vk, it->second.c_str());
		return TRUE;
	}
	else
	{
		sprintf(vk, "%i", num);
		return FALSE;
	}
}


UINT GetNumByVKey(string s)
{
	map<string, UINT>::iterator it = VKey_num_array.find(s);
	if (it != VKey_num_array.end())
	{
		return it->second;
	}

	return 0;
}

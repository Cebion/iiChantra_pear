#ifndef __CONFIG_H_
#define __CONFIG_H_

#include "misc.h"
#include "script/api.h"

// Скисок тех кнопок, которые хранятся в конфиге.
// Для добавления новой кнопки в игру необходимо:
// 1. Дать ей назвние в этом списке (cakLastKey - всегда последняя и кнопкой не является)
// 2. Дать в списке configKeysNames имя полю, которое будет храниться в файле конфига.
// 3. В списке standartKeys задать стандартное значение для кнопки.
enum ConfigActionKeys
{
	cakLeft, cakRight, cakDown, cakUp, cakJump, cakSit, cakFire,
	cakChangeWeapon, cakChangePlayer, 
	cakGuiNavAccept, cakGuiNavDecline,
	cakGuiNavPrev, cakGuiNavNext,
	cakGuiNavMenu,
	cakScreenshot,
	cakPlayerUse,

	cakLastKey		// Используется для опеределения размера массива
};

typedef struct tagCONFIG {
	UINT scr_width;
	UINT scr_height;
	UINT scr_bpp;

	float near_z;
	float far_z;

	UINT window_width;
	UINT window_height;

	UINT joystick_sensivity;

	BOOL fullscreen;
	BOOL vert_sync;

	BOOL debug;
	BOOL show_fps;


	float backcolor_r;
	float backcolor_g;
	float backcolor_b;

	float volume;
	float volume_music;
	float volume_sound;

#ifdef GAMETICK_FROM_CONFIG
	UINT gametick;
#endif // GAMETICK_FROM_CONFIG

	UINT cfg_keys[KEY_SETS_NUMBER][cakLastKey];


	UINT gui_nav_mode;
	UINT gui_nav_cycled;

	UINT shadows;
	
	LogLevel log_level;

	char* language;
	BOOL weather;

	tagCONFIG()
	{
		memset(this, 0, sizeof(this));
	}

	~tagCONFIG()
	{
		DELETEARRAY(this->language);
	}
} config;

bool LoadConfig();
bool SaveConfig();
bool RecreateConfig();

void InitVKeysArrays();
void ClearVKeysArrays();
UINT GetNumByVKey(string s);
BOOL GetVKeyByNum(UINT num, char* vk);

const char* GetConfigKeyName(ConfigActionKeys key);

#endif

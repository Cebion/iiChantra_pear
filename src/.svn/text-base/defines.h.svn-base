#ifndef __DEFINES_H_
#define __DEFINES_H_

#if defined(WIN32) && !defined(__MINGW32__)
#define __INLINE __forceinline
#else
#define __INLINE inline
#endif

#if defined(WIN32) && !defined(__MINGW32__)
	#define SAP_VC_ASM
#else
	#define SAP_GCC_ASM
#endif // WIN32

#ifndef MAX_PATH
	#define MAX_PATH PATH_MAX
#endif

#define UNUSED_ARG(x)	(void)x
//////////////////////////////////////////////////////////////////////////
template<typename T>
__INLINE void DELETESINGLE(T*& a)
{
	delete a, a = NULL;
}

template<typename T>
__INLINE void DELETEARRAY(T*& a)
{
	delete [] a, a = NULL;
}
//////////////////////////////////////////////////////////////////////////
// Умолчания

//  Стандартный размер окна
#define DEFAULT_WINDOW_WIDTH		640
#define DEFAULT_WINDOW_HEIGHT		480

// Стандартные пути и файлы
#define DEFAULT_CONFIG_PATH				"/config/"
#define DEFAULT_CONFIG_NAME				"/default.lua"

#define DEFAULT_TEXTURES_PATH			"/textures/"
#define DEFAULT_FONTS_PATH				"/fonts/"
//#define DEFAULT_DATA_PATH				"/data"
#define DEFAULT_PROTO_PATH				"/proto/"
#define DEFAULT_SCRIPTS_PATH			"/scripts/"
#define DEFAULT_LOG_PATH				"/log/"
#define DEFAULT_LEVELS_PATH				"/levels/"
#define DEFAULT_SOUNDS_PATH				"/sounds/"
#define DEFAULT_SCREENSHOTS_PATH		"/screenshots/"
#define DEFAULT_SAVES_PATH				"/saves/"

#define DEFAULT_LOG_NAME				"/iichantra.log"
#define DEFAULT_GUI_LOG_NAME			"/gui.log"
#define DEFAULT_SCRIPT_LOG_NAME			"/script.log"
#define DEFAULT_NET_LOG_NAME			"/network.log"
//#define DEFAULT_CONS_LOG_NAME			"/console.log"

#ifdef MAP_EDITOR
	#define DEFAULT_INIT_SCRIPT				"editor_init.lua"
#else
	#define DEFAULT_INIT_SCRIPT				"init.lua"
#endif //MAP_EDITOR

#define DEFAULT_FONT					"default"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define MAXIMUM_SOUND_DISTANCE			1024.0f

//////////////////////////////////////////////////////////////////////////
enum LogLevel
{
	logLevelInfo, logLevelWarning, logLevelError, logLevelScript, logLevelNone
};

#define LOG_ERROR_EV					logLevelError
#define LOG_WARNING_EV					logLevelWarning
#define LOG_INFO_EV						logLevelInfo
#define LOG_SCRIPT_EV					logLevelScript


//////////////////////////////////////////////////////////////////////////
enum ProgrammStates {
	PROGRAMM_LOADING,
	PROGRAMM_RUNNING,
	PROGRAMM_INACTIVE,
	//GAME_RUNNING,
	//GAME_GAMEMENU,
	//GAME_PAUSED,
	PROGRAMM_EXITING
};

//////////////////////////////////////////////////////////////////////////
#define GAMENAME "iiChantra"
#define VERSION "iiChantra kernel_engine"
#define ENGINE "v. 0.0.1"

//////////////////////////////////////////////////////////////////////////

//#define KEYDOWN(vk_code)((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)

//////////////////////////////////////////////////////////////////////////


#define COORD_LEFT_UP_CORNER	// При создании объектов координаты указывают
								// на левый верхний угол спрайта, а не на центр
								// физ. объекта. Сделано для совместимости с редактором.


//#define STOP_RENDER_ON_INACTIVE_NETGAME		// Останавливать рендер при неактивном окне
											// приложения при сетевой игре
#define RENDER_PARTICLES_WITH_STD_RENDER	// Использовать для частиц стандартный рендер, использующийся для всего отсального в игре


#define GAMETICK_FROM_CONFIG	// Загружать GAMETICK из файла CONFIG
#define USE_TIME_DEPENDENT_PHYS	// Использовать физику, зависящуюю от времени


#define DEBUG_LOG_INFO

#ifdef DEBUG_LOG_INFO
	//#define DEBUG_SAP
	//#define DEBUG_SAP_PAIRS_COORDS
#endif // DEBUG_LOG_INFO

//#define DEBUG_INSTANT_RENDERING		// Включает прямой рендеринг, не использующий массив отрисовок.

#define DEBUG_INGAME_INFO		// Вывод разнообразной отладочной информации и графики

#ifdef DEBUG_INGAME_INFO
	//#define DEBUG_PRINT				// Вывод отладочных письмен
	#define DEBUG_RENDER			// Отладочная графика

	#ifdef DEBUG_RENDER
		//#define DEBUG_RENDER_UNLOADED_TEXTURES		// Отрисовка незагруженых текстур в виде цветных прямоугольников
		#define DEBUG_UNLOADED_TEXTURES_COLOR 1.0f, 0.5f, 0.25f		// Цвет прямоугольника вместо незагруженной текстуры

		//#define DEBUG_DRAW_SPRITES_BORDERS		// Рисование границ спрайтов
		#define DEBUG_SPRITES_BORDERS_COLOR	0.0f, 0.5f, 0.25f, 1.0f
		//#define DEBUG_DRAW_OBJECTS_BORDERS		// Рисование границ объектов	(не работает)
		//#define DEBUG_OBJECTS_BORDERS_COLOR 0.0f, 0.5f, 0.5f, 1.0f

		#define DEBUG_DRAW_WAYPOINTS							// Отрисовка вейпоинтов
		#define DEBUG_WAYPOINT_COLOR 0.25f, 0.25f, 1.0f, 1.0f	// 

		#define DEBUG_DRAW_PHYSOBJ_BORDERS_ONLY	// Рисование гранифц только физических объектов
		#ifdef DEBUG_DRAW_PHYSOBJ_BORDERS_ONLY
			#define DEBUG_PHYSOBJ_BORDER_COLOR 1.0f, 0.5f, 0.0f, 1.0f
			#undef DEBUG_DRAW_OBJECTS_BORDERS
		#endif // DEBUG_DRAW_PHYSOBJ_BORDERS_ONLY

		//#define DEBUG_DRAW_MP0_POINT
		#ifdef DEBUG_DRAW_MP0_POINT
			#define DEBUG_MP0_POINT_COLOR 1.0f, 0.0f, 0.0f, 1.0f
		#endif // DEBUG_DRAW_PHYSOBJ_BORDERS_ONLY

		//#define DEBUG_DRAW_COLLISION_PAIRS					// Отрисовка пар пересечений
		#ifdef DEBUG_DRAW_COLLISION_PAIRS
			#define DEBUG_COLLISION_PAIRS_COLOR 1.0f, 1.0f, 0.0f, 1.0f
		#endif // DEBUG_DRAW_PHYSOBJ_BORDERS_ONLY

		#define DEBUG_DRAW_SUSPECTED_PLAIN					// Отрисовка связи с suspected_plane
		#ifdef DEBUG_DRAW_SUSPECTED_PLAIN
			#define DEBUG_DRAW_SUSPECTED_PLAIN_COLOR 0.7f, 0.7f, 0.7f, 1.0f
		#endif // DEBUG_DRAW_PHYSOBJ_BORDERS_ONLY

		#define DEBUG_DRAW_SPEED_VECTORS				// Отрисовка пар пересечений
		#ifdef DEBUG_DRAW_SPEED_VECTORS
			#define DEBUG_DRAW_SPEED_VEC_COLOR 1.0f, 1.0f, 1.0f, 1.0f
			#define DEBUG_DRAW_GRAV_BONUS_COLOR 1.0f, 1.0f, 0.0f, 1.0f
		#endif // DEBUG_DRAW_PHYSOBJ_BORDERS_ONLY

		#define DEBUG_DRAW_PHYSOBJ_ID							// Рисует на физических объектах их id объекта
		#ifdef DEBUG_DRAW_PHYSOBJ_ID
			#define DEBUG_PHYSOBJ_ID_FONT_COLOR 0.0f, 0.0f, 0.0f, 1.0f
		#endif // DEBUG_DRAW_PHYSOBJ_BORDERS_ONLY

	#endif // DEBUG_RENDER
#endif // DEBUG_INGAME_INFO


//#define BMP_SCREENSHOTS

#define MIN_X_BOUNCE_SPEED 0.0f
#define MIN_Y_BOUNCE_SPEED 4.0f


#define GUI_SETFOCUS_ON_MOUSEMOVE		// Устанавливать фокус на виджете, когда над ним движется мышка
//#define GUI_SETFOCUS_ON_MOUSECLICK		// Устанавливать фокус на виджете, когда по нем щелкают <- Но зачем? Мышь уже над ним...
#define GUI_UNSETFOCUS_ON_MOUSELEAVE	// Убирать фокус с виджета, когда с него убирают курсор мышки

#define  CLOSE_ON_ALT_F4				// Приложение будет закрыто по нажатию alt+F4

#define BOUNCE_Y(dyn) dyn->vel.y = (fabs(dyn->vel.y) <= MIN_Y_BOUNCE_SPEED) ? 0.0f : -(dyn->bounce+dyn->env->bounce_bonus)*dyn->vel.y; dyn->acc.y = (dyn->acc.y <= MIN_Y_BOUNCE_SPEED) ? 0.0f : -(dyn->bounce+dyn->env->bounce_bonus)*dyn->acc.y
#define BOUNCE_X(dyn) dyn->vel.x = (fabs(dyn->vel.x) <= MIN_X_BOUNCE_SPEED) ? 0.0f : -(dyn->bounce+dyn->env->bounce_bonus)*dyn->vel.x; dyn->acc.x = (dyn->acc.x <= MIN_X_BOUNCE_SPEED) ? 0.0f : -(dyn->bounce+dyn->env->bounce_bonus)*dyn->acc.x

// Для win32 SDL_win32_main.cpp
#define NO_STDIO_REDIRECT		// Не перенаправляет стандартные потоки в файл
#define USE_MESSAGEBOX			// Использует MessageBox для сообщения ошибок инициализации

#define SHADOW_DISTANCE			480
#define SHADOW_Z				-0.3f
#define SHADOW_COLOR			0, 0, 0, 0.4f

// Количество массивов кнопок в конфигурации
#define KEY_SETS_NUMBER 4
// Максимальная длина названия кнопки. Используется для выделения строки-буфера, 
// можно спокойно увеличить, когда будут более длинные зназвания.
#define MAX_VKEY_NAME_LEN 30


//#define MAP_EDITOR
#define NO_WRITE_SCREEN_CFG_ON_RESIZE_EDITOR

//#define NET_DEBUG
#define NET_TIMEOUT 1000
#define NET_MAX_PLAYERS 15 //+1 server
#define NET_DEFAULT_PORT 2800

#endif // __DEFINES_H_

#include "StdAfx.h"

#include <SDL/SDL.h>
#include <IL/il.h>

#include "config.h"
#include "scene.h"

#include "main.h"
#include "misc.h"

#include "script/script.h"
#include "script/timerevent.h"

#include "game/game.h"
#include "game/camera.h"
#include "game/editor.h"

#include "render/font.h"
#include "render/draw.h"
#include "render/texture.h"
#include "render/image.h"
#include "render/renderer.h"

#include "gui/gui.h"

#include "sound/snd.h"

#include "input_mgr.h"
#include "resource_mgr.h"

#include "game/net.h"


//////////////////////////////////////////////////////////////////////////

#define FPSTICK 1000

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Подсчет времени и FPS
UINT fps = 0;			// Количество отрисованых в прошлую секунду кадров
UINT frames = 0;		// Количество отрисованых с начала этой секунды кадров
UINT startup_time = 0;	// Начальное время (прмерно время запуска приложения)
UINT last_time = 0;
UINT current_time = 0;	// Текущее время

UINT internal_time = 0;	// Внутреннее время движка. Считается только когда окно активно.

UINT last_fps_tick = 0;		// Время предыдущего тика фпс
UINT last_timerevent_tick = 0;	// Время предыдущего тика таймера событий
UINT last_game_tick = 0;		// Время предыдущего тика игры
//UINT last_keyb_tick = 0;		// Время предыдущей реакции на зажатую клавишу
//UINT last_gui_tick = 0;		// Время предыдущего тика обработки gui
///

InputMgr inpmgr;

ProgrammStates current_state = PROGRAMM_LOADING;				// Текущее состояние игры

config cfg;

Gui* gui;

extern SoundMgr * soundMgr;

extern game::GameStates game_state;

bool netgame = false;	// Идет сетевая игра
extern char path_screenshots[MAX_PATH];

extern ResourceMgr<Texture> * textureMgr;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Подсчет FPS
__INLINE void CountFPS()
{
	if(internal_time - last_fps_tick >= FPSTICK)
	{
		fps = frames;
		frames = 0;
		last_fps_tick = internal_time;
	}
}

// Вывод FPS на экран
// TODO: Когда появится интерфейс, перевети вывод текста на него.
__INLINE void PrintFPS()
{
	extern float CAMERA_OFF_X;
	extern float CAMERA_OFF_Y;

	static Font* font = FontByName(DEFAULT_FONT);
	if (!font)
	{
		return;
	}

	font->p = Vector2(5.0f - CAMERA_OFF_X, 0.0f - CAMERA_OFF_Y);
	font->z = 1.0f;
	font->tClr = RGBAf(0.0f, 1.0f, 0.0f, 1.0f);
	font->Print("FPS: %u\0", fps);
}

#ifdef DEBUG_PRINT

float DBG_PRINT_VAL1 = 0;
float DBG_PRINT_VAL2 = 0;
string DBG_PRINT_VAL3;


//float DBG_PRINT_VAL4 = 0;
//float DBG_PRINT_VAL5 = 0;
//float DBG_PRINT_VAL6 = 0;
//float DBG_PRINT_VAL7 = 0;
//float DBG_PRINT_VAL8 = 0;
//float DBG_PRINT_VAL9 = 0;

__INLINE void DbgPrint()
{
	static Font* font = FontByName(DEFAULT_FONT);
	if (!font)
	{
		return;
	}
	font->tClr = RGBAf(0.0f, 1.0f, 0.0f, 1.0f);
	font->z = 1.0f;

	extern float CAMERA_OFF_X;
	extern float CAMERA_OFF_Y;
	extern float CAMERA_X;
	extern float CAMERA_Y;

	font->p = Vector2(5.0f - CAMERA_OFF_X, 10.0f - CAMERA_OFF_Y);
	font->Print("CAMX: %f\0", CAMERA_X);

	font->p = Vector2(5.0f - CAMERA_OFF_X, 20.0f - CAMERA_OFF_Y);
	font->Print("CAMY: %f\0", CAMERA_Y);

	//font->p = Vector2(5.0f - CAMERA_OFF_X, 30.0f - CAMERA_OFF_Y);
	//font->Print("Rendered: %f\0", DBG_PRINT_VAL1);
	DBG_PRINT_VAL1 = 0;

	//extern UINT dbgPairsCount;
	//font->p = Vector2(5.0f - CAMERA_OFF_X, 40.0f - CAMERA_OFF_Y);
	//font->Print("NbPairs: %d\0", dbgPairsCount);

	//font->p = Vector2(5.0f - CAMERA_OFF_X, 40.0f - CAMERA_OFF_Y);
	//font->Print("events: %f\0", DBG_PRINT_VAL6);

	//font->p = Vector2(5.0f - CAMERA_OFF_X, 50.0f - CAMERA_OFF_Y);
	//font->Print("max ev: %f\0", DBG_PRINT_VAL7);

	//sprintf(frames_buf, "VAL7: %f\0", DBG_PRINT_VAL7);
	//DrawText(frames_buf, strlen(frames_buf), font, 5.0f - CAMERA_OFF_X, 90.0f - CAMERA_OFF_Y);

	//sprintf(frames_buf, "VAL8: %f\0", DBG_PRINT_VAL8);
	//DrawText(frames_buf, strlen(frames_buf), font, 5.0f - CAMERA_OFF_X, 100.0f - CAMERA_OFF_Y);

	//sprintf(frames_buf, "VAL9: %f\0", DBG_PRINT_VAL9);
	//DrawText(frames_buf, strlen(frames_buf), font, 5.0f - CAMERA_OFF_X, 110.0f - CAMERA_OFF_Y);

}

#endif // DEBUG_PRINT

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Игровой цикл

inline void UpdateInternalTimer()
{
	current_time = (SDL_GetTicks() - startup_time);
	internal_time += current_time - last_time;
	last_time = current_time;
}

// Обработа событий
bool ProcessEvents()
{
	SDL_Event event;
	while(SDL_PollEvent(&event)) // TODO: Look here!!!!!! http://osdl.sourceforge.net/main/documentation/rendering/SDL-inputs.html
	{

		switch(event.type)
		{
		case SDL_KEYDOWN:
			{
				SDL_keysym keysym = event.key.keysym;
#ifdef CLOSE_ON_ALT_F4
				// Реакция на alt+F4. Возможно стоит проверять, что другие клавиши-модификаторы
				// не нажаты, но сама Windows вроде этого не деалет.
				if (keysym.sym == SDLK_F4 && (keysym.mod & KMOD_ALT))
				{
					current_state = PROGRAMM_EXITING;
					return false;
					break;
				}
#endif // CLOSE_ON_ALT_F4

				inpmgr.AddEvent((SDL_KeyboardEvent&)event);
				break;
			}
		case SDL_KEYUP:
			{
				inpmgr.AddEvent((SDL_KeyboardEvent&)event);

// 				char TempChar = TranslateKeyFromUnicodeToChar(event);
// 				SDL_keysym keysym = event.key.keysym;
// 				for(int i=0;i<KeyInputFuncCount;i++)
// 					(KeyFuncCallers[i]->*KeyInputFunctions[i])(KEY_RELEASED, keysym.sym, keysym.mod, TempChar);
// 				keys[keysym.sym] = 0;
				break;
			}
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			{
				inpmgr.AddEvent((SDL_MouseButtonEvent&)event);
				break;
			}
		case SDL_JOYAXISMOTION:
			{
				inpmgr.AddEvent((SDL_JoyAxisEvent&)event);
				break;
			}
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			{
				inpmgr.AddEvent((SDL_JoyButtonEvent&)event);
				break;
			}
		case SDL_JOYHATMOTION:
			{
				inpmgr.AddEvent((SDL_JoyHatEvent&)event);
				break;
			}
		case SDL_MOUSEMOTION:
			{
				inpmgr.AddEvent((SDL_MouseMotionEvent&)event);
				break;
			}
		case SDL_ACTIVEEVENT:
			{
				if (current_state != PROGRAMM_EXITING)
				{
					if (event.active.state != SDL_APPMOUSEFOCUS)
					{
						if (event.active.gain == 0)
							current_state = PROGRAMM_INACTIVE;
						else
							current_state = PROGRAMM_RUNNING;
						// 					current_state = PROGRAMM_RUNNING;
						// 		case WA_INACTIVE: current_state = PROGRAMM_INACTIVE; break;

					}
				}
				break;
			}
		// SDL_VIDEORESIZE вознинет при задании флага SDL_RESIZABLE.
		case SDL_VIDEORESIZE:
			{
#ifdef MAP_EDITOR
				cfg.scr_width = (UINT) (cfg.scr_width * event.resize.w / cfg.window_width);
				cfg.scr_height = (UINT) (cfg.scr_height * event.resize.h / cfg.window_height);
				cfg.window_width = event.resize.w;
				cfg.window_height = event.resize.h;
				//scene::ResizeScene(event.resize.w, event.resize.h);
				scene::ApplyConfig(true, false);
#ifndef NO_WRITE_SCREEN_CFG_ON_RESIZE_EDITOR
				SaveConfig();
#endif // NO_WRITE_SCREEN_CFG_ON_RESIZE_EDITOR
#endif // MAP_EDITOR
				break;
			}
		case SDL_QUIT:
			{
				current_state = PROGRAMM_EXITING;
				return false;
				break;
			}
		default:
			{
				break;
			}
		}
	}
	return true;
}

// Главный цикл
void scene::MainLoop()
{
	bool inactive = false;

	while(current_state != PROGRAMM_EXITING)
	{
		if (current_state == PROGRAMM_RUNNING)
			inpmgr.PreProcess();


		if (!ProcessEvents())
			break;

		if (current_state == PROGRAMM_INACTIVE && !netgame)
		{
			// Приложение неактивно
			inactive = true;
			if (soundMgr)
				soundMgr->PauseAll();

			UINT t = SDL_GetTicks();

			SDL_WaitEvent(NULL);

			// Определяем время, которое провели в неактивном состоянии
			t = SDL_GetTicks() - t;
			// Так все таймеры это время неактивности не заметят.
			last_time += t;
		}
		else
		{
			if (inactive)
			{
				// Восстановление после состояния неактивности
				// К сожалению, нельзя просто оставить этот код после WaitMessage,
				// т.к. даже неактивное приложение периодически получает собщения.
				// Тогда звук, например, начинает "квакать" даже при неактивном приложении.
				inactive = false;
				if (soundMgr)
					soundMgr->ResumeAll();
			}
#ifdef STOP_RENDER_ON_INACTIVE_NETGAME
			if (current_state != PROGRAMM_INACTIVE)
#endif // STOP_RENDER_ON_INACTIVE_NETGAME
			{
				scene::ClearScene();
			}

			scene::UpdateScene();

#ifdef STOP_RENDER_ON_INACTIVE_NETGAME
			if (current_state != PROGRAMM_INACTIVE)
#endif // STOP_RENDER_ON_INACTIVE_NETGAME
			{
				CountFPS();
				scene::DrawScene();
				scene::Flip();

				if (inpmgr.IsPressed(cakScreenshot))
				{
					SaveScreenshot();
				}
			}
		}

	}
	inactive = true;
}

// Обновление сцены, обработка все логики и т.п.
void scene::UpdateScene()
{
	UpdateInternalTimer();


	if (current_state == PROGRAMM_RUNNING)
		inpmgr.Process();


	if (internal_time - last_timerevent_tick >= TIMEREVENTTICK)
	{
		ProcessTimerEvents();
		last_timerevent_tick = internal_time;
	}

	if (internal_time - last_game_tick >= GAMETICK)
	{
		game::UpdateGame(internal_time - last_game_tick);
		last_game_tick = internal_time;
	}

	ProcessEditor();


	if (gui/* && internal_time - last_gui_tick >= GUITICK*/)
	{
		gui->Process();
		//last_gui_tick = internal_time;
	}


}

// Очистка сцены
void scene::ClearScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	r_ZeroRenderData();

#ifdef _DEBUG
	//RENDERED_TEXTURES = 0;
#endif
}

// Отрисовка всего
void scene::DrawScene()
{
	/*glColor4ub(255 - GLOBAL_COLOR_RED,
		255 - GLOBAL_COLOR_GREEN,
		255 - GLOBAL_COLOR_BLUE,
		255 - GLOBAL_COLOR_ALPHA);*/
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	game::DrawGame();

	if (gui)
	{
		gui->Draw();
	}

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	if(cfg.show_fps)
	{
		PrintFPS();
	}

	r_RenderAll();

#ifdef DEBUG_PRINT
	DbgPrint();

#endif // DEBUG_PRINT

	frames++;
}

// Флип :) (блин, мне нравится, как это слово звучит)
void scene::Flip()
{
	glFlush();
	SDL_GL_SwapBuffers();
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Инициализация
void scene::InitScene()
{
	//startup_time = GetTickCount();
	InitSDL();
	if ( !Net::Init() )
	{
		current_state = PROGRAMM_EXITING;
		return;
	}
	startup_time = SDL_GetTicks();

	if (!InitWindow(false))
	{
		current_state = PROGRAMM_EXITING;
		return;
	}
	ilInit();

	gui = new Gui();

	InitResourceManagers();

	// Создание скриптовой подсистемы и исполнение скрипта инициализаии.
	if(SCRIPT::InitScript() || SCRIPT::ExecFile(DEFAULT_INIT_SCRIPT))
	{
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "No init: aborting game");
		current_state = PROGRAMM_EXITING;
		return;
	}
	else
	{
		current_state = PROGRAMM_RUNNING;
	}

	//TestLoading();

	if (game_state != game::GAME_RUNNNIG)
	{
		CameraMoveToPos(cfg.scr_width * 0.5f, cfg.scr_height * 0.5f);
		CameraUpdatePosition();
	}


}

// Применяет конфиг и вызывает изменение окна
void scene::ApplyConfig(bool video_mode_changed, bool backcolor_changed)
{
	if (video_mode_changed || current_state == PROGRAMM_LOADING)
	{
		// Изменился видеорежим или это первая загрузка настроек.
		// Заодно там будет выставлен backcolor, дополнительно выставлять не надо.
		scene::ResizeScene();
		inpmgr.SetScreenToWindowCoeff((float)cfg.scr_width / (float)cfg.window_width,
			(float)cfg.scr_height / (float)cfg.window_height);
	}
	else if (backcolor_changed)
	{
		glClearColor(cfg.backcolor_r, cfg.backcolor_g, cfg.backcolor_b, 1.0f);
	}

	if (soundMgr)
		soundMgr->SetVolume(cfg.volume);

	if (gui)
	{
		gui->nav_mode = (GuiNavigationMode)cfg.gui_nav_mode;
		gui->nav_cycled = cfg.gui_nav_cycled != 0;
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Деинициализации

// Освобождает все подсистемы и сцену
void scene::FreeScene()
{
	DeleteAllEvents();
	game::FreeGame(true);

	DELETESINGLE(gui);

	r_ClearRenderData();
	FreeFonts();

	DestroyResourceManagers();

	SCRIPT::FreeScript();
	scene::KillWindow();

	SDL_Quit();
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Окно игры

// Настройки OpenGL
static void InitOGLSettings()
{
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearDepth( 1.0f );              // Разрешить очистку буфера глубины
	glEnable( GL_DEPTH_TEST );            // Разрешить тест глубины
	glDepthFunc( GL_LEQUAL );            // Тип теста глубины

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.1f);
	glReadBuffer(GL_BACK);
	glClearColor(cfg.backcolor_r, cfg.backcolor_g, cfg.backcolor_b, 1.0f);
	glEnable(GL_BLEND);
}

// Инициализация SDL
bool scene::InitSDL()
{
	if( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK) < 0 )
	{
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Video initialization failed: %s\n", SDL_GetError());
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Last WARNING was critical. Now exiting...");
		return false;
	}

	if ( SDL_NumJoysticks() > 0 )
	{
		inpmgr.joystick = SDL_JoystickOpen( 0 );
		sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Joystick or gamepad found, opening...");
		if ( inpmgr.joystick == NULL )
		{
			sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Failed to open joystick or pad!");
		}
	}

	SDL_EnableUNICODE(1);
	return true;
}

SDL_Surface* screen;

// Создание окна
bool scene::InitWindow(bool fullscreen)
{
	SDL_WM_SetCaption(GAMENAME, NULL);
	const SDL_VideoInfo* info = NULL;

	info = SDL_GetVideoInfo();

	if (info == NULL)
	{
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Aaaah! SDL WARNING: %s", SDL_GetError());
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Last ERROR was critical. Now exiting...");
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE,		8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,		8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,		8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,		16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,	1);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,		8);
	
	int flags = SDL_OPENGL;
	if(fullscreen == true)
	{
		flags |= SDL_FULLSCREEN;
	}
#ifdef MAP_EDITOR
	flags |= SDL_RESIZABLE;
#endif

	int width = cfg.window_width > 0 ? cfg.window_width : DEFAULT_WINDOW_WIDTH;
	int height = cfg.window_height > 0 ? cfg.window_height : DEFAULT_WINDOW_HEIGHT;

	// Возможно, здесь стоит уничтожать страый сурфейс руками. Хотя вроде и так не плохо.
	screen = SDL_SetVideoMode(width, height, cfg.scr_bpp, flags);
	if (screen == NULL)
	{
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Setting video mode failed: %s\n", SDL_GetError());
		//sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Last error was critical. Now exiting...");
		return false;
	}

// 	SDL_Event resizeEvent;
// 	resizeEvent.type=SDL_VIDEORESIZE;
// 	resizeEvent.resize.w = DEFAULT_WINDOW_WIDTH;
// 	resizeEvent.resize.h = DEFAULT_WINDOW_HEIGHT;
// 	SDL_PushEvent(&resizeEvent);

	InitOGLSettings();

	return true;
}

// Уничтожает окно
void scene::KillWindow()
{
// 	if(fullscreenflag)
// 	{
// 		ChangeDisplaySettings(NULL, 0);
// 		ShowCursor(TRUE);
// 	}
//
// 	if(hRC)
// 	{
// 		if(!wglMakeCurrent(NULL, NULL))
// 		{
// 			sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Failed to release current context");
// 			MessageBox(NULL, "Failed to release current context", "wglMakeCurrent() Error", MB_OK | MB_ICONEXCLAMATION);
// 		}
//
// 		if(!wglDeleteContext(hRC))
// 		{
// 			sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Failed to delete rendering context");
// 			MessageBox(NULL, "Failed to delete rendering context", "wglDeleteContext() Error", MB_OK | MB_ICONEXCLAMATION);
// 		}
//
// 		hRC = NULL;
// 	}
//
// 	if(hDC && ReleaseDC(game_window, hDC) != 1)
// 	{
// 		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Failed to release device context");
// 		MessageBox(NULL, "Failed to release device context", "ReleaseDC() Error", MB_OK | MB_ICONEXCLAMATION);
// 		hDC = NULL;
// 	}
//
// 	if(game_window && DestroyWindow(game_window) == 0)
// 	{
// 		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Failed to destroy game window");
// 		MessageBox(NULL, "Failed to destroy game window", "DestroyWindow() Error", MB_OK | MB_ICONEXCLAMATION);
// 		game_window = NULL;
// 	}
//
// 	if(!UnregisterClass(window_class.lpszClassName, hInstance))
// 	{
// 		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Failed to unregister game window class");
// 		MessageBox(NULL, "Failed to unregister game window class", "UnregisterClass() Error", MB_OK | MB_ICONEXCLAMATION);
// 	}
}



// Изменяет размер окна. Вызывает настройку OpenGL.
void scene::ResizeScene()
{
	if (!scene::InitWindow(cfg.fullscreen != 0))
	{
		current_state = PROGRAMM_EXITING;
		return;
	}
	//InitOGLSettings();		// уже вызвано в InitWindow
	glViewport(0, 0, cfg.window_width, cfg.window_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, cfg.scr_width, cfg.scr_height, 0.0, cfg.near_z, cfg.far_z);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	textureMgr->RecoverAll();
	RecoverFonts();

	//if(!cfg.fullscreen)
	//{
	//	scene::InitWindow(FALSE);
	//	//InitOGLSettings();
	//	glViewport(0, 0, width, height);
	//	glMatrixMode(GL_PROJECTION);
	//	glLoadIdentity();
	//	glOrtho(0.0, width, height, 0.0, cfg.near_z, cfg.far_z);
	//	glMatrixMode(GL_MODELVIEW);
	//	glLoadIdentity();
	//}
	//else
	//{
	//	scene::KillWindow();
	//	scene::InitWindow(TRUE);
	//	//InitOGLSettings();

	//	glViewport(0, 0,
	//		cfg.window_width,
	//		cfg.window_height);

	//	glMatrixMode(GL_PROJECTION);
	//	glLoadIdentity();

	//	gluOrtho2D(0.0,
	//		cfg.window_width,
	//		cfg.window_height,
	//		0.0);

	//	glMatrixMode(GL_MODELVIEW);
	//	glLoadIdentity();
	//}
	//fullscreenflag = cfg.fullscreen;
}

//////////////////////////////////////////////////////////////////////////



bool scene::SaveScreenshot()
{
	char file_name[MAX_PATH];
	static UINT counter = 0;

	// Долго же оно будет в первый раз искать номер следующего скриншота, если их уже много
	do
	{
	counter++;
#ifdef BMP_SCREENSHOTS
	sprintf(file_name, "%sscreenshot%05d.bmp", path_screenshots, counter);
#else
	sprintf(file_name, "%sscreenshot%05d.png", path_screenshots, counter);
#endif // BMP_SCREENSHOTS
	}
	while(FileExists(file_name));

	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Saving screenshot %s", file_name);

	CGLImageData ImageData;
	return ImageData.SaveScreenshot(file_name);
}

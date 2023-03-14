#include "StdAfx.h"


#include "input_mgr.h"
#include "config.h"

//////////////////////////////////////////////////////////////////////////

extern UINT internal_time;

//////////////////////////////////////////////////////////////////////////



// Получает на вход символ unicode и пытается вернуть символ кириллицы из CP1251.
__INLINE Uint16 GetCyrillicCP1251(Uint16 symb)
{
	if (0x0410 <= symb && symb <= 0x044f)
		return symb - 0x350;	// Основной алфавит просто тупо сдвинут
	else if (symb == 0x0401)
		return 0x00a8;			// Ё
	else if (symb == 0x0451)
		return 0x00b8;			// ё
	else
		return (Uint16)'?';		// Все остальное заменим вопросительными знаками
}



InputMgr::InputMgr()
{
	memset(pressed, false, keys_count);
	memset(holded, false, keys_count);
	memset(released, false, keys_count);

	mouseX = 0;
	mouseY = 0;
	mouseX_last = 0;
	mouseY_last = 0;
	mouse_moved = false;
	memset(mouse, false, mousebtns_count);

	repeatingState = rs_none;
	repeat_timer = 0;
	repeatTime = 30;
	repeatWaitingTime = 500;
	event_to_repeat = NULL;
}

InputMgr::~InputMgr()
{
	FlushEvents();
	if (event_to_repeat) //Могли не отпустить кнопку.
		DELETESINGLE(event_to_repeat);
}

void InputMgr::PreProcess()
{
	ArrayFlush();
	
	mouse_moved = false;

	FlushEvents();
}



void InputMgr::Process()
{
	if ((mouseX != mouseX_last) || (mouseY != mouseY_last))
	{
		// Это необходимо скорее для отладки. Сообщение MOUSEMOVED передается окну,
		// когда то получает фокус.
		mouseX_last = mouseX;
		mouseY_last = mouseY;
		mouse_moved = true;
	}

	if (repeatingState == rs_waiting && internal_time - repeat_timer >= repeatWaitingTime)
	{
		// Дождались, можно повторять
		repeat_timer = internal_time - repeatTime;
		repeatingState = rs_repeating;
	}
	if (repeatingState == rs_repeating && internal_time - repeat_timer >= repeatTime)
	{
		// Дождались очередного повторения
		ASSERT(event_to_repeat);
		repeat_timer = internal_time;
		InputEvent* new_ev = new InputEvent(*event_to_repeat);
		repeated[event_to_repeat->kb.key] = true;
		events.push_back(new_ev);
	}
	
}

void InputMgr::AddEvent(SDL_KeyboardEvent &ev)
{
	InputEvent* ie = new InputEvent;
	ie->type = InputKBoard;

	ie->kb.key = ev.keysym.sym;

	if ( ev.keysym.unicode & 0xFF80 )
	{
		// Символ, не входящий в нижнюю половину ASCII. Попробуем превратить в кириллицу CP1251.
		ie->kb.symbol = GetCyrillicCP1251(ev.keysym.unicode);
	}
	else
	{
		ie->kb.symbol = ev.keysym.unicode;
	}


	if (ev.type == SDL_KEYDOWN)
	{
		ie->state = InputStatePressed;
		pressed[ev.keysym.sym] = true;
		holded[ev.keysym.sym] = true;
		released[ev.keysym.sym] = false;

		//prolonged_pressed[ev.keysym.sym] = true;

		if (!event_to_repeat || (event_to_repeat && event_to_repeat->kb.key != ie->kb.key))
		{
			// Запоминаем событие для повторения
			DELETESINGLE(event_to_repeat);
			event_to_repeat = new InputEvent(*ie);
			repeatingState = rs_waiting;
			repeat_timer = internal_time;
		}
	}
	else
	{
		ie->state = InputStateReleased;
		pressed[ev.keysym.sym] = false;
		holded[ev.keysym.sym] = false; 
		released[ev.keysym.sym] = true;

		prolonged_released[ev.keysym.sym] = true;

		if (event_to_repeat && event_to_repeat->kb.key == ie->kb.key)
		{
			// Отпустили клавишу, которую повторяем. Больше повторять не надо
			repeatingState = rs_none;
			DELETESINGLE(event_to_repeat);
		}
	}

	events.push_back(ie);
}

void InputMgr::AddEvent(SDL_MouseButtonEvent &ev)
{
	InputEvent* ie = new InputEvent;
	ie->type = InputMouse;

	bool st = (ev.type == SDL_MOUSEBUTTONDOWN ? true : false);
	switch (ev.button)
	{
	case SDL_BUTTON_LEFT:
		mouse[MOUSE_BTN_LEFT] = st;
		ie->mouse.key = MOUSE_BTN_LEFT;
		break;
	case SDL_BUTTON_RIGHT:
		mouse[MOUSE_BTN_RIGHT] = st;
		ie->mouse.key = MOUSE_BTN_RIGHT;
		break;
	case SDL_BUTTON_MIDDLE:
		mouse[MOUSE_BTN_MIDDLE] = st;
		ie->mouse.key = MOUSE_BTN_MIDDLE;
		break;
	}

	ie->state = st ? InputStatePressed : InputStateReleased;
	events.push_back(ie);
}

extern config cfg;

void InputMgr::AddEvent(SDL_JoyAxisEvent &ev)
{
	if ( ev.axis > 2 )
	{
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Joysticks with more than 2 axes are not fully supported, sorry.");
		return;
	}
	if (!joystick_axis_pressed[ev.axis])
	{
		if ( (UINT)abs(ev.value) < cfg.joystick_sensivity  ) return;
		joystick_axis_pressed[ev.axis] = true;
		InputEvent* ie = new InputEvent;
		ie->type = InputKBoard;
		
		int index = SDLK_LAST + 2 * ev.axis + 1;

		if (ev.value < 0 )
			index += 1;
				
		ie->state = InputStatePressed;
		pressed[index] = true;
		holded[index] = true;
		released[index] = false;

		//prolonged_pressed[index] = true;

		events.push_back(ie);
	}
	else
	{
		if ( (UINT)abs(ev.value) > cfg.joystick_sensivity ) return;
		joystick_axis_pressed[ev.axis] = false;
		InputEvent* ie = new InputEvent;
		ie->type = InputKBoard;
		
		int index = SDLK_LAST + 2 * ev.axis + 1;

		ie->state = InputStateReleased;
		pressed[index] = false;
		holded[index] = false;
		released[index] = true;
		prolonged_released[index] = true;

		events.push_back(ie);

		index++;

		ie = new InputEvent;

		ie->state = InputStateReleased;
		pressed[index] = false;
		holded[index] = false;
		released[index] = true;
		prolonged_released[index] = true;
	}
}

void InputMgr::AddEvent(SDL_JoyHatEvent &ev)
{
	bool joy_hat_new[4];
	for ( int i = 0; i < 4; i++ ) joy_hat_new[i] = false;
	switch( ev.value )
	{
		case SDL_HAT_LEFT: joy_hat_new[2] = true; break;
		case SDL_HAT_UP: joy_hat_new[0] = true; break;
		case SDL_HAT_DOWN: joy_hat_new[1] = true; break;
		case SDL_HAT_RIGHT: joy_hat_new[3] = true; break;
		case SDL_HAT_LEFTUP: joy_hat_new[2] = joy_hat_new[0] = true; break;
		case SDL_HAT_LEFTDOWN: joy_hat_new[2] = joy_hat_new[1] = true; break;
		case SDL_HAT_RIGHTUP: joy_hat_new[3] = joy_hat_new[0] = true; break;
		case SDL_HAT_RIGHTDOWN: joy_hat_new[3] = joy_hat_new[1] = true; break;
	}
	for ( int i = 0; i < 4; i++ )
	{
		pressed[SDLK_LAST+20+i] = joy_hat_new[i];
		holded[SDLK_LAST+20+i] = joy_hat_new[i];
		released[SDLK_LAST+20+i] = !joy_hat_new[i] & joy_hat[i];
	}
	memcpy(&joy_hat, &joy_hat_new, sizeof(joy_hat_new));
}

void InputMgr::AddEvent(SDL_JoyButtonEvent &ev)
{
	if ( ev.button > 12 )
	{
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Joysticks with more than 13 buttons are not fully supported, sorry.");
		return;
	}
	InputEvent* ie = new InputEvent;
	ie->type = InputKBoard;

	int index = SDLK_LAST + 7 + ev.button;
	if (ev.type == SDL_JOYBUTTONDOWN)
	{
		ie->state = InputStatePressed;
		pressed[index] = true;
		holded[index] = true;
		released[index] = false;

		//prolonged_pressed[index] = true;
	}
	else
	{
		ie->state = InputStateReleased;
		pressed[index] = false;
		holded[index] = false;
		released[index] = true;

		prolonged_released[index] = true;
	}

	events.push_back(ie);
}

void InputMgr::AddEvent(SDL_MouseMotionEvent &ev)
{
	mouseX = (UINT)(ev.x * scrwinX);
	mouseY = (UINT)(ev.y * scrwinY);
}

void InputMgr::SetScreenToWindowCoeff(float kx, float ky)
{
	mouseX = (UINT)((mouseX / scrwinX) * kx);
	mouseY = (UINT)((mouseY / scrwinY) * ky);

	scrwinX = kx;
	scrwinY = ky;
}

// Очистка списка сохраненных событий ввода
void InputMgr::FlushEvents()
{
	if (events.empty())
		return;
	else
	{
		EventsIterator it;
		for (it = events.begin(); it != events.end(); it++)
		{
			DELETESINGLE((*it));
		}
		events.clear();
	}
}

void InputMgr::ArrayFlush()
{
	//memcpy(holded, pressed, keys_count * sizeof(pressed[0]));
	memset(pressed, false, keys_count);
	memset(released, false, keys_count);
	memset(repeated, false, keys_count);
}

void InputMgr::ProlongedArrayFlush()
{
	//memset(prolonged_pressed, false, keys_count);
	memset(prolonged_released, false, keys_count);
}

bool InputMgr::CheckKeyInArray(const bool* arr, ConfigActionKeys key) const
{
	for (size_t i = 0; i < KEY_SETS_NUMBER; i++)
	{
		if (arr[ cfg.cfg_keys[i][key] ])
			return true;
	}
	return false;
}

bool InputMgr::IsPressed(ConfigActionKeys key) const
{
	return CheckKeyInArray(pressed, key);
}
bool InputMgr::IsReleased(ConfigActionKeys key) const
{
	return CheckKeyInArray(released, key);
}

bool InputMgr::IsHolded(ConfigActionKeys key) const
{
	return CheckKeyInArray(holded, key);
}

bool InputMgr::IsRepeated(ConfigActionKeys key) const
{
	return CheckKeyInArray(repeated, key);
}

bool InputMgr::IsProlongedReleased(ConfigActionKeys key) const
{
	return CheckKeyInArray(prolonged_released, key);
}


bool InputMgr::IsHolded(UINT key) const
{
	return holded[key];
}
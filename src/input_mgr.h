#ifndef __KEYB_MGR_H_
#define __KEYB_MGR_H_

#include "config.h"

enum InputEventType { InputKBoard, InputMouse };
enum InputEventState { InputStatePressed, InputStateReleased };

struct InputKBEvent
{
	SDLKey key;
	Uint16 symbol;
};

struct InputMouseEvent
{
	int key;
};

// ����������� ������� �����
struct InputEvent
{
	InputEventType type;
	InputEventState state;
	union 
	{
		InputKBEvent kb;
		InputMouseEvent mouse;
	};
};

class InputMgr
{
public:
	enum MosueButtons {MOUSE_BTN_LEFT, MOUSE_BTN_RIGHT, MOUSE_BTN_MIDDLE, MOUSE_BTN_LAST};

	SDL_Joystick* joystick;
	bool joystick_axis_pressed[3];

	static const UINT keys_count = SDLK_LAST + 24; //24 more for joystick input
	static const UINT mousebtns_count = MOUSE_BTN_LAST;


	bool joy_hat[4];



	typedef list<InputEvent*> EventsContainer;
	typedef EventsContainer::iterator EventsIterator;

	EventsContainer events;			// ������ ����������� ������� �����

	// ���������� �������
	UINT mouseX;
	UINT mouseY;
	UINT mouseX_last;
	UINT mouseY_last;
	bool mouse_moved;					// ���������� ������� ����������
	bool mouse[mousebtns_count];		// ��������� ������ ����
	
	// ����������� �������� ������ � ���� (��������� ����������) ��� ��������� ��������� ������� �����
	float scrwinX;
	float scrwinY;

	UINT repeatWaitingTime;				// ����� ��������, ������ ��� �������� ����������
	UINT repeatTime;					// ����� ����� ������������


	InputMgr();
	~InputMgr();

	void PreProcess();
	void Process();

	void AddEvent(SDL_JoyAxisEvent &ev);
	void AddEvent(SDL_JoyHatEvent &ev);
	void AddEvent(SDL_JoyButtonEvent &ev);
	void AddEvent(SDL_KeyboardEvent &ev);
	void AddEvent(SDL_MouseButtonEvent &ev);
	void AddEvent(SDL_MouseMotionEvent &ev);

	void SetScreenToWindowCoeff(float kx, float ky);
	
	void FlushEvents();
	void ArrayFlush();
	void ProlongedArrayFlush();


	bool IsPressed(ConfigActionKeys key) const;
	bool IsReleased(ConfigActionKeys key) const;
	bool IsHolded(ConfigActionKeys key) const;
	bool IsRepeated(ConfigActionKeys key) const;
	bool IsProlongedReleased(ConfigActionKeys key) const;


	bool IsHolded(UINT key) const;
private:
	bool pressed[keys_count];
	bool holded[keys_count];
	bool released[keys_count];
	bool repeated[keys_count];

	// ��� ������� ��������� ����, � ����� ������ UpdateGame
	//bool prolonged_pressed[keys_count];
	//bool long_holded[keys_count];
	bool prolonged_released[keys_count];

	bool CheckKeyInArray(const bool* arr, ConfigActionKeys key) const;

	enum ERepeatingState { rs_none, rs_waiting, rs_repeating };
	ERepeatingState repeatingState;			// ��������� ����������
	UINT repeat_timer;						// ������ ��� ���������� �������
	InputEvent* event_to_repeat;			// ������� ������� ��� ����������
};

#endif // __KEYB_MGR_H_

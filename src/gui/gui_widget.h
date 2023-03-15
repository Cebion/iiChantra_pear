#ifndef __GUI_VIGET_H_
#define __GUI_VIGET_H_

#include "../game/phys/phys_misc.h"
#include "../game/sprite.h"
#include "../render/renderer.h"
#include "../render/font.h"
#include "../script/script.h"
#include "../input_mgr.h"

#include "text_typer.h"

enum MousePosition { MouseIsOut, MouseIsIn }; 
enum MouseButtonState { MouseButtonUnpresed, MouseButtonPressed };


// Сферический виджет
class GuiWidget
{
public:
	UINT id;

	char* name;

	bool fully_added;			// Добавлен ли виджет в основной список (прошел ли BatchCreate)

	bool active;				// активность, фокус
	bool visible;
	bool dead;
	bool staticWidget;
	bool focusable;

	CAABB aabb;
	float z;
	float min_z;
	Sprite* sprite;
	bool fixed_position;
	//RGBAf active_color;
	//RGBAf inactive_color;

	bool border;				// Рисовать ли рамку
	RGBAf border_active_color;
	RGBAf border_inactive_color;

	char* caption;				// Надпись на виджете
	RGBAf caption_act_color;
	RGBAf caption_inact_color;
	Font* caption_font;
	bool caption_multiline;

	TextTyper* typer;

	MousePosition lastMousePos;					// Предыдущее положение мышки (на виджете или вне его)
	MouseButtonState lastmouseButtonState[InputMgr::mousebtns_count];	// Предыдущее состояние клавиш мышки

	GuiWidget()
	{
		Init();
	}

	virtual ~GuiWidget();



	bool SetSprite(const Proto* proto, const char* start_anim);
	bool SetSprite(const char* proto_name, const char* start_anim);
	bool SetColorBox(const RGBAf& color);
	bool SetSpriteRenderMethod(RenderSpriteMethod rsm);

	void SetZ(float z);
	void SetFixedPosition(bool fixed);

	virtual void Process();
	virtual void Draw();


	void BringToFront();

	virtual void OnMouseEnter() {};
	virtual void OnMouseLeave() {};
	virtual void OnLMouseClick() {};
	virtual void OnRMouseClick() {};
	virtual void OnMouseClick(BYTE button);
	virtual void OnKeyDown(USHORT symbol);
	virtual void OnKeyInput(Uint16 vkey);
	virtual void OnKeyPress(USHORT vkey);
	virtual void OnFocus();
	virtual void OnUnFocus();
	virtual void OnResize();

	int onMouseEnterProc;
	int onMouseLeaveProc;
	int onMouseLClickProc;
	int onMouseRClickProc;
	int onKeyDown;
	int onKeyInput;
	int onKeyPress;
	int onFocus;
	int onUnFocus;
	int onResize;


	virtual void SetCaption(const char* cap, bool multiline);
	const char* GetCaption() const;
	void UseTyper( UINT period, bool pausable );
	void UnUseTyper();
	void StartTyper();
	void StopTyper();

protected:

	

	//void ProcessMouseMoves();
	//void ProcessMouseClicks();
	void ProcessSprite();

	void DrawCaption();
	void DrawSprite();
	void DrawBorder();

	void Init()
	{
		id = 0;
		name = NULL;
		sprite = NULL;
		caption = NULL;
		caption_font = FontByName(DEFAULT_FONT);
		caption_multiline = false;
		active = dead = false;
		visible = true;
		border = false;
		focusable = true;
		caption_act_color = RGBAf(1.0f, 0.48f, 0.09f, 1.0f);
		caption_inact_color = RGBAf(0.5f, 0.5f, 0.5f, 1.0f);
		border_inactive_color = RGBAf(0.5f, 0.5f, 0.5f, 1.0f);
		border_active_color = RGBAf(1.0f, 1.0f, 1.0f, 1.0f);
		staticWidget = false;
		
		fully_added = false;

		z = 0.95f;
		fixed_position = true;
		lastMousePos = MouseIsOut;
		for (BYTE i = 0; i < InputMgr::mousebtns_count; i++) 
			lastmouseButtonState[i] = MouseButtonUnpresed;

		onMouseEnterProc = onMouseLeaveProc = onMouseLClickProc = 
			onMouseRClickProc = onKeyDown = onKeyPress = 
			onFocus = onUnFocus = onKeyInput = onResize = LUA_NOREF;

		typer = NULL;
	}

};



#endif // __GUI_VIGET_H_
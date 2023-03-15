#include "StdAfx.h"

#include "gui_widget.h"
#include "../config.h"

#include "../resource_mgr.h"

//////////////////////////////////////////////////////////////////////////
extern ResourceMgr<Proto> * protoMgr;

extern config cfg;
extern lua_State* lua;
//////////////////////////////////////////////////////////////////////////

GuiWidget::~GuiWidget()
{
	DELETEARRAY(caption);

	DELETESINGLE(typer);

	DELETESINGLE(this->sprite);

	SCRIPT::RemoveFromRegistry(this->onMouseEnterProc);
	SCRIPT::RemoveFromRegistry(this->onMouseLeaveProc);
	SCRIPT::RemoveFromRegistry(this->onMouseRClickProc);
	SCRIPT::RemoveFromRegistry(this->onMouseLClickProc);
	SCRIPT::RemoveFromRegistry(this->onKeyDown);
	SCRIPT::RemoveFromRegistry(this->onKeyPress);
	SCRIPT::RemoveFromRegistry(this->onFocus);
	SCRIPT::RemoveFromRegistry(this->onUnFocus);

	sLog(DEFAULT_GUI_LOG_NAME, LOG_INFO_EV, "Уничтожен виджет %s, id = %d по адресу %p", this->name, this->id, this);
	DELETEARRAY(name);
}


//////////////////////////////////////////////////////////////////////////

bool GuiWidget::SetSprite(const Proto* proto, const char* start_anim)
{
	if (!proto)
		return false;

	DELETESINGLE(this->sprite);

	this->sprite = new Sprite(proto);
	// Установка спрайта сбивает тот z, котрый был и заменяет на z из прототипа.
	this->z = this->sprite->z;
	if (start_anim)
		this->sprite->SetAnimation(string(start_anim));
	else
		this->sprite->SetAnimation(string("idle"));

	this->sprite->SetFixed();

	return true;
}

bool GuiWidget::SetSprite(const char* proto_name, const char* start_anim)
{
	if (!proto_name)
		return false;

	return SetSprite(protoMgr->GetByName(proto_name, "widgets/"), start_anim);
}

bool GuiWidget::SetSpriteRenderMethod(RenderSpriteMethod rsm)
{
	if (!sprite)
		return false;

	sprite->renderMethod = rsm;

	return true;
}

bool GuiWidget::SetColorBox(const RGBAf& color)
{
	bool is_visible = (this->sprite && this->sprite->IsVisible());
	DELETESINGLE(this->sprite);
	this->sprite = new Sprite();
	this->sprite->color = color;
	this->sprite->render_without_texture = true;
	this->sprite->z = z;
	this->sprite->frameWidth = 2*(USHORT)this->aabb.W;
	this->sprite->frameHeight = 2*(USHORT)this->aabb.H;
	if (is_visible) this->sprite->SetVisible();
	this->sprite->SetFixed();
	return true;
}

void GuiWidget::SetCaption(const char *cap, bool multiline)
{
	DELETEARRAY(this->caption);
	this->caption = StrDupl(cap);
	this->caption_multiline = multiline;
	if (typer)
	{
		TextTyper* t = new TextTyper(caption, *typer);
		DELETESINGLE(typer);
		typer = t;
	}
		
}

const char* GuiWidget::GetCaption() const
{
	return this->caption;
}

void GuiWidget::SetZ(float z)
{
	this->z = z;
	if (this->sprite)
	{
		this->sprite->z = z;
	}
}

void GuiWidget::SetFixedPosition(bool fixed)
{
	this->fixed_position = fixed;
	if (sprite)
	{
		if (fixed)
			this->sprite->SetFixed();
		else
			this->sprite->ClearFixed();
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void GuiWidget::ProcessSprite()
{
	if (!sprite)
		return;

	if (sprite->anims && sprite->animsCount > 0/* && sprite->animNames*/)
	{
		Animation* a = this->sprite->GetAnimation(this->sprite->cur_anim_num);

		if (a)
		{
			while (!this->sprite->ChangeFrame(a))
			{
				switch (a->frames[a->current].command)
				{
				case afcSetAnim:
					this->sprite->SetAnimation(string(a->frames[a->current].txt_param));
					this->ProcessSprite();
					return;
					break;
				case afcMirror:
					this->sprite->SetMirrored();
					this->sprite->realX = this->sprite->frameWidth;
					break;
				case afcJumpRandom:
					{
						StackElement* sd = this->sprite->stack->Pop();
						if (  (rand() % 256) > sd->data.intData && !this->sprite->JumpFrame( a->frames[a->current].param ) )
							sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Невозможный прыжок в анимации %s.", this->sprite->cur_anim.c_str());
						DELETESINGLE(sd);
					}
					break;
				case afcLoop:
					this->sprite->SetCurrentFrame(0);
				default:
					break;
				}

			}
		}

	}
}


void GuiWidget::Process()
{
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

extern float CAMERA_OFF_X;
extern float CAMERA_OFF_Y;

void GuiWidget::DrawCaption()
{
	if (this->caption && this->caption_font)
	{
		caption_font->tClr = this->active ? this->caption_act_color :
			this->caption_inact_color;

		caption_font->p = Vector2( floor(this->aabb.Left()+0.5f), floor(this->aabb.Top()+0.5f));
		if (this->fixed_position)
		{
			caption_font->p.x -= CAMERA_OFF_X;
			caption_font->p.y -= CAMERA_OFF_Y;
		}

		caption_font->z = this->z + 0.000003f;

		const char* cap = typer ? typer->GetBuffer() : this->caption;

		if ( this->caption_multiline )
			caption_font->PrintMultiline(cap, this->aabb);
		else
			caption_font->Print(cap);
	}
}

void GuiWidget::DrawSprite()
{
	if (this->sprite)
	{
		if (this->visible) 
			this->sprite->SetVisible(); 
		else 
			this->sprite->ClearVisible();

		this->sprite->z = this->z;
		this->sprite->Draw(this->aabb);
	}
}

void GuiWidget::DrawBorder()
{
	if (this->border)
	{

		float x = this->aabb.Left();
		float y = this->aabb.Top();
		if (this->fixed_position)
		{
			x -= CAMERA_OFF_X;
			y -= CAMERA_OFF_Y;
		}
		RenderBox(x, y, this->z,
			this->aabb.W * 2, this->aabb.H * 2,
			this->active ? this->border_active_color : this->border_inactive_color);
	}
}

void GuiWidget::Draw()
{
	//if (!this->visible)
	//	return;

	if (min_z > z)
		z = min_z;

	this->DrawBorder();
	this->DrawSprite();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void GuiWidget::OnMouseClick(BYTE button)
{
	switch(button)
	{
	case InputMgr::MOUSE_BTN_LEFT: OnLMouseClick(); break;
	case InputMgr::MOUSE_BTN_RIGHT: OnRMouseClick(); break;
	case InputMgr::MOUSE_BTN_MIDDLE: break;
	default: break;
	}
}

void GuiWidget::OnKeyDown( USHORT vkey )
{
	if (this->onKeyDown >= 0)
	{
		lua_pushinteger(lua, this->id);
		lua_pushinteger(lua, vkey);
		if (SCRIPT::ExecChunkFromReg(this->onKeyDown, 2))
		{
			// В скрипте произошла какая-то ошибка.
			sLog(DEFAULT_GUI_LOG_NAME, LOG_WARNING_EV, "В обработчике OnKeyDown() виджета %s произошла ошибка", this->name);
		}
	}
}

void GuiWidget::OnKeyInput( Uint16 symbol )
{
	if (this->onKeyInput >= 0)
	{
		lua_pushinteger(lua, this->id);
		lua_pushinteger(lua, (int)symbol);
		if (SCRIPT::ExecChunkFromReg(this->onKeyInput, 2))
		{
			// В скрипте произошла какая-то ошибка.
			sLog(DEFAULT_GUI_LOG_NAME, LOG_WARNING_EV, "В обработчике OnKeyInput() виджета %s произошла ошибка", this->name);
		}
	}
}

void GuiWidget::OnKeyPress( USHORT vkey )
{
	if (this->onKeyPress >= 0)
	{
		lua_pushinteger(lua, this->id);
		lua_pushinteger(lua, vkey);
		if (SCRIPT::ExecChunkFromReg(this->onKeyPress, 2))
		{
			// В скрипте произошла какая-то ошибка.
			sLog(DEFAULT_GUI_LOG_NAME, LOG_WARNING_EV, "В обработчике OnKeyPress() виджета %s произошла ошибка", this->name);
		}
	}
}

void GuiWidget::OnFocus()
{
	if (this->onFocus >= 0)
	{
		lua_pushinteger(lua, this->id);
		if (SCRIPT::ExecChunkFromReg(this->onFocus, 1))
		{
			// В скрипте произошла какая-то ошибка.
			sLog(DEFAULT_GUI_LOG_NAME, LOG_WARNING_EV, "В обработчике OnFocus() виджета %s произошла ошибка", this->name);
		}
	}
}

void GuiWidget::OnUnFocus()
{
	if (this->onUnFocus >= 0)
	{
		lua_pushinteger(lua, this->id);
		if (SCRIPT::ExecChunkFromReg(this->onUnFocus, 1))
		{
			// В скрипте произошла какая-то ошибка.
			sLog(DEFAULT_GUI_LOG_NAME, LOG_WARNING_EV, "В обработчике OnUnFocus() виджета %s произошла ошибка", this->name);
		}
	}
}

void GuiWidget::OnResize()
{
	if (this->onResize >= 0)
	{
		lua_pushinteger(lua, this->id);
		if (SCRIPT::ExecChunkFromReg(this->onResize, 1))
		{
			// В скрипте произошла какая-то ошибка.
			sLog(DEFAULT_GUI_LOG_NAME, LOG_WARNING_EV, "В обработчике OnResize() виджета %s произошла ошибка", this->name);
		}
	}
}

void GuiWidget::UseTyper( UINT period, bool pausable )
{
	if (typer)
	{
		typer->pausable = pausable;
		typer->SetPeriod(period);
		if (!typer->IsStarted())
			typer->Start();
	}
	else
	{
		typer = new TextTyper(this->caption, period, pausable);
		//typer->Start();
	}
}

void GuiWidget::UnUseTyper()
{
	DELETESINGLE(typer);
}

void GuiWidget::StartTyper()
{
	if (typer)
		typer->Start();
	else
		sLog(DEFAULT_GUI_LOG_NAME, logLevelWarning, "Попытка запустить TextTyper, когда его нет в виджете %d %s", id, name);
}


void GuiWidget::StopTyper()
{
	if (typer)
		typer->Stop();
	else
		sLog(DEFAULT_GUI_LOG_NAME, logLevelWarning, "Попытка остановить TextTyper, когда его нет в виджете %d %s", id, name);
}

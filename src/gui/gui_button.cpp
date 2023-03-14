#include "StdAfx.h"

#include "gui_button.h"

#include "../misc.h"

//////////////////////////////////////////////////////////////////////////

extern lua_State* lua;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void GuiButton::Process()
{
	if (!this->visible)
		return;

	//this->ProcessMouseMoves();
	//this->ProcessMouseClicks();
	this->ProcessSprite();
	//this->ProcessChildren();
}

void GuiButton::Draw()
{
	if (!this->visible)
		return;

	this->DrawCaption();
	this->DrawSprite();
	this->DrawBorder();
}


void GuiButton::OnMouseEnter()
{
	//this->active = true;

	if (this->onMouseEnterProc >= 0)
	{
		lua_pushinteger(lua, this->id);
		if (SCRIPT::ExecChunkFromReg(this->onMouseEnterProc, 1))
		{
			// � ������� ��������� �����-�� ������.
			sLog(DEFAULT_GUI_LOG_NAME, LOG_WARNING_EV, "� ����������� OnMouseEnter() ������ %s ��������� ������", this->name);
		}
	}
}


void GuiButton::OnMouseLeave()
{
	//this->active = false;

	if (this->onMouseLeaveProc >= 0)
	{
		lua_pushinteger(lua, this->id);
		if (SCRIPT::ExecChunkFromReg(this->onMouseLeaveProc, 1))
		{
			// � ������� ��������� �����-�� ������.
			sLog(DEFAULT_GUI_LOG_NAME, LOG_WARNING_EV, "� ����������� OnMouseLeave() ������ %s ��������� ������", this->name);
		}
	}
}




void GuiButton::OnLMouseClick()
{
	//sLog(DEFAULT_GUI_LOG_NAME, LOG_INFO_EV, "l-click");

	if (this->onMouseLClickProc >= 0)
	{
		lua_pushinteger(lua, this->id);
		if (SCRIPT::ExecChunkFromReg(this->onMouseLClickProc, 1))
		{
			// � ������� ��������� �����-�� ������.
			sLog(DEFAULT_GUI_LOG_NAME, LOG_WARNING_EV, "� ����������� OnLMouseClick() ������ %s ��������� ������", this->name);
		}
	}
}

void GuiButton::OnRMouseClick()
{
	//sLog(DEFAULT_GUI_LOG_NAME, LOG_INFO_EV, "r-click");

	if (this->onMouseRClickProc >= 0)
	{
		lua_pushinteger(lua, this->id);
		if (SCRIPT::ExecChunkFromReg(this->onMouseRClickProc, 1))
		{
			// � ������� ��������� �����-�� ������.
			sLog(DEFAULT_GUI_LOG_NAME, LOG_WARNING_EV, "� ����������� OnRMouseClick() ������ %s ��������� ������", this->name);
		}
	}
}

#include "StdAfx.h"

#include "../gui/gui.h"



#include "api.h"

//////////////////////////////////////////////////////////////////////////

extern Gui* gui;
extern lua_State* lua;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


int scriptApi::CreateWidget(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается тип виджета");
	luaL_argcheck(L, lua_isstring(L, 2), 2, "Ожидается имя виджета");
	luaL_argcheck(L, lua_islightuserdata(L, 3) || lua_isnil(L, 3), 3, "Ожидается предок");
	luaL_argcheck(L, lua_isnumber(L, 4), 4, "Ожидается x");
	luaL_argcheck(L, lua_isnumber(L, 5), 5, "Ожидается y");
	luaL_argcheck(L, lua_isnumber(L, 6), 6, "Ожидается w");
	luaL_argcheck(L, lua_isnumber(L, 7), 7, "Ожидается h");

	WidgetTypes wt = (WidgetTypes)lua_tointeger(L, 1);
	const char* name = lua_tostring(L, 2);
	//void* parent = lua_touserdata(L, 3);
	float x = (float)lua_tonumber(L, 4);
	float y = (float)lua_tonumber(L, 5);
	float w = (float)lua_tonumber(L, 6);
	float h = (float)lua_tonumber(L, 7);

	UINT id = gui->CreateWidget(wt, name, x, y, w, h);
	lua_pop(L, lua_gettop(L));
	lua_pushinteger(L, id);
	return 1;
}

int scriptApi::DestroyWidget(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");
	gui->DestroyWidget((UINT)lua_tointeger(L, 1));
	return 0;
}


int scriptApi::WidgetSetBorder(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");
	luaL_argcheck(L, lua_isboolean(L, 2), 2, "Ожидается bool");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
		wi->border = lua_toboolean(L, 2) != 0;
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetBorder: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}

int scriptApi::WidgetSetMaxTextfieldSize(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");
	luaL_argcheck(L, lua_isnumber(L, 2), 1, "Ожидается размер");

	size_t size = (size_t)lua_tointeger(L, 2);
	GuiTextfield* wt = dynamic_cast<GuiTextfield*>(gui->GetWidget((UINT)lua_tointeger(L, 1)));
	if (wt)
	{
		wt->max_size = size;
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetMaxTextfieldSize: Виджет id=%d не существует или не Textfield", (UINT)lua_tointeger(L, 1));
	}
	return 0;
}

int scriptApi::WidgetSetCaption(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");
	luaL_argcheck(L, lua_isstring(L, 2), 2, "Ожидается caption");
	luaL_argcheck(L, lua_isboolean(L, 3)||lua_isnil(L, 3)||lua_isnone(L, 3), 2, "Ожидается bool multiline");

	bool multiline = false;
	if ( lua_isboolean(L, 3) ) multiline = lua_toboolean(L, 3) != 0;

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
		wi->SetCaption(lua_tostring(L, 2), multiline);
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetCaption: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}
	return 0;
}

int scriptApi::WidgetGetCaption(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
		lua_pushstring(L, wi->caption);
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetGetCaption: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
		lua_pushnil(L);
	}
	return 1;
}

int scriptApi::WidgetGetName(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
		lua_pushstring(L, wi->name);
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetGetName: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
		lua_pushnil(L);
	}
	return 1;
}

int scriptApi::WidgetSetCaptionColor(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");
	luaL_argcheck(L, lua_istable(L, 2), 2, "Ожидается таблица color");
	luaL_argcheck(L, lua_isboolean(L, 3), 3, "Ожидается bool");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		if (lua_toboolean(L, 3))
			SCRIPT::GetColorFromTable(L, 2, wi->caption_act_color);
		else
			SCRIPT::GetColorFromTable(L, 2, wi->caption_inact_color);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetCaptionColor: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}
	return 0;
}

int scriptApi::WidgetSetBorderColor(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");
	luaL_argcheck(L, lua_istable(L, 2), 2, "Ожидается таблица color");
	luaL_argcheck(L, lua_isboolean(L, 3), 3, "Ожидается bool");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		if (lua_toboolean(L, 3))
			SCRIPT::GetColorFromTable(L, 2, wi->border_active_color);
		else
			SCRIPT::GetColorFromTable(L, 2, wi->border_inactive_color);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetBorderColor: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}

int scriptApi::WidgetSetCaptionFont(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");
	luaL_argcheck(L, lua_isstring(L, 2), 2, "Ожидается fontname");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		wi->caption_font = FontByName(lua_tostring(L, 2));
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetCaptionFont: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}

int scriptApi::GetCaptionSize(lua_State* L)
{
	luaL_argcheck(L, lua_isstring(L, 1), 1, "Ожидается шрифт");
	luaL_argcheck(L, lua_isstring(L, 2), 2, "Ожидается текст");

	Font* font = FontByName(lua_tostring(L, 1));
	if (font)
	{
		int result_y = font->GetStringHeight(lua_tostring(L, 2));
		int result_x = font->GetStringWidth(lua_tostring(L, 2));
		lua_pushinteger(L, result_x);
		lua_pushinteger(L, result_y);
		return 2;
	}
	return 0;
}

int scriptApi::WidgetUseTyper(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");
	luaL_argcheck(L, lua_isboolean(L, 2), 2, "Ожидается bool");
	

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		bool use = lua_toboolean(L, 2) != 0;
		if (use)
		{
			luaL_argcheck(L, lua_isnumber(L, 3), 3, "Ожидается int");
			luaL_argcheck(L, lua_isboolean(L, 4) || lua_isnil(L,4) || lua_isnone(L, 4), 4, "Ожидается bool или nil или none");
			bool pausable = lua_isboolean(L, 4) && lua_toboolean(L, 4);

			wi->UseTyper((UINT)lua_tointeger(L, 3), pausable);
		}
		else
		{
			wi->UnUseTyper();
		}
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetUseTyper: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}

int scriptApi::WidgetStartTyper(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		wi->StartTyper();
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetStartTyper: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}

int scriptApi::WidgetStopTyper(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		wi->StopTyper();
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetStopTyper: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}

int scriptApi::WidgetSetOnTyperEndedProc(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		SCRIPT::RegProc(L, &wi->typer->onTyperEnded, 2);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetMouseLeaveProc: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}


int scriptApi::WidgetSetVisible(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");
	luaL_argcheck(L, lua_isboolean(L, 2), 2, "Ожидается bool");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		wi->visible = lua_toboolean(L, 2) != 0;
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetVisible: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}

int scriptApi::WidgetGetVisible(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		lua_pushboolean(L, wi->visible);
	}
	else
	{
		lua_pushnil(L);
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetGetVisible: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 1;
}

int scriptApi::WidgetSetFixedPosition(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");
	luaL_argcheck(L, lua_isboolean(L, 2), 2, "Ожидается bool");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		wi->SetFixedPosition(lua_toboolean(L, 2) != 0);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetFixedPosition: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}

int scriptApi::WidgetGetSize(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		lua_pushnumber(L, 2*wi->aabb.W);
		lua_pushnumber(L, 2*wi->aabb.H);
	}
	else
	{
		lua_pushnil(L);
		lua_pushnil(L);
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetGetSize: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 2;
}

int scriptApi::WidgetGetPos(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		lua_pushnumber(L, wi->aabb.Left());
		lua_pushnumber(L, wi->aabb.Top());
	}
	else
	{
		lua_pushnil(L);
		lua_pushnil(L);
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetGetSize: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 2;
}

int scriptApi::WidgetSetFocusable(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");
	luaL_argcheck(L, lua_isboolean(L, 2), 2, "Ожидается bool");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		wi->focusable = lua_toboolean(L, 2) != 0;
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetFocusable: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}



int scriptApi::WidgetSetLMouseClickProc(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		SCRIPT::RegProc(L, &wi->onMouseLClickProc, 2);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetLMouseClickProc: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}

int scriptApi::WidgetSetRMouseClickProc(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		SCRIPT::RegProc(L, &wi->onMouseRClickProc, 2);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetRMouseClickProc: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}

int scriptApi::WidgetSetMouseEnterProc(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");
	
	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		SCRIPT::RegProc(L, &wi->onMouseEnterProc, 2);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetMouseEnterProc: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}

int scriptApi::WidgetSetMouseLeaveProc(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		SCRIPT::RegProc(L, &wi->onMouseLeaveProc, 2);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetMouseLeaveProc: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}

int scriptApi::WidgetSetKeyDownProc(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		SCRIPT::RegProc(L, &wi->onKeyDown, 2);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetKeyDownProc: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}

int scriptApi::WidgetSetKeyPressProc(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		SCRIPT::RegProc(L, &wi->onKeyPress, 2);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetKeyPressProc: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}

int scriptApi::WidgetSetKeyInputProc(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		SCRIPT::RegProc(L, &wi->onKeyInput, 2);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetKeyInputProc: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}

int scriptApi::WidgetSetFocusProc(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		SCRIPT::RegProc(L, &wi->onFocus, 2);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetFocusProc: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}

int scriptApi::WidgetSetUnFocusProc(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		SCRIPT::RegProc(L, &wi->onUnFocus, 2);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetUnFocusProc: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}

int scriptApi::WidgetSetResizeProc(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		SCRIPT::RegProc(L, &wi->onResize, 2);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetResizeProc: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}

int scriptApi::WidgetGainFocus(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");
	UINT id = (UINT)lua_tointeger(L, 1);

	gui->SetFocus(id);

	return 0;
}

int scriptApi::WidgetBringToFront(lua_State* L)
{
	luaL_error(L, "WidgetBringToFront не работает");
	return 0;
	//luaL_argcheck(L, lua_islightuserdata(L, 1), 1, "Ожидается виджет");

	//void* p = lua_touserdata(L, 1);
	//GuiWidget* wi = (GuiWidget*)p;

	//wi->BringToFront();

	//return 0;
}

int scriptApi::WidgetSetSize(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1) || lua_isnil(L, 1), 1, "Ожидается id виджета");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается ширина");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Ожидается высота");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		float w = (float)lua_tonumber(L, 2);
		float h = (float)lua_tonumber(L, 3);

		float top = wi->aabb.Top();
		float left = wi->aabb.Left();
		wi->aabb = CAABB(left, top, left + w, top + h);
		// TODO: Вроде бы будет тоже самое, но быстрее. Надо протестить.
		//wi->aabb.W = w/2;
		//wi->aabb.H = w/2;
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetSize: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}


	return 0;
}

int scriptApi::WidgetSetPos(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1) || lua_isnil(L, 1), 1, "Ожидается id виджета");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается x1");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Ожидается y1");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		float x1 = (float)lua_tonumber(L, 2);
		float y1 = (float)lua_tonumber(L, 3);

		float top = wi->aabb.Top();
		float left = wi->aabb.Left();
		float bottom = wi->aabb.Bottom();
		float right = wi->aabb.Right();
		wi->aabb = CAABB(x1, y1, x1 + (right - left), y1 + (bottom - top));
		// TODO: Вроде бы будет тоже самое, но быстрее. Надо протестить.
		//wi->aabb.p.x = x1 - wi->aabb.W;
		//wi->aabb.p.y = y1 - wi->aabb.H;
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetPos: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}

int scriptApi::WidgetSetZ(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается z");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		wi->SetZ((float)lua_tonumber(L, 2) );
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetZ: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}

int scriptApi::WidgetSetSprite(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");
	luaL_argcheck(L, lua_isstring(L, 2), 2, "Ожидается proto_name");
	luaL_argcheck(L, lua_isstring(L, 3) || lua_isnone(L, 3), 3, "Ожидается start_anim");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		wi->SetSprite(lua_tostring(L, 2), lua_tostring(L, 3));
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetSprite: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}

int scriptApi::WidgetSetColorBox(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");
	luaL_argcheck(L, lua_istable(L, 2), 2, "Ожидается таблица color");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	RGBAf color;
	SCRIPT::GetColorFromTable(L, 2, color);
	if (wi)
	{
		wi->SetColorBox(color);
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetColorBox: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}
	return 0;
}

int scriptApi::WidgetSetSpriteRenderMethod(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается render_method (constants.rsm*");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi)
	{
		wi->SetSpriteRenderMethod((RenderSpriteMethod)lua_tointeger(L, 2));
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetSpriteRenderMethod: Виджет id=%ud не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}

int scriptApi::WidgetSetAnim(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");
	luaL_argcheck(L, lua_isstring(L, 2), 2, "Ожидается anim_name");

	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (wi && wi->sprite)
	{
		wi->sprite->SetAnimation(string(lua_tostring(L, 2)));
	}
	else
	{
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetAnim: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
	}

	return 0;
}


int scriptApi::GlobalSetKeyDownProc(lua_State* L)
{
	SCRIPT::RegProc(L, &gui->onKeyDownGlobal, 1);	
	return 0;
}

int scriptApi::GlobalSetKeyReleaseProc(lua_State* L)
{
	SCRIPT::RegProc(L, &gui->onKeyReleaseGlobal, 1);	
	return 0;
}

int scriptApi::GlobalGetKeyDownProc(lua_State* L)
{
	SCRIPT::GetFromRegistry(L, gui->onKeyDownGlobal);
	return 1;
}

int scriptApi::GlobalGetKeyReleaseProc(lua_State* L)
{
	SCRIPT::GetFromRegistry(L, gui->onKeyReleaseGlobal);
	return 1;
}

int scriptApi::GlobalSetMouseKeyDownProc(lua_State* L)
{
	SCRIPT::RegProc(L, &gui->onMouseKeyDownGlobal, 1);	
	return 0;
}

int scriptApi::GlobalSetMouseKeyReleaseProc(lua_State* L)
{
	SCRIPT::RegProc(L, &gui->onMouseKeyReleaseGlobal, 1);	
	return 0;
}

int scriptApi::GlobalGetMouseKeyDownProc(lua_State* L)
{
	SCRIPT::GetFromRegistry(L, gui->onMouseKeyDownGlobal);
	return 1;
}

int scriptApi::GlobalGetMouseKeyReleaseProc(lua_State* L)
{
	SCRIPT::GetFromRegistry(L, gui->onMouseKeyReleaseGlobal);
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int scriptApi::PushWidget(lua_State* L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id виджета");
	GuiWidget* wi = gui->GetWidget((UINT)lua_tointeger(L, 1));
	if (!wi)
	{
		lua_pushnil(L);
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "Ошибка WidgetSetAnim: Виджет id=%d не существует", (UINT)lua_tointeger(L, 1));
		return 1;
	}

	lua_newtable(L);

	lua_pushnumber(L, wi->id);					lua_setfield(L, -2, "id");
	lua_pushstring(L, wi->name);				lua_setfield(L, -2, "name");

	lua_pushboolean(L, wi->active);				lua_setfield(L, -2, "active");
	lua_pushboolean(L, wi->visible);			lua_setfield(L, -2, "visible");
	lua_pushboolean(L, wi->dead);				lua_setfield(L, -2, "dead");

	SCRIPT::PushAABB(L, wi->aabb);				lua_setfield(L, -2, "aabb");

	if(wi->sprite)
	{
		lua_newtable(L);	// Стек: obj sprite
		lua_pushboolean(L, wi->sprite->IsMirrored());	lua_setfield(L, -2, "mirrored");
		lua_pushboolean(L, wi->sprite->IsFixed());		lua_setfield(L, -2, "fixed");
		lua_pushboolean(L, wi->sprite->IsVisible());	lua_setfield(L, -2, "visible");
		lua_pushboolean(L, wi->sprite->IsAnimDone());	lua_setfield(L, -2, "animDone");
		lua_pushstring(L, wi->sprite->cur_anim.c_str()); lua_setfield(L, -2, "cur_anim");
		lua_setfield(L, -2, "sprite");	// Стек: obj
	}

	lua_pushboolean(L, wi->border);					lua_setfield(L, -2, "border");


	lua_pushstring(L, wi->caption);					lua_setfield(L, -2, "caption");
	if (wi->caption_font)
	{
		//lua_pushstring(L, wi->caption_font->name
	}

	lua_pushnumber(L, wi->lastMousePos);			lua_setfield(L, -2, "lastMousePos");

	{
		lua_newtable(L);
		lua_pushnumber(L, wi->lastmouseButtonState[InputMgr::MOUSE_BTN_LEFT]);			lua_setfield(L, -2, "left");
		lua_pushnumber(L, wi->lastmouseButtonState[InputMgr::MOUSE_BTN_RIGHT]);			lua_setfield(L, -2, "right");
		//lua_pushnumber(L, wi->lastmouseButtonState[InputMgr::MOUSE_BTN_MIDDLE]);		lua_setfield(L, -2, "middle");
		lua_setfield(L, -2, "lastmouseButtonState");
	}


	return 1;
}


#include "StdAfx.h"
#include "gui.h"

#include "../config.h"

#include "../input_mgr.h"

#include "../misc.h"

extern InputMgr inpmgr;
extern config cfg;
extern lua_State* lua;

Gui::Gui()
{
	focusedWidget = NULL;
	nav_mode = (GuiNavigationMode)cfg.gui_nav_mode;
	nav_cycled = cfg.gui_nav_cycled != 0;
	onKeyDownGlobal = LUA_NOREF;
	onKeyReleaseGlobal = LUA_NOREF;
	onMouseKeyDownGlobal = LUA_NOREF;
	onMouseKeyReleaseGlobal = LUA_NOREF;
	nextId = 1;
}


Gui::~Gui()
{
	DestroyAllWidgets();

	SCRIPT::RemoveFromRegistry(this->onKeyDownGlobal);
	SCRIPT::RemoveFromRegistry(this->onKeyReleaseGlobal);
	SCRIPT::RemoveFromRegistry(this->onMouseKeyDownGlobal);
	SCRIPT::RemoveFromRegistry(this->onMouseKeyReleaseGlobal);
}

// Обработчик Gui, вызов обработчиков виджетов, обработка событий ввода
void Gui::Process()
{
	BatchCreate();

	// Передача ввода глобальным скриптовым обработчикам
	if (this->onKeyDownGlobal >= 0 || this->onKeyReleaseGlobal >= 0)
	{
		int proc = LUA_NOREF;
		for(InputMgr::EventsIterator it = inpmgr.events.begin();
			it != inpmgr.events.end();
			it++)
		{
			InputEvent* ie = *it;
			if (ie->type == InputKBoard)
			{
				if (ie->state == InputStatePressed && this->onKeyDownGlobal >= 0)
				{
					proc = onKeyDownGlobal;
				}
				else if (ie->state == InputStateReleased && this->onKeyReleaseGlobal >= 0)
				{
					proc = onKeyReleaseGlobal;
				}
				else
					continue;

				lua_pushinteger(lua, ie->kb.key);
			}
			else if (ie->type == InputMouse)
			{
				if (ie->state == InputStatePressed && this->onMouseKeyDownGlobal >= 0)
				{
					proc = onMouseKeyDownGlobal;
				}
				else if (ie->state == InputStateReleased && this->onMouseKeyReleaseGlobal >= 0)
				{
					proc = onMouseKeyReleaseGlobal;
				}
				else
					continue;

				lua_pushinteger(lua, ie->mouse.key);
			}
			else
				continue;

			if (SCRIPT::ExecChunkFromReg(proc, 1))
			{
				// В скрипте произошла какая-то ошибка.
				sLog(DEFAULT_GUI_LOG_NAME, LOG_WARNING_EV, "В глобальном обработчике клавиш произошла ошибка");
			}

			if ( lua_isboolean(lua, -1) && lua_toboolean(lua, -1)) 
				return;
		}
	}

	if (widgets.size() == 0)
		return;

	WidgetIter fit;
	WidgetConstRevIter rit;

	for(fit = widgets.begin(); fit != widgets.end(); fit++)
	{
		fit->second->Process();
	}

	// Переключение фокуса между виджетами с клавиатуры
	if (nav_mode != gnm_None && widgets.size() > 1)
	{
		int direct = 0;		// Направление переключения

		if (inpmgr.IsPressed(cakGuiNavPrev) || inpmgr.IsRepeated(cakGuiNavPrev))
			direct = -1;	// Назад
		else if (inpmgr.IsPressed(cakGuiNavNext) || inpmgr.IsRepeated(cakGuiNavNext))
			direct = 1;		// Вперед

		if (direct != 0)
		{
			//fit = find(widgets.begin(), widgets.end(), focusedWidget);
			if (focusedWidget)
			{
				//ASSERT(GetWidget(focusedWidget->id));
				fit = widgets.find(focusedWidget->id);
			}
			else
			{
				fit = widgets.end();
			}

			WidgetIter last = widgets.end();	// Последний в списке виджет
			last--;

			size_t i = 0;

			// Поиск следующего виджета, которого можно сделать активным
			do {
				if (fit == widgets.begin())		// В начале
				{
					if (direct == 1) fit++;
					else if (direct == -1 && nav_cycled) fit = last;
				}
				else if (fit == widgets.end())	// Нет сфокусированных
				{
					fit = widgets.begin();
				}
				else if (fit == last)			// В конце
				{
					if (direct == -1) fit--;
					else if (direct == 1 && nav_cycled) fit = widgets.begin();
				}
				else							// Посередине
				{
					if (direct == 1) fit++;
					else fit--;
				}

				if (fit == widgets.end())
					break;
				if (fit == last && !nav_cycled)
					break;
				if (++i == widgets.size())
					break;

			} while (fit->second->staticWidget || !fit->second->visible || !fit->second->focusable);

			if (fit != widgets.end() && !fit->second->staticWidget && fit->second->visible)
				SetFocus(fit->second);
		}
	}

	// Передача ввода клавиатуры виджету, на котром фокус
	if (focusedWidget && focusedWidget->fully_added  && focusedWidget->visible)
	{
		for(InputMgr::EventsIterator it = inpmgr.events.begin();
			it != inpmgr.events.end();
			it++)
		{
			InputEvent* ie = *it;
			if (ie->type == InputKBoard)
			{
				if (ie->state == InputStatePressed)
				{
					focusedWidget->OnKeyDown((USHORT)ie->kb.key);
					//Вызываем только для печатаемых в cp1251 символов (всё равно шрифт других не знает)
					if ( ie->kb.symbol >= 21/* && ie->kb.symbol <= 255 */) 
						focusedWidget->OnKeyInput(ie->kb.symbol);
				}
				else
				{
					focusedWidget->OnKeyPress((USHORT)ie->kb.key);
				}
			}
		}


		if (inpmgr.IsReleased(cakGuiNavAccept))
			focusedWidget->OnMouseClick(InputMgr::MOUSE_BTN_LEFT);

		if (inpmgr.IsReleased(cakGuiNavDecline))
			focusedWidget->OnMouseClick(InputMgr::MOUSE_BTN_RIGHT);
	}

	// Обработка движений мышки
	if (inpmgr.mouse_moved)
	{
		for(fit = widgets.begin(); fit != widgets.end(); fit++)
		{
			GuiWidget* w = fit->second;
			//if (w->static) continue;
			if (!w->visible) continue;
			if (!w->focusable) continue;
			//if (!w->enabled) continue;

			if (w->aabb.PointInCAABB((float)inpmgr.mouseX, (float)inpmgr.mouseY))
			{
				if (w->lastMousePos == MouseIsOut)
				{
					// Только что попала
					w->OnMouseEnter();
				}
#ifdef GUI_SETFOCUS_ON_MOUSEMOVE
				if (w->focusable) this->SetFocus(w);
#endif // GUI_SETFOCUS_ON_MOUSEMOVE
				w->lastMousePos = MouseIsIn;
			}
			else
			{
				if (w->lastMousePos == MouseIsIn)
				{
					// А до этого была
					w->OnMouseLeave();

#ifdef GUI_UNSETFOCUS_ON_MOUSELEAVE
					if (w == this->focusedWidget)
						SetFocus((GuiWidget*)NULL);
#endif // GUI_UNSETFOCUS_ON_MOUSELEAVE

					for (BYTE i = 0; i < InputMgr::mousebtns_count; i++)
						w->lastmouseButtonState[i] = MouseButtonUnpresed;
				}
				w->lastMousePos = MouseIsOut;
			}
		}
	}

	// Обработка нажатий кнопок мышки
#ifdef GUI_SETFOCUS_ON_MOUSECLICK
	for(rit = widgets.rbegin(); rit != widgets.rend(); rit++)
	{
		GuiWidget* w = rit->second;
		if (!w->visible) continue;
		if (w->lastMousePos == MouseIsIn)
		{
			for (BYTE i = 0; i < InputMgr::mousebtns_count; i++)
			{
				if (inpmgr.mouse[i] && w->lastmouseButtonState[i] == MouseButtonUnpresed)
				{
					w->lastmouseButtonState[i] = MouseButtonPressed;
				}
				else if (!inpmgr.mouse[i] && w->lastmouseButtonState[i] == MouseButtonPressed)
				{
					// mouse up
					w->OnMouseClick(i);

					this->SetFocus(w);

					w->lastmouseButtonState[i] = MouseButtonUnpresed;
				}
			}
#else	//Only send events to the widget in focus
	if (this->focusedWidget && this->focusedWidget->fully_added && 
		this->focusedWidget->visible && 
		this->focusedWidget->lastMousePos == MouseIsIn)
	{
		GuiWidget* focus = this->focusedWidget;
		for (BYTE i = 0; i < InputMgr::mousebtns_count; i++)
			{
				if (inpmgr.mouse[i] && focus->lastmouseButtonState[i] == MouseButtonUnpresed)
				{
					focus->lastmouseButtonState[i] = MouseButtonPressed;
				}
				else if (!inpmgr.mouse[i] && focus->lastmouseButtonState[i] == MouseButtonPressed)
				{
					// mouse up
					focus->lastmouseButtonState[i] = MouseButtonUnpresed;
					focus->OnMouseClick(i);
				}
			}
	}
#endif // GUI_SETFOCUS_ON_MOUSECLICK

	BatchDestroy();
}

void Gui::Draw()
{
	//_main->Draw();
	for(WidgetIter it = widgets.begin(); it != widgets.end(); it++)
	{
		GuiWidget* w = it->second;

		if (w->visible)
			w->Draw();
	}
}


UINT Gui::CreateWidget(WidgetTypes wt, const char* name, float x, float y, float w, float h )
{
	GuiWidget* wi = NULL;

	switch (wt)
	{
	case wt_Widget: wi = new GuiWidget(); break;
	case wt_Button: wi = new GuiButton(); break;
	case wt_Label: wi = new GuiLabel(); break;
	case wt_Picture: wi = new GuiPicture(); break;
	case wt_Textfield: wi = new GuiTextfield(); break;
	default:
		sLog(DEFAULT_GUI_LOG_NAME, LOG_WARNING_EV, "Попытка создать виджет неправильного типа. Ничего не создано");
		return NULL;
	}

	wi->name = StrDupl(name);
	wi->aabb = CAABB(x, y, x+w, y+h);

	createdWidgets[wi->id = nextId] = wi;

	sLog(DEFAULT_GUI_LOG_NAME, LOG_INFO_EV, "Создан виджет %s, id = %d  по адресу %p", wi->name, wi->id, wi);
	return nextId++;
}


void Gui::DestroyWidget(UINT id)
{
	GuiWidget* wi = GetWidget(id);
	if (wi && !wi->dead)
	{
		wi->dead = true;
		destroyedWidgets.push_back(wi);
	}
}

void Gui::DestroyAllWidgets()
{
	WidgetIter it;
	for(it = widgets.begin(); it != widgets.end(); it++)
	{
		ASSERT(!it->second->dead);
		DELETESINGLE(it->second);
	}
	widgets.clear();

	for(it = createdWidgets.begin(); it != createdWidgets.end(); it++)
	{
		// Новосозданный вджет может быть мертвым, если его из скриптов убили еще до того, как он прошел через BatchCreate
		if (!it->second->dead)
			DELETESINGLE(it->second);
	}
	createdWidgets.clear();

	for(list<GuiWidget*>::iterator it = destroyedWidgets.begin(); it != destroyedWidgets.end(); it++)
	{
		DELETESINGLE((*it));
	}
	destroyedWidgets.clear();

	focusedWidget = NULL;

	nextId = 1;
}

void Gui::BatchCreate()
{
	if (!createdWidgets.empty())
	{
		for (WidgetIter it = createdWidgets.begin(); it != createdWidgets.end(); it++)
			it->second->fully_added = true;		

		widgets.insert(createdWidgets.begin(), createdWidgets.end());
		createdWidgets.clear();
	}
}

void Gui::BatchDestroy()
{
	if (!destroyedWidgets.empty())
	{
		for(list<GuiWidget*>::iterator it = destroyedWidgets.begin();
			it != destroyedWidgets.end(); it++)
		{
			GuiWidget* w = (*it);

			if (widgets.erase(w->id) == 0)
				createdWidgets.erase(w->id);

			if (w == focusedWidget)
				focusedWidget = NULL;

			DELETESINGLE(w);
		}
		destroyedWidgets.clear();
	}
}

// Делает активным виджет
void Gui::SetFocus( GuiWidget* wi )
{
	if (wi == focusedWidget)
		return;

	if (focusedWidget)
	{
		focusedWidget->active = false;
		focusedWidget->OnUnFocus();
	}

	if (wi && !wi->staticWidget && wi->visible)
	{
		focusedWidget = wi;
		focusedWidget->active = true;
		focusedWidget->OnFocus();
	}
	else
		focusedWidget = NULL;
}

void Gui::SetFocus(UINT id)
{
	SetFocus(GetWidget(id));
}

GuiWidget* Gui::GetWidget(UINT id)
{
	WidgetIter it = widgets.find(id);
	if (it == widgets.end())
	{
		it = createdWidgets.find(id);
		return it == createdWidgets.end() ? NULL : it->second;
	}
	
	return it->second;	
}
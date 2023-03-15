#ifndef __GUI_H_
#define  __GUI_H_


#include "gui_widget.h"
#include "gui_button.h"
#include "gui_picture.h"
#include "gui_label.h"
#include "gui_textfield.h"

// Типы виджетов
enum WidgetTypes { wt_Widget, wt_Button, wt_Picture, wt_Label, wt_Textfield };
// Режимы переключения фокуса между виджетами
enum GuiNavigationMode { gnm_None, gnm_Normal };


class Gui
{
public:

	Gui();
	~Gui();


	void Process();
	void Draw();

	UINT CreateWidget(WidgetTypes wt, const char* name, float x, float y, float w, float h );

	void DestroyWidget(UINT id);
	void DestroyAllWidgets();

	void SetFocus(UINT id);

	GuiWidget* GetWidget(UINT id);

	GuiNavigationMode nav_mode;			// Режим переключения фокуса между виджетами
	bool nav_cycled;					// Циклическое переключение между виджетами

	int onKeyDownGlobal;				// Скриптовый обработчик нажатий клавиш
	int onKeyReleaseGlobal;				// Скриптовый обработчик отжатий клавиш

	int onMouseKeyDownGlobal;				// Скриптовый обработчик нажатий клавиш мышки
	int onMouseKeyReleaseGlobal;			// Скриптовый обработчик отжатий клавиш мышки
private:

	void SetFocus(GuiWidget* wi);

	void BatchCreate();
	void BatchDestroy();

	map<UINT, GuiWidget*> widgets;
	map<UINT, GuiWidget*> createdWidgets;
	list<GuiWidget*> destroyedWidgets;

	typedef map<UINT, GuiWidget*>::iterator WidgetIter;
	typedef map<UINT, GuiWidget*>::const_reverse_iterator WidgetConstRevIter;

	GuiWidget* focusedWidget;			// Виджет, на который в данный момент установлен фокус
	UINT nextId;						// id, который будет присвоен следующему созданному виджету
};





//#define GUITICK 10

#endif // __GUI_H_

#ifndef __GUI_BUTTON_H_
#define __GUI_BUTTON_H_

#include "gui_widget.h"

// Кнопочка, реагирует на нажатия
class GuiButton : public GuiWidget
{
public:

	GuiButton()
	{
	
	}

	~GuiButton()
	{
	
	}



	virtual void Process();
	virtual void Draw();

	virtual void OnMouseEnter();
	virtual void OnMouseLeave();
	virtual void OnLMouseClick();
	virtual void OnRMouseClick();
};

#endif // __GUI_BUTTON_H_
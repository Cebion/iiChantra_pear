#ifndef __GUI_PICTURE_H_
#define __GUI_PICTURE_H_

#include "gui_widget.h"

// Кнопочка, реагирует на нажатия
class GuiPicture : public GuiWidget
{
public:

	GuiPicture()
	{
		staticWidget = true;
	}

	~GuiPicture()
	{

	}



	virtual void Process();
	virtual void Draw();
};

#endif // __GUI_PICTURE_H_
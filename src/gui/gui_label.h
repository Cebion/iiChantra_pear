#ifndef __GUI_LABEL_H_
#define __GUI_LABEL_H_

#include "gui_widget.h"

// ��������, ��������� �� �������
class GuiLabel : public GuiWidget
{
public:

	GuiLabel()
	{
		staticWidget = true;
	}

	~GuiLabel()
	{

	}



	virtual void Process();
	virtual void Draw();
};

#endif // __GUI_LABEL_H_
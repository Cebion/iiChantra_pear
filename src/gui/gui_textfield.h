#ifndef __GUI_TEXTFIELD_H_
#define __GUI_TEXTFIELD_H_

#include "gui_widget.h"

// Кнопочка, реагирует на нажатия
class GuiTextfield : public GuiWidget
{
public:

	size_t reserved_size;			// Объем памяти, выделенной под caption
	size_t max_size;				// Ограничение на размер caption. 0 - без ограничения.
	size_t used_size;				// Объем памяти, реально использованной под надпись (без символа '/0' в конце).
	size_t cursor_position;			// Положение курсора. Курсор находится перед символом номер cursor_position

	GuiTextfield()
	{		
		max_size = 256;
		used_size = 0;
		reserved_size = 0;
		caption = NULL;

		cursor_position = 0;	//Курсор _перед_ символом с этим номером.
	}

	virtual ~GuiTextfield() { }



	virtual void Process();
	virtual void Draw();
	virtual void DrawCaption();

	virtual void OnKeyInput(Uint16 vkey);
	virtual void OnKeyDown(USHORT vkey);

	virtual void SetCaption(const char *cap, bool multiline);

	void SetMaxSize(size_t size);

	void RecalcTextfieldSize();
};

#endif // __GUI_TEXTFIELD_H_
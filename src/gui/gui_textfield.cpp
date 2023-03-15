#include "StdAfx.h"

#include "gui_textfield.h"
#include "gui.h"

#include "../misc.h"

//////////////////////////////////////////////////////////////////////////

extern lua_State* lua;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void GuiTextfield::Process()
{
	//if (!this->visible)
	//	return;

	//this->ProcessSprite();
}

void GuiTextfield::Draw()
{
	if (!this->visible)
		return;

	this->DrawCaption();
	this->DrawBorder();
}

void GuiTextfield::OnKeyInput( Uint16 symbol )
{
	//Бывают разные кодировки, а у нас однобайтово всё.
	if ( symbol == 127 )	//TODO: some clever way to figure out bad symbols
		return;
	Uint16 symbol_bits = symbol;
	int char_size = (symbol & 0xFF00) ? 2 : 1;		// Пока испольуется Uint16, цикл не нужен

	if ( max_size == 0 || used_size + char_size <= max_size )
	{
		if ( used_size + char_size + 1 > reserved_size ) // Строка бесконечная, необходимо перевыделить память
		{
			// Так как caption это строка, в которую тут добавляются символы, набиаремые руками, 
			// считаю, что достаточно добавлять линейно по 64 байта.
			size_t new_size = reserved_size + 64;
			char* new_caption = new char[new_size];
			memcpy(new_caption, caption, reserved_size);
			DELETEARRAY(this->caption);
			this->caption = new_caption;
			reserved_size = new_size;
		}

		// TODO: Считаю, что код ниже - какая-то неведомая херня и работать не будет для смеси двубайтовых и
		// однобайтовых символов. Думаю, те строки придется обрабатывать как-то иначе.
		// Но раз пока работает для однобайтовых, то пускай остается. kernel_bug
		for (size_t i = used_size+char_size; i > cursor_position; i-- )
			caption[i] = caption[i-1];
		for ( int i = 1; i <= char_size; i++ )
		{
			symbol_bits = symbol >> 8 * ( char_size - i );
			symbol_bits = symbol_bits & 255;
			caption[ cursor_position + i - 1 ] = (BYTE)symbol_bits;
		}

		used_size += char_size;
		caption[used_size] = '\0';
		cursor_position += char_size;
	}

	RecalcTextfieldSize();

	GuiWidget::OnKeyInput(symbol);
}

void GuiTextfield::OnKeyDown( USHORT vkey )
{
	//TODO: кнопки из конфига?
	if ( vkey == SDLK_LEFT && cursor_position > 0 )
		cursor_position--;
	else if ( vkey == SDLK_RIGHT && cursor_position < used_size )
		cursor_position++;

	if ( vkey == SDLK_BACKSPACE && cursor_position > 0 )
	{
		for ( size_t i = cursor_position-1; i <= used_size+1; i++ )
			caption[i] = caption[i+1];
		used_size--;
		//caption[ used_size + 1 ] = '\0';
		cursor_position--;
	}
	if ( vkey == SDLK_DELETE && cursor_position < used_size )
	{
		for ( size_t i = cursor_position; i <= used_size+1; i++ )
			caption[i] = caption[i+1];
		used_size--;
	}
	if ( vkey == SDLK_END )
	{
		cursor_position = used_size;
	}
	if ( vkey == SDLK_HOME )
	{
		cursor_position = 0;
	}

	RecalcTextfieldSize();

	GuiWidget::OnKeyDown(vkey);
}

extern float CAMERA_OFF_X;
extern float CAMERA_OFF_Y;
extern Gui* gui;

void GuiTextfield::DrawCaption()
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
		int cp = this->active ? (int)this->cursor_position : -1;		// Курсор только у активного виджета
		caption_font->PrintMultiline(this->caption, this->aabb, cp);
	}
}

void GuiTextfield::SetCaption(const char *cap, bool multiline)
{
	//cursor_position = 0;
	//strcpy( this->caption, cap );
	//used_size = strlen( cap );
	//this->caption[used_size] = '\0';
	
	size_t new_len = strlen(cap);
	// TODO:  тут никак не учитывается max_size
	if (new_len < reserved_size)
	{
		// Памяти хватает, просто копируем надпись
		memcpy(caption, cap, new_len+1);
	}
	else
	{
		DELETEARRAY(caption);
		caption = StrDupl(cap);
		reserved_size = new_len;
	}

	used_size = new_len;
	cursor_position = used_size;
	caption[new_len] = '\0';

	this->caption_multiline = multiline;

	RecalcTextfieldSize();
}

void GuiTextfield::RecalcTextfieldSize()
{
	if (!caption_multiline && this->caption && this->caption_font)
	{
		this->aabb = CAABB(aabb.Left(), aabb.Top(),
			aabb.Left() + caption_font->GetStringWidth(caption), aabb.Top() + caption_font->GetStringHeight(caption));

		this->OnResize();
	}
}
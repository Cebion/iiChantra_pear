#include "StdAfx.h"

#include "gui_picture.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void GuiPicture::Process()
{
	this->ProcessSprite();
}

void GuiPicture::Draw()
{
	if (!this->visible)
		return;

	this->DrawSprite();
	this->DrawBorder();
}

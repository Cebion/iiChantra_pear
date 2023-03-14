// Тут находятся функции рисования всякого разного в обход главного рендерера.
#include "StdAfx.h"

#include "font.h"
#include "draw.h"


#include "../config.h"

//void DrawText(const char* text, UINT length, const WinFont* font, float x, float y)
//{
//	if(!text || !font)
//		return;
//
//	glDisable(GL_TEXTURE_2D);
//
//	glListBase(font->base);
//
//	glRasterPos2f(x, y + font->height);
//
//	glCallLists(length, GL_UNSIGNED_BYTE, text);
//
//	//glEnable(GL_TEXTURE_2D);
//}

void DrawPixel(UINT Xcoord, UINT Ycoord, BYTE red, BYTE green, BYTE blue, BYTE alpha)
{
	glPushMatrix();
	glDisable(GL_TEXTURE_2D);
	glColor4ub(red, green, blue, alpha);
	glBegin(GL_POINTS);
	glVertex2i(Xcoord, Ycoord + 1);
	glEnd();
	//glEnable(GL_TEXTURE_2D);
	glPopMatrix();
}



//extern config cfg;
//void Echo(const char* c)
//{
////	static const WinFont* font = FontByName(DEFAULT_FONT);
//
//	//DrawText(c, strlen(c), font, cfg.window_width/2 - 70, 20);
//}



#ifndef __FONT_H_
#define __FONT_H_

#include "renderer.h"

#define FONT_BASE_COUNT 256		// Количество символов

// Базовый класс шрифта
class Font
{
public:
	Vector2 p;				// Положение отрисовки шрифта
	float z;
	RGBAf tClr;				// Цвет.

	Font()
	{
		z = 1.0f;
	}

	virtual ~Font() { }
	virtual bool Load() { return false; }
	virtual void Recover() { }
	virtual void Print(const char *text, ...) { UNUSED_ARG(text); }
	virtual void PrintMultiline(const char *text, CAABB &area) { UNUSED_ARG(text); UNUSED_ARG(area); }
	virtual void PrintMultiline(const char *text, CAABB &area, int cursor) { UNUSED_ARG(text); UNUSED_ARG(area); UNUSED_ARG(cursor); }

	virtual int	GetStringWidth(const char *text) { UNUSED_ARG(text); return 0; }
	virtual int	GetStringWidthEx(UINT t1, UINT t2, const char *text) { UNUSED_ARG(t1); UNUSED_ARG(t2); UNUSED_ARG(text); return 0; }
	virtual int	GetStringHeight(const char *text) { UNUSED_ARG(text); return 0; }
	virtual int	GetStringHeightEx(UINT t1, UINT t2, const char *text) { UNUSED_ARG(t1); UNUSED_ARG(t2); UNUSED_ARG(text); return 0; }
};

//////////////////////////////////////////////////////////////////////////

#ifdef WIN32
// Класс шрифта Windows
class WinFont : public Font
{
public:
	BYTE height;		// Высота шрифта
	long weight;		// Толщина шрифта
	HFONT font;			// Идентификатор шрифта

	string in_name;		// Название шрифта в Windows

	WinFont();
	~WinFont();

	virtual void Print(const char *text, ...);
	virtual bool Load();
	virtual void Recover();

private:
	GLuint base;			// Base List of 256 glLists for font
	void _Print(const char *text);
};

bool LoadWindowsFont(const char* in_name, BYTE height, long weight, const char* out_name);
#endif // WIN32

bool LoadTextureFont(const char* in_name, const char* out_name);
void RecoverFonts();
//void FreeFont(WinFont* font);
void FreeFont(const char* font_name);
void FreeFonts(void);
Font* FontByName(const char* font_name);

//Satana врывается в этот тред

struct CRectf
{
public:
	Vector2 Min;
	Vector2 Max;
};


#define CFONT_DEFAULT_DISTANCE	1
#define CFONT_VALIGN_TOP		0x00
#define CFONT_VALIGN_BOTTOM		0x01
#define CFONT_VALIGN_CENTER		0x02
#define CFONT_VALIGN_MASK		0x0f
#define CFONT_HALIGN_LEFT		0x00
#define CFONT_HALIGN_RIGHT		0x10
#define CFONT_HALIGN_CENTER		0x20
#define CFONT_HALIGN_MASK		0xf0


#define CFONT_DEPTH_HIGH		1.0f
#define CFONT_DEPTH_LOW			0.0f
#define CFONT_MAX_STRING_LENGTH	256

class CFont : public Font
{
public:
	CFont();
	~CFont();

	// из CObject
	//unsigned int type, id;	// type - флаги свойств объекта. id - идентификационный номер объекта. Пока не используется. TODO!
	//string name;			// name - имя объекта. Удобно обращаться к объектам по именам. И в лог писать удобно.

	// из CResource
	//bool loaded;		// loaded должна быть истина если экземпляр объекта был РЕАЛЬНО загружен, а не просто проиндексирован.
	string filename;	// Полный^W хоть-какой-нибуть путь к файлу.

	//RGBAf				tClr;					//	Цвет.
	byte				dist;					//	Расстояние между символами
	Vector2				wh;						//	Вектор с шириной и высотой чего-то. Это для боксов. x - w, y - h
	bool				doUseGlobalCoordSystem;	//	Использовать ли для вывода глобальную систему коодинат

	bool		Load();
	virtual void Recover();
	bool		SaveToFile();

	void		Print(const char *text, ...);
	void		PrintMultiline(const char *text, CAABB &area);
	void		PrintMultiline(const char *text, CAABB &area, int cursor);

	virtual int	GetStringWidth(const char *text);
	virtual int	GetStringWidthEx(UINT t1, UINT t2, const char *text);
	virtual int	GetStringHeight(const char *text);
	virtual int	GetStringHeightEx(UINT t1, UINT t2, const char *text);

private:
	CRectf		bbox[256];		// Баундинг бокс каждого для каждого символа
	byte		width[256];		// Ширина каждого символа
	byte		height[256];	// Высота каждого символа
	char		*FontImageName;	// Имя файла текстуры
	const Texture* tex;

	void		_Print(const UCHAR *text);
	void		_Print(const UCHAR *text, int cursor);
};


namespace ControlSeqParser
{
	enum CSeqType {ENone, EColor, ENewLine};

	CSeqType CheckControlSeq(const char* text, size_t& shift);
	void GetColor(const char* text, RGBAf& color);
}

#endif // __FONT_H_

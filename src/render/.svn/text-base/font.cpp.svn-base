#include "StdAfx.h"

#include "../misc.h"
#include "../cfile.h"
#include "font.h"

#include "../resource_mgr.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

extern char path_fonts[MAX_PATH];
extern ResourceMgr<Texture> * textureMgr;

//////////////////////////////////////////////////////////////////////////

map<string, Font*> fonts;	// Загруженные шрифты

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifdef WIN32
// Загрузка шрифта Windows
// in_name - имя шрифта (например 'Courier New')
// height - высота
// weight (400 - NORMAL, 700 - BOLD, и т. д...)
// out_name - шрифт будет использоваться в игре под таким именем
bool LoadWindowsFont(const char* in_name, BYTE height, long weight, const char* out_name)
{
	if(!in_name || !out_name)
	{
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Incorrect in_name or out_name for loading windows font");
		return false;
	}

	if (FontByName(out_name))
	{
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Уже есть.");
		return false;
	}

	WinFont* font = NULL;
	font = new WinFont;

	font->height = height;
	font->weight = weight;
	font->in_name = string(in_name);

	if (!font->Load())
	{
		DELETESINGLE(font);
		return false;
	}

	fonts[string(out_name)] = font;

	return true;
}
#endif // WIN32

// Загрузка шрифта из текстуры
// in_name - имя файла описания шрифта
// out_name - шрифт будет использоваться в игре под таким именем
bool LoadTextureFont(const char* in_name, const char* out_name)
{
	if(!in_name || !out_name)
	{
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Incorrect in_name or out_name for loading windows font");
		return false;
	}

	if (FontByName(out_name))
	{
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Уже есть.");
		return false;
	}

	char* font_file = new char[strlen(path_fonts) + strlen(in_name) + 1];
	memset(font_file, '\0', strlen(path_fonts) + strlen(in_name) + 1);
	sprintf(font_file, "%s%s", path_fonts, in_name);

	CFont* font = new CFont;
	font->filename = font_file;

	DELETEARRAY(font_file);

	if (!font->Load())
	{
		DELETESINGLE(font);
		return false;
	}

	fonts[string(out_name)] = font;

	return true;
}

// Перезагружает все шрифты.
void RecoverFonts()
{
	map<string, Font*>::iterator it;
	for(it = fonts.begin();
		it != fonts.end();
		it++)
	{
		Font* font = it->second;
		font->Recover();
	}
}

void FreeFont(const char* font_name)
{
	map<string, Font*>::iterator it = fonts.find(string(font_name));
	if (it != fonts.end())
	{
		Font* font = it->second;
		DELETESINGLE(font);
		fonts.erase(it);
	}
}



// free all fonts
void FreeFonts(void)
{
	map<string, Font*>::iterator it;
	for(it = fonts.begin();
		it != fonts.end();
		it++)
	{
		Font* font = it->second;
		DELETESINGLE(font);
	}
	fonts.clear();
}



// get font by name
Font* FontByName(const char* font_name)
{
	map<string, Font*>::iterator it = fonts.find(string(font_name));
	if (it != fonts.end())
	{
		return it->second;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// WinFont

#ifdef WIN32
WinFont::WinFont()
{
	height = 0;
	weight = 0;
	font = (HFONT)INVALID_HANDLE_VALUE;
	base = 0;
}

WinFont::~WinFont()
{
	DeleteObject(this->font);

	// free gl-list
	glDeleteLists(this->base, FONT_BASE_COUNT);
}

// Загрузка шрифта.
// Предварительно надо задать in_name, height, weight
bool WinFont::Load()
{
	this->font = CreateFont(-this->height, 0,	0, 0, this->weight, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		FF_DONTCARE || DEFAULT_PITCH, this->in_name.data());

	if(!this->font)
	{
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Failed to create font");
		return false;
	}

	SelectObject(/*hDC*/0, this->font);
	this->base = glGenLists(FONT_BASE_COUNT);
	wglUseFontBitmaps(/*hDC*/0, 0, FONT_BASE_COUNT, this->base);

	return true;
}

void WinFont::Recover()
{
	SelectObject(/*hDC*/0, this->font);
	this->base = glGenLists(FONT_BASE_COUNT);
	wglUseFontBitmaps(/*hDC*/0, 0, FONT_BASE_COUNT, this->base);
}

// Подготовка строки к выводу на экран
void WinFont::Print(const char *text, ...)
{
	if (text == NULL)
		return;

	static char	*temp;//[CFONT_MAX_STRING_LENGTH];
	temp = new char [strlen(text)+CFONT_MAX_STRING_LENGTH];
	va_list	ap;
	va_start(ap, text);
	vsprintf(temp, text, ap);
	va_end(ap);
	_Print(temp);
	delete [] temp;
}

// Вывод на экран строки
void WinFont::_Print(const char *text)
{
	glDisable(GL_TEXTURE_2D);
	glColor4fv(&(tClr.r));
	glListBase(this->base);
	glRasterPos3f(this->p.x, this->p.y/* + this->height*/, this->z);
	glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);
}

#endif // WIN32


//-------------------------------------------//
//				Font stuff					 //
//-------------------------------------------//
CFont::CFont()
{
	FontImageName = NULL;
	dist = CFONT_DEFAULT_DISTANCE;
	tClr = RGBAf(1.0f, 1.0f, 1.0f, 1.0f);
	// TODO: забить нулями bbox
}

CFont::~CFont()
{
	if (FontImageName != NULL)
		delete [] FontImageName;
}

// Загрузка шрифта из файла
// Предварительно надо задать filename
bool CFont::Load()
{
	if (filename == "")
		return false;
	CFile file;

	if (!file.Open(filename.c_str(), CFile::OPEN_MODE_READ))
	{
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Can't Load Font %s: file  couldn't be opened.", filename.data());
		return false;
	}

	file.ReadLine(FontImageName);
	tex = textureMgr->GetByName(FontImageName);
	if (!this->tex)
	{
		file.Close();
		return false;
	}
	file.Read(bbox, sizeof(bbox));
	scalar y;

	// TODO: Этот цикл - костыль. Потому что редактор шрифтов и готовые шрифты
	// размечены как размечены, легче конвернтуть во время загрузки.
	for (size_t i = 0; i < sizeof(bbox) / sizeof(bbox[0]); i++)
	{
		y = (scalar)tex->height - bbox[i].Max.y;
		bbox[i].Max.y = (scalar)tex->height - bbox[i].Min.y;
		bbox[i].Min.y = y;

		width[i] = (BYTE)(bbox[i].Max.x - bbox[i].Min.x);
		height[i] = (BYTE)(bbox[i].Max.y - bbox[i].Min.y);
	}
	file.Close();

	return true;
}

void CFont::Recover()
{
	this->Load();
}

bool CFont::SaveToFile()
{
	if (filename == "")
		return false;
	CFile			file;
	file.Open(filename, CFile::OPEN_MODE_WRITE);
	file.Write(FontImageName, (size_t)strlen(FontImageName));
	file.WriteByte((byte)0x00);
	file.Write(bbox, sizeof(bbox));
	file.Close();
	return true;
}

void CFont::_Print(const UCHAR *text)
{
	unsigned char c;
	float x = p.x;
	float y = p.y;
	for (const UCHAR* t = text; *t; t++)
	{
		if (*t < 32)
			continue;
		c = *t - 32;
		RenderSprite(x, y, z, bbox[c].Min.x, bbox[c].Min.y, bbox[c].Max.x, bbox[c].Max.y, tex, false, tClr);
		x +=width[c] + dist;
	}
}

extern UINT current_time;

void CFont::_Print(const UCHAR *text, int cursor)
{
	unsigned char c;
	float x = p.x;
	float y = p.y;
	int pos = 0;
	for (const UCHAR* t = text; *t; t++)
	{
		if (*t < 32)
			continue;
		c = *t - 32;
		if ( pos == cursor )
		{
			//TODO:надо как-то получше это всё.
			if ( current_time % 1000 > 500 ) RenderSprite(x, y, z, bbox['|'-32].Min.x, bbox['|'-32].Min.y, bbox['|'-32].Max.x, bbox['|'-32].Max.y, tex, false, tClr);
			x +=width['|'-32] + dist;
			RenderSprite(x, y, z, bbox[c].Min.x, bbox[c].Min.y, bbox[c].Max.x, bbox[c].Max.y, tex, false, tClr);
			x +=width[c] + dist;
		}
		else
		{
			RenderSprite(x, y, z, bbox[c].Min.x, bbox[c].Min.y, bbox[c].Max.x, bbox[c].Max.y, tex, false, tClr);
			x +=width[c] + dist;
		}
		pos++;
	}
	if ( pos == cursor && current_time % 1000 > 500 )
		RenderSprite(x, y, z, bbox['|'-32].Min.x, bbox['|'-32].Min.y, bbox['|'-32].Max.x, bbox['|'-32].Max.y, tex, false, tClr);
}

void CFont::Print( const char *text, ... )
{
	if (text == NULL)
		return;

	static UCHAR	*temp;//[CFONT_MAX_STRING_LENGTH];
	temp = new UCHAR [strlen(text)+CFONT_MAX_STRING_LENGTH];
	va_list	ap;
	va_start(ap, text);
	vsprintf((char*)temp, text, ap);
	va_end(ap);
	_Print(temp);
	delete [] temp;
}

void CFont::PrintMultiline( const char *text, CAABB &area, int cursor )
{
	if (text == NULL)
		return;

	size_t br = 1;				// Номер текущего символа.
	size_t back = 0;			
	int x_size = 0;
	int y_size = 0;
	float current_shift = 0;	// Сдвиг блока текста вправо. Позиция, скотрой начинаем рисовать
	float old_x = this->p.x;
	float old_y = this->p.y;
	size_t max_br;

	size_t left_text_len = 0;	// Длина строки для печати

	ControlSeqParser::CSeqType seq_type =  ControlSeqParser::ENone;
	size_t seq_shift = 0;

	size_t text_len = strlen( text );
	UCHAR *temp = new UCHAR [text_len+CFONT_MAX_STRING_LENGTH];

	while ( br <= text_len )
	{
		for (x_size = 0; x_size < 2*area.W - current_shift && br <= text_len; br++)
		{	
			seq_type = ControlSeqParser::CheckControlSeq(&text[br-1], seq_shift);
			if (seq_type != ControlSeqParser::ENone)
				break;

			x_size += width[(unsigned char)(text[br-1]-32)] + dist;
		}

		br += seq_shift;
		left_text_len = br-seq_shift-back-1;

		switch(seq_type)
		{
		case ControlSeqParser::EColor:
			{
				// Печатаем левую часть
				if (br-back-1 > 0)
				{
					memcpy(temp, text + back, left_text_len);
					temp[left_text_len] = 0;
					_Print(temp, cursor - back);
				}

				this->p.x += x_size;
				current_shift += x_size;

				ControlSeqParser::GetColor(&text[br - seq_shift - 1], this->tClr);

				back = br-1;
				continue;
			}
			break;
		case ControlSeqParser::ENewLine: break;
		default:
			{
				max_br = br;
				while ( text[br-1] != 32 && br-1 > back && br != text_len+1  )	//Пытаемся перенести по пробелу
					br--;
				if ( br-1 == back ) br = max_br; //Пробелов нет.
				
				left_text_len = br-seq_shift-back-1;
			}
		}

		seq_type = ControlSeqParser::ENone;

		memcpy(temp, text + back, left_text_len);
		temp[left_text_len] = 0;
		_Print(temp, cursor - back);

		y_size = GetStringHeight((char*)temp);

		area = CAABB( area.Left(), area.Top(), area.Right(), max( area.Bottom(), area.Top() + this->p.y - old_y + y_size  ) );
		
		this->p.y += y_size * 1.5f;
		this->p.x = old_x;
		current_shift = 0;

		back = br-1;
		while ( text[back] == 32 && back < text_len )	//Отрезаем пробелы в начале после переноса
		{
			back++;
			br++;
		}
		
	}
	DELETEARRAY(temp);
	this->p.y = old_y;
	this->p.x = old_x;
	current_shift = 0;

}

void CFont::PrintMultiline( const char *text, CAABB &area )
{
	PrintMultiline( text, area, -1 );
}


int	CFont::GetStringWidth(const char *text)
{
	if (text == NULL)
		return -1;
	int r = 0, l = (int)strlen(text);
	for (int i=0;i<l;i++)
		r += width[(unsigned char)(text[i]-32)] + dist;
	return r;
}

int CFont::GetStringWidthEx(UINT t1, UINT t2, const char *text)
{
	if (text == NULL)
		return -1;
	if (t1 > t2)
		return -1;
	if (t2 >= strlen(text))
		return -1;
	int r = 0;
	for (UINT i = t1; i <= t2; i++)
		r += width[(unsigned char)(text[i]-32)]+dist;
	return r;
}

int	CFont::GetStringHeight(const char *text)
{
	if (text == NULL)
		return -1;
	int r = 0, l = (UINT)strlen(text);
	for (int i=0;i<l;i++)
		r = max((int)height[(unsigned char)(text[i]-32)], r);
	if (l == 0) r = height[32];
	return r;
}

int CFont::GetStringHeightEx( UINT t1, UINT t2, const char *text )
{
	if (text == NULL)
		return -1;
	if (t1 > t2)
		return -1;
	if (t2 >= strlen(text))
		return -1;
	UINT r = 0, l = (UINT)strlen(text);
	for (UINT i=0; i<l; i++)
		r = max((UINT)height[(unsigned char)(text[i]-32)], r);
	return r;
}


// В этом неймспасе находится код для выделения управляющих последовательностей для отрисовки строк.
// Выделение производится с помощью автомата.
namespace ControlSeqParser
{
	// Схема автомата:
	//		[0]
	//	'/'  |
	//		[1]
	//	'c'	/ \ 'n'
	//	  [2] [3]
	//
	const int states_count = 5;				// Коилчство состояний. Состояние states_count-1 всегда - ошибка.
	const size_t classes_count = 4;			// Количество классов символов
	const CSeqType seqs[states_count] = {ENone, ENone, EColor, ENewLine, ENone};	// Состветсиве номеров состояний типам контрольных последовательностей
	const bool end_state[states_count] = {false, false, true, true, false};		// Окончательность состяния
	const char classes[classes_count] = {'/', 'c', 'n', '\0'};					// Классы
	const int table[states_count][classes_count] = {	// Таблица переходов автомата. Строки - текущие состояния, столбцы - классы, в таблице - номер следующего состояния.
		{1, 4, 4, 4},
		{4, 2, 3, 4},
		{4, 4, 4, 4},
		{4, 4, 4, 4},
		{4, 4, 4, 4} };
	
	const size_t seq_len[] = {0, 8, 2};		// Длины контрольных последовательностей для сдвига в строке. Индексы сосответствуют CSeqType.

	// Возвращает номер класса символа
	size_t GetClass(char c)
	{
		for (size_t i = 0; i < classes_count; i++)
		{
			if (classes[i] == c)
				return i;
		}
		return classes_count-1;
	}

	// Код автомата, выделяет последовательность и проверяет, чтоы она входила по длине до конца строки
	// В shift возвращает номер следующего после последовательности символа.
	CSeqType CheckControlSeq(const char* text, size_t& shift)
	{
		shift = 0;
		int s = 0;
		size_t i;
		for (i = 0; text[i] != 0 && !end_state[s]; i++)
		{
			s = table[s][GetClass(text[i])];
			if (s == states_count - 1)
				break;
		}

		CSeqType t = seqs[s];
		if (t == ENone)
		{
			return ENone;
		}

		for (; text[i] != 0 && i < seq_len[t]; i++);

		if (i == seq_len[t])
		{
			shift = i;
			return t;
		}
		
		return ENone;
	}

	// Парсит последовательность задания цвета и заносит в color закодированный цвет
	void GetColor(const char* text, RGBAf& color)
	{
		ASSERT(text[0] == '/' && text[1] == 'c');

		char number[3] = "\0\0";
		size_t shift = 2;
		for (int i = 0; i < 3; i++, shift+=2)
		{
			number[0] = text[shift];
			number[1] = text[shift+1];
			color.v[i] = strtoul(number, NULL, 16) / 255.0f;
		}
	}
};
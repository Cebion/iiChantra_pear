#ifndef _TEXT_TYPER_H__
#define _TEXT_TYPER_H__

#include "../render/font.h"
#include "../script/timerevent.h"


// Этот класс подготовливает заданную строку так, что она выводится на экран постепенно.
// Выводом он не занимается.
// Используется игровой таймер из timerevent.
// В исходную строку вставляется разделитель '\0' на позицию 0. Затем он последовательно
// сдвигается вперед через время period. Всего таких сдвигов должно получиться max_pos. 
class TextTyper : ITimerEventPerformer
{
	UINT period;					// Период вывода символов	
	UINT timerEventId;				// Ид события таймера
	char* buffer;					// Измененая строка

	size_t cur_pos;					// Текущее положение разделителя видимой и не видимой части
	size_t max_pos;					// Конец строки - там должен будет встать разделитель

	bool ended;						// Завершил вывод

	void Init()
	{
		timerEventId = 0;
		buffer = NULL;
		cur_pos = 0;
		max_pos = 0;
		ended = false;
		onTyperEnded = LUA_NOREF;
		pausable = false;
	}

	void PrepareText(const char* text)
	{
		if (text)
		{
			max_pos = strlen(text);
			buffer = new char[max_pos+2];
			buffer[0] = 0;
			memcpy(buffer+1, text, max_pos+1);
		}
		else
		{
			sLog(DEFAULT_LOG_NAME, logLevelWarning, "Создание TextTyper без текста");
		}
	}

public:
	int onTyperEnded;				// Ссылка на обработчик события того, что тайпер завершил вывод
	bool pausable;					// Признак того, что работа приостанавливается во время паузы.

	TextTyper(const char* text, UINT _period, bool _pausable = false)
		: period(_period), pausable(_pausable)
	{
		Init();

		PrepareText(text);
	}

	TextTyper(const char* text, const TextTyper& old_typer)
	{
		Init();
		PrepareText(text);
		period = old_typer.GetPeriod();
		pausable = old_typer.pausable;

		if (old_typer.onTyperEnded >= 0)
		{
			extern lua_State* lua;
			STACK_CHECK_INIT(lua);
			SCRIPT::GetFromRegistry(lua, old_typer.onTyperEnded);
			SCRIPT::RegProc(lua, &this->onTyperEnded, -1);
			STACK_CHECK(lua);
		}

		if (old_typer.IsStarted()) this->Start();
	}

	~TextTyper()
	{
		DELETEARRAY(buffer);

		SCRIPT::RemoveFromRegistry(onTyperEnded);

		if (timerEventId)
		{
			DeleteTimerEventById(timerEventId);
		}		
}

	void Start();
	void Stop();

	virtual void OnTimer(InternalTimerEvent& ev);
	virtual void OnTimetEventDestroy(const InternalTimerEvent& ev);
	
	UINT GetPeriod() const { return period; }
	void SetPeriod (UINT val) 
{ 
		period = val;
		if (timerEventId)
			Start();
	}

	bool IsStarted() const { return timerEventId > 0; }
	
	const char* GetBuffer() const { return buffer; }

};


#endif // _TEXT_TYPER_H__
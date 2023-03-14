#include "StdAfx.h"


#include "text_typer.h"

extern lua_State* lua;

void TextTyper::Start()
{
	if (!buffer)
	{
		sLog(DEFAULT_LOG_NAME, logLevelWarning, "Попытка запустить TextTyper без текста");
		return;
	}

	if (timerEventId)
		DeleteTimerEventById(timerEventId);

	if (ended)
		return;

	timerEventId = AddInternalTimerEvent(this, 0, period, max_pos - cur_pos, true, pausable);
}

void TextTyper::Stop()
{
	if (timerEventId)
	{
		DeleteTimerEventById(timerEventId);
	}
	else
	{
		sLog(DEFAULT_LOG_NAME, logLevelWarning, "Попытка остановить не запущенный TextTyper");
	}
}

void TextTyper::OnTimer(InternalTimerEvent& ev)
{
	ASSERT(buffer);
	size_t new_pos = cur_pos+1;
	size_t shift = 0;
	
	// Пропускаем все идущие подряд контрольные последовательности (иначе проскакивают символы '/')
	while(ControlSeqParser::CheckControlSeq(&buffer[new_pos], shift) != ControlSeqParser::ENone)
		new_pos += shift;
	
	for (size_t i = cur_pos; i <= max_pos; i++)
	{
		if (i < new_pos)
		{
			buffer[i] = buffer[i+1];
		}
		else if (i == new_pos)
		{
			buffer[i] = 0;
		}
		else
			break;
	}

	cur_pos = new_pos;

	if (cur_pos == max_pos)
	{
		ev.ClearPeriodic();
		ended = true;

		if (this->onTyperEnded >= 0)
		{
			if (SCRIPT::ExecChunkFromReg(this->onTyperEnded, 0))
			{
				// В скрипте произошла какая-то ошибка.
				sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "В обработчике onTyperEnded()");
			}
		}
	}
}

void TextTyper::OnTimetEventDestroy(const InternalTimerEvent& ev)
{	
	ASSERT(timerEventId);
	ASSERT(timerEventId == ev.id);
	timerEventId = 0;
}
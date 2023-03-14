#include "StdAfx.h"

#include "luathread.h"
#include "timerevent.h"
#include "../game/game.h"

//////////////////////////////////////////////////////////////////////////

extern UINT internal_time;
extern UINT last_timerevent_tick;

//////////////////////////////////////////////////////////////////////////

UINT TimerEvent::idCounter;

//////////////////////////////////////////////////////////////////////////
// Список событий
list <TimerEvent*> timerEvents;
UINT nbPausableEvents = 0;

typedef list<TimerEvent*>::iterator TEvIter;

// События в списке всегда отсортированы так, что в начале находятся
// те, что наступят раньше всего. Таким образом, достаточно смотреть только
// первое событие, если оно не наступило, то другие тоже еще не наступили.


// Добавление события в массив createdEvents и сортировка массива
void AddTimerEvent(TimerEvent* te)
{
	if (te->IsPausable())
		nbPausableEvents++;

	// Ищем событие, которое наступит позже, чем добавляемое.
	for(TEvIter it = timerEvents.begin();
		it != timerEvents.end();
		it++)
	{
		if (te->execTime < (*it)->execTime)
		{
			// Вставляем перед найденным событием
			timerEvents.insert(it, te);
			return;
		}
	}
	// Событие произойдет позже всех, добавляем в конец
	timerEvents.push_back(te);
}

// Добавление периодического события
// dt - время, черз которое событие сработает
// action - номер скрипта в реестре
// period - период повторения после первого срабатываения
// maxCalls - количество срабатываний
UINT AddTimerEvent(UINT dt, int action, UINT period, UINT maxCalls)
{
	TimerEvent* te = new TimerEvent();

	te->action = action;
	te->execTime = internal_time + dt;

	te->SetPeriodic();
	te->period = period;
	te->maxCalls = maxCalls;

	AddTimerEvent(te);
	return 1;
}

// Добавление события
// dt - время, черз которое событие сработает
// action - номер скрипта в реестре
UINT AddTimerEvent(UINT dt, int action)
{
	TimerEvent* te = new TimerEvent();

	te->action = action;
	te->execTime = internal_time + dt;

	AddTimerEvent(te);

	return 1;
}

// Добавлеят событие восстановления приостановленного потока
void AddThreadTimerEvent(UINT dt, int action, bool pausable)
{
	TimerEvent* te = new TimerEvent();
	te->action = action;
	te->execTime = internal_time + dt;
	te->SetThreadResume();
	if (pausable) te->SetPausable();

	AddTimerEvent(te);
}


int AddInternalTimerEvent(ITimerEventPerformer* performer, UINT dt, UINT period, UINT max_Calls, bool periodic, bool pausable)
{
	InternalTimerEvent* te = new InternalTimerEvent();
	te->performer = performer;
	te->execTime = internal_time + dt;
	if (periodic)
	{
		te->SetPeriodic();
		te->period = period;
		te->maxCalls = max_Calls;
	}
	if (pausable) te->SetPausable();

	AddTimerEvent(te);
	
	return te->id;
}


// Удаление события таймера с заданным action
int DeleteTimerEvent(int action)
{
	// Посик ближайшего подходящего события и его удаление
	TimerEvent* t = NULL;
	TEvIter it;
	for(it = timerEvents.begin();
		it != timerEvents.end();
		it++)
	{
		t = *it;
		if (t->action == action)
			break;
	}

	if (it != timerEvents.end())
	{
		timerEvents.erase(it);
		if (t->IsPausable())
		{
			ASSERT(nbPausableEvents);
			nbPausableEvents--;
		}
		DELETESINGLE(t);
		return 1;
	}
	return 0;
	// TODO: возможно стоит обходить весь список, а не удалять первое ближайшее
	// подходящее событие. Вот код из библиотеки для удаелния нескольких элементов.
	//iterator _Last = end();
	//for (iterator _First = begin(); _First != _Last; )
	//	if ((*_First)->)
	//		_First = erase(_First);
	//	else
	//		++_First;
}

// Удаление события таймера с заданным action
bool DeleteTimerEventById(UINT id)
{
	// Посик ближайшего подходящего события и его удаление
	TimerEvent* t = NULL;
	TEvIter it;
	for(it = timerEvents.begin();
		it != timerEvents.end();
		it++)
	{
		t = *it;
		if (t->id == id)
			break;
	}

	if (it != timerEvents.end())
	{
		timerEvents.erase(it);
		if (t->IsPausable())
		{
			ASSERT(nbPausableEvents);
			nbPausableEvents--;
		}
		DELETESINGLE(t);
		return true;
	}
	return false;
}

// Удаляет все события запуска тредов
void DeleteAllThreadEvents()
{
	TimerEvent* t = NULL;
	TEvIter it;
	for(it = timerEvents.begin(); it != timerEvents.end(); )
	{
		t = *it;
		if (t->IsThreadResume())
		{
			DELETESINGLE(t);
			it = timerEvents.erase(it);
		}
		else
		{
			++it;
		}
	}
}

// Удаляет все события из массива
void DeleteAllEvents()
{
	for(TEvIter it = timerEvents.begin();
		it != timerEvents.end();
		it++)
	{
		DELETESINGLE(*it);
	}

	timerEvents.clear();
}


// Обработка событий таймера
// Вызывается минимум через каждые TIMEREVENTTICK милисекунд
void ProcessTimerEvents()
{
	if (timerEvents.empty())
		return;

	TimerEvent* first = NULL;

	if (nbPausableEvents && game::GetPause())
	{
		// К execTime каждого приостановленного события добавится shift
		UINT shift = internal_time - last_timerevent_tick;


		list<TimerEvent*>::reverse_iterator rit = timerEvents.rbegin();
		list<TimerEvent*>::reverse_iterator prev_rit = rit;
		list<TimerEvent*>::reverse_iterator temp_rit = rit;
		// Поиск приостановленных событий с конца
		for(; rit != timerEvents.rend(); prev_rit = rit, rit++)
		{
			if (!(*rit)->IsPausable())
				continue;
			
			first = *rit;
			first->execTime += shift;
	
			temp_rit = rit;
			// Перенос события так, чтобы список оставался отсортированным
			while((*prev_rit)->execTime < first->execTime)
			{
				*temp_rit = *prev_rit;
				*prev_rit = first;
				if (prev_rit == timerEvents.rbegin())
					break;
				temp_rit = prev_rit;
				prev_rit--;				
			}
		}
	}

	while (!timerEvents.empty())
	{
		first = *(timerEvents.begin());	// Указатель на ближайшее событие

		if (first->execTime > internal_time)
			break;							// Время еще не пришло

		ASSERT((!first->IsPausable() && game::GetPause()) || !game::GetPause());

		first->ExecEvent();

		if (timerEvents.empty() || 
			first != *(timerEvents.begin()))
		{
			// Первое событие уже удалили где-то в его обработчике
			continue;
		}

		if (first->IsPeriodic())
		{
			first->calls++;
			if (first->calls >= first->maxCalls - 1)
			{
				// Предпоследний вызов периодического события
				// В следующий раз оно уже вызовется как обыкновенное и исчезнет
				first->ClearPeriodic();
			}

			first->execTime = internal_time + first->period;
			// Удаляем периодическое событие и опять добавляем его в массив
			timerEvents.pop_front();
			AddTimerEvent(first);

		}
		else
		{
			// Удаляем отработавшее событие
			timerEvents.pop_front();
			DELETESINGLE(first);
		}
	}

}


// Исполняет настпуившее событие.
void TimerEvent::ExecEvent()
{
	if (this->IsThreadResume())
	{
		ProcessThread(this->action);
	}
	else
	{
		SCRIPT::ExecChunkFromReg(this->action);
	}
}

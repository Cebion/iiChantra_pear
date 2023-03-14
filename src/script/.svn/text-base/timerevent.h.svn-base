#ifndef __TIMEEVENT_H_
#define __TIMEEVENT_H_

#include "script.h"

class TimerEvent
{
	static UINT idCounter;

public:
	UINT id;					// Возможно будет использоваться для управления из скрипта.

	int action;					// Ссылка на реестр луа

	UINT execTime;				// Время наступления события

	UINT period;				// Период периодического события
	UINT maxCalls;				// Количество срабатыванй 

	UINT calls;					// Текущее количество срабатываний

	UINT flags;	
	// 1 - threadResume				данное событие - запуск приостановленной coroutine
	// 2 - periodic					событие переодическое
	// 4 - pausable					на время игровой паузы прекращается обработка этого события, его время "замораживается"
									// и продолжится после снятия игровой паузы

	__INLINE BYTE IsThreadResume()		{ return flags & 1; }
	__INLINE void SetThreadResume()		{ flags |= 1; }
	__INLINE void ClearThreadResume()	{ flags &= ~1; }

	__INLINE BYTE IsPeriodic()		{ return flags & 2; }
	__INLINE void SetPeriodic()		{ flags |= 2; }
	__INLINE void ClearPeriodic()	{ flags &= ~2; }

	__INLINE BYTE IsPausable()		{ return flags & 4; }
	__INLINE void SetPausable()		{ flags |= 4; }
	__INLINE void ClearPausable()	{ flags &= ~4; }

	virtual void ExecEvent();

	TimerEvent()
	{
		id = ++idCounter;
		action = 0;
		execTime = 0;
		flags = 0;
		period = 0;
		maxCalls = 0;
		calls = 0;
	}

	virtual ~TimerEvent()
	{
		//DELETEARRAY(action);
		if (!IsThreadResume())
			SCRIPT::RemoveFromRegistry(action);
	}

};


class InternalTimerEvent;
// Интерфейс для клссов, которые хотят подвписаться на события таймера.
class ITimerEventPerformer
{
public:
	// Возникает при наступлении событии таймера.
	virtual void OnTimer(InternalTimerEvent& ev) = 0;
	// Возникает при удалении TimerEvent
	virtual void OnTimetEventDestroy(const InternalTimerEvent& ev) = 0;
};

// Событие таймера для кода движка. Посылает событие таймера создавшему его объекту.
// Объект этого класса хранит указатель на объект подписавшиегося на событие, так что 
// подписавшивася доложен нормально удалить событие.
class InternalTimerEvent : public TimerEvent
{
public:
	ITimerEventPerformer* performer;	// Указатель на объект, которому посылаются события.

	virtual void ExecEvent()
	{
		performer->OnTimer(*this);
	}


	virtual ~InternalTimerEvent()
	{
		performer->OnTimetEventDestroy(*this);
	}
};

UINT AddTimerEvent(UINT dt, int action, UINT period, UINT maxCalls);
UINT AddTimerEvent(UINT dt, int action);
void AddThreadTimerEvent(UINT dt, int action, bool pausable);
int AddInternalTimerEvent(ITimerEventPerformer* performer, UINT dt, UINT period, UINT max_Calls, bool periodic, bool pausable);
void DeleteAllEvents();
void DeleteAllThreadEvents();
int DeleteTimerEvent(int action);
bool DeleteTimerEventById(UINT id);

void ProcessTimerEvents();

#define TIMEREVENTTICK 10


#endif // __TIMEEVENT_H_
#ifndef __TIMEEVENT_H_
#define __TIMEEVENT_H_

#include "script.h"

class TimerEvent
{
	static UINT idCounter;

public:
	UINT id;					// �������� ����� �������������� ��� ���������� �� �������.

	int action;					// ������ �� ������ ���

	UINT execTime;				// ����� ����������� �������

	UINT period;				// ������ �������������� �������
	UINT maxCalls;				// ���������� ����������� 

	UINT calls;					// ������� ���������� ������������

	UINT flags;	
	// 1 - threadResume				������ ������� - ������ ���������������� coroutine
	// 2 - periodic					������� �������������
	// 4 - pausable					�� ����� ������� ����� ������������ ��������� ����� �������, ��� ����� "��������������"
									// � ����������� ����� ������ ������� �����

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
// ��������� ��� ������, ������� ����� ������������ �� ������� �������.
class ITimerEventPerformer
{
public:
	// ��������� ��� ����������� ������� �������.
	virtual void OnTimer(InternalTimerEvent& ev) = 0;
	// ��������� ��� �������� TimerEvent
	virtual void OnTimetEventDestroy(const InternalTimerEvent& ev) = 0;
};

// ������� ������� ��� ���� ������. �������� ������� ������� ���������� ��� �������.
// ������ ����� ������ ������ ��������� �� ������ ��������������� �� �������, ��� ��� 
// �������������� ������� ��������� ������� �������.
class InternalTimerEvent : public TimerEvent
{
public:
	ITimerEventPerformer* performer;	// ��������� �� ������, �������� ���������� �������.

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
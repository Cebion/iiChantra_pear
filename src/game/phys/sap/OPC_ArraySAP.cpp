///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains an array-based version of the sweep-and-prune algorithm
 *	\file		OPC_ArraySAP.cpp
 *	\author		Pierre Terdiman
 *	\date		December, 2, 2007
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Precompiled Header
#include "StdAfx.h"
//#include "SweepAndPrune.h"

#include "../phys_misc.h"


#include "OPC_ArraySAP.h"



#if defined(DEBUG_SAP) || defined(DEBUG_RAYSAP)
#include "../../../misc.h"
#define TRACEMSG(msg, x1, x2) {sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "%s %d, %d", msg, x1, x2);}
#define TRACEMSG2(msg, x1, x2, p1, p2) {sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "%s %d, %d - 0x%p, 0x%p", msg, x1, x2, p1, p2);}
#else
#define TRACEMSG(msg, x1, x2)
#define TRACEMSG2(msg, x1, x2, p1, p2)
#endif // DEBUG_SAP


using namespace IceCore;
using namespace Opcode;

//#include "SAP_Utils.h"


struct Opcode::IAABB : public Allocateable
{
	udword mMinX;
	udword mMinY;
	udword mMinZ;
	udword mMaxX;
	udword mMaxY;
	udword mMaxZ;

	inline_ udword	GetMin(udword i)	const	{	return (&mMinX)[i];	}
	inline_ udword	GetMax(udword i)	const	{	return (&mMaxX)[i];	}
};



/*
	- already sorted for batch create?
	- better axis selection batch create
*/

//#define USE_WORDS		// Use words or dwords for box indices. Words save memory but seriously limit the max number of objects in the SAP.
#define USE_PREFETCH
//#define USE_INTEGERS

#define USE_OVERLAP_TEST_ON_REMOVES	// "Useless" but faster overall because seriously reduces number of calls (from ~10000 to ~3 sometimes!)
#define RELEASE_ON_RESET	// Release memory instead of just doing a reset

#include "OPC_ArraySAP.h"

struct Opcode::CreateData
{
	udword	mHandle;
	ASAP_AABB	mBox;
};

ArraySAP::ArraySAP()
{
	mNbBoxes	= 0;
	mMaxNbBoxes	= 0;
#ifdef KERNEL_BUG_ASAP_FIX
	mNbAddedBoxes = 0;
	mNbUsedBoxes = 0;
#endif // KERNEL_BUG_ASAP_FIX
	mBoxes		= null;
	mEndPoints[0] = mEndPoints[1] = mEndPoints[2] = null;
	mFirstFree	= INVALID_ID;
}

ArraySAP::~ArraySAP()
{
	mNbBoxes	= 0;
	mMaxNbBoxes	= 0;
#ifdef KERNEL_BUG_ASAP_FIX
	mNbUsedBoxes = 0;
#endif // KERNEL_BUG_ASAP_FIX
	DELETEARRAY(mBoxes);
	for(udword i=0;i<3;i++)
	{
		DELETEARRAY(mEndPoints[i]);
	}
}

void ArraySAP::ResizeBoxArray()
{
	const udword NewMaxBoxes = mMaxNbBoxes ? mMaxNbBoxes*2 : 64;

	ASAP_Box* NewBoxes = ICE_NEW_TMP(ASAP_Box)[NewMaxBoxes];
	const udword NbSentinels=2;
	ASAP_EndPoint* NewEndPointsX = ICE_NEW_TMP(ASAP_EndPoint)[NewMaxBoxes*2+NbSentinels];
	ASAP_EndPoint* NewEndPointsY = ICE_NEW_TMP(ASAP_EndPoint)[NewMaxBoxes*2+NbSentinels];
	ASAP_EndPoint* NewEndPointsZ = ICE_NEW_TMP(ASAP_EndPoint)[NewMaxBoxes*2+NbSentinels];

	if(mNbBoxes)
	{
		// Копируем весь массив боксов (в том числе и уже удаленые, если они есть)
		CopyMemory(NewBoxes, mBoxes, sizeof(ASAP_Box)*mNbBoxes);
		// Копируем массивы КТ по осям. Их в 2 р. больше, чем реального количества боксов + 2 сентинела.
#ifdef KERNEL_BUG_ASAP_FIX
		CopyMemory(NewEndPointsX, mEndPoints[0], sizeof(ASAP_EndPoint)*(mNbAddedBoxes*2+NbSentinels));
		CopyMemory(NewEndPointsY, mEndPoints[1], sizeof(ASAP_EndPoint)*(mNbAddedBoxes*2+NbSentinels));
		CopyMemory(NewEndPointsZ, mEndPoints[2], sizeof(ASAP_EndPoint)*(mNbAddedBoxes*2+NbSentinels));
#else
		CopyMemory(NewEndPointsX, mEndPoints[0], sizeof(ASAP_EndPoint)*(mNbBoxes*2+NbSentinels));
		CopyMemory(NewEndPointsY, mEndPoints[1], sizeof(ASAP_EndPoint)*(mNbBoxes*2+NbSentinels));
		CopyMemory(NewEndPointsZ, mEndPoints[2], sizeof(ASAP_EndPoint)*(mNbBoxes*2+NbSentinels));
#endif // KERNEL_BUG_ASAP_FIX
	}
	else
	{
		// Initialize sentinels
#ifdef USE_INTEGERS
		const udword Min = EncodeFloat(MIN_FLOAT);
		const udword Max = EncodeFloat(MAX_FLOAT);
#else
		const float Min = MIN_FLOAT;
		const float Max = MAX_FLOAT;
#endif
		NewEndPointsX[0].SetData(Min, INVALID_INDEX, FALSE);
		NewEndPointsX[1].SetData(Max, INVALID_INDEX, TRUE);
		NewEndPointsY[0].SetData(Min, INVALID_INDEX, FALSE);
		NewEndPointsY[1].SetData(Max, INVALID_INDEX, TRUE);
		NewEndPointsZ[0].SetData(Min, INVALID_INDEX, FALSE);
		NewEndPointsZ[1].SetData(Max, INVALID_INDEX, TRUE);
	}
	DELETEARRAY(mBoxes);
	DELETEARRAY(mEndPoints[2]);
	DELETEARRAY(mEndPoints[1]);
	DELETEARRAY(mEndPoints[0]);
	mBoxes = NewBoxes;
	mEndPoints[0] = NewEndPointsX;
	mEndPoints[1] = NewEndPointsY;
	mEndPoints[2] = NewEndPointsZ;

	mMaxNbBoxes = NewMaxBoxes;
}

inline_ BOOL Intersect(const IAABB& a, const IAABB& b, udword axis)
{
	if(b.GetMax(axis) < a.GetMin(axis) || a.GetMax(axis) < b.GetMin(axis))	return FALSE;
	return TRUE;
}

// ### TODO: the sorts here might be useless, as the values have been sorted already
bool ArraySAP::CompleteBoxPruning2(udword nb, const IAABB* array, const Axes& axes, const CreateData* batched)
{
	// nb - количество боксов, которые сталкиваем
	// array - массив индексов КТ сталкиваемых боксов
	// axes - оси
	// batched - массив данных  о новых боксах
	// Checkings
	if(!nb || !array)	return false;

	// Catch axes
	const udword Axis0 = axes.mAxis0;
	const udword Axis1 = axes.mAxis1;
	const udword Axis2 = axes.mAxis2;

	// Allocate some temporary data
	// Массив минимальных КТ по главной оси
	udword* PosList = (udword*)ICE_ALLOC_TMP(sizeof(udword)*(nb+1));

	// 1) Build main list using the primary axis
	for(udword i=0;i<nb;i++)	PosList[i] = array[i].GetMin(Axis0);
//PosList[nb++] = ConvertToSortable(MAX_FLOAT);

	// 2) Sort the list
//	static RadixSort r;
	RadixSort r;
	RadixSort* RS = &r;

//	const udword* Sorted = RS->Sort(PosList, nb, RADIX_SIGNED).GetRanks();
	// Массив индексов отсотритрованных минКТ по глав оси
	// НАдо сортировать, потму что они илут в том порядке, в котором создавались боксы
	const udword* Sorted = RS->Sort(PosList, nb, RADIX_UNSIGNED).GetRanks();

	// 3) Prune the list
	const udword* const LastSorted = &Sorted[nb];	// Адресс конца массива (вернее массив закончился на прошлом элементе)
	const udword* RunningAddress = Sorted;
	udword Index0, Index1;
	while(RunningAddress<LastSorted && Sorted<LastSorted)		// Пока не вышли за границы массива
	{
		Index0 = *Sorted++;

		// Ищем следующую большую КТ или конец массива добавленных КТ
		while(RunningAddress<LastSorted && PosList[*RunningAddress++]<PosList[Index0]);
//		while(PosList[*RunningAddress++]<PosList[Index0]);

		if(RunningAddress<LastSorted)
		{
			const udword* RunningAddress2 = RunningAddress;

			// Ищем, находится ли между мин. и макс. точками бокса Index0 мин. точка других боксов Index1
			// Если такие находятся, значит на этой оси боксы Index0 и Index1 пересекаются
			while(RunningAddress2<LastSorted && PosList[Index1 = *RunningAddress2++]<=array[Index0].GetMax(Axis0))
//			while(PosList[Index1 = *RunningAddress2++]<=(udword)ConvertToSortable(array[Index0].GetMax(Axis0)))
			{
				if(Intersect(array[Index0], array[Index1], Axis1))
				{
					if(Intersect(array[Index0], array[Index1], Axis2))
					{
						// ПРойдена проверка по остальным двум осям, объекты пересекаются
						const ASAP_Box* Box0 = mBoxes + batched[Index0].mHandle;
						const ASAP_Box* Box1 = mBoxes + batched[Index1].mHandle;
						assert(Box0->mGUID != 65535);
						assert(Box1->mGUID != 65535);
						AddPair(Box0->mObject, Box1->mObject, Box0->mGUID, Box1->mGUID);
					}
				}
			}
		}
	}

	ICE_FREE(PosList);
	return true;
}

bool ArraySAP::BipartiteBoxPruning2(udword nb0, const IAABB* array0, udword nb1, const IAABB* array1, const Axes& axes, const CreateData* batched, const udword* box_indices)
{
	// nb0 - количество новых боксов
	// Массив индексов КТ новых боксов
	// nb1 - Кол-во старых боксов
	// array1 - Массив КТ старых боксов
	// axes - оси
	// batched - массив данных о новых боксах
	// box_indices - масив индексов старых боксов
	// Checkings
	if(!nb0 || !array0 || !nb1 || !array1)	return false;

	// Catch axes
	const udword Axis0 = axes.mAxis0;
	const udword Axis1 = axes.mAxis1;
	const udword Axis2 = axes.mAxis2;

	// Allocate some temporary data
	udword* MinPosList0 = (udword*)ICE_ALLOC_TMP(sizeof(udword)*nb0);
	udword* MinPosList1 = (udword*)ICE_ALLOC_TMP(sizeof(udword)*nb1);

	// 1) Build main lists using the primary axis
	for(udword i=0;i<nb0;i++)	MinPosList0[i] = array0[i].GetMin(Axis0);
	for(udword i=0;i<nb1;i++)	MinPosList1[i] = array1[i].GetMin(Axis0);

	// 2) Sort the lists
/*	static RadixSort r0;
	static RadixSort r1;
	RadixSort* RS0 = &r0;
	RadixSort* RS1 = &r1;*/
	RadixSort r0;
	RadixSort r1;
	RadixSort* RS0 = &r0;
	RadixSort* RS1 = &r1;

	const udword* Sorted0 = RS0->Sort(MinPosList0, nb0, RADIX_UNSIGNED).GetRanks();
	const udword* Sorted1 = RS1->Sort(MinPosList1, nb1, RADIX_UNSIGNED).GetRanks();

	// 3) Prune the lists
	udword Index0, Index1;

	const udword* const LastSorted0 = &Sorted0[nb0];
	const udword* const LastSorted1 = &Sorted1[nb1];
	const udword* RunningAddress0 = Sorted0;
	const udword* RunningAddress1 = Sorted1;

	while(RunningAddress1<LastSorted1 && Sorted0<LastSorted0)
	{
		Index0 = *Sorted0++;

		while(RunningAddress1<LastSorted1 && MinPosList1[*RunningAddress1]<MinPosList0[Index0])	RunningAddress1++;

		const udword* RunningAddress2_1 = RunningAddress1;

		while(RunningAddress2_1<LastSorted1 && MinPosList1[Index1 = *RunningAddress2_1++]<=array0[Index0].GetMax(Axis0))
		{
			if(Intersect(array0[Index0], array1[Index1], Axis1))
			{
				if(Intersect(array0[Index0], array1[Index1], Axis2))
				{
					const ASAP_Box* Box0 = mBoxes + batched[Index0].mHandle;
					const ASAP_Box* Box1 = mBoxes + box_indices[Index1];
					assert(Box0->mGUID != 65535);
					assert(Box1->mGUID != 65535);
					// Вот здесь и обнаруживаются удаленые боксы, если они попали в список старых боксов
					assert(Box1->mGUID != INVALID_ID);
					AddPair(Box0->mObject, Box1->mObject, Box0->mGUID, Box1->mGUID);
				}
			}
		}
	}

	////

	while(RunningAddress0<LastSorted0 && Sorted1<LastSorted1)
	{
		Index0 = *Sorted1++;

		while(RunningAddress0<LastSorted0 && MinPosList0[*RunningAddress0]<=MinPosList1[Index0])	RunningAddress0++;

		const udword* RunningAddress2_0 = RunningAddress0;

		while(RunningAddress2_0<LastSorted0 && MinPosList0[Index1 = *RunningAddress2_0++]<=array1[Index0].GetMax(Axis0))
		{
			if(Intersect(array0[Index1], array1[Index0], Axis1))
			{
				if(Intersect(array0[Index1], array1[Index0], Axis2))
				{
					const ASAP_Box* Box0 = mBoxes + batched[Index1].mHandle;
					const ASAP_Box* Box1 = mBoxes + box_indices[Index0];
					assert(Box0->mGUID != 65535);
					assert(Box1->mGUID != 65535);
					// Вот здесь и обнаруживаются удаленые боксы, если они попали в список старых боксов
					assert(Box1->mGUID != INVALID_ID);
					AddPair(Box0->mObject, Box1->mObject, Box0->mGUID, Box1->mGUID);
				}
			}

		}
	}

	ICE_FREE(MinPosList0);
	ICE_FREE(MinPosList1);
	return true;
}

// В этой функции создается запрос добавления нового объекта в SAP. Реальное добавление
// произойдет в BatchCreate. Здесь же только определеяется id в массиве и вызывается 
// увеличение выделенной под массив памяти при необходимости.
udword ArraySAP::AddObject(void* object, uword guid, const ASAP_AABB& box)
{
	assert(!(size_t(object)&3));	// We will use the 2 LSBs

#ifdef _DEBUG
	int a = sizeof(ASAP_Box);		// 32
	int b = sizeof(ASAP_EndPoint);	// 8
#endif


	TRACEMSG(">>>>>>>>>> nb = %d, bnu = %d", mNbBoxes, mNbUsedBoxes);

	// Определение свободной позиции в массиве и вызов перераспределения памяти (при необходимости)
	udword BoxIndex;
	if(mFirstFree!=INVALID_ID)
	{
		// Добавление внутрь массива на место удаленного бокса
		BoxIndex = mFirstFree;
		mFirstFree = mBoxes[BoxIndex].mGUID;
#ifdef KERNEL_BUG_ASAP_FIX
		assert(mNbBoxes > mNbAddedBoxes);
		if (mNbBoxes == mNbUsedBoxes) mNbBoxes++;
		mNbAddedBoxes++;
#endif // KERNEL_BUG_ASAP_FIX
	}
	else
	{
		// Добавление в конец масива
		if(mNbBoxes==mMaxNbBoxes)
			ResizeBoxArray();
		BoxIndex = mNbBoxes;
#ifdef KERNEL_BUG_ASAP_FIX
		assert(mNbBoxes == mNbAddedBoxes);
		mNbBoxes++;
		mNbAddedBoxes++;
#endif // KERNEL_BUG_ASAP_FIX
	}

	TRACEMSG("<<<<<<<<<<< nb, bna = ", mNbBoxes, mNbAddedBoxes);

	ASAP_Box* Box = &mBoxes[BoxIndex];
	// Initialize box
	Box->mObject	= object;
	Box->mGUID		= guid;
	for(udword i=0;i<3;i++)
	{
		Box->mMin[i] = INVALID_INDEX;
		Box->mMax[i] = INVALID_INDEX;
	}

#ifndef KERNEL_BUG_ASAP_FIX
	mNbBoxes++;
#endif // KERNEL_BUG_ASAP_FIX

	CreateData* CD = (CreateData*)mCreated.Reserve(sizeof(CreateData)/sizeof(udword));
	CD->mHandle = BoxIndex;
	CD->mBox = box;

	return BoxIndex;
}

void ArraySAP::InsertEndPoints(udword axis, const ASAP_EndPoint* end_points, udword nb_endpoints)
{
	ASAP_EndPoint* const BaseEP = mEndPoints[axis];

#ifdef KERNEL_BUG_ASAP_FIX
	const udword OldSize = mNbAddedBoxes*2 - nb_endpoints;
	const udword NewSize = mNbAddedBoxes*2;
#else
	const udword OldSize = mNbBoxes*2 - nb_endpoints;
	const udword NewSize = mNbBoxes*2;
#endif // KERNEL_BUG_ASAP_FIX
	//assert(BaseEP[OldSize + 1].mValue == MAX_FLOAT);
	BaseEP[NewSize + 1] = BaseEP[OldSize + 1];

	sdword WriteIdx = NewSize;
	udword CurrInsIdx = 0;

	const ASAP_EndPoint* First = &BaseEP[0];
	const ASAP_EndPoint* Current = &BaseEP[OldSize];
	while(Current>=First)
	{
		const ASAP_EndPoint& Src = *Current;
		const ASAP_EndPoint& Ins = end_points[CurrInsIdx];

		// We need to make sure we insert maxs before mins to handle exactly equal endpoints correctly
		const bool ShouldInsert = Ins.IsMax() ? (Src.mValue <= Ins.mValue) : (Src.mValue < Ins.mValue);

		const ASAP_EndPoint& Moved = ShouldInsert ? Ins : Src;
		BaseEP[WriteIdx] = Moved;
		mBoxes[Moved.GetOwner()].mMin[axis + Moved.IsMax()] = WriteIdx--;

		if(ShouldInsert)
		{
			CurrInsIdx++;
			if(CurrInsIdx >= nb_endpoints)
				break;//we just inserted the last endpoint
		}
		else
		{
			Current--;
		}
	}
	//assert(BaseEP[NewSize + 1].mValue == MAX_FLOAT);
}

// В этой функции происходит реальное добавление новых объектов в массив и их правильное
// размещение на осях.
void ArraySAP::BatchCreate()
{
	udword NbBatched = mCreated.GetNbEntries();
	if(!NbBatched)	return;	// Early-exit if no object has been created
	NbBatched /= sizeof(CreateData)/sizeof(udword);
	const CreateData* Batched = (const CreateData*)mCreated.GetEntries();
	mCreated.Reset();

	{
#ifdef KERNEL_BUG_ASAP_FIX
	mNbUsedBoxes = mNbAddedBoxes;
#endif // KERNEL_BUG_ASAP_FIX

	const udword NbEndPoints = NbBatched*2;			// Количесво добавляемых конечных точек
	ASAP_EndPoint* NewEPSorted = ICE_NEW_TMP(ASAP_EndPoint)[NbEndPoints];	// Массив новых конечных точек
	ASAP_EndPoint* Buffer = (ASAP_EndPoint*)ICE_ALLOC_TMP(sizeof(ASAP_EndPoint)*NbEndPoints);	// Массив отсортированых новых КТ
	RadixSort RS;

	// Проверка, что сентинелы проставлены правильно
	//for(udword Axis=0;Axis<3;Axis++){
	//assert(mEndPoints[Axis][0].mValue == MIN_FLOAT);
	//assert(mEndPoints[Axis][mNbUsedBoxes*2 + 1 - NbEndPoints].mValue == MAX_FLOAT);
	//}

	// Заполнение массива новых КТ значениями, сортировка и внесение в массив существующих КТ по каждой оси
	for(udword Axis=0;Axis<3;Axis++)
	{
		// Заполнеие массива новых КТ
		for(udword i=0;i<NbBatched;i++)
		{
			const udword BoxIndex = (udword)Batched[i].mHandle;
			assert(mBoxes[BoxIndex].mMin[Axis]==INVALID_INDEX);
			assert(mBoxes[BoxIndex].mMax[Axis]==INVALID_INDEX);

			// Берем границы ASAP_AABB для бокса
			const float MinValue = Batched[i].mBox.GetMin(Axis);
			const float MaxValue = Batched[i].mBox.GetMax(Axis);

			//NewEPSorted[i*2+0].SetData(EncodeFloat(MinValue), BoxIndex, FALSE);
			//NewEPSorted[i*2+1].SetData(EncodeFloat(MaxValue), BoxIndex, TRUE);

			// Кладем границы в виде КТ в массив
			NewEPSorted[i*2+0].SetData(MinValue, BoxIndex, FALSE);
			NewEPSorted[i*2+1].SetData(MaxValue, BoxIndex, TRUE);
		}

		// Sort endpoints backwards
		{
			//udword* Keys = (udword*)Buffer;
			float* Keys = (float*)Buffer;
			for(udword i=0;i<NbEndPoints;i++)
				Keys[i] = NewEPSorted[i].mValue;

			//const udword* Sorted = RS.Sort(Keys, NbEndPoints, RADIX_UNSIGNED).GetRanks();
			const udword* Sorted = RS.Sort(Keys, NbEndPoints).GetRanks();

			// Переписываем в буфер отсортированые КТ в обратном порядке
			for(udword i=0;i<NbEndPoints;i++)
				Buffer[i] = NewEPSorted[Sorted[NbEndPoints-1-i]];
		}

		// Добавляем КТ к уже существующим
		InsertEndPoints(Axis, Buffer, NbEndPoints);
	}

	ICE_FREE(Buffer);
	DELETEARRAY(NewEPSorted);
	}

#ifdef _DEBUG
	// Проверка, что для каждого добавленого бокса обработаны КТ
	for(udword i=0;i<NbBatched;i++)
	{
		udword BoxIndex = (udword)Batched[i].mHandle;
		ASAP_Box* Box = mBoxes + BoxIndex;
		assert(Box->HasBeenInserted());
	}
	// Проверка, что все КТ отсортированы правильно
#ifdef KERNEL_BUG_ASAP_FIX
	for(udword i=0;i<mNbUsedBoxes*2+1;i++)
#else
	for(udword i=0;i<mNbBoxes*2+1;i++)
#endif // KERNEL_BUG_ASAP_FIX
	{
		assert(mEndPoints[0][i].mValue <= mEndPoints[0][i+1].mValue);
		assert(mEndPoints[1][i].mValue <= mEndPoints[1][i+1].mValue);
		assert(mEndPoints[2][i].mValue <= mEndPoints[2][i+1].mValue);
	}
#endif

	if(1)
	{
		// Битовый массив, в котором отмечены только новые боксы
		BitArray BA(mMaxNbBoxes);

		// Using box-pruning on array indices....
		// Массив индексов конечных точек новых боксов
		IAABB* NewBoxes = ICE_NEW_TMP(IAABB)[NbBatched];
		for(udword i=0;i<NbBatched;i++)
		{
			const ASAP_Box* Box = mBoxes + (udword)Batched[i].mHandle;		// Новый бокс
			assert((udword)Batched[i].mHandle<mMaxNbBoxes);					// Хендл не вышел за границы
			BA.SetBit((udword)Batched[i].mHandle);					// Омечаем в битовом массиве
			// Переписываем индексы его КТ
			NewBoxes[i].mMinX = Box->mMin[0];
			NewBoxes[i].mMaxX = Box->mMax[0];
			NewBoxes[i].mMinY = Box->mMin[1];
			NewBoxes[i].mMaxY = Box->mMax[1];
			NewBoxes[i].mMinZ = Box->mMin[2];
			NewBoxes[i].mMaxZ = Box->mMax[2];
		}

		// Ведем сталкивание новых боксов и проверяем, не пересекаются ли они
		CompleteBoxPruning2(NbBatched, NewBoxes, Axes(AXES_XZY), Batched);

		// the old boxes are not the first ones in the array

#ifdef KERNEL_BUG_ASAP_FIX
		const udword NbOldBoxes = mNbUsedBoxes - NbBatched;// Количство боксов, котрые уже были
#else
		const udword NbOldBoxes = mNbBoxes - NbBatched;// Количство боксов, котрые уже были
#endif // KERNEL_BUG_ASAP_FIX
		if(NbOldBoxes)
		{
			// Боксы блыи, надо проверить, не пересекаются ли новые со старыми
			IAABB* OldBoxes = ICE_NEW_TMP(IAABB)[NbOldBoxes];		// Массив индексов конечных точек старых боксов
			udword* OldBoxesIndices = (udword*)ICE_ALLOC_TMP(sizeof(udword)*NbOldBoxes);	// Массив индексов старых боксов
			udword Offset=0;		// Счетчик-итератор по массиву всех боксов. Перескакивает через новые
			udword i=0;				// Счетчик цикла, чтобы обработали только старые боксы
			while(i<NbOldBoxes)
			{
#ifdef KERNEL_BUG_ASAP_FIX
				// Проверка на то, что мы не пытаемся использовать удаленный бокс.
				// Удаленным боксам при удалении присваивается mMin[0] = INVALID_INDEX
				while (BA.IsSet(Offset) || (mBoxes + Offset)->mMin[0] == INVALID_INDEX)
				{
					// Этим циклом мы переескакиваем через новые и удаленные боксы
					Offset++;
					assert(Offset<mNbBoxes);
				}
#else
				while(BA.IsSet(Offset))
				{
					// Этим циклом мы переескакиваем через новые боксы
					Offset++;
					assert(Offset<mNbBoxes);
				}
#endif // KERNEL_BUG_ASAP_FIX
				const ASAP_Box* Box = mBoxes + Offset;		// Старый бокс
				OldBoxesIndices[i] = Offset;		// Индекс старого бокса пишем в массив
				// Переписываем индексы его КТ
				OldBoxes[i].mMinX = Box->mMin[0];
				OldBoxes[i].mMaxX = Box->mMax[0];
				OldBoxes[i].mMinY = Box->mMin[1];
				OldBoxes[i].mMaxY = Box->mMax[1];
				OldBoxes[i].mMinZ = Box->mMin[2];
				OldBoxes[i].mMaxZ = Box->mMax[2];
				Offset++;
				i++;
			}
			assert(i==NbOldBoxes);

			BipartiteBoxPruning2(NbBatched, NewBoxes, NbOldBoxes, OldBoxes, Axes(AXES_XZY), Batched, OldBoxesIndices);

			ICE_FREE(OldBoxesIndices);
			DELETEARRAY(OldBoxes);
		}
		DELETEARRAY(NewBoxes);
	}
#ifdef RELEASE_ON_RESET
	mCreated.Empty();
#endif
}

void ArraySAP::BatchRemove()
{
	udword NbRemoved = mRemoved.GetNbEntries();
	if(!NbRemoved)	return;	// Early-exit if no object has been removed
	const udword* Removed = mRemoved.GetEntries();
	mRemoved.Reset();

	for(udword Axis=0;Axis<3;Axis++)
	{
		ASAP_EndPoint* const BaseEP = mEndPoints[Axis];
		udword MinMinIndex = MAX_UDWORD;
		for(udword i=0;i<NbRemoved;i++)
		{
			assert(Removed[i]<mMaxNbBoxes);
			const ASAP_Box* RemovedObject = mBoxes + Removed[i];
			const udword MinIndex = RemovedObject->mMin[Axis];
			assert(MinIndex<mMaxNbBoxes*2+2);
			const udword MaxIndex = RemovedObject->mMax[Axis];
			assert(MaxIndex<mMaxNbBoxes*2+2);
			assert(BaseEP[MinIndex].GetOwner()==Removed[i]);
			assert(BaseEP[MaxIndex].GetOwner()==Removed[i]);
			assert(MinIndex < MaxIndex);	// Где-то конечные точки сдвигались не правильно
			BaseEP[MinIndex].mData = 0xfffffffe;
			BaseEP[MaxIndex].mData = 0xfffffffe;
			if(MinIndex<MinMinIndex)	MinMinIndex = MinIndex;
		}

		udword ReadIndex = MinMinIndex;
		udword DestIndex = MinMinIndex;
#ifdef KERNEL_BUG_ASAP_FIX
		const udword Limit = mNbUsedBoxes*2+2;
#else
		const udword Limit = mNbBoxes*2+2;
#endif // KERNEL_BUG_ASAP_FIX
		while(ReadIndex!=Limit)
		{
			while(ReadIndex!=Limit && BaseEP[ReadIndex].mData == 0xfffffffe)
			{
				ReadIndex++;
			}
			if(ReadIndex!=Limit)
			{
				if(ReadIndex!=DestIndex)
				{
					BaseEP[DestIndex] = BaseEP[ReadIndex];
					assert(BaseEP[DestIndex].mData != 0xfffffffe);

					if(!BaseEP[DestIndex].IsSentinel())
					{
						udword BoxOwner = BaseEP[DestIndex].GetOwner();
						assert(BoxOwner<mMaxNbBoxes);
						ASAP_Box* Box = mBoxes + BoxOwner;

						Box->mMin[Axis + BaseEP[DestIndex].IsMax()] = DestIndex;
					}
				}
				DestIndex++;
				ReadIndex++;
			}
		}
	}

	BitArray BA(65536);
	const udword Saved = NbRemoved;
	TRACEMSG(">>>>>>>>>>>> nb, bnu = ", mNbBoxes, mNbUsedBoxes);
	while(NbRemoved--)
	{
		udword Index = *Removed++;
		assert(Index<mMaxNbBoxes);

		ASAP_Box* Object = mBoxes + Index;
		assert(Object->mGUID < 65536);
		BA.SetBit(Object->mGUID);

#ifdef KERNEL_BUG_ASAP_FIX
		// Задаем mMin[0] = INVALID_INDEX, чтобы потом через этот бокс перепрыгивать
		Object->mMin[0] = INVALID_INDEX;
		//if (Index + 1 == mNbBoxes) mNbBoxes--;	// Приводит к глюкам все равно, так что закоментил :(
#endif // KERNEL_BUG_ASAP_FIX

		Object->mGUID = mFirstFree;
		mFirstFree = Index;
	}
#ifdef KERNEL_BUG_ASAP_FIX
	mNbUsedBoxes -= Saved;
	mNbAddedBoxes = mNbUsedBoxes;
#else
	mNbBoxes -= Saved;
#endif // KERNEL_BUG_ASAP_FIX
	TRACEMSG("<<<<<<<<<<< nb, bnu = ", mNbBoxes, mNbUsedBoxes);
	mPairs.RemovePairs(BA);

#ifdef RELEASE_ON_RESET
	mRemoved.Empty();
#endif
}

bool ArraySAP::RemoveObject(udword handle)
{
#ifdef DEBUG_SAP
	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "ArraySAP::RemoveObject sap_handle = %d", handle);
#endif // DEBUG_SAP
	mRemoved.Add(handle);
	return true;
}

#ifdef USE_INTEGERS
bool ArraySAP::UpdateObject(udword handle, const ASAP_AABB& box_)
#else
bool ArraySAP::UpdateObject(udword handle, const ASAP_AABB& box)
#endif
{
	const ASAP_Box* Object = mBoxes + handle;
	assert(Object->HasBeenInserted());
	const void* UserObject = Object->mObject;
	const udword UserGUID = Object->mGUID;

#ifdef USE_INTEGERS
	IAABB box;
	box.mMinX = EncodeFloat(box_.GetMin(0));
	box.mMinY = EncodeFloat(box_.GetMin(1));
	box.mMinZ = EncodeFloat(box_.GetMin(2));
	box.mMaxX = EncodeFloat(box_.GetMax(0));
	box.mMaxY = EncodeFloat(box_.GetMax(1));
	box.mMaxZ = EncodeFloat(box_.GetMax(2));
#endif

	for(udword Axis=0;Axis<3;Axis++)
	{
		const udword Axis1 = (1  << Axis) & 3;
		const udword Axis2 = (1  << Axis1) & 3;

		ASAP_EndPoint* const BaseEP = mEndPoints[Axis];

		// Update min
		{
			ASAP_EndPoint* CurrentMin = BaseEP + Object->mMin[Axis];
			ASSERT(!CurrentMin->IsMax());

			const ValType Limit = box.GetMin(Axis);
			if(Limit < CurrentMin->mValue)
			{
				CurrentMin->mValue = Limit;

				// Min is moving left:
				ASAP_EndPoint Saved = *CurrentMin;
				udword EPIndex = (size_t(CurrentMin) - size_t(BaseEP))/sizeof(ASAP_EndPoint);
				const udword SavedIndex = EPIndex;

				while((--CurrentMin)->mValue > Limit)
				{
#ifdef USE_PREFETCH
					_prefetch(CurrentMin-1);
#endif
					ASAP_Box* id1 = mBoxes + CurrentMin->GetOwner();
					const BOOL IsMax = CurrentMin->IsMax();
					if(IsMax)
					{
						// Our min passed a max => start overlap
						if(Object!=id1
							&& Intersect2D(*Object, *id1, Axis1, Axis2)
							&& Intersect1D_Min(box, *id1, BaseEP, Axis)
							)
						{
							assert(UserGUID != 65535);
							assert(id1->mGUID != 65535);
							AddPair(UserObject, id1->mObject, UserGUID, id1->mGUID);
						}
					}

					id1->mMin[Axis + IsMax] = EPIndex--;
					*(CurrentMin+1) = *CurrentMin;
				}

				if(SavedIndex!=EPIndex)
				{
					mBoxes[Saved.GetOwner()].mMin[Axis + Saved.IsMax()] = EPIndex;
					BaseEP[EPIndex] = Saved;
				}
			}
			else if(Limit > CurrentMin->mValue)
			{
				CurrentMin->mValue = Limit;

				// Min is moving right:
				ASAP_EndPoint Saved = *CurrentMin;
				udword EPIndex = (size_t(CurrentMin) - size_t(BaseEP))/sizeof(ASAP_EndPoint);
				const udword SavedIndex = EPIndex;

				while((++CurrentMin)->mValue < Limit)
				{
#ifdef USE_PREFETCH
					_prefetch(CurrentMin+1);
#endif
					ASAP_Box* id1 = mBoxes + CurrentMin->GetOwner();
					const BOOL IsMax = CurrentMin->IsMax();
					if(IsMax)
					{
						// Our min passed a max => stop overlap
						if(Object!=id1
#ifdef USE_OVERLAP_TEST_ON_REMOVES
							&& Intersect2D(*Object, *id1, Axis1, Axis2)
#endif
							)
							RemovePair(UserObject, id1->mObject, UserGUID, id1->mGUID);
					}

					id1->mMin[Axis + IsMax] = EPIndex++;
					*(CurrentMin-1) = *CurrentMin;
				}

				if(SavedIndex!=EPIndex)
				{
					mBoxes[Saved.GetOwner()].mMin[Axis + Saved.IsMax()] = EPIndex;
					BaseEP[EPIndex] = Saved;
				}
			}
		}

		// Update max
		{
			ASAP_EndPoint* CurrentMax = BaseEP + Object->mMax[Axis];
			ASSERT(CurrentMax->IsMax());

			const ValType Limit = box.GetMax(Axis);
			if(Limit > CurrentMax->mValue)
			{
				CurrentMax->mValue = Limit;

				// Max is moving right:
				ASAP_EndPoint Saved = *CurrentMax;
				udword EPIndex = (size_t(CurrentMax) - size_t(BaseEP))/sizeof(ASAP_EndPoint);
				const udword SavedIndex = EPIndex;

				while((++CurrentMax)->mValue < Limit)
				{
#ifdef USE_PREFETCH
					_prefetch(CurrentMax+1);
#endif
					ASAP_Box* id1 = mBoxes + CurrentMax->GetOwner();
					const BOOL IsMax = CurrentMax->IsMax();
					if(!IsMax)
					{
						// Our max passed a min => start overlap
						if(Object!=id1
							&& Intersect2D(*Object, *id1, Axis1, Axis2)
							&& Intersect1D_Min(box, *id1, BaseEP, Axis)
							)
						{
							assert(UserGUID != 65535);
							assert(id1->mGUID != 65535);
							AddPair(UserObject, id1->mObject, UserGUID, id1->mGUID);
						}
					}

					id1->mMin[Axis + IsMax] = EPIndex++;
					*(CurrentMax-1) = *CurrentMax;
				}

				if(SavedIndex!=EPIndex)
				{
					mBoxes[Saved.GetOwner()].mMin[Axis + Saved.IsMax()] = EPIndex;
					BaseEP[EPIndex] = Saved;
				}
			}
			else if(Limit < CurrentMax->mValue)
			{
				CurrentMax->mValue = Limit;

				// Max is moving left:
				ASAP_EndPoint Saved = *CurrentMax;
				udword EPIndex = (size_t(CurrentMax) - size_t(BaseEP))/sizeof(ASAP_EndPoint);
				const udword SavedIndex = EPIndex;

				while((--CurrentMax)->mValue > Limit)
				{
#ifdef USE_PREFETCH
					_prefetch(CurrentMax-1);
#endif
					ASAP_Box* id1 = mBoxes + CurrentMax->GetOwner();
					const BOOL IsMax = CurrentMax->IsMax();
					if(!IsMax)
					{
						// Our max passed a min => stop overlap
						if(Object!=id1
#ifdef USE_OVERLAP_TEST_ON_REMOVES
							&& Intersect2D(*Object, *id1, Axis1, Axis2)
#endif
							)
							RemovePair(UserObject, id1->mObject, UserGUID, id1->mGUID);
					}

					id1->mMin[Axis + IsMax] = EPIndex--;
					*(CurrentMax+1) = *CurrentMax;
				}

				if(SavedIndex!=EPIndex)
				{
					mBoxes[Saved.GetOwner()].mMin[Axis + Saved.IsMax()] = EPIndex;
					BaseEP[EPIndex] = Saved;
				}
			}
		}

		// Проверка положения сентинелов
		assert(BaseEP[0].mValue == MIN_FLOAT);
#ifdef KERNEL_BUG_ASAP_FIX
		assert(BaseEP[mNbUsedBoxes*2 + 1].mValue == MAX_FLOAT);
#endif // KERNEL_BUG_ASAP_FIX
		assert(Object->mMin[Axis] < Object->mMax[Axis]);	// Неправильный сдвиг конечных точек
	}
	return true;
}



udword ArraySAP::DumpPairs(SAP_CreatePair create_cb, SAP_DeletePair delete_cb, void* cb_user_data, ASAP_Pair** pairs)
{
#ifdef DEBUG_SAP
	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "DumpPairs");
#endif // DEBUG_SAP
	BatchCreate();

	const udword* Entries = mData.GetEntries();
	const udword* Last = Entries + mData.GetNbEntries();


	udword* ToRemove = (udword*)Entries;	// Сейчас оба указывают на начло массива
	// Проходим по массиву новых/удаляемых пар, ищем удаляемые
	while(Entries!=Last)
	{
		const udword ID = *Entries++;
		ASAP_Pair* UP = mPairs.mActivePairs + ID;

		{
			ASSERT(UP->IsInArray());
			if(UP->IsRemoved())
			{
				// No need to call "ClearInArray" in this case, since the pair will get removed anyway

				// Remove
				if(delete_cb && !UP->IsNew())
				{
#ifdef PAIR_USER_DATA
					(delete_cb)(UP->GetObject0(), UP->GetObject1(), cb_user_data, UP->userData);
#else
					(delete_cb)(UP->GetObject0(), UP->GetObject1(), cb_user_data, null);
#endif
				}

				// Сохраняем индексы объектов удаляемой пары
				*ToRemove++ = udword(UP->id0)<<16|UP->id1;
			}
			else
			{
				UP->ClearInArray();
				// Add => already there... Might want to create user data, though
				if(UP->IsNew())
				{
					if(create_cb)
					{
#ifdef PAIR_USER_DATA
						UP->userData = (create_cb)(UP->GetObject0(), UP->GetObject1(), cb_user_data);
#else
						(create_cb)(UP->GetObject0(), UP->GetObject1(), cb_user_data);
#endif
					}
					UP->ClearNew();
				}
			}
		}
	}

	// #### try batch removal here
	Entries = mData.GetEntries();	// На самом деле тут индексы объектов
	while(Entries!=ToRemove)
	{
		const udword ID = *Entries++;
		const udword id0 = ID>>16;
		const udword id1 = ID&0xffff;
#ifdef _DEBUG
		bool Status = mPairs.RemovePair(id0, id1);	// Удаляем по настоящему
		ASSERT(Status);
#else
		mPairs.RemovePair(id0, id1);	// Удаляем по настоящему
#endif // _DEBUG
	}

#ifdef RELEASE_ON_RESET
	mData.Empty();
#endif
	// Окончательно удаляем объекты и пары их пересечений
	BatchRemove();		

	if(pairs)	*pairs = mPairs.mActivePairs;

	return mPairs.mNbActivePairs;
}



inline_	void				ArraySAP::AddPair(const void* object0, const void* object1, uword id0, uword id1)
{
	ASSERT(object0);
	ASAP_Pair* UP = (ASAP_Pair*)mPairs.AddPair(id0, id1, null, null);
	ASSERT(UP);

	if(UP->object0)
	{
		// Persistent pair
	}
	else
	{
		// New pair
		ASSERT(!(int(object0)&1));
		ASSERT(!(int(object1)&1));
		UP->object0 = object0;
		UP->object1 = object1;
		TRACEMSG2("Добавляется пара:", id0, id1, object0, object1);
		UP->SetInArray();
		mData.Add(mPairs.GetPairIndex(UP));
		UP->SetNew();
	}
	UP->ClearRemoved();
}

/*inline_*/	void ArraySAP::RemovePair(const void* object0, const void* object1, uword id0, uword id1)
{
	UNUSED_ARG(object0);
	UNUSED_ARG(object1);
	ASAP_Pair* UP = (ASAP_Pair*)mPairs.FindPair(id0, id1);
	if(UP)
	{
		if(!UP->IsInArray())
		{
			UP->SetInArray();
			mData.Add(mPairs.GetPairIndex(UP));
		}
		UP->SetRemoved();
	}
}


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
 *	\file		OPC_ArraySAP.h
 *	\author		Pierre Terdiman
 *	\date		December, 2, 2007
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef OPC_ARRAYSAP_H
#define OPC_ARRAYSAP_H

#include "SweepAndPrune.h"

#include "ASAP_Types.h"

// Исправления ошибки в хранении боксов.
#define KERNEL_BUG_ASAP_FIX


#ifdef USE_WORDS
	typedef uword			IndexType;
	#define	INVALID_INDEX	0xffff
#else
	typedef udword			IndexType;
	#define	INVALID_INDEX	0xffffffff
#endif

#ifdef USE_INTEGERS
	typedef udword	ValType;
	typedef IAABB	SAP_AABB;
#else
	typedef float	ValType;
	typedef ASAP_AABB	SAP_AABB;
#endif


class CRay;

namespace Opcode
{
	#include "ASAP_PairManager.h"


	class ASAP_EndPoint : public IceCore::Allocateable
	{
	public:
		inline_					ASAP_EndPoint()		{}
		inline_					~ASAP_EndPoint()	{}

		ValType			mValue;		// Min or Max value
		udword			mData;		// Parent box | MinMax flag
	public:

		inline_	bool			IsSentinel()	const	{ return (mData&~3)==0xfffffffc;	}

		inline_	void			SetData(ValType v, udword owner_box_id, BOOL is_max)
		{
			mValue = v;
			mData = owner_box_id<<2;
			if(is_max)	mData |= 3;
		}
		inline_	BOOL			IsMax()		const	{ return mData & 3;		}
		inline_	udword			GetOwner()	const	{ return mData>>2;		}
	};

	class ASAP_Box : public IceCore::Allocateable
	{
	public:
		inline_					ASAP_Box()	{}
		inline_					~ASAP_Box()	{}

		IndexType		mMin[3];		// Номера конечных точек на осях
		IndexType		mMax[3];		// Номера конечных точек на осях
		void*			mObject;		// Указатель на игровой объект
		udword			mGUID;			// Номер игрового объекта (по счетчику самой игры, не SAP'a)

		inline_	ValType			GetMaxValue(udword i, const ASAP_EndPoint* base)	const
		{
			return base[mMax[i]].mValue;
		}

		inline_	ValType			GetMinValue(udword i, const ASAP_EndPoint* base)	const
		{
			return base[mMin[i]].mValue;
		}
	#ifdef _DEBUG
		bool			HasBeenInserted()	const
		{
			assert(mMin[0]!=INVALID_INDEX);
			assert(mMax[0]!=INVALID_INDEX);
			assert(mMin[1]!=INVALID_INDEX);
			assert(mMax[1]!=INVALID_INDEX);
			assert(mMin[2]!=INVALID_INDEX);
			assert(mMax[2]!=INVALID_INDEX);
			return true;
		}
	#endif
	};



	typedef void* (*SAP_CreatePair)(const void* object0, const void* object1, void* user_data);
	typedef void  (*SAP_DeletePair)(const void* object0, const void* object1, void* user_data, void* pair_user_data);

	// Forward declarations
	class ASAP_EndPoint;
	class ASAP_Box;
	struct IAABB;
	struct CreateData;

	// Главный класс, хранит боксы, двигает их и определяет пересечения
	class OPCODE_API ArraySAP : public IceCore::Allocateable
	{
		public:
									ArraySAP();
									~ArraySAP();

				udword				AddObject(void* object, uword guid, const ASAP_AABB& box);
				bool				RemoveObject(udword handle);
				bool				UpdateObject(udword handle, const ASAP_AABB& box);

				udword				DumpPairs(SAP_CreatePair create_cb, SAP_DeletePair delete_cb, void* cb_user_data, ASAP_Pair** pairs=null);

		const	ASAP_EndPoint*		GetEndPoints(udword axis) { return mEndPoints[axis]; }
		const	ASAP_Box*			GetBoxes() { return mBoxes; }
				udword				GetNbUsedBoxes() { return mNbUsedBoxes; }

		/*inline_*/	void			RemovePair(const void* object0, const void* object1, uword id0, uword id1);

		private:
				IceCore::Container	mData;	// Тут хранятся индексы пар пересечений, которые удалялись или добавлялись
				ASAP_PairManager	mPairs;	// Менеджер пар пересечений

				// Код перенесен в OPC_ArraySAP.cpp
		inline_	void				AddPair(const void* object0, const void* object1, uword id0, uword id1);
		//inline_	void				RemovePair(const void* object0, const void* object1, uword id0, uword id1);

	#ifdef KERNEL_BUG_ASAP_FIX
				udword				mNbAddedBoxes;	// Количество боксов в массиве, которые ждут обработки в BatchAdd
				udword				mNbUsedBoxes;	// Количество настоящих боксов, котрые храняться в массиве и используются в данный момент
	#endif // KERNEL_BUG_ASAP_FIX
				udword				mNbBoxes;		// Количество боксов, котрые храняться в массиве
				udword				mMaxNbBoxes;	// Количесвто боксов, под которое выделена память
				ASAP_Box*			mBoxes;			// Массив боксов
				ASAP_EndPoint*		mEndPoints[3];	// Массив конечных точек боксов по трем осям
				udword				mFirstFree;		// Следующий свободный номер бокса - следующий свободный индекс в массиве

				void				ResizeBoxArray();
			// For batch creation
				IceCore::Container	mCreated;
				void				BatchCreate();
				void				InsertEndPoints(udword axis, const ASAP_EndPoint* end_points, udword nb_endpoints);
				bool				CompleteBoxPruning2(udword nb, const IAABB* array, const Axes& axes, const CreateData* batched);
				bool				BipartiteBoxPruning2(udword nb0, const IAABB* array0, udword nb1, const IAABB* array1, const Axes& axes, const CreateData* batched, const udword* box_indices);

			// For batch removal
				IceCore::Container	mRemoved;
				void				BatchRemove();
	};

	
	inline_ BOOL Intersect2D(const ASAP_Box& c, const ASAP_Box& b, udword axis1, udword axis2)
	{
		if(		b.mMax[axis1] < c.mMin[axis1] || c.mMax[axis1] < b.mMin[axis1]
		||	b.mMax[axis2] < c.mMin[axis2] || c.mMax[axis2] < b.mMin[axis2])	return FALSE;
		return TRUE;
	}

	inline_ BOOL Intersect1D_Min(const SAP_AABB& a, const ASAP_Box& b, const ASAP_EndPoint* const base, udword axis)
	{
		if(b.GetMaxValue(axis, base) < a.GetMin(axis))
			return FALSE;
		return TRUE;
	}

}

#endif // OPC_ARRAYSAP_H

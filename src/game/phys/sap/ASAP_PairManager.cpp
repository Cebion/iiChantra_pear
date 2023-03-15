#include "StdAfx.h"

#include "SweepAndPrune.h"
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


#define INVALID_USER_ID	0xffff

inline_ void Sort(uword& id0, uword& id1)										{ if(id0>id1)	TSwap(id0, id1);						}
inline_ void Sort(uword& id0, uword& id1, const void*& obj0, const void*& obj1)	{ if(id0>id1)	{ TSwap(id0, id1); TSwap(obj0, obj1);	}	}

inline_ udword Hash(uword id0, uword id1)								{ return Hash32Bits_1( udword(id0)|(udword(id1)<<16) );		}
inline_ bool DifferentPair(const ASAP_Pair& p, uword id0, uword id1)	{ return (id0!=p.id0) || (id1!=p.id1);						}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ASAP_PairManager::ASAP_PairManager() :
mHashSize		(0),
mMask			(0),
mNbActivePairs	(0),
mHashTable		(null),
mNext			(null),
mActivePairs	(null)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ASAP_PairManager::~ASAP_PairManager()
{
	Purge();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ASAP_PairManager::Purge()
{
	ICE_FREE(mNext);
	ICE_FREE(mActivePairs);
	ICE_FREE(mHashTable);
	mHashSize		= 0;
	mMask			= 0;
	mNbActivePairs	= 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const ASAP_Pair* ASAP_PairManager::FindPair(uword id0, uword id1) const
{
	if(!mHashTable)	return null;	// Nothing has been allocated yet

	// Order the ids
	Sort(id0, id1);

	// Compute hash value for this pair
	udword HashValue = Hash(id0, id1) & mMask;

	// Look for it in the table
	udword Offset = mHashTable[HashValue];
	while(Offset!=INVALID_ID && DifferentPair(mActivePairs[Offset], id0, id1))
	{
		ASSERT(mActivePairs[Offset].id0!=INVALID_USER_ID);
		Offset = mNext[Offset];		// Better to have a separate array for this
	}
	if(Offset==INVALID_ID)	return null;
	ASSERT(Offset<mNbActivePairs);
	// Match mActivePairs[Offset] => the pair is persistent
	return &mActivePairs[Offset];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Internal version saving hash computation
inline_ ASAP_Pair* ASAP_PairManager::FindPair(uword id0, uword id1, udword hash_value) const
{
	if(!mHashTable)	return null;	// Nothing has been allocated yet

	// Look for it in the table
	udword Offset = mHashTable[hash_value];
	while(Offset!=INVALID_ID && DifferentPair(mActivePairs[Offset], id0, id1))
	{
		ASSERT(mActivePairs[Offset].id0!=INVALID_USER_ID);
		Offset = mNext[Offset];		// Better to have a separate array for this
	}
	if(Offset==INVALID_ID)	return null;
	ASSERT(Offset<mNbActivePairs);
	// Match mActivePairs[Offset] => the pair is persistent
	return &mActivePairs[Offset];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Добавлене пары пересечений.
// TODO: Ссылки на объекты реально никогда не передаются, всегда NULL, зачем тогда параметры функции?
const ASAP_Pair* ASAP_PairManager::AddPair(uword id0, uword id1, const void* object0, const void* object1)
{
	// Order the ids
	Sort(id0, id1, object0, object1);

	udword HashValue = Hash(id0, id1) & mMask;

	ASAP_Pair* P = FindPair(id0, id1, HashValue);
	if(P)
	{
		return P;	// Persistent pair
	}

	// This is a new pair
	if(mNbActivePairs >= mHashSize)
	{
		// Get more entries
		mHashSize = NextPowerOfTwo(mNbActivePairs+1);
		mMask = mHashSize-1;

		ReallocPairs();

		// Recompute hash value with new hash size
		HashValue = Hash(id0, id1) & mMask;
	}

	ASAP_Pair* p = &mActivePairs[mNbActivePairs];
	p->id0		= id0;	// ### CMOVs would be nice here
	p->id1		= id1;
	p->object0	= object0;
	p->object1	= object1;
	TRACEMSG2("Добавляется пара:", id0, id1, p->object0, p->object1)

		mNext[mNbActivePairs] = mHashTable[HashValue];
	mHashTable[HashValue] = mNbActivePairs++;
	return p;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ASAP_PairManager::RemovePair(uword id0, uword id1, udword hash_value, udword pair_index)
{
#ifndef _DEBUG
	UNUSED_ARG(id0);
	UNUSED_ARG(id1);
#endif // _DEBUG
	TRACEMSG("Удаляется пара:", id0, id1)
		// Walk the hash table to fix mNext
		udword Offset = mHashTable[hash_value];
	ASSERT(Offset!=INVALID_ID);

	udword Previous=INVALID_ID;
	while(Offset!=pair_index)
	{
		Previous = Offset;
		Offset = mNext[Offset];
	}

	// Let us go/jump us
	if(Previous!=INVALID_ID)
	{
		ASSERT(mNext[Previous]==pair_index);
		mNext[Previous] = mNext[pair_index];
	}
	// else we were the first
	else mHashTable[hash_value] = mNext[pair_index];
	// we're now free to reuse mNext[PairIndex] without breaking the list

#ifdef _DEBUG
	mNext[pair_index]=INVALID_ID;
#endif
	// Invalidate entry

	// Fill holes
	{
		// 1) Remove last pair
		const udword LastPairIndex = mNbActivePairs-1;
		if(LastPairIndex==pair_index)
		{
			mNbActivePairs--;
		}
		else
		{
			const ASAP_Pair* Last = &mActivePairs[LastPairIndex];
			const udword LastHashValue = Hash(Last->id0, Last->id1) & mMask;

			// Walk the hash table to fix mNext
			udword Offset = mHashTable[LastHashValue];
			ASSERT(Offset!=INVALID_ID);

			udword Previous=INVALID_ID;
			while(Offset!=LastPairIndex)
			{
				Previous = Offset;
				Offset = mNext[Offset];
			}

			// Let us go/jump us
			if(Previous!=INVALID_ID)
			{
				ASSERT(mNext[Previous]==LastPairIndex);
				mNext[Previous] = mNext[LastPairIndex];
			}
			// else we were the first
			else mHashTable[LastHashValue] = mNext[LastPairIndex];
			// we're now free to reuse mNext[LastPairIndex] without breaking the list

#ifdef _DEBUG
			mNext[LastPairIndex]=INVALID_ID;
#endif

			// Don't invalidate entry since we're going to shrink the array

			// 2) Re-insert in free slot
			mActivePairs[pair_index] = mActivePairs[LastPairIndex];
#ifdef _DEBUG
			ASSERT(mNext[pair_index]==INVALID_ID);
#endif
			mNext[pair_index] = mHashTable[LastHashValue];
			mHashTable[LastHashValue] = pair_index;

			mNbActivePairs--;
		}
	}
}

bool ASAP_PairManager::RemovePair(uword id0, uword id1)
{
	// Order the ids
	Sort(id0, id1);

	const udword HashValue = Hash(id0, id1) & mMask;
	const ASAP_Pair* P = FindPair(id0, id1, HashValue);
	if(!P)	return false;
	ASSERT(P->id0==id0);
	ASSERT(P->id1==id1);

	RemovePair(id0, id1, HashValue, GetPairIndex(P));

	ShrinkMemory();
	return true;
}

bool ASAP_PairManager::RemovePairs(const BitArray& array)
{
	udword i=0;
	while(i<mNbActivePairs)
	{
		const uword id0 = mActivePairs[i].id0;
		const uword id1 = mActivePairs[i].id1;
		if(array.IsSet(id0) || array.IsSet(id1))
		{
			const udword HashValue = Hash(id0, id1) & mMask;
			RemovePair(id0, id1, HashValue, i);
		}
		else i++;
	}
	ShrinkMemory();
	return true;
}

void ASAP_PairManager::ShrinkMemory()
{
	// Check correct memory against actually used memory
	const udword CorrectHashSize = NextPowerOfTwo(mNbActivePairs);
	if(mHashSize==CorrectHashSize)	return;

	// Reduce memory used
	mHashSize = CorrectHashSize;
	mMask = mHashSize-1;

	ReallocPairs();
}

void ASAP_PairManager::ReallocPairs()
{
	ICE_FREE(mHashTable);
	mHashTable = (udword*)ICE_ALLOC(mHashSize*sizeof(udword));
	StoreDwords(mHashTable, mHashSize, INVALID_ID);					// А еще есть memset, здесь он сработал бы, забивая 0xff. Интересно, быстрее ли...

	// Get some bytes for new entries
	ASAP_Pair* NewPairs	= (ASAP_Pair*)ICE_ALLOC(mHashSize * sizeof(ASAP_Pair));	ASSERT(NewPairs);
	udword* NewNext		= (udword*)ICE_ALLOC(mHashSize * sizeof(udword));		ASSERT(NewNext);

	// Copy old data if needed
	if(mNbActivePairs)	CopyMemory(NewPairs, mActivePairs, mNbActivePairs*sizeof(ASAP_Pair));
	// ### check it's actually needed... probably only for pairs whose hash value was cut by the and
	// yeah, since Hash(id0, id1) is a constant
	// However it might not be needed to recompute them => only less efficient but still ok
	for(udword i=0;i<mNbActivePairs;i++)
	{
		const udword HashValue = Hash(mActivePairs[i].id0, mActivePairs[i].id1) & mMask;	// New hash value with new mask
		NewNext[i] = mHashTable[HashValue];
		mHashTable[HashValue] = i;
	}

	// Delete old data
	ICE_FREE(mNext);
	ICE_FREE(mActivePairs);

	// Assign new pointer
	mActivePairs = NewPairs;
	mNext = NewNext;
}

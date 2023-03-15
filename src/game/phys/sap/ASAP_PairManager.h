#ifndef __ASAP_PAIR_MANAGER_H_
#define __ASAP_PAIR_MANAGER_H_

#define PAIR_USER_DATA				// Добавляет место для каких-то UserData

#pragma pack(1)

// Одна пара номеров объектов и указателей на них
struct OPCODE_API ASAP_Pair
{
	uword		id0;
	uword		id1;
	const void*	object0;
	const void*	object1;
#ifdef PAIR_USER_DATA
	void*		userData;
#endif
	inline_		const void*	GetObject0()		const	{ return (const void*)(size_t(object0) & ~3);		}
	inline_		const void*	GetObject1()		const	{ return (const void*)(size_t(object1) & ~3);		}
	inline_		size_t		IsInArray()			const	{ return size_t(object0) & 1;						}
	inline_		size_t		IsRemoved()			const	{ return size_t(object1) & 1;						}
	inline_		size_t		IsNew()				const	{ return size_t(object0) & 2;						}
private:
	inline_		void		SetInArray()				{ object0 = (const void*)(size_t(object0) | 1);		}
	inline_		void		SetRemoved()				{ object1 = (const void*)(size_t(object1) | 1);		}
	inline_		void		SetNew()					{ object0 = (const void*)(size_t(object0) | 2);		}
	inline_		void		ClearInArray()				{ object0 = (const void*)(size_t(object0) & ~1);	}
	inline_		void		ClearRemoved()				{ object1 = (const void*)(size_t(object1) & ~1);	}
	inline_		void		ClearNew()					{ object0 = (const void*)(size_t(object0) & ~2);	}

	friend class ArraySAP;
};
#pragma pack()

// Менеджер, хранящий пары пересечений и ведущий по ним поиск
class OPCODE_API ASAP_PairManager
{
public:
								ASAP_PairManager();
								~ASAP_PairManager();

			void				Purge();
			void				ShrinkMemory();

	const	ASAP_Pair*			AddPair(uword id0, uword id1, const void* object0, const void* object1);
			bool				RemovePair(uword id0, uword id1);
			bool				RemovePairs(const IceCore::BitArray& array);
	const	ASAP_Pair*			FindPair(uword id0, uword id1)	const;
	inline_	udword				GetPairIndex(const ASAP_Pair* pair)	const
								{
									return ((udword)((size_t(pair) - size_t(mActivePairs)))/sizeof(ASAP_Pair));
								}

			udword				mHashSize;
			udword				mMask;
			udword				mNbActivePairs;
			udword*				mHashTable;
			udword*				mNext;
			ASAP_Pair*			mActivePairs;
			inline_	ASAP_Pair*	FindPair(uword id0, uword id1, udword hash_value) const;
			void				RemovePair(uword id0, uword id1, udword hash_value, udword pair_index);
			void				ReallocPairs();
};


#endif // __ASAP_PAIR_MANAGER_H_

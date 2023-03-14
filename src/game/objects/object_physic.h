#ifndef __OBJECT_PHYSIC_H_
#define __OBJECT_PHYSIC_H_

#include "object.h"

class ObjPhysic : public GameObject
{
public:

	UINT sap_handle;

	BYTE physFlags;
	float bounce;
	BYTE solid_to;
	BYTE slopeType; // 0 - ���, 1 - "/", 2 - "\"

#ifdef MAP_EDITOR
	RGBAf border_color;
	RGBAf editor_color;
	CAABB old_aabb;
	bool show_border;
#endif // MAP_EDITOR

	// �����
	// solid					1		������ ��������� �� �����������
	// bullet_collidable		2		������ �������� ����������� ��� ����
	// dynamic					4		������������, ����������
	// touchable				8		����������� �� �������
	// forced					16		�� ��������� ����������
	// one-side					64		��������� �������� ������ ����. 64 - ����� �� ������������ � OnPlane, ����� ��� ����.

	__INLINE BYTE IsSolid()				const	{ return physFlags & 1; }
	__INLINE BYTE IsBulletCollidable()	const	{ return physFlags & 2; }
	__INLINE BYTE IsDynamic()			const	{ return physFlags & 4; }
	__INLINE BYTE IsTouchable()			const	{ return physFlags & 8; }
	__INLINE BYTE IsForced()			const	{ return physFlags & 16; }
	__INLINE BYTE IsOneSide()			const	{ return physFlags & 64; }

	__INLINE void SetSolid()				{ physFlags |= 1; }
	__INLINE void SetBulletCollidable()		{ physFlags |= 2; }
	__INLINE void SetDynamic()				{ physFlags |= 4; }
	__INLINE void SetTouchable()			{ physFlags |= 8; }
	__INLINE void SetForced()				{ physFlags |= 16; }
	__INLINE void SetOneSide()				{ physFlags |= 64; }

	__INLINE void ClearSolid()				{ physFlags &= ~1; }
	__INLINE void ClearBulletCollidable() 	{ physFlags &= ~2; }
	__INLINE void ClearDynamic()			{ physFlags &= ~4; }
	__INLINE void ClearTouchable()			{ physFlags &= ~8; }
	__INLINE void ClearForced()				{ physFlags &= ~16; }
	__INLINE void ClearOneSide()			{ physFlags &= ~64; }

	ObjPhysic()
	{
		sap_handle = 0;
		physFlags = 0;
		bounce = 0;
		slopeType = 0;
		solid_to = 255;

		aabb.W = 0.5f;
		aabb.H = 0.5f;

#ifdef MAP_EDITOR
		border_color = RGBAf(1.0f, 0.5f, 0.0f, 1.0f);
		show_border = false;
#endif // MAP_EDITOR

		this->SetPhysic();
	}

	virtual void Touch( ObjPhysic* obj )
	{ UNUSED_ARG(obj); }

#ifdef MAP_EDITOR
	void PhysProcess();
#else
	virtual void PhysProcess()
	{ }
#endif // MAP_EDITOR

};


#endif // __OBJECT_PHYSIC_H_

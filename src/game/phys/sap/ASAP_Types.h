#ifndef __ASAP_TYPES_H_
#define __ASAP_TYPES_H_

#define ICEMATHS_API

#include "SweepAndPrune.h"


enum AxisIndex
{
	X_					= 0,
	Y_					= 1,
	Z_					= 2,
	W_					= 3,

	AXIS_FORCE_DWORD	= 0x7fffffff
};

enum AxisOrder
{
	AXES_XYZ			= (X_)|(Y_<<2)|(Z_<<4),
	AXES_XZY			= (X_)|(Z_<<2)|(Y_<<4),
	AXES_YXZ			= (Y_)|(X_<<2)|(Z_<<4),
	AXES_YZX			= (Y_)|(Z_<<2)|(X_<<4),
	AXES_ZXY			= (Z_)|(X_<<2)|(Y_<<4),
	AXES_ZYX			= (Z_)|(Y_<<2)|(X_<<4),

	AXES_FORCE_DWORD	= 0x7fffffff
};

class ICEMATHS_API Axes : public IceCore::Allocateable
{
public:

	inline_			Axes(AxisOrder order)
	{
		mAxis0 = (order   ) & 3;
		mAxis1 = (order>>2) & 3;
		mAxis2 = (order>>4) & 3;
	}
	inline_			~Axes()		{}

	udword	mAxis0;
	udword	mAxis1;
	udword	mAxis2;
};

class ICEMATHS_API ASAP_Point : public IceCore::Allocateable
{
public:
	//! Constructor
	inline_						ASAP_Point()		{}
	//! Destructor
	inline_						~ASAP_Point()	{}

	inline_					operator	const	float*() const	{ return &x; }
	inline_					operator			float*()		{ return &x; }

	float		x,y,z;
};

class ICEMATHS_API ASAP_AABB : public IceCore::Allocateable
{
public:
	//! Constructor
	inline_						ASAP_AABB()	{}
	//! Destructor
	inline_						~ASAP_AABB()	{}

	//! Get component of the box's min point along a given axis
	inline_			float		GetMin(udword axis)						const		{ return mMin[axis];						}
	//! Get component of the box's max point along a given axis
	inline_			float		GetMax(udword axis)						const		{ return mMax[axis];						}

	ASAP_Point		mMin;
	ASAP_Point		mMax;
};

#endif // __ASAP_TYPES_H_
// This is a hook file used to compile the sweep-and-prune code out of ICE.
// This is a made-up file just to compile the thing. This is NOT the way it usually look in ICE.
#ifndef SWEEPANDPRUNE_H
#define SWEEPANDPRUNE_H

	#define ICE_NO_DLL
	#define OPCODE_API
	#define ICE_DONT_CHECK_COMPILER_OPTIONS
	#define TRUE	1
	#define FALSE	0
	#define	ASSERT	assert



// Standard includes
#ifdef _DEBUG
//	#include <crtdbg.h>			// C runtime debug functions
#endif
	//#include <stdio.h>			// Standard Input/Output functions
	//#include <stdlib.h>
	//#include <string.h>
	#include <float.h>
	//#include <malloc.h>			// For _alloca
	//#include <math.h>
	//#include <assert.h>

	#include "IcePreprocessor.h"
	#include "IceAssert.h"
	#include "IceTypes.h"
	#include "IceMemoryMacros.h"

	namespace IceCore
	{
		#include "IceUtils.h"
		#include "IceAllocator.h"
		#include "IceFPU.h"
		#include "IceBitArray.h"
		#include "IceContainer.h"
		#include "IceRevisitedRadix.h"
		#include "IceHashing.h"
	}

	inline_ void IceTrace(const char*){}
	inline_ void Log(const char*){}

#endif

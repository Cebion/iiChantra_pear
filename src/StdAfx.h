#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
	#define _CRT_SECURE_NO_WARNINGS
	#define _CRT_SECURE_NO_DEPRECATE			//
	#define _CRT_RAND_S
	#define _DEBUG_

	#ifdef _DEBUG
		#define FIND_MEM_LEAKS

		#ifdef FIND_MEM_LEAKS
			// ����� ������ ������
			// � VS2005 ����������� _CRTDBG_MAP_ALLOC �� ���� ������� ������ ��-�� ������
			// � ������ �����������. �������������, ��� ���� ����������������.
			// � VS2005 SP1 ��� ���������.
			// � �����, _CRTDBG_MAP_ALLOC �� ��� �� � �����.
			#define _CRTDBG_MAP_ALLOC
			#include <crtdbg.h>
		#endif // FIND_MEM_LEAKS
	#endif // _DEBUG

#include <windows.h>

#endif // WIN32


#define __STDC_LIMIT_MACROS		// ����� ��� g++ ��� SIZE_MAX

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include <ctime>
#include <stdio.h>

#include <stdlib.h>

#define _USE_MATH_DEFINES  // ��� ������ �������� ���� M_PI, M_SQRT2
#include <math.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <limits>
#include <limits.h>

#ifdef FIND_MEM_LEAKS
	// ��� ��� ��������� new ��������� �� ���� ���������� ������, � ���� ������
	// ����� ���������� ���������� � ����� � ������, �� ������� ��� ����� new.
	#define DEBUG_CLIENTBLOCK   new( _NORMAL_BLOCK, __FILE__, __LINE__)
	#define new DEBUG_CLIENTBLOCK
#endif // FIND_MEM_LEAKS


#include <malloc.h>

#include <assert.h>

#include "types.h"
#include "defines.h"

using namespace std;

// for compatibility with 2005 studio
#if !defined DISP_CHANGE_BADDUALVIEW
#define DISP_CHANGE_BADDUALVIEW -6
#endif

#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)

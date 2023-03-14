#include "StdAfx.h"

#include "../config.h"

#include "camera.h"
#include "objects/object_player.h"

//////////////////////////////////////////////////////////////////////////

extern config cfg;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Положение камеры
float CAMERA_X = 0;
float CAMERA_Y = 0;
Vector2 camera_pos = Vector2(0,0);

// Коэффициент, отвечающий за скорость реакции камеры на изменение положения объекта
float CAMERA_LAG = 1.0f;

// Положение границ экрана
float CAMERA_LEFT = 0;
float CAMERA_RIGHT = 0;
float CAMERA_TOP = 0;
float CAMERA_BOTTOM = 0;

// Это смещенеие системы кординат
float CAMERA_OFF_X = 0;
float CAMERA_OFF_Y = 0;

float CAMERA_OLD_OFF_X = 0;
float CAMERA_OLD_OFF_Y = 0;

// Смещение камеры относительно объекта
float CAMERA_OBJ_OFF_X = 0;
float CAMERA_OBJ_OFF_Y = 0;

// Точка объекта, на которую нацелена камера
CameraFocusObjectPoint CAMERA_FOCUS_ON_OBJ_POS = CamFocusCenter;

// Объект, на который нацелена камера
GameObject* attached_object = NULL;

bool camera_attach_x = true;
bool camera_attach_y = true;

bool camera_use_bounds = false;
float CAMERA_LEFT_BOUND = 0;
float CAMERA_RIGHT_BOUND = 0;
float CAMERA_TOP_BOUND = 0;
float CAMERA_BOTTOM_BOUND = 0;
//////////////////////////////////////////////////////////////////////////
// Расчеты

__INLINE void CalcCamOffX()
{
	CAMERA_OFF_X = -(CAMERA_X - cfg.scr_width * 0.5f);
	CAMERA_LEFT = -CAMERA_OFF_X;
	CAMERA_RIGHT = CAMERA_LEFT + cfg.scr_width;
}

__INLINE void CalcCamOffY()
{
	CAMERA_OFF_Y = -(CAMERA_Y - cfg.scr_height * 0.5f);
	CAMERA_TOP = -CAMERA_OFF_Y;
	CAMERA_BOTTOM = CAMERA_TOP + cfg.scr_height;
}

__INLINE void ApplyBounds()
{
	if (CAMERA_LEFT < CAMERA_LEFT_BOUND)
	{
		CAMERA_X += CAMERA_LEFT_BOUND - CAMERA_LEFT;
		CalcCamOffX();
	}

	if (CAMERA_RIGHT >  CAMERA_RIGHT_BOUND)
	{
		CAMERA_X -= CAMERA_RIGHT - CAMERA_RIGHT_BOUND;
		CalcCamOffX();
	}

	if (CAMERA_TOP < CAMERA_TOP_BOUND)
	{
		CAMERA_Y += CAMERA_TOP_BOUND - CAMERA_TOP;
		CalcCamOffY();
	}

	if (CAMERA_BOTTOM >  CAMERA_BOTTOM_BOUND)
	{
		CAMERA_Y -= CAMERA_BOTTOM - CAMERA_BOTTOM_BOUND;
		CalcCamOffY();
	}
}
//////////////////////////////////////////////////////////////////////////
void CameraAttachToAxis(bool x, bool y)
{
	camera_attach_x = x;
	camera_attach_y = y;
}

// Прицепляет камеру к объекту
// Если передать NULL, то отцеплет ее
void CameraAttachToObject(GameObject* obj)
{
	attached_object = obj;
}

void SetCameraAttachedObjectOffset(float x, float y)
{
	CAMERA_OBJ_OFF_X = x;
	CAMERA_OBJ_OFF_Y = y;

	if ( CAMERA_LAG > 0 )
		return;

	if ( attached_object )
	{
		CAMERA_X = attached_object->aabb.p.x + x;
		CAMERA_Y = attached_object->aabb.p.y + y;
	}
	else
	{
		CAMERA_X += x;
		CAMERA_Y += y;
	}
}

// Возвращает объект
GameObject* GetCameraAttachedObject()
{
	return attached_object;
}

// Смещает камеру
// x, y - координаты точки, которая будет в центре экрана
void CameraMoveToPos(float x, float y)
{
	attached_object = NULL;

	camera_pos.x = x;
	camera_pos.y = y;
	CAMERA_X = floor( camera_pos.x + 0.5f);
	CAMERA_Y = floor( camera_pos.y + 0.5f);

	CalcCamOffX();
	CalcCamOffY();
}

// Устанавливает, необходимо ли использовать границы для камеры
void CameraUseBounds(bool b)
{
	camera_use_bounds = b;
}

// Устанавливает значения границ
// Если границы таковы, что камера не влезет в экран, то праввая и нижняя будут сдвинуты
void CameraSetBounds(float left, float right, float top, float bottom)
{
	CAMERA_LEFT_BOUND = left;
	CAMERA_RIGHT_BOUND = right;
	if (right - left < cfg.scr_width) right = left + cfg.scr_width;
	CAMERA_TOP_BOUND = top;
	CAMERA_BOTTOM_BOUND = bottom;
	if (bottom - top < cfg.scr_height) bottom = top + cfg.scr_height;
}

// Возвращает в x, y сдвиг относительно объекта, к которому прицпленеа камера
void GetCameraAttachedFocusShift(float& x, float& y)
{
	if (attached_object)
	{
		switch (CAMERA_FOCUS_ON_OBJ_POS)
		{
		case CamFocusLeftCenter:
			x = -attached_object->aabb.W;
			break;
		case CamFocusRightCenter:
			x = attached_object->aabb.W;
			break;
		case CamFocusBottomCenter:
			y = attached_object->aabb.H;
			break;
		case CamFocusTopCenter:
			y = -attached_object->aabb.H;
			break;
		case CamFocusLeftTopCorner:
			x = -attached_object->aabb.W;
			y = -attached_object->aabb.H;
			break;
		case CamFocusLeftBottomCorner:
			x = -attached_object->aabb.W;
			y = attached_object->aabb.H;
			break;
		case CamFocusRightTopCorner:
			x = attached_object->aabb.W;
			y = -attached_object->aabb.H;
			break;
		case CamFocusRightBottomCorner:
			x = attached_object->aabb.W;
			y = attached_object->aabb.H;
			break;
		default: break;
		}
	}
}

// Обновляет положение камеры
void CameraUpdatePosition()
{
	// Мы сдвигаем систему координат OpenGL - в экран попадает то, что
	// мы должны увидеть.

	//glTranslated( floor(CAMERA_OFF_X+0.5f)+0.375, floor(CAMERA_OFF_Y+0.5f)+0.375, 0);
	glTranslated( CAMERA_OFF_X+0.375, CAMERA_OFF_Y+0.375, 0);
}


void CameraUpdateFocus()
{
	CAMERA_OLD_OFF_X = CAMERA_OFF_X;
	CAMERA_OLD_OFF_Y = CAMERA_OFF_Y;

	if (attached_object)
	{
		float foc_x = 0;
		float foc_y = 0;
		GetCameraAttachedFocusShift(foc_x, foc_y);

		if (camera_attach_x)
		{
			camera_pos.x = camera_pos.x + (attached_object->aabb.p.x - CAMERA_OBJ_OFF_X + foc_x - camera_pos.x)*CAMERA_LAG;
			CAMERA_X = floor( camera_pos.x + 0.5f);
			CalcCamOffX();
		}
		if (camera_attach_y)
		{
			float target_y = attached_object->aabb.p.y - CAMERA_OBJ_OFF_Y + foc_y;
			if (attached_object->type == objPlayer) target_y +=((ObjPlayer*)attached_object)->camera_offset;
			camera_pos.y = camera_pos.y + ( target_y - camera_pos.y)*CAMERA_LAG;
			CAMERA_Y = floor( camera_pos.y + 0.5f);
			CalcCamOffY();
		}
	}

	if (camera_use_bounds)
		ApplyBounds();
}

void CameraSetLag( float lag )
{
	CAMERA_LAG = 1-lag;
}
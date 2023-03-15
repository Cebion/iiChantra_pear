#ifndef __CAMERA_H_
#define __CAMERA_H_

#include "objects/object.h"

// Точки объекта, на которых может фокусироваться камера
enum CameraFocusObjectPoint { 
	CamFocusCenter,					// Центр
	CamFocusBottomCenter,			// Середина нижнего края
	CamFocusTopCenter,				// Середина верхнего края
	CamFocusLeftCenter,				// Середина левого края
	CamFocusRightCenter,			// Середина правого края
	CamFocusLeftTopCorner,			// Левый верхний угол
	CamFocusLeftBottomCorner,		// Левый нижний угол
	CamFocusRightTopCorner,			// Правый верхний угол
	CamFocusRightBottomCorner		// Правый нижний угол
};

void CameraMoveToPos(float x, float y);

void CameraUpdatePosition();
void CameraUpdateFocus();

void SetCameraAttachedObjectOffset(float x, float y);
void CameraAttachToAxis(bool x, bool y);
void CameraAttachToObject(GameObject* obj);
void CameraUseBounds(bool b);
void CameraSetBounds(float left, float right, float top, float bottom);
GameObject* GetCameraAttachedObject();
void GetCameraAttachedFocusShift(float& x, float& y);
void CameraSetLag( float lag );

#endif // __CAMERA_H_
#ifndef __CAMERA_H_
#define __CAMERA_H_

#include "objects/object.h"

// ����� �������, �� ������� ����� �������������� ������
enum CameraFocusObjectPoint { 
	CamFocusCenter,					// �����
	CamFocusBottomCenter,			// �������� ������� ����
	CamFocusTopCenter,				// �������� �������� ����
	CamFocusLeftCenter,				// �������� ������ ����
	CamFocusRightCenter,			// �������� ������� ����
	CamFocusLeftTopCorner,			// ����� ������� ����
	CamFocusLeftBottomCorner,		// ����� ������ ����
	CamFocusRightTopCorner,			// ������ ������� ����
	CamFocusRightBottomCorner		// ������ ������ ����
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
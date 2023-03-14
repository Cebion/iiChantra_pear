#ifndef __PHYSCOLLISIONSOLVER_H_
#define __PHYSCOLLISIONSOLVER_H_

#include "../objects/object_physic.h"

void ProcessCollisions();

void SolveCollision(ObjPhysic* obj1, ObjPhysic* obj2);
void SolveResizeCollision(ObjPhysic* objD, ObjPhysic* objS);
bool SolveVSSlope(const CAABB& aabb, ObjPhysic* slope, Vector2 shift );
float GetSlopeTop(ObjPhysic* slope, float x );
float GetSlopeX(ObjPhysic* slope, float y );

void UpdateSAPState();
void UpdateSAPObject(UINT sap_handle, const ASAP_AABB& aabb);

#endif // __PHYSCOLLISIONSOLVER_H_
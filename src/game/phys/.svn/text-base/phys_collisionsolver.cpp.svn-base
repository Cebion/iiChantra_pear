#include "StdAfx.h"

#include "../objects/object_dynamic.h"
#include "../objects/object_bullet.h"
#include "../objects/object_item.h"
#include "../objects/object_player.h"
#include "../objects/object_enemy.h"
#include "../objects/object_ray.h"

#include "phys_collisionsolver.h"

#include "sap/OPC_ArraySAP.h"
#include "../object_manager.h"

#include "../editor.h"

//////////////////////////////////////////////////////////////////////////

extern Opcode::ArraySAP *asap;

#ifdef DEBUG_DRAW_COLLISION_PAIRS
extern list<Vector2> collPairs;
#endif // DEBUG_DRAW_COLLISION_PAIRS

extern bool isNewGameCreated;

#ifdef MAP_EDITOR
extern UINT editorAreaSelectorObject;
extern bool editorPhysicProcessed;
extern bool editorAreaSelectNeeded;
#endif // MAP_EDITOR

//////////////////////////////////////////////////////////////////////////

void SolveDynamicVSStatic(ObjDynamic* objD, ObjPhysic* objS);
void SolveBullet(ObjBullet* bul, ObjPhysic* obj);
void SolveEnvironment(ObjEnvironment* env, ObjDynamic* obj);
void SolveDelayedEnvCollision();
void SolveEnvLeave(ObjEnvironment* env, ObjDynamic* obj);

bool SolveRay(ObjRay* ray, ObjPhysic* obj);

void ResolveCollisionMTD(ObjDynamic* dyn, const CAABB &bb);
bool ResolveCollision( int sX, int sY, ObjDynamic* dyn, const CAABB &bb );

void ProcessRayCollision();

//////////////////////////////////////////////////////////////////////////

// Список окружений и объектов, которые, возможно, будут пересекаться в конце хода.
// Первым идет окружение, вторым - объект.
list<GameObject*> delayed_env_coll;

#ifdef DEBUG_PRINT
UINT dbgPairsCount = 0;
#endif // DEBUG_PRINT

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Тут объявдены callback-и на создание и удаление пар пересечений. Пока что использовались только для отладки.
//#if defined(DEBUG_SAP_PAIRS_COORDS)
	#define USE_PAIR_CALLBACK
//#endif // defined(DEBUG_SAP_PAIRS_COORDS)

#ifdef USE_PAIR_CALLBACK
void* OnPairCreate(const void* object0, const void* object1, void* user_data)
{
#ifdef DEBUG_SAP_PAIRS_COORDS
	const ObjPhysic* o0 = static_cast<const ObjPhysic*>(object0);
	const ObjPhysic* o1 = static_cast<const ObjPhysic*>(object1);

	sLog(DEFAULT_LOG_NAME, logLevelInfo, "Added pair %d (%.2f, %.2f - %.2f, %.2f) - %d (%.2f, %.2f - %.2f, %.2f)",
		o0->id, o0->aabb.Left(), o0->aabb.Top(), o0->aabb.Right(), o0->aabb.Bottom(), o1->id, o1->aabb.Left(), o1->aabb.Top(), o1->aabb.Right(), o1->aabb.Bottom());
#endif // DEBUG_SAP_PAIRS_COORDS
	return NULL;
}

void  OnPairRemove(const void* object0, const void* object1, void* user_data, void* pair_user_data)
{
	const ObjPhysic* o0 = static_cast<const ObjPhysic*>(object0);
	const ObjPhysic* o1 = static_cast<const ObjPhysic*>(object1);
#ifdef DEBUG_SAP_PAIRS_COORDS
	sLog(DEFAULT_LOG_NAME, logLevelInfo, "Removed pair %d (%.2f, %.2f - %.2f, %.2f) - %d (%.2f, %.2f - %.2f, %.2f)",
		o0->id, o0->aabb.Left(), o0->aabb.Top(), o0->aabb.Right(), o0->aabb.Bottom(), o1->id, o1->aabb.Left(), o1->aabb.Top(), o1->aabb.Right(), o1->aabb.Bottom());
#endif // DEBUG_SAP_PAIRS_COORDS

	if (o0->type == objEnvironment && o1->IsDynamic())
		SolveEnvLeave((ObjEnvironment*)o0, (ObjDynamic*)o1);
	else if  (o1->type == objEnvironment && o0->IsDynamic())
		SolveEnvLeave((ObjEnvironment*)o1, (ObjDynamic*)o0);
}
#endif // USE_PAIR_CALLBACK


////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////// 


// Этот блок код отвечает за то, чтобы никто ничего не делал с SAP в то время, когда 
// работает функция ProcessCollisions. Если кто-то вызовет asap->DumpPairs или asap->UpdateObject,
// то почти наверняка изменится содержимаое массива asap->mPairs.mActivePairs или даже его адрес,
// что привелет к тому, что в ProcessCollisions окажется указатель на устаревший массив.
// Чтобы этого не произошло, мы "лочим" состояние SAP и запоминаем, что нас просили вызвать 
// DumpPairs или UpdateObject. Затем, в конце ProcessCollisions мы "разлочим" SAP и 
// выполним все запросы.

bool sapStateLocked = false;
bool sapStateUpdatePending = false;
bool sapObjUpdatePending = false;
class UpdateObjInfo
{
public:
	UINT handle;
	ASAP_AABB aabb;
	
	UpdateObjInfo(UINT h, const ASAP_AABB& a) 
		: handle(h), aabb(a)
	{ }


};

list<UpdateObjInfo> updateObjList;

// Вызывать эту функцию вместо DumpPairs(NULL, NULL, NULL, NULL)
void UpdateSAPState()
{
	if (sapStateLocked)
	{
		sapStateUpdatePending = true;
		return;
	}

#ifdef USE_PAIR_CALLBACK
	asap->DumpPairs(OnPairCreate, OnPairRemove, NULL, NULL);
#else
	asap->DumpPairs(NULL, NULL, NULL, NULL);
#endif // USE_PAIR_CALLBACK

	sapStateUpdatePending = false;
}

// Вызывать эту функцию вместо UpdateObject
void UpdateSAPObject(UINT sap_handle, const ASAP_AABB& aabb)
{
	if (sapStateLocked)
	{
		sapObjUpdatePending = true;
		updateObjList.push_back(UpdateObjInfo(sap_handle, aabb));
	}
	else
	{
		asap->UpdateObject(sap_handle, aabb);
	}

}

void BatchSAPObjUpdate()
{
	if (!sapObjUpdatePending)
		return;
	ASSERT(!sapStateLocked);
	ASSERT(!updateObjList.empty());
	for (list<UpdateObjInfo>::iterator it = updateObjList.begin(); it != updateObjList.end(); it++)
	{
		UpdateObjInfo& uoi = *it;
		asap->UpdateObject(uoi.handle, uoi.aabb);
	}
	updateObjList.clear();
	sapObjUpdatePending = false;
}




//////////////////////////////////////////////////////////////////////////

// Обработка столкновений
void ProcessCollisions()
{
	Opcode::ASAP_Pair* pairs;

#ifdef DEBUG_SAP
	sLog(DEFAULT_LOG_NAME, logLevelInfo, "Beginning ProcessCollisions");
#endif // DEBUG_SAP

#ifdef DEBUG_DRAW_COLLISION_PAIRS
	collPairs.clear();
#endif // DEBUG_DRAW_COLLISION_PAIRS

#ifdef DEBUG_PRINT
	dbgPairsCount = 0;
#endif // DEBUG_PRINT

#ifdef MAP_EDITOR
	if (!editorAreaSelectNeeded) return;
#endif // MAP_EDITOR

	ASSERT(!sapStateLocked);	// Если упало тут, то значит как-то получился вложенный вызов ProcessCollisions
	sapStateLocked = true;

#ifdef USE_PAIR_CALLBACK
	UINT pairsCount = asap->DumpPairs(OnPairCreate, OnPairRemove, NULL, &pairs);
#else
	UINT pairsCount = asap->DumpPairs(NULL, NULL, NULL, &pairs);
#endif // USE_PAIR_CALLBACK
	
#ifdef DEBUG_PRINT
	dbgPairsCount = pairsCount;
#endif // DEBUG_PRINT
#ifdef DEBUG_SAP
	sLog(DEFAULT_LOG_NAME, logLevelInfo, "pairs count = %d", pairsCount);
#endif // DEBUG_SAP

	if (pairsCount)
	{
		ObjPhysic** odArr = new ObjPhysic*[pairsCount * 2];
		for(UINT i = 0; i < pairsCount; i++)
		{
			odArr[i*2 + 0] = (ObjPhysic*)pairs[i].object0;
			odArr[i*2 + 1] = (ObjPhysic*)pairs[i].object1;
			// Решение конкретной пары столкновений
			SolveCollision(odArr[i*2 + 0], odArr[i*2 + 1]);
			if (isNewGameCreated)
			{
				DELETEARRAY(odArr);
				delayed_env_coll.clear();
				ASSERT(sapStateLocked);
				sapStateLocked = false;
				updateObjList.clear();
				return;
			}
		}
		for(UINT i = 0, j = 0; i < pairsCount * 2; i+=2)
		{
			j = i+1;
			// Обновление положения объектов после того, как их столкновение было решено
			asap->UpdateObject(odArr[i]->sap_handle, odArr[i]->aabb.GetASAP_AABB() );
			asap->UpdateObject(odArr[j]->sap_handle, odArr[j]->aabb.GetASAP_AABB() );
		}
		DELETEARRAY(odArr);
	}

	SolveDelayedEnvCollision();
	ProcessRayCollision();

	ASSERT(sapStateLocked);
	sapStateLocked = false;
	if (sapStateUpdatePending) UpdateSAPState();
	BatchSAPObjUpdate();

#ifdef MAP_EDITOR
	if (editorAreaSelectNeeded) editorPhysicProcessed = true;
#endif // MAP_EDITOR

#ifdef DEBUG_SAP
	sLog(DEFAULT_LOG_NAME, logLevelInfo, "Ending ProcessCollisions");
#endif // DEBUG_SAP

}

//////////////////////////////////////////////////////////////////////////

// Массив для указателй объектов, с котрыми луч пересекся
ObjPhysic* arRayObjIntersections[maxRayIntersectionsCount];
// Массив расстояний от начала луча до точек пересечения луча с обхъектами
float arRayObjIntersectionsDistances[maxRayIntersectionsCount];

// В это функции идет заполнение массивов объектов, с которыми луч пересекатеся, 
// и расстояний до них.
UINT GetRayIntersections(bool dir, scalar posFirstCoord, scalar limit, 
			  AxisIndex fa, AxisIndex sa, ObjRay* ray) 
{
	CRay& r = ray->ray;

	const Opcode::ASAP_Box* boxes = asap->GetBoxes();
	const Opcode::ASAP_EndPoint* firstAxis	= asap->GetEndPoints(fa);
	const Opcode::ASAP_EndPoint* secondAxis = asap->GetEndPoints(sa);

	const Opcode::ASAP_EndPoint* current = firstAxis + ( dir ? 0 : (asap->GetNbUsedBoxes()*2 + 1) );
	assert(current->mValue == (dir ? MIN_FLOAT : MAX_FLOAT));

	const Opcode::ASAP_Box* object = NULL;
	const Opcode::ASAP_EndPoint* anotherEP;				// Это будет точка, противоположная найденной на firstAxis
	const Opcode::ASAP_EndPoint *minEP_SA, *maxEP_SA;	// Это точки на secondAxis
	scalar leftVal, rightVal, topVal, bottomVal;

	// В принципе, постоянный мемсет, кажется, не нужен. Все равно мы перебиваем создержимое с самого начала
	// и потом читаем ровно столько, сколько забили.
	//memset(arRayObjIntersections, 0, sizeof(arRayObjIntersections));
	//memset(arRayObjIntersectionsDistances, 0, sizeof(arRayObjIntersectionsDistances));
	UINT intersCount = 0;
	Vector2 intersPoint;

	// Далее идет перебор точек на оси firstAxis, берутся объекты и проверяется их пересечение с лучом
	if (dir)
	{
		// Движемся вправо/вниз

		// Отбрасываем те, что слева
		// TODO: тут бы бинарный поиск, а не линейный
		while((++current)->mValue < posFirstCoord);

		--current;

		// Ищем пересечения
		while( (++current)->mValue < limit && intersCount < maxRayIntersectionsCount )
		{
			object = boxes + current->GetOwner();

			if (current->IsMax())
			{
				anotherEP = firstAxis + object->mMin[fa];
				if (anotherEP->mValue <= posFirstCoord)
				{
					// Случай когда возможно внутри, все равно проверяем
					minEP_SA = secondAxis + object->mMin[sa]; 
					maxEP_SA = secondAxis + object->mMax[sa];

					if (fa == X_)
					{
						leftVal		= anotherEP->mValue;
						rightVal	= current->mValue;
						topVal		= minEP_SA->mValue;
						bottomVal	= maxEP_SA->mValue;
					}
					else
					{
						topVal		= anotherEP->mValue;
						bottomVal	= current->mValue;
						leftVal		= minEP_SA->mValue;
						rightVal	= maxEP_SA->mValue;
					}


					if (r.GetIntersectionPoint(leftVal, topVal, rightVal, bottomVal, intersPoint) && 
						ray->CheckIntersection((ObjPhysic*)object->mObject))
					{
						arRayObjIntersections[intersCount] = (ObjPhysic*)object->mObject;
						intersPoint -= r.p;
						arRayObjIntersectionsDistances[intersCount++] = intersPoint.Length();
					}
				}
			}
			else
			{
				// Нашли левую/верхнюю границу, проверяем
				anotherEP	= firstAxis + object->mMax[fa];
				minEP_SA	= secondAxis + object->mMin[sa]; 
				maxEP_SA	= secondAxis + object->mMax[sa];

				if (fa == X_)
				{
					leftVal		= current->mValue;
					rightVal	= anotherEP->mValue;
					topVal		= minEP_SA->mValue;
					bottomVal	= maxEP_SA->mValue;
				}
				else
				{
					topVal		= current->mValue;
					bottomVal	= anotherEP->mValue;
					leftVal		= minEP_SA->mValue;
					rightVal	= maxEP_SA->mValue;
				}


				if (r.GetIntersectionPoint(leftVal, topVal, rightVal, bottomVal, intersPoint) && 
					ray->CheckIntersection((ObjPhysic*)object->mObject))
				{
					arRayObjIntersections[intersCount] = (ObjPhysic*)object->mObject;
					intersPoint -= r.p;
					arRayObjIntersectionsDistances[intersCount++] = intersPoint.Length();
				}
			}
		}
	}
	else
	{
		// Движемся влево/верх

		// Отбрасываем те, что справа
		// TODO: тут бы бинарный поиск, а не линейный
		while((--current)->mValue > posFirstCoord);

		++current;

		// Ищем пересечения
		while( (--current)->mValue > limit && intersCount < maxRayIntersectionsCount)
		{
			object = boxes + current->GetOwner();

			if (current->IsMax())
			{
				// Нашли правую границу, проверяем
				anotherEP	= firstAxis + object->mMin[fa];
				minEP_SA	= secondAxis + object->mMin[sa]; 
				maxEP_SA	= secondAxis + object->mMax[sa];

				if (fa == X_)
				{
					leftVal		= anotherEP->mValue;
					rightVal 	= current->mValue;
					topVal		= minEP_SA->mValue;
					bottomVal	= maxEP_SA->mValue;
				}
				else
				{
					topVal 		= anotherEP->mValue;
					bottomVal 	= current->mValue;
					leftVal		= minEP_SA->mValue;
					rightVal 	= maxEP_SA->mValue;
				}


				if (r.GetIntersectionPoint(leftVal, topVal, rightVal, bottomVal, intersPoint) && 
					ray->CheckIntersection((ObjPhysic*)object->mObject))
				{
					arRayObjIntersections[intersCount] = (ObjPhysic*)object->mObject;
					intersPoint -= r.p;
					arRayObjIntersectionsDistances[intersCount++] = intersPoint.Length();
				}
			}
			else
			{
				anotherEP = firstAxis + object->mMax[fa];
				if (anotherEP->mValue >= posFirstCoord)
				{
					// Случай когда возможно внутри, все равно проверяем
					minEP_SA = secondAxis + object->mMin[sa]; 
					maxEP_SA = secondAxis + object->mMax[sa];

					if (fa == X_)
					{
						leftVal		= current->mValue;
						rightVal	= anotherEP->mValue;
						topVal		= minEP_SA->mValue;
						bottomVal	= maxEP_SA->mValue;
					}
					else
					{
						topVal		= current->mValue;
						bottomVal	= anotherEP->mValue;
						leftVal		= minEP_SA->mValue;
						rightVal	= maxEP_SA->mValue;
					}


					if (r.GetIntersectionPoint(leftVal, topVal, rightVal, bottomVal, intersPoint) && 
						ray->CheckIntersection((ObjPhysic*)object->mObject))
					{
						arRayObjIntersections[intersCount] = (ObjPhysic*)object->mObject;
						intersPoint -= r.p;
						arRayObjIntersectionsDistances[intersCount++] = intersPoint.Length();
					}
				}
			}
		}
	}

	return intersCount;
}

// В этой функции идет поиск пересечения данного луча со всеми боксами в SAP
void FindRayAABBintersection(ObjRay* ray)
{
	// TODO: используется для отладки пересечений лучей, убрать
	ray->debug_color = RGBAf(1.0f, 1.0f, 1.0f, 1.0f);

	// Возможно, не стоит здесь так сразу перерывать.
	if (ray->activity == oatDying)
		return;

	if(asap->GetNbUsedBoxes() == 0)
		return;

	CRay& r = ray->ray;
	bool dir = r.dir;
	bool isVert = r.IsVertical();
	//bool isHor = r.IsHorizontal();

	assert(asap);	

	// Определяем первую ось (ту, по которой будем двигаться) и вторую
	// Для вертикального луча будем двигаться по оси Y
	AxisIndex fa = isVert ? Y_ : X_;
	AxisIndex sa = isVert ? X_ : Y_;

	scalar posFirstCoord = isVert ? r.p.y : r.p.x;		// Начало луча на первой оси

	scalar limit;		// Граничное точка на оси firstAxis, на которой будет вестись поиск

	if (ray->searchDistance == 0)
	{
		// Без ограничений на дальность - почти до краев SAP
		limit = dir ? (MAX_FLOAT - 100) : (MIN_FLOAT + 100);
	}
	else
	{
		// Есть ограничение на дальность
		if (isVert)
		{
			// Луч строго вертикален, считать легко
			if (dir)
				limit = posFirstCoord + ray->searchDistance;
			else
				limit = posFirstCoord - ray->searchDistance;
		}
		else
		{
			// TODO: вычислять ray->searchDistance / sqrt(r.k*r.k + 1) - проекцию на ось
			// имеет смысл только при изменении k - угла наклона или расстояния поиска
			if (dir)
				limit = posFirstCoord + ray->searchDistance / sqrt(r.k*r.k + 1);
			else
				limit = posFirstCoord - ray->searchDistance / sqrt(r.k*r.k + 1);
		}
	}
	
	// TODO: Возможно стоит для горизонтальных и вертикальный лучей использовать 
	// упрощенный поиск без сортировки (котрый был в rev. 1198). 

	UINT intersCount = GetRayIntersections(dir, posFirstCoord, limit, fa, sa, ray);

	if (intersCount > 0)
	{
		// Массивы заполнены, теперь мы сортируем массив длин, получаем отсортированные индексы и 
		// используем их, чтобы в правильном порядке обойти объеты в массиве
		IceCore::RadixSort r;
		IceCore::RadixSort* RS = &r;
		const UINT* sorted = RS->Sort(arRayObjIntersectionsDistances, intersCount).GetRanks();

		const UINT* const lastSorted = &sorted[intersCount];
		UINT index;
		// Обходим объекты
		while(sorted < lastSorted)
		{
			index =  *sorted++;

			if (SolveRay(ray, arRayObjIntersections[index]))
				break;
		}
	}
}


// Тут перебираются все лучи для поиска их пересечений
void ProcessRayCollision()
{
	extern LSQ_HandleT RayTree;
	LSQ_IteratorT rayIter = LSQ_GetFrontElement(RayTree);
	if (rayIter != LSQ_HandleInvalid)
	{
		ObjRay* ray = NULL;
		for (; !LSQ_IsIteratorPastRear(rayIter); LSQ_AdvanceOneElement(rayIter))
		{
			ray = ((ObjRay*)LSQ_DereferenceIterator(rayIter));
			assert(ray);
			FindRayAABBintersection(ray);
		}
		LSQ_DestroyIterator(rayIter);
	}
}

// Возвращает true, если больше луч ни с кем не пересекается.
bool SolveRay(ObjRay* ray, ObjPhysic* obj)
{
	// TODO: временное средство для отладки пересечения лучей
	ray->debug_color = RGBAf(1.0f, 0.2f, 0.2f, 1.0f);

	if (ray->Hit(obj))
	{
		ray->debug_color = RGBAf(0.2f, 1.0f, 0.2f, 1.0f);
	}

	return ray->activity == oatDying;
}

//////////////////////////////////////////////////////////////////////////


//Проверяем, стоит ли объект на склоне.
bool SolveVSSlope(const CAABB& aabb, ObjPhysic* slope, Vector2 shift )
{
	if (!slope || !slope->slopeType) return false;

	if (!Collide(aabb, slope->aabb))
		return false;

	if ( aabb.Bottom() > slope->aabb.Bottom() ) return false;

	//if (obj->aabb.p.x+shift.x < slope->aabb.Left() || obj->aabb.p.x + shift.x > slope->aabb.Right() ) return false;
	//  /|
	// /_|
	if (slope->slopeType == 1)
	{
		if (aabb.Bottom()+shift.y > slope->aabb.Bottom() - 2*slope->aabb.H*((aabb.p.x+shift.x-slope->aabb.Left())/(2*slope->aabb.W)) &&
			aabb.p.x+shift.x > slope->aabb.Left()+2*slope->aabb.W/(aabb.Bottom()+shift.y-slope->aabb.Bottom()))
			return true;
		return false;
	}
	// |\ /
	// |_\/
	if (slope->slopeType == 2)
	{
		if (aabb.Bottom()+shift.y > slope->aabb.Bottom() - (2*slope->aabb.H)*((slope->aabb.Right() - aabb.p.x+shift.x)/(2*slope->aabb.W)) &&
			aabb.p.x+shift.x < slope->aabb.Right() - 2*slope->aabb.W/(aabb.Bottom()+shift.y - slope->aabb.Bottom()))
			return true;
		return false;
	}
	return false;
}

float GetSlopeTop(ObjPhysic* slope, float x )
{
	if (!slope) return false;
	//  /|
	// /_|
	if (slope->slopeType == 1)
		return slope->aabb.Bottom()-2*slope->aabb.H*((x-slope->aabb.Left())/(2*slope->aabb.W));
	// |\ /
	// |_\/
	if (slope->slopeType == 2)
		return slope->aabb.Bottom()-2*slope->aabb.H*((slope->aabb.Right()-x)/(2*slope->aabb.W));
	return false;
}

float GetSlopeX(ObjPhysic* slope, float y )
{
	if (!slope) return false;
	return slope->aabb.Left()+(y-slope->aabb.Bottom())/(2*slope->aabb.H);
}

bool TouchCondition( ObjDynamic* toucher, GameObject* obj )
{
	if ( obj->IsSleep() || toucher->IsSleep() || !toucher->sprite->IsVisible() ) return false;
	switch (obj->touch_detection)
	{
		case tdtFromEverywhere:
			{
				if (toucher->aabb.p.x - toucher->vel.x > obj->aabb.Right() || toucher->aabb.p.x - toucher->vel.x < obj->aabb.Left())
					return true;
				if (toucher->aabb.p.y - toucher->vel.y > obj->aabb.Bottom() || toucher->aabb.p.y - toucher->vel.y < obj->aabb.Top())
					return true;
				return false;
			}
		case tdtFromTop:
			{
				if (toucher->vel.y > 0 && toucher->aabb.p.y - toucher->vel.y < obj->aabb.Top() )
					return true;
				return false;
			}
		case tdtFromBottom:
			{
				if (toucher->vel.y < 0 && toucher->aabb.p.y - toucher->vel.y > obj->aabb.Bottom() )
					return true;
				return false;
			}
		case tdtFromSides:
			{
				if (toucher->aabb.p.x - toucher->vel.x > obj->aabb.Right() || toucher->aabb.p.x - toucher->vel.x < obj->aabb.Left())
					return true;
				return false;
			}
		case tdtTopAndSides:
			{
				if (toucher->aabb.p.y - toucher->vel.y > obj->aabb.Bottom())
					return false;
				return true;
			}
		default:
			return true;
	}
}

bool SolidTo( ObjPhysic* obj, ObjectType type )
{
	switch (type)
	{
		case objPlayer:
			return (obj->solid_to & 1) != 0;
		case objBullet:
			return (obj->solid_to & 2) != 0;
		case objEffect:
			return (obj->solid_to & 4) != 0;
		case objSprite:
			return (obj->solid_to & 8) != 0;
		case objItem:
			return (obj->solid_to & 16) != 0;
		case objEnemy:
			return (obj->solid_to & 32) != 0;
		default:
			return true;
	}
}

void SolveCollision(ObjPhysic* obj1, ObjPhysic* obj2)
{
	assert(obj1);
	assert(obj2);

#ifdef MAP_EDITOR
	if (obj1->id == editorAreaSelectorObject)
	{
		AddObjectInArea(obj2->id);
	}
	else if (obj2->id == editorAreaSelectorObject)
	{
		AddObjectInArea(obj1->id);
	}

	asap->RemovePair(obj1, obj2, obj1->id, obj2->id);
	return;
#endif

	if ( !SolidTo(obj1, obj2->type) || !SolidTo(obj2, obj1->type) )
	{
		asap->RemovePair(obj1, obj2, obj1->id, obj2->id);
		return;
	}

#ifdef DEBUG_DRAW_COLLISION_PAIRS
	collPairs.push_back(obj1->aabb.p);
	collPairs.push_back(obj2->aabb.p);
#endif // DEBUG_DRAW_COLLISION_PAIRS

	if ( (obj1->parentConnection && obj1->parentConnection->getParent() == obj2)
		|| (obj2->parentConnection && obj2->parentConnection->getParent() == obj1) )
	{
		asap->RemovePair(obj1, obj2, obj1->id, obj2->id);
	}
	else if (obj1->IsDynamic() && obj2->IsDynamic())
	{ 
		if (((ObjDynamic*)obj1)->IsBullet() )
		{
			SolveBullet((ObjBullet*)obj1, obj2);
			return;
		}

		if (((ObjDynamic*)obj2)->IsBullet() )
		{
			SolveBullet((ObjBullet*)obj2, obj1);
			return;
		}

		if ( !obj1->IsSolid() && obj2->IsSolid() ) //С пулей плохая проверка, но иначе они выталкиываются
			SolveDynamicVSStatic( (ObjDynamic*)obj1, obj2 );
		else if ( obj1->IsSolid() && !obj2->IsSolid() )
			SolveDynamicVSStatic( (ObjDynamic*)obj2, obj1 );

		if ( obj1->type == objPlayer && obj2->IsTouchable() && TouchCondition((ObjDynamic*)obj1, obj2) ) //Без этого не работают предметы. Бида.
		{
			switch ( obj2->type )
			{
				case objItem:
					((ObjItem*)obj2)->Touch( (ObjDynamic*)obj1 );
					return;
				case objEnemy:
					((ObjEnemy*)obj2)->Touch( (ObjDynamic*)obj1 );
					return;
				default:
					obj2->Touch( (ObjDynamic*)obj1 );
					return;
			}
		}
		if ( obj2->type == objPlayer && obj1->IsTouchable() && TouchCondition((ObjDynamic*)obj2, obj1) ) //Без этого не работают предметы. Бида.
		{
			switch ( obj1->type )
			{
				case objItem:
					((ObjItem*)obj1)->Touch( (ObjDynamic*)obj2 );
					return;
				case objEnemy:
					((ObjEnemy*)obj1)->Touch( (ObjDynamic*)obj2 );
					return;
				default:
					obj1->Touch( (ObjDynamic*)obj2 );
					return;
			}
		}

	}
	else if (obj1->IsDynamic() && !obj2->IsDynamic())
	{
		CAABB old_aabb = obj1->aabb;
		if (((ObjDynamic*)obj1)->IsBullet() )
			SolveBullet((ObjBullet*)obj1, obj2);
		else 
			SolveDynamicVSStatic((ObjDynamic*)obj1, obj2);
		
		if (obj1->childrenConnection && obj1->aabb != old_aabb)
			obj1->childrenConnection->Event( ObjectEventInfo( eventCollisionPushed, obj1->aabb.p-old_aabb.p));
	}
	else if (!obj1->IsDynamic() && obj2->IsDynamic())
	{
		CAABB old_aabb = obj2->aabb;
		if (((ObjDynamic*)obj1)->IsBullet() )
			SolveBullet((ObjBullet*)obj2, obj1);
		else 
			SolveDynamicVSStatic((ObjDynamic*)obj2, obj1);
		
		if (obj2->childrenConnection && obj1->aabb != old_aabb)
			obj2->childrenConnection->Event( ObjectEventInfo( eventCollisionPushed, obj2->aabb.p-old_aabb.p));
	}
	else
	{
		asap->RemovePair(obj1, obj2, obj1->id, obj2->id);
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//Разрешает столкновение строго к центру динамического объекта.
void SolveDynamicVSStatic(ObjDynamic* objD, ObjPhysic* objS)
{
	//Летающие объекты не пересекаются с односторонними платформами и спокойно летают через них.
	if ( objS->IsOneSide() && objD->gravity == Vector2::Blank() )
		return;

	//Если вдруг это окружение
	if ( objS->type == objEnvironment )
	{
		SolveEnvironment((ObjEnvironment*)objS, objD);
		return;
	}

	//Если вдруг это склон
	if ( objS->slopeType > 0)
	{
		if ( objD->type == objBullet ) return; //Выстрелы не ходят по склонам


		if (SolveVSSlope( objD->aabb, objS, Vector2(0,0)))
		{
			if (objS->IsOneSide() && (objD->IsDropping() || objD->drop_from == objS) )
			{
				// Спрыниваем с контра-лайка
				objD->ClearOnPlane();
			}
			else
			{
				float slopeY = GetSlopeTop(objS, objD->aabb.p.x) - objD->aabb.H;
				// Выталкиваем вверх к склону. Если контралайк, то выталкиваем вверх, если движемся вних и не удалились ниже, чем на 10 точек.
				// TODO: 10 работает неплохо, но это магическое число. Магия отказывает в самый неподходящий момент.
				if (objD->aabb.p.y > slopeY && (!objS->IsOneSide() || (objD->vel.y > 0 && abs(slopeY - objD->aabb.p.y) < 10) ) ) 
				{
					objD->aabb.p.y = slopeY;
			
					objD->SetOnPlane();
					objD->drop_from = NULL;
				
					objD->vel.y = 0;
					objD->suspected_plane = objS;
				}
			}
		}
		return;
	}
	//Конец склонов

	if (!objS->IsSolid())
		return;

	if (objD->ghostlike)
		return;

	bool collided = false;
	CAABB tmp(objD->aabb);
	tmp.p -= objD->vel;

	int SegStateX = SegProjIntersect(Vector2(tmp.p.x - tmp.W, tmp.p.x + tmp.W),
		Vector2(objS->aabb.p.x - objS->aabb.W, objS->aabb.p.x + objS->aabb.W));

	int SegStateY = SegProjIntersect(Vector2(tmp.p.y - tmp.H, tmp.p.y + tmp.H),
		Vector2(objS->aabb.p.y - objS->aabb.H, objS->aabb.p.y + objS->aabb.H));

	int oSx = SegProjIntersect(Vector2(objD->old_aabb.p.x - objD->old_aabb.W, objD->old_aabb.p.x + objD->old_aabb.W),
		Vector2(objS->aabb.p.x - objS->aabb.W, objS->aabb.p.x + objS->aabb.W));

	int oSy = SegProjIntersect(Vector2(objD->old_aabb.p.y - objD->old_aabb.H, objD->old_aabb.p.y + objD->old_aabb.H),
		Vector2(objS->aabb.p.y - objS->aabb.H, objS->aabb.p.y + objS->aabb.H));

	// Если мы пересекаемся ещё до применения скорости, то....
	if (SegStateX == SEG_PROJ_STATE_INTERSECT && SegStateY == SEG_PROJ_STATE_INTERSECT && !(objS->IsOneSide()))
	{
		// ...используем данные с прошлого шага
		// А вдруг в угол нах на прошлом шаге?!+
		if ((oSx != SEG_PROJ_STATE_INTERSECT) && (oSy != SEG_PROJ_STATE_INTERSECT))
			CornerProblemSolve(oSx, oSx, tmp, objS->aabb, objD->vel);

		if (!(oSx == SEG_PROJ_STATE_INTERSECT && oSy == SEG_PROJ_STATE_INTERSECT))
		{
			if (ResolveCollision(oSx, oSy, objD, objS->aabb)) objD->suspected_plane = objS;
		}
		else
			// Но если и там бида, то резолвим по мин дистанции
			ResolveCollisionMTD(objD, objS->aabb);
		// хз зачем мы это опять присваиваем...
		tmp = CAABB(objD->aabb);
		//oSx = SegStateX;
		//oSy = SegStateY;
		return;
	}

	if (Collide(objD->aabb, objS->aabb))
	{
		collided = true;

		//Собственно если угол, то мы его решаем, и потом вызывем резолв колллижен.
		if ((SegStateX != SEG_PROJ_STATE_INTERSECT) && (SegStateY != SEG_PROJ_STATE_INTERSECT) && !(objS->IsOneSide()))
			CornerProblemSolve(SegStateX, SegStateY, tmp, objS->aabb, objD->vel);

		if ( (objS->IsOneSide() && objD->IsDropping() && objS->IsForced() && objD->type == objPlayer)
			|| (!objS->IsOneSide() && objD->IsDropping()))
			{
				//Вот такой хак, чтобы не показывать приземление снова.
				((ObjCharacter*)objD)->movement = omtDropSitting;
			}
		if ( objS->IsOneSide() && !(objD->IsDropping() && objS->IsForced()) )
		{
			if ( ((objD->aabb.p.y)-2*(objD->vel.y)+(objD->aabb.H)-1 <= (objS->aabb.p.y)-(objS->aabb.H)) && (objD->vel.y > 0 && (!objD->IsDropping() || objS->IsForced()) )
				&& !(objD->IsBullet() && objD->bounce+objD->env->bounce_bonus ==0 ))
			{
					BOUNCE_Y(objD);
					objD->aabb.p.y =objS->aabb.p.y - objS->aabb.H - objD->aabb.H - 1;
					objD->SetOnPlane();
					objD->drop_from = NULL;
					objD->suspected_plane = objS;
			}
		}
		else if (ResolveCollision(SegStateX, SegStateY, objD, objS->aabb)) objD->suspected_plane = objS;
		// хз зачем мы это опять присваиваем...
		//tmp = AABB(dyn->aabb);
	}
}

void SolveBullet(ObjBullet* bul, ObjPhysic* obj)
{
	if ( obj->slopeType > 0 )
	{
		if (SolveVSSlope(bul->aabb, obj, Vector2(0,0)))
		{
			bul->Hit(obj);
		}
	}
	else bul->Hit(obj);
	//Если пуля уничтожается при попадании, то она уже начала этот процесс. Но на случай, если нет - нужно решить столкновение.
	if ( bul->activity != oatDying && SolidTo( obj, objBullet ) ) SolveDynamicVSStatic( bul, obj );
}

void SolveEnvironment(ObjEnvironment* env, ObjDynamic* obj)
{
	
	if(env == obj->env && env != default_environment)
	{
		//obj->env = env;\e
		env->OnStay(obj);
	}
	else
	{
		// Откладываем обработку пересечения с новым окружением
		delayed_env_coll.push_back(env);
		delayed_env_coll.push_back(obj);
	}
	

}

// Отлоденная обработка пересечений с окружениями. Отрабатывает после того, как 
// к объекту применяться все изменения полоедния, вызванные дургими пересечениями.
void SolveDelayedEnvCollision()
{
	if (delayed_env_coll.empty())
		return;

	ObjDynamic* obj = NULL;
	ObjEnvironment* env = NULL;
	list<GameObject*>::iterator it = delayed_env_coll.begin();
	
	while(it != delayed_env_coll.end())
	{
		env = (ObjEnvironment*)*it;
		it++;
		obj = (ObjDynamic*)*it;
		it++;

		if (Collide(env->aabb, obj->aabb))
		{
			// Объект действительно попал в новое окружение.
			if ( obj->env->id < env->id )
			{
				obj->SetEnv(env);
			}
		}
	}

	delayed_env_coll.clear();
}


// Вызовется раньше, чем SolveEnvironment и SolveDelayedEnvCollision
void SolveEnvLeave(ObjEnvironment* env, ObjDynamic* obj)
{
	if (env == obj->env)
		obj->SetEnv(default_environment);
}

//////////////////////////////////////////////////////////////////////////

// Резолв коллиежена по MTD MTD = Minimal Translation Distance
// Код из функции void PhysDynamicBody::SatanaResolveCollisionMTD(const ASAP_AABB &bb)
void ResolveCollisionMTD(ObjDynamic* dyn, const CAABB &bb)
{
	scalar deltaX, deltaY;
	SegIntersect(	Vector2(dyn->aabb.p.x - dyn->aabb.W, dyn->aabb.p.x + dyn->aabb.W),
		Vector2(bb.p.x - bb.W, bb.p.x + bb.W), deltaX	);
	SegIntersect(	Vector2(dyn->aabb.p.y - dyn->aabb.H, dyn->aabb.p.y + dyn->aabb.H),
		Vector2(bb.p.y - bb.H, bb.p.y + bb.H), deltaY	);
	if (fabs(deltaX) < fabs(deltaY))
	{
		dyn->aabb.p.x += deltaX;
		dyn->vel.x = 0.0f;
	}
	else
	{
		dyn->aabb.p.y += deltaY;
		dyn->vel.y = 0.0f;
	}
}

// Там внизу всё очевидно
bool ResolveCollision( int sX, int sY, ObjDynamic* dyn, const CAABB &bb )
{
	bool mark_as_suspected = false;
	if (sX == SEG_PROJ_STATE_INTERSECT)
	{
		if (sY == SEG_PROJ_STATE_BEFORE)
		{
			BOUNCE_Y(dyn);
			dyn->aabb.p.y = bb.p.y - bb.H - dyn->aabb.H - 1;
			dyn->SetOnPlane();
			dyn->drop_from = NULL;
			mark_as_suspected = true;
		}
		if (sY == SEG_PROJ_STATE_AFTER)
		{
			BOUNCE_Y(dyn);
			dyn->aabb.p.y = bb.p.y + bb.H + dyn->aabb.H + 1;
			dyn->ClearOnPlane();
		}
	}

	if (sY == SEG_PROJ_STATE_INTERSECT)
	{
		if (sX == SEG_PROJ_STATE_BEFORE)
		{
			BOUNCE_X(dyn);
			dyn->aabb.p.x = bb.p.x - bb.W - dyn->aabb.W - 1;
		}
		if (sX == SEG_PROJ_STATE_AFTER)
		{
			BOUNCE_X(dyn);
			dyn->aabb.p.x = bb.p.x + bb.W + dyn->aabb.W + 1;
		}
	}

	if (sY == SEG_PROJ_STATE_BEFORE)
	{
		if (sX == SEG_PROJ_STATE_BEFORE )
		{

			BOUNCE_X(dyn);
			BOUNCE_Y(dyn);
			dyn->aabb.p.x = bb.p.x - bb.W - dyn->aabb.W - 1;
			dyn->aabb.p.y = bb.p.y - bb.H - dyn->aabb.H - 1;
		}
		if (sX == SEG_PROJ_STATE_AFTER )
		{

			BOUNCE_X(dyn);
			BOUNCE_Y(dyn);
			dyn->aabb.p.x = bb.p.x + bb.W + dyn->aabb.W + 1;
			dyn->aabb.p.y = bb.p.y - bb.H - dyn->aabb.H - 1;
		}
	}

	if (sY == SEG_PROJ_STATE_AFTER)
	{
		if (sX == SEG_PROJ_STATE_BEFORE )
		{

			BOUNCE_Y(dyn);
			dyn->aabb.p.y = bb.p.y + bb.H + dyn->aabb.H + 1;
		}
		if (sX == SEG_PROJ_STATE_AFTER )
		{

			BOUNCE_Y(dyn);
			dyn->aabb.p.y = bb.p.y + bb.H + dyn->aabb.H + 1;
		}
	}
	return mark_as_suspected;
}

#include "StdAfx.h"

#include "../../render/draw.h"
#include "../../render/renderer.h"

#include "../player.h"
#include "object.h"
#include "object_dynamic.h"
#include "object_character.h"
#include "object_player.h"
#include "object_sprite.h"
#include "object_bullet.h"
#include "object_waypoint.h"
#include "object_item.h"
#include "object_enemy.h"
#include "object_effect.h"
#include "object_environment.h"
#include "object_particle_system.h"
#include "object_ray.h"
#include "../phys/phys_collisionsolver.h"
#include "../phys/phys_misc.h"
#include "../phys/sap/OPC_ArraySAP.h"
#include "../../script/api.h"
#include "../camera.h"
#include "../object_manager.h"

#include "../camera.h"

#include "../../misc.h"

#include "../../config.h"

#include "../../sound/snd.h"

#include "../../input_mgr.h"

#include "../../resource_mgr.h"

#include "../game.h"

extern GameObject* Waypoints;

//////////////////////////////////////////////////////////////////////////

#ifdef DEBUG_LOG_INFO
const char* tNames[] = {"objNone", "objPlayer", "objSprite", "objBullet", "objSpatialTrigger", "objEffect", "objEnemy", "objItem", "objEnvironment", "objRay", "objParticleSystem"};
#endif // DEBUG_LOG_INFO

extern Opcode::ArraySAP *asap;
extern UINT internal_time;
extern float CAMERA_X;
extern float CAMERA_Y;
extern Player* playerControl;
extern config cfg;

extern SoundMgr* soundMgr;

#ifdef DEBUG_PRINT
extern string DBG_PRINT_VAL3;
#endif // DEBUG_PRINT

extern config cfg;

extern InputMgr inpmgr;

extern bool isNewGameCreated;
extern game::GameStates game_state;

//////////////////////////////////////////////////////////////////////////
LSQ_HandleT ObjTree = NULL;		//Указатель на дерево объектов
LSQ_HandleT AddTree = NULL;		//Указатель на дерево только что добавленных объектов

LSQ_HandleT RayTree = NULL;		// Временное дерево-хранилище лучей

LSQ_HandleT ActiveObjTree = NULL;
LSQ_HandleT PassiveObjTree = NULL;

LSQ_HandleT tempObjTree = NULL;			
Opcode::ArraySAP *tempAsap = NULL;

vector<GameObject*> removedObjects;


UINT last_id = 1;

bool DRAW_DEBUG_RECTS = false;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Регистрация нового объекта. Добавляет физические объекты в SAP.
void AddObject(GameObject* obj)
{
	if (!ObjTree) ObjTree = LSQ_CreateSequence();
	if (!AddTree) AddTree = LSQ_CreateSequence();
	if (!RayTree) RayTree = LSQ_CreateSequence();
	if (!ActiveObjTree) ActiveObjTree = LSQ_CreateSequence();
	if (!PassiveObjTree) PassiveObjTree = LSQ_CreateSequence();

	obj->id = ++last_id;
	LSQ_InsertNewMaxElement(AddTree, obj->id, obj);

	if (obj->IsPhysic())
	{
		if (obj->aabb.W == 0) obj->aabb.W = 1;
		if (obj->aabb.H == 0) obj->aabb.H = 1;
		((ObjPhysic*)obj)->sap_handle = asap->AddObject(obj, (uword)obj->id, obj->aabb.GetASAP_AABB());
#ifdef DEBUG_SAP
		const char* protoName = obj->sprite ? obj->sprite->proto_name : NULL;
		sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Adding to sap %s(%s) 0x%p id: %d, sap: %d, p:%f,%f", 
			tNames[obj->type], protoName, obj, obj->id, ((ObjPhysic*)obj)->sap_handle, obj->aabb.p.x, obj->aabb.p.y);
#endif // DEBUG_SAP
	}

}

void RemoveObject(GameObject* obj)
{
	// Добавляем объект в список к удалению
	removedObjects.push_back(obj);
	if (obj->IsPhysic())
	{
#ifdef DEBUG_SAP
		sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Removing from sap %s 0x%p id: %d, sap: %d", tNames[obj->type], obj, obj->id, ((ObjPhysic*)obj)->sap_handle);
#endif // DEBUG_SAP
		// Удаляем объект из ASAP
		asap->RemoveObject( ((ObjPhysic*)obj)->sap_handle );
	}

	// Убираем из списка лучей так же, как физические объекты удаляются из SAP
	if (obj->type == objRay) LSQ_DeleteElement(RayTree, obj->id);
}

// Внесение всех новых объектов в основной контейнер объектов.
void BatchAdd()
{
	if ( LSQ_GetSize(AddTree) == 0 )
		return;

	bool update_sap = false;
	LSQ_IteratorT AddIter = NULL;
	for ( GameObject* obj = LSQ_DereferenceIterator(AddIter = LSQ_GetFrontElement(AddTree));
		  !LSQ_IsIteratorPastRear(AddIter); obj = LSQ_DereferenceIterator(AddIter) )
	{
		if ( obj->IsPhysic() ) update_sap = true;
		LSQ_InsertElement(ObjTree, obj->id, obj);
		if (obj->type == objRay) LSQ_InsertNewMaxElement(RayTree, obj->id, obj);
		// Тут задается критерий отбора пассивных объектов
		if (!obj->IsPhysic() && 
			(!obj->sprite || (obj->sprite && obj->sprite->animsCount == 0)) &&
			!obj->ParticleSystem && obj->type != objSpawner)
		{
			LSQ_InsertElement(PassiveObjTree, obj->id, obj);
		}
		else
		{
			LSQ_InsertElement(ActiveObjTree, obj->id, obj);
		}


		LSQ_AdvanceOneElement(AddIter);
	}

	LSQ_DestroyIterator(AddIter);
	LSQ_DestroySequence(AddTree);
	AddTree = LSQ_CreateSequence();

	if (update_sap)
		UpdateSAPState();
}

// Уничтожение объекта
void RemObj(GameObject* obj)
{
	if (obj)
	{
		if (GetCameraAttachedObject() == obj)
		{
			// На этом объекте сфокусирована камера. Убираем это.
			CameraAttachToObject(NULL);
		}
		if (Waypoints == obj)
			Waypoints = NULL;
		if ( obj->childrenConnection )
		{
			if ( obj->childrenConnection->children.size() > 0 )
			{
				for ( vector<GameObject*>::iterator iter = obj->childrenConnection->children.begin(); iter != obj->childrenConnection->children.end(); ++iter )
					(*iter)->ParentEvent( ObjectEventInfo( eventDead ) );
			}
			obj->childrenConnection->parentDead();
		}
		if ( obj->parentConnection )
			obj->parentConnection->childDead( obj->id );

		DELETESINGLE(obj);
	}
}

// Убирает все удаленые объекты из основного контейнера объектов и вызывает их уничтожение.
void BatchRemove()
{
	if (!removedObjects.size())
		return;

	GameObject* obj = NULL;

	bool update_sap = false;

	for(vector<GameObject*>::iterator it = removedObjects.begin();
		it != removedObjects.end();
		it++)
	{
		obj = *it;
		LSQ_DeleteElement(ObjTree, obj->id);
		LSQ_DeleteElement(ActiveObjTree, obj->id);
		LSQ_DeleteElement(PassiveObjTree, obj->id);
		if (obj->IsPhysic()) update_sap = true;	

		RemObj(obj);
	}

	if (update_sap)
		UpdateSAPState();

	removedObjects.clear();
}

// 
void PrepareRemoveAllObjects()
{
	tempObjTree = ObjTree;
	ObjTree = NULL;
	LSQ_DestroySequence(RayTree);			RayTree = NULL;
	LSQ_DestroySequence(ActiveObjTree);		ActiveObjTree = NULL;
	LSQ_DestroySequence(PassiveObjTree);	PassiveObjTree = NULL;
	
	DefaultEnvDelete();
	EmptyWPStack();

	// Удаляем объекты, которые были созданы, но еще не были добавлены в основной контейнер.
	if ( LSQ_GetSize(AddTree) > 0 )
	{
		LSQ_IteratorT AddIter = NULL;
		for ( GameObject* obj = LSQ_DereferenceIterator(AddIter = LSQ_GetFrontElement(AddTree));
			!LSQ_IsIteratorPastRear(AddIter); obj = LSQ_DereferenceIterator(AddIter) )
		{
			if (obj->IsPhysic()) asap->RemoveObject(static_cast<ObjPhysic*>(obj)->sap_handle);
			RemObj(obj);
			LSQ_AdvanceOneElement(AddIter);
		}
		LSQ_DestroyIterator(AddIter);
		asap->DumpPairs(NULL, NULL, NULL);
	}
	LSQ_DestroySequence(AddTree);
	AddTree = NULL;

	// Само удаление будет в RemoveAllObjects
	if (removedObjects.size() > 0)
		removedObjects.clear();

	tempAsap = asap;
	asap = NULL;
}


// Уничтожает все объекты.
void RemoveAllObjects()
{
	game::GameStates temp_game_state = game_state;
	game_state = game::GAME_DESTROYING;

	if ( tempObjTree )
	{
		LSQ_IteratorT ObjIter = LSQ_GetFrontElement(tempObjTree);
		GameObject* obj;
		while ( !LSQ_IsIteratorPastRear(ObjIter) )
		{
			obj = (GameObject*)LSQ_DereferenceIterator(ObjIter);
			if (obj->IsPhysic()) tempAsap->RemoveObject(static_cast<ObjPhysic*>(obj)->sap_handle);
			RemObj(obj);
			LSQ_AdvanceOneElement(ObjIter);
		}
		LSQ_DestroyIterator(ObjIter);
		LSQ_DestroySequence(tempObjTree);
		tempObjTree = NULL;
		tempAsap->DumpPairs(NULL, NULL, NULL);
	}

	Waypoints = NULL;

	DELETESINGLE(tempAsap);
	game_state = temp_game_state;
}

//////////////////////////////////////////////////////////////////////////

GameObject* GetGameObject(UINT id)
{
	GameObject* obj = NULL;
	LSQ_IteratorT it = NULL;

	if ( LSQ_GetSize(AddTree) > 0 )
	{
		obj = (GameObject*)LSQ_DereferenceIterator(it = LSQ_GetElementByIndex(AddTree, id));
		LSQ_DestroyIterator(it);
		it = NULL;

		if ( obj )
			return obj;
	}

	if ( !ObjTree ) return NULL;
	obj = (GameObject*)LSQ_DereferenceIterator(it = LSQ_GetElementByIndex(ObjTree, id));
	LSQ_DestroyIterator(it);
	return obj;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

BOOL LoadObjectFromProto(const Proto* proto, GameObject* obj)
{
	if (!proto || !obj)
		return TRUE;

	obj->sprite = new Sprite(proto);

	if (obj->IsPhysic())
	{
		ObjPhysic* obp = (ObjPhysic*)obj;
		obp->slopeType = proto->slopeType;
		obp->touch_detection = (TouchDetectionType)proto->touch_detection;
		if (proto->phys_solid) obp->SetSolid();
		if (proto->phys_one_sided) obp->SetOneSide();
		if (proto->phys_one_sided & 2) obp->SetForced();
		if (proto->phys_bullet_collidable) obp->SetBulletCollidable();
		obp->solid_to = ~((BYTE)proto->ghost_to);
#ifdef MAP_EDITOR
		obp->editor_color = proto->editor_color;
#endif //MAP_EDITOR
		if (obp->IsDynamic())
		{
			ObjDynamic* obd = (ObjDynamic*)obp;
			obd->mass = proto->mass;
			obd->max_x_vel = proto->phys_max_x_vel;
			obd->max_y_vel = proto->phys_max_y_vel;
			obd->walk_acc = proto->phys_walk_acc;
			obd->jump_vel = proto->phys_jump_vel;
			obd->bounce = proto->bounce;
			obd->trajectory = (TrajectoryType)proto->trajectory;
			obd->t_param1 = proto->trajectory_param1;
			obd->t_param2 = proto->trajectory_param2;
			obd->t_value = Random_Float( proto->min_param, proto->max_param );
			obd->ghostlike = 1 == proto->phys_ghostlike;
			obd->drops_shadow = 1 == proto->drops_shadow;
			obd->shadow_width = abs(proto->shadow_width);
			obd->facing = (ObjectFacingType)proto->facing;
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////

GameObject::~GameObject()
{
	DELETESINGLE(sprite);
	DELETESINGLE(ParticleSystem);

	DELETEARRAY(mem_anim);
#ifdef MAP_EDITOR
	DELETEARRAY(proto_name);
#endif //MAP_EDITOR

	if (this->scriptProcess)
		SCRIPT::RemoveFromRegistry(this->scriptProcess);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void GameObject::SetAnimation(string anim_name, bool restart)
{
	if (this->sprite)
	{
		if (!restart && this->sprite->cur_anim == anim_name)
			return;

		if (this->sprite->SetAnimation(anim_name))
		{
			this->ProcessSprite();
		}
		else
		{
			this->aabb.H = this->sprite->frameHeight * 0.5f;
			this->aabb.W = this->sprite->frameWidth * 0.5f;
		}

	}

#ifdef DEBUG_PRINT
	if (this->type == objPlayer)
	{
		DBG_PRINT_VAL3 = this->sprite->cur_anim;
	}
#endif // DEBUG_PRINT

}

void GameObject::SetAnimationIfPossible(string anim_name, bool restart)
{
	if ( this->sprite->GetAnimation(anim_name) != NULL )
		SetAnimation( anim_name, restart );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

extern lua_State *lua;
extern UINT internal_time;
UINT old_time = 0;

void ProcessAllActiveObjects()
{
#ifdef DEBUG_SAP
	static UINT turn_number = 0;
	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "=================== Начало хода #%d ====================", turn_number);
	turn_number++;
#endif // DEBUG_SAP
	
	BatchAdd();		// Будут доступны на следующем ходу

	//sLog(DEFAULT_LOG_NAME, logLevelInfo, "objTree = %d, activeTree = %d", LSQ_GetSize(ObjTree), LSQ_GetSize(ActiveObjTree));
	//LSQ_IteratorT ObjIter = LSQ_GetFrontElement(ObjTree);
	LSQ_IteratorT ObjIter = LSQ_GetFrontElement(ActiveObjTree);
	if (ObjIter != LSQ_HandleInvalid)
	{
		while (!LSQ_IsIteratorPastRear(ObjIter))
		{
			GameObject* obj = ((GameObject*)LSQ_DereferenceIterator(ObjIter));
#ifndef MAP_EDITOR
			if (obj->scriptProcess >= 0)
			{
				lua_pushinteger(lua, obj->id);
				if (SCRIPT::ExecChunkFromReg(obj->scriptProcess, 1))
				{
					// В скрипте произошла какая-то ошибка. Будем испльзовать стандартный в следующий раз.
					SCRIPT::RemoveFromRegistry(obj->scriptProcess);
				}
			}
			else
			{
				obj->Process();
			}

			if (obj->sprite && !obj->IsSleep())
			{
				obj->ProcessSprite();
			}

			if (isNewGameCreated)
				return;

			if (obj->ParticleSystem)
			{
				obj->ParticleSystem->Update();
				if ( obj->ParticleSystem->dead && obj->ParticleSystem->Info.ParticlesActive == 0)
					obj->SetDead();
			}


			if (obj->type == objPlayer/* || obj->type == objEnemy*/)
			{
				ObjCharacter* ch = dynamic_cast<ObjCharacter*>(obj);
				if (NULL != ch)
				{
					ch->ProcessShooting();
				}
			}
#endif
			if (obj->IsPhysic())
			{
				((ObjPhysic*)obj)->PhysProcess();
			}

			if (obj->IsDead())
			{
				RemoveObject(obj);
			}

			LSQ_AdvanceOneElement(ObjIter);
		}
		LSQ_DestroyIterator(ObjIter);

		// Обработка пар столкновений
#ifdef DEBUG_SAP
		sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "ProcessCollisions");
#endif // DEBUG_SAP
		ProcessCollisions();
	}

	BatchRemove();
	inpmgr.ProlongedArrayFlush(); // <- временное (надеюсь) решение, нужно посовещаться.
	//BatchAdd();		// Будут доступны на следующем ходу
}

void ProcessAllPassiveObjects()
{
	LSQ_IteratorT ObjIter = LSQ_GetFrontElement(PassiveObjTree);
	if (ObjIter != LSQ_HandleInvalid)
	{
		while (!LSQ_IsIteratorPastRear(ObjIter))
		{
			GameObject* obj = ((GameObject*)LSQ_DereferenceIterator(ObjIter));

			if (obj->IsDead())
			{
				RemoveObject(obj);
			}

			LSQ_AdvanceOneElement(ObjIter);
		}
		LSQ_DestroyIterator(ObjIter);
	}

	BatchRemove();
}


//////////////////////////////////////////////////////////////////////////
void GroupObjects( int objGroup, int objToAdd )
{
	GameObject * group = GetGameObject( objGroup );
#ifdef MAP_EDITOR
	group->grouped = true;
#endif //MAP_EDITOR
	GameObject * obj = GetGameObject( objToAdd );
	if ( !group || !obj || !group->IsPhysic() || !obj->IsPhysic() )
		return;

//	if (obj->IsPhysic())
	{
		//Vector2 old_corner = Vector2( group->aabb.GetMin(0)+group->sprite->realX,
		//							  group->aabb.GetMin(1)+group->sprite->realY );
		group->aabb = CAABB( min( group->aabb.GetMin(0), obj->aabb.GetMin(0) ),
							 	  min( group->aabb.GetMin(1), obj->aabb.GetMin(1) ),
								  max( group->aabb.GetMax(0), obj->aabb.GetMax(0) ),
								  max( group->aabb.GetMax(1), obj->aabb.GetMax(1) ));
		//DELETESINGLE( group->aabb );
		asap->RemoveObject( ((ObjPhysic*)obj)->sap_handle );
		UpdateSAPState();
		UpdateSAPObject( ((ObjPhysic*)group)->sap_handle, group->aabb.GetASAP_AABB() );
		//group->sprite->realX = old_corner.x - group->aabb.GetMin(0);
		//group->sprite->realY = old_corner.y - group->aabb.GetMin(1);
	}
	//DELETESINGLE( obj->aabb );
	obj->ClearPhysic();
	obj->SetDead();
	if ( group->sprite ) group->sprite->renderMethod = rsmRepeatXY;
}

//////////////////////////////////////////////////////////////////////////
void DrawAllObjects()
{
	GameObject* obj;
	LSQ_IteratorT ObjIter = LSQ_GetFrontElement(ObjTree);
	if (ObjIter == LSQ_HandleInvalid)
		return;

	while (!LSQ_IsIteratorPastRear(ObjIter))
	{
		obj = ((GameObject*)LSQ_DereferenceIterator(ObjIter));
		obj->Draw();
		if ( obj->IsPhysic() && ((ObjPhysic*)obj)->IsDynamic() )
			((ObjDynamic*)obj)->DropShadow();
		LSQ_AdvanceOneElement(ObjIter);
	}
	LSQ_DestroyIterator(ObjIter);
}

#ifdef MAP_EDITOR
extern bool editor_ShowBorders;
#endif //MAP_EDITOR

void GameObject::Draw()
{
#ifdef MAP_EDITOR
	if ( (this->sprite && !this->sprite->IsVisible()) || this->grouped )
		RenderSprite( this->aabb.Left(), this->aabb.Top(), 0.8, &coord2f_t(2*this->aabb.W, 2*this->aabb.H), NULL, NULL, ((ObjPhysic*)this)->editor_color );
#endif //MAP_EDITOR
	if (this->sprite && this->type != objParticleSystem)
		this->sprite->Draw(this->aabb);
	if (this->ParticleSystem)
		ParticleSystem->Render();

#ifdef DEBUG_DRAW_WAYPOINTS
	if ( cfg.debug && this->type == objWaypoint)
	{
		RenderLine( aabb.Left(), aabb.Top(), aabb.Right(), aabb.Bottom(), 1.0, RGBAf(DEBUG_WAYPOINT_COLOR) );
		RenderLine( aabb.Left(), aabb.Bottom(), aabb.Right(), aabb.Top(), 1.0, RGBAf(DEBUG_WAYPOINT_COLOR) );
	}

	if (cfg.debug && this->type == objEnemy)
	{
		ObjEnemy* oe = static_cast<ObjEnemy*>(this);
		GameObject* ow = NULL;
		if (oe->current_waypoint && ( (ow = GetGameObject(oe->current_waypoint)) != NULL) )
		{
			RenderLine( this->aabb.p.x, this->aabb.p.y, ow->aabb.p.x, ow->aabb.p.y, 1.0, RGBAf(DEBUG_WAYPOINT_COLOR) );
		}
	}
#endif //DEBUG_DRAW_WAYPOINTS

#ifdef DEBUG_DRAW_PHYSOBJ_BORDERS_ONLY
#ifdef __MINGW32__
    RGBAf tmp = RGBAf(DEBUG_PHYSOBJ_BORDER_COLOR);
	if (this->IsPhysic() && cfg.debug)
		RenderBox(aabb.p.x - aabb.W, aabb.p.y - aabb.H, this->sprite->z,
		2 * aabb.W, 2* aabb.H,
        tmp);
#else
	if (this->IsPhysic() 
		#ifndef MAP_EDITOR
		&& cfg.debug
		#else
		&& (editor_ShowBorders || ((ObjPhysic*)this)->show_border)
		#endif // MAP_EDITOR
		)
	{
		RenderBox(aabb.p.x - aabb.W, aabb.p.y - aabb.H, 1,
			2 * aabb.W, 2* aabb.H,
			#ifdef MAP_EDITOR
			((ObjPhysic*)this)->border_color
			#else
			RGBAf(DEBUG_PHYSOBJ_BORDER_COLOR)
			#endif // MAP_EDITOR
			);
		if ( ((ObjPhysic*)this)->slopeType == 1 )
		{
			RenderLine(aabb.Left(), aabb.Bottom(), 
				aabb.Right(), aabb.Top(), 
				sprite->z, 
				RGBAf(DEBUG_PHYSOBJ_BORDER_COLOR));
		}
		else if( ((ObjPhysic*)this)->slopeType == 2 )
		{
			RenderLine(aabb.Left(), aabb.Top(), 
				aabb.Right(), aabb.Bottom(), 
				sprite->z, 
				RGBAf(DEBUG_PHYSOBJ_BORDER_COLOR));
		}
		
		if (((ObjPhysic*)this)->IsDynamic())
		{
			ObjDynamic* dyn = ((ObjDynamic*)this);
			#ifdef DEBUG_DRAW_SPEED_VECTORS
			Vector2 vv = 10 * (dyn->vel + 0.5f * dyn->acc) + this->aabb.p;
			RenderLine(aabb.p.x, aabb.p.y, 
				vv.x, vv.y, 1.0f, RGBAf(DEBUG_DRAW_SPEED_VEC_COLOR));
			
			vv = 10 * dyn->env->gravity_bonus + this->aabb.p;
			RenderLine(vv.x, vv.y, this->aabb.p.x, this->aabb.p.y, 1.0f, RGBAf(DEBUG_DRAW_GRAV_BONUS_COLOR));
			#endif // DEBUG_DRAW_SPEED_VECTORS

			#ifdef DEBUG_DRAW_SUSPECTED_PLAIN
			if (dyn->suspected_plane)
			{
				RenderLine(aabb.p.x, aabb.p.y, dyn->suspected_plane->aabb.p.x, dyn->suspected_plane->aabb.p.y, 1.0f, RGBAf(DEBUG_DRAW_SUSPECTED_PLAIN_COLOR));
			}
			#endif // DEBUG_DRAW_SUSPECTED_PLAIN
		}

	}
#endif
#endif // DEBUG_DRAW_PHYSOBJ_BORDERS_ONLY
	
#ifdef DEBUG_DRAW_PHYSOBJ_ID
	if (this->IsPhysic() && cfg.debug)
	{
		static Font* font = FontByName(DEFAULT_FONT);
		if (font)
		{
			font->p = this->aabb.p;
			font->z = 1.0f;
			font->tClr = RGBAf(DEBUG_PHYSOBJ_ID_FONT_COLOR);
			font->Print("%d", this->id);
		}
	}
#endif // DEBUG_DRAW_PHYSOBJ_ID
}

void GameObject::ProcessSprite()
{
	//if ( this->IsSleep() ) return;
	Sprite* s= this->sprite;

	// Если наследуетесь от класса ObjCharacter, то здесь надо добавить соответсвующую проверку.
	ObjCharacter* ch = (this->type == objPlayer || this->type == objEnemy) ? static_cast<ObjCharacter*>(this) : NULL;
	ParametersStack* ps = this->sprite->stack;
	int param;
	char* txt_param;

	if (s->animNames)
	{
		Animation* a = s->GetAnimation(this->sprite->cur_anim_num);
		if (a)
		{
			while (!s->ChangeFrame(a))
			{
				param = a->frames[a->current].param;
				txt_param = a->frames[a->current].txt_param;
				switch (a->frames[a->current].command)
				{
				case afcRandomOverlayColor:
					if ( this->sprite->overlayCount )
					{
						if (!this->sprite->ocolor)
						{
							sLog(DEFAULT_LOG_NAME, logLevelWarning, "ProcessSprite(), afcRandomOverlayColor ocolor==NULL, прототип %s, анимация %s, кадр %d",
								this->sprite->proto_name, a->name, a->current);
							break;
						}
						if (this->sprite->overlayCount <= (UINT)param || param < 0)
						{
							sLog(DEFAULT_LOG_NAME, logLevelWarning, "ProcessSprite(), afcControlledOverlayColor неправильный param, прототип %s, анимация %s, кадр %d",
								this->sprite->proto_name, a->name, a->current);
							break;
						}
						this->sprite->ocolor[param] = RGBAf( (rand() % 101)/100.0f, (rand() % 101)/100.0f, (rand() % 101)/100.0f, 1.0f);
					}
					break;
				case afcSetColor:
					if ( this->sprite )
					{
						if ( !ps->CheckParamTypes(3, stInt, stInt, stInt) )
							break;
						float b = ps->PopInt()/255.0f;
						float g = ps->PopInt()/255.0f;
						float r = ps->PopInt()/255.0f;
						this->sprite->color = RGBAf(r, g, b, 1.0f);
					}
					break;
				case afcSetWaypoint:
					if ( this->type == objEnemy )
					{
						ObjWaypoint* wp = GetWaypoint( param );
						if ( wp )
							((ObjEnemy*)this)->current_waypoint = wp->id;
					}
					break;
				case afcMenuLoop:	//TODO: Убрать
					{
						StackElement* sd1 = this->sprite->stack->Pop();
						this->sprite->stack->Push(sd1->data.intData);
						GameObject* sign = GetGameObject(sd1->data.intData);
						if ( sign && this->aabb.p.x-sign->aabb.p.x >= 1280  )
							sign->aabb.p.x = this->aabb.p.x + 640;
						DELETESINGLE( sd1 );
					}
					break;
				case afcSetDamageMult:
					{
						((ObjCharacter*)this)->damage_mult = param / 1000.0f;
					}
					break;
				case afcSetZ:
					{
						this->sprite->z = param / 1000.0f;
					}
					break;
				case afcStartDying:
					{
						if ( this->type == objBullet )
						{
							if (param != -1) 
							{
								if (((ObjBullet*)this)->multiple_targets)
									((ObjBullet*)this)->activity = oatUndead;
								else
									((ObjBullet*)this)->activity = oatDying;
							}
							else ((ObjBullet*)this)->activity = oatIdling;
						}
						if ( this->type == objEnemy )
						{
							((ObjEnemy*)this)->activity = oatDying;
							((ObjEnemy*)this)->health = 0;
						}
					}
					break;
				case afcControlledOverlayColor:
					if ( this->sprite->overlayCount )
					{
						if (!this->sprite->ocolor)
						{
							sLog(DEFAULT_LOG_NAME, logLevelWarning, "ProcessSprite(), afcControlledOverlayColor ocolor==NULL, прототип %s, анимация %s, кадр %d",
								this->sprite->proto_name, a->name, a->current);
							break;
						}
						if (this->sprite->overlayCount <= (UINT)param || param < 0)
						{
							sLog(DEFAULT_LOG_NAME, logLevelWarning, "ProcessSprite(), afcControlledOverlayColor неправильный param, прототип %s, анимация %s, кадр %d",
								this->sprite->proto_name, a->name, a->current);
							break;
						}
						if ( !ps->CheckParamTypes(6, stInt, stInt, stInt, stInt, stInt, stInt) )
							break;
						int b2 = ps->PopInt();
						int b1 = ps->PopInt();
						int g2 = ps->PopInt();
						int g1 = ps->PopInt();
						int r2 = ps->PopInt();
						int r1 = ps->PopInt();
						float r = 0.0f;
						if ( r1 != r2 )
							r = (r1 + rand() %(r2 - r1))/255.0f;
						else
							r = r1/255.0f;
						float g = 0.0f;
						if ( g1 != g2 )
							g = (g1 + rand() %(g2 - g1))/255.0f;
						else
							g = g1/255.0f;
						float b = 0.0f;
						if ( b1 != b2 )
							b = (b1 + rand() %(b2 - b1))/255.0f;
						else
							b = b1/255.0f;
						this->sprite->ocolor[param] = RGBAf( r, g, b, 1.0);
					}
					break;
				case afcRealX:
					s->realX = param;
					break;
				case afcBreakpoint:
					{
						string cmt = txt_param ? txt_param : "(empty)";
						sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Breakpoint triggered at %s with comment %s.", this->sprite->cur_anim.c_str(), cmt.c_str());
					}
					break;
				case afcRealY:
					s->realY = param;
					break;
				case afcRealW:
					s->realWidth = param;
					this->aabb.W = s->realWidth * 0.5f;
					if ( this->IsPhysic() && !((ObjPhysic*)this)->IsDynamic() ) UpdateSAPObject(((ObjPhysic*)this)->sap_handle, this->aabb.GetASAP_AABB());
					break;
				case afcRealH:
					s->realHeight = param;
					this->aabb.p.y += this->aabb.H - s->realHeight * 0.5f;
					this->aabb.H = s->realHeight * 0.5f;
					if ( this->IsPhysic() && !((ObjPhysic*)this)->IsDynamic() ) UpdateSAPObject(((ObjPhysic*)this)->sap_handle, this->aabb.GetASAP_AABB());
					break;
				case afcInitW:
					{
						int mode = this->sprite->stack->PopInt();
						s->realWidth = param;
						if ( mode & 1 ) this->aabb.p.x -= this->aabb.W - s->realWidth * 0.5f;
						else this->aabb.p.x += this->aabb.W - s->realWidth * 0.5f;
						this->aabb.W = s->realWidth * 0.5f;
						if ( this->IsPhysic() && !((ObjPhysic*)this)->IsDynamic() ) UpdateSAPObject(((ObjPhysic*)this)->sap_handle, this->aabb.GetASAP_AABB());
					}
					break;
				case afcInitH:
					{
						if ( !ps->CheckParamTypes(1, stIntOrNone) )
							break;
						s->realHeight = param;
						int mode = ps->GetInt();
						if ( mode == -1 )
							this->aabb.p.y -= this->aabb.H - s->realHeight * 0.5f;
						else if ( mode == 1 )
							this->aabb.p.y += this->aabb.H - s->realHeight * 0.5f;
						if ( this->IsPhysic() && !((ObjPhysic*)this)->IsDynamic() ) UpdateSAPObject(((ObjPhysic*)this)->sap_handle, this->aabb.GetASAP_AABB());
						this->aabb.H = s->realHeight * 0.5f;
						break;
					}
				case afcShootDir:
					if (NULL != ch)
					{
						ch->gunDirection = (CharacterGunDirection)param;
					}
					break;
				case afcShootBeh:
					if (NULL != ch)
					{
						ch->shootingBeh = (CharacterShootingBeh)param;
					}
					break;
				case afcShoot:
					if (NULL != ch)
					{
						ch->shootingBeh = csbOnAnimCommand;
					}
					break;
				case afcSetAnimIfGunDirection:
					{
						if (!txt_param)
							break;
						ObjCharacter* oc = (ObjCharacter*)this;
						if ( param == 1 && oc->gunDirection == cgdUp )
						{
							this->SetAnimation(txt_param, true);
							return;
						}
						if ( param == -1 && oc->gunDirection == cgdDown )
						{
							this->SetAnimation(txt_param, true);
							return;
						}
						if (param == 0 && oc->gunDirection == cgdNone )
						{
							this->SetAnimation(txt_param, true);
							return;
						}
					}
					break;
				case afcSetAnim:
					if (!txt_param)
					{
						sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "SetAnim with no animation name at %s.", this->sprite->cur_anim.c_str());
						break;
					}
					if ( param <= 0 || param >= 256 || rand()%256 > param )
					{
						this->SetAnimation(txt_param, true);
						return;
					}
					break;
				case afcSetAnimIfWeaponNotReady:
					if (!txt_param)
					{
						sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "SetAnimIfWeaponNotReady with no animation name at %s.", this->sprite->cur_anim.c_str());
						break;
					}
					if ( this->type == objPlayer && ((ObjPlayer*)this)->cur_weapon && !((ObjPlayer*)this)->cur_weapon->IsReady() && ((ObjPlayer*)this)->HasAmmo())
						this->SetAnimation(txt_param, true);
					return;
				case afcPushInt:
					this->sprite->stack->Push( param );
					break;
				case afcPushString:
					this->sprite->stack->Push( txt_param );
					break;
				case afcPushRandomInt:
					{
						if ( !ps->CheckParamTypes(2, stInt, stInt) )
							break;
						int hi = ps->PopInt();
						int lo = ps->PopInt();
						if (hi <= lo) this->sprite->stack->Push( lo );
						else this->sprite->stack->Push( lo + rand()%(hi - lo) );
					}
					break;
				case afcJump:
					this->sprite->JumpFrame( param );
					break;
				case afcRecover:
					{
						ASSERT(this->mem_anim);
						if (!this->mem_anim)
						{
							sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Недопустимый вызов afcRecover, mem_anim=NULL. Прототип %s, анимация %s, кадр %d",
								this->sprite->proto_name, a->name, a->current);
						}
						this->sprite->SetAnimation( string(this->mem_anim) );
						this->sprite->JumpFrame(mem_frame);
						DELETEARRAY(this->mem_anim);
						this->aabb.W = ((scalar)sprite->realWidth)/2;
						this->aabb.H = ((scalar)sprite->realHeight)/2;
						return;
					}
				case afcJumpRandom:
					{
						if ( !ps->CheckParamTypes(1,stInt) )
							break;
						int var = ps->PopInt();
						if (  (rand() % 256) > var )
							this->sprite->JumpFrame( param );
					}
					break;
				case afcJumpIfXLess:
					{
						if ( !ps->CheckParamTypes(1, stInt) )
							break;
						int x = ps->PopInt();
						if ( this->aabb.p.x <= x )
							this->sprite->JumpFrame( param );
					}
				case afcJumpIfXGreater:
					{
						if ( !ps->CheckParamTypes(1, stInt) )
							break;
						int x = ps->PopInt();
						if ( this->aabb.p.x >= x )
							this->sprite->JumpFrame( param );

					}
				case afcJumpIfYSpeedGreater:
					if ( this->IsPhysic() && ( (ObjPhysic*)this)->IsDynamic() )
					{
						if ( !ps->CheckParamTypes(1, stIntOrNone) )
							break;
						ObjDynamic* dno = (ObjDynamic*)this;
						int sparam = ps->PopInt();
						if ( dno->vel.y > sparam )
							this->sprite->JumpFrame( param );
					}
					break;
				case afcSetInvisible:
					if ( true )
					{
						if ( !this->sprite ) break;
						if ( ( param ) == 0 ) this->sprite->SetVisible();
						else this->sprite->ClearVisible();
					}
					break;
				case afcSetShielding:
					if ( this->type == objBullet )
					{
						if ( ( param ) == 1 ) ((ObjBullet*)this)->SetShielding();
						else ((ObjBullet*)this)->ClearShielding();
					}
					break;
				case afcSetLifetime:
					{
						ObjDynamic* od = (ObjDynamic*)this;
						od->lifetime = (float)param;
					}
					break;
				case afcSetBulletCollidable:
					if ( this->IsPhysic() )
					{
						if ( !this->sprite ) break;
						if ( ( param ) == 1 ) ((ObjPhysic *)this)->SetBulletCollidable();
						else ((ObjPhysic *)this)->ClearBulletCollidable();
					}
					break;
				case afcSetSolid:
					if ( this->IsPhysic() )
					{
						if ( !this->sprite ) break;
						if ( ( param ) == 1 ) ((ObjPhysic *)this)->SetSolid();
						else ((ObjPhysic *)this)->ClearSolid();
					}
					break;
				case afcJumpIfCloseToCamera:
					if ( true )
					{
						if ( !ps->CheckParamTypes(2, stInt, stInt) )
							break;
						int y = ps->PopInt();
						int x = ps->PopInt();
						if ( fabs(this->aabb.p.x - CAMERA_X) < x && fabs(this->aabb.p.y - CAMERA_Y ) <= y)
							this->sprite->JumpFrame( param );

					}
					break;
				case afcJumpCheckFOV:	//TODO: учитывать направление взгляда.
					if ( this->type == objEnemy )
					{
						if ( !ps->CheckParamTypes(1, stInt) )
							break;
						ObjEnemy* oe = (ObjEnemy*)this;
						ObjCharacter* otarget = oe->GetTarget();
						if (!otarget) break;
						int angle = ps->PopInt();
						if ( atan2( otarget->aabb.p.y - oe->aabb.p.y, otarget->aabb.p.x - oe->aabb.p.x ) <= angle
							&& oe->aabb.p.x > otarget->aabb.p.x )
								this->sprite->JumpFrame( param );

					}
					break;
				case afcJumpIfObjectExists:
					if ( true)
					{
						if ( !ps->CheckParamTypes(1, stIntOrNone) )
						{
							break;
						}
						int oid = ps->GetInt();
						if ( oid == 0 ) break;
						GameObject* go = GetGameObject(oid);
						if ( go && !go->IsDead() )
							this->sprite->JumpFrame( param );
					}
					break;
				case afcJumpIfCloseToCameraRight:
					if ( true )
					{
						if ( !ps->CheckParamTypes(2, stInt, stInt) )
							break;
						int y = ps->PopInt();
						int x = ps->PopInt();
						if ( CAMERA_X > this->aabb.p.x - x && fabs(this->aabb.p.y - CAMERA_Y ) <= y)
							this->sprite->JumpFrame( param );
					}
					break;
				case afcJumpIfCloseToCameraLeft:
					if ( true )
					{
						if ( !ps->CheckParamTypes(2, stInt, stInt) )
							break;
						int y = ps->PopInt();
						int x = ps->PopInt();
						if ( CAMERA_X > this->aabb.p.x + x && fabs(this->aabb.p.y - CAMERA_Y ) <= y)
							this->sprite->JumpFrame( param );
					}
					break;
				case afcJumpIfCloseToCameraUp:
					if ( true )
					{
						if ( !ps->CheckParamTypes(2, stInt, stInt) )
							break;
						int y = ps->PopInt();
						int x = ps->PopInt();
						if ( CAMERA_Y < this->aabb.p.y - y && fabs(this->aabb.p.x - CAMERA_X ) <= x)
							this->sprite->JumpFrame( param );
					}
					break;
				case afcJumpIfCloseToCameraDown:
					if ( true )
					{
						if ( !ps->CheckParamTypes(2, stInt, stInt) )
							break;
						int y = ps->PopInt();
						int x = ps->PopInt();
						if ( CAMERA_Y > this->aabb.p.y + y && fabs(this->aabb.p.x - CAMERA_X ) <= x)
							this->sprite->JumpFrame( param );
					}
					break;
				case afcJumpIfSquashCondition: //TODO: Заменить на JumpIfYLess + JumpIfObjectYSpeedGreater?
					if ( this->type == objEnemy )
					{
						ObjDynamic* dno = (ObjDynamic*)this;
						if ( !ps->CheckParamTypes(1, stInt) )
							break;
						int oid = ps->GetInt();
						GameObject* go = GetGameObject(oid);
						if ( !go->IsPhysic() || !(((ObjPhysic*)go)->IsDynamic()) || go->type == objItem || go->type == objBullet ) break;
						ObjDynamic * od = (ObjDynamic*)go;
						if ( od->vel.y > 0 && od->aabb.p.y + od->aabb.H < dno->aabb.p.y )
							this->sprite->JumpFrame( param );
					}
					break;
				case afcBounceObject:
					{
						if ( !ps->CheckParamTypes(1, stInt) )
							break;
						int oid = ps->GetInt();
						GameObject* go = GetGameObject(oid);
						if ( !go->IsPhysic() || !(((ObjPhysic*)go)->IsDynamic()) ) break;
						go->SetAnimation("jump", true);

						ObjDynamic* od = (ObjDynamic *)go;

						if ( go->type == objPlayer && inpmgr.IsPressed(cakJump) )
							od->vel.y = min(-10.0f, od->vel.y * -1.25f);
						else
							od->vel.y = min(-10.0f, od->vel.y * -0.75f);
					}
					break;
				case afcPushObject:
					{
						if ( !ps->CheckParamTypes(3, stInt, stInt, stInt) )
							break;
						int y = ps->PopInt();
						int x = ps->PopInt();
						int oid = ps->GetInt();
						GameObject* go = GetGameObject(oid);
						if ( !go->IsPhysic() || !(((ObjPhysic*)go)->IsDynamic()) ) break;
						ObjDynamic* od = (ObjDynamic *)go;
						od->vel.x += x;
						od->vel.y += y;
					}
					break;
				case afcJumpIfPlayerId:
					if ( playerControl->current )
					{
						if ( !ps->CheckParamTypes(1, stInt) )
							break;
						UINT oid = (UINT)ps->GetInt();
						if ( (playerControl->current->id == oid) )
							this->sprite->JumpFrame( param );
					}
					break;
				case afcJumpIfXSpeedGreater:
					if ( this->IsPhysic() && ( (ObjPhysic*)this)->IsDynamic() )
					{
						ObjDynamic* dno = (ObjDynamic*)this;
						if ( !ps->CheckParamTypes(1, stInt) )
							break;
						int sparam = ps->PopInt();
						if ( dno->vel.x > sparam || dno->vel.x < -sparam )
							this->sprite->JumpFrame( param );
					}
					break;
				case afcSetHealth:
					if ( this->type == objEnemy )
					{
						((ObjEnemy*)this)->health = param;
						((ObjEnemy*)this)->health_max = param * 2; //TODO: читать из прототипа?
					}
					if ( this->type == objEffect )
					{
						((ObjEffect*)this)->health = param;
					}
					break;
				case afcSetGravity:
					//if ( this->IsPhysic() && ((ObjPhysic*)this)->IsDynamic() )
					{
						if ( !ps->CheckParamTypes(2, stInt, stInt) )
							break;
						int y = ps->PopInt();
						int x = ps->PopInt();
						((ObjDynamic*)this)->gravity = Vector2( x/1000.0f, y/1000.0f );
					}
					break;
				case afcDamage:
					{
						if ( !ps->CheckParamTypes(1, stInt) )
							break;
						int oid = ps->GetInt();
						ObjCharacter* oc = (ObjCharacter*)GetGameObject( oid );
						if (oc)
							oc->ReduceHealth(param);
						//Вынести бездну в отдельный параметр.
						if (GetCameraAttachedObject() == oc )
						{
							// На этом объекте сфокусирована камера. Убираем это.
							CameraAttachToObject(NULL);
						}
					}
					break;
				case afcDealDamage:
					{
						if ( !ps->CheckParamTypes(1, stInt) )
							break;
						int oid = ps->GetInt();
						ObjCharacter* oc = (ObjCharacter*)GetGameObject( oid );
						if (oc)
							oc->ReceiveDamage(param, 0);
					}
					break;
				case afcDrop:
					{
						if ( this->IsPhysic() && ((ObjPhysic*)this)->IsDynamic() )
						{
							ObjDynamic* od = (ObjDynamic*)this;
							if ( param == 1  )
							{
								od->SetDropping();
								od->ClearOnPlane();
							}
							else
								od->ClearDropping();
						}
					}
					break;
				case afcReduceHealth:
					{
						if ( !ps->CheckParamTypes(1, stInt) )
							break;
						int damage = ps->PopInt();
						if ( this->type == objPlayer || this->type == objEnemy || this->type == objItem )
						{
							if ( damage > 0 )
							{
								if (((ObjCharacter*)this)->ReduceHealth( (UINT)damage )) return;
							}
							else
							{
								((ObjCharacter*)this)->ReceiveHealing( (UINT)(-damage) );
							}
						}
						break;
					}
				case afcSetInvincible:
					{
						if ( !ps->CheckParamTypes(1, stInt) )
							break;
						int oid = ps->GetInt();
						ObjCharacter* oc = (ObjCharacter*)GetGameObject( oid );
						if (oc)
						{
							if ( param == 1 )
								oc->is_invincible = true;
							else
								oc->is_invincible = false;
						}
					}
					break;
				case afcSetTouchable:
					{
						if (this->type == objItem || this->type == objEnemy)
						{
							if (param == 1) ((ObjPhysic*)this)->SetTouchable();
							else ((ObjPhysic*)this)->ClearTouchable();
						}
					}
					break;
				case afcGiveHealth:
					{
						if ( !ps->CheckParamTypes(1, stInt) )
							break;
						int oid = ps->GetInt();
						ObjCharacter* oc = (ObjCharacter*)GetGameObject(oid);
						if (oc)
							oc->ReceiveHealing(param);
					}
					break;
				case afcGiveWeapon:
					{
						if ( !ps->CheckParamTypes(1, stInt) || !txt_param )
							break;
						int oid = ps->GetInt();
						GameObject* go = GetGameObject(oid);
						if ( go->type != objPlayer ) break;
						ObjPlayer* op = (ObjPlayer*)go;
						if (op)
						{
							lua_getglobal(lua, "WeaponSystem");
							if ( lua_isfunction(lua, -1) )
							{
								lua_pushinteger(lua, op->id);
								lua_pushstring(lua, txt_param);
								lua_call(lua, 2, 1);
								if ( lua_isstring(lua, -1) )
									op->GiveWeapon(lua_tostring(lua, -1), false);
								else
								{
									DELETESINGLE(op->alt_weapon);
									op->cur_weapon = op->weapon;
								}
							}
							else
								op->GiveWeapon(txt_param, false);
						}
					}
					break;
				case afcReplaceWithRandomTile:
					{
						if ( !ps->CheckParamTypes(2, stInt, stInt) || !txt_param )
							break;
						this->SetDead();
						int to = ps->PopInt();
						int from = ps->PopInt();
						GameObject* obj1 = CreateSprite( txt_param, this->aabb.p-Vector2( this->aabb.W, this->aabb.H ), false, NULL);
						int tilenum = Random_Int( from, to );
						if (obj1 && obj1->sprite)
						{
							obj1->sprite->SetCurrentFrame(tilenum);
							obj1->sprite->animsCount = 0;
						}
					}
					break;
				case afcGiveAmmo:
					{
						if ( !ps->CheckParamTypes(1, stInt) )
							break;
						int oid = ps->GetInt();
						GameObject* obj = GetGameObject( oid );
						if (obj)
						{
							if ( obj->type != objPlayer )
								break;
							( (ObjPlayer*)obj )->RecieveAmmo(param);
						}
					}
					break;
				case afcSetAccY:
					{
						/*if ( !(this->IsPhysic() || this->type == objBullet ) || !((ObjPhysic*)this)->IsDynamic() )
							break;*/ //Почему-то не работает для бочек. Проверить.
						((ObjDynamic*)this)->acc.y = param/1000.0f;
					}
					break;
				case afcSetVelY:
					{
						/*if ( !(this->IsPhysic() || this->type == objBullet ) || !((ObjPhysic*)this)->IsDynamic() )
							break;*/
						((ObjDynamic*)this)->vel.y = param/1000.0f;
					}
					break;
				case afcSetAccX:
					{
						/*if ( !(this->IsPhysic() || this->type == objBullet ) || !((ObjPhysic*)this)->IsDynamic() )
							break;*/
						((ObjDynamic*)this)->acc.x = param/1000.0f;
					}
					break;
				case afcAdjustAccY:
					{
						/*if ( !(this->IsPhysic() || this->type == objBullet ) || !((ObjPhysic*)this)->IsDynamic() )
							break;*/
						((ObjDynamic*)this)->acc.y += param/1000.0f;
					}
					break;
				case afcSetVelX:
					{
						/*if ( !(this->IsPhysic() || this->type == objBullet ) || !((ObjPhysic*)this)->IsDynamic() )
							break;*/
						((ObjDynamic*)this)->vel.x = param/1000.0f;
					}
					break;
				case afcSetRelativeVelX:
					{
						if ( this->type == objEffect )
						{
							ObjEffect* oe = (ObjEffect*)this;
							ObjDynamic* origin = (ObjDynamic*)oe->parentConnection->getParent();
							if ( origin && origin->GetFacing() )
								((ObjDynamic*)this)->vel.x = -param/1000.0f;
							else
								((ObjDynamic*)this)->vel.x = param/1000.0f;
						}
						else
						{
							float speed = param/1000.0f;
							ObjDynamic* od = (ObjDynamic*)this;
							od->vel.x = ( od->GetFacing() ? -speed : speed  );
						}
					}
					break;
				case afcSetRelativePos:
					{
						if ( this->type != objEffect )
							break;
						if ( !ps->CheckParamTypes(2, stIntOrNone, stIntOrNone) )
							break;
						ObjEffect* oe = (ObjEffect*)this;
						int x = ps->PopInt();
						int y = ps->PopInt();
						ObjDynamic* origin = (ObjDynamic*)oe->parentConnection->getParent();
						if (origin)
						{
							if ( origin->GetFacing() ) 
							{
								Vector2 mp = origin->sprite->mp[ oe->origin_point ];
								mp.x *= -1;
								oe->aabb.p = origin->aabb.p +  Vector2((scalar)-x, (scalar)y) + mp;
							}
							else oe->aabb.p = origin->aabb.p +  Vector2((scalar)x, (scalar)y) + origin->sprite->mp[ oe->origin_point ];
						}
					}
					break;
				case afcSetMaxVelX:
					{
						if ( !(this->IsPhysic() || this->type == objBullet ) || !((ObjPhysic*)this)->IsDynamic() )
							break;
						((ObjDynamic*)this)->max_x_vel = param/1000.0f;
					}
					break;
				case afcSetMaxVelY:
					{
						if ( !(this->IsPhysic() || this->type == objBullet ) || !((ObjPhysic*)this)->IsDynamic() )
							break;
						((ObjDynamic*)this)->max_y_vel = param/1000.0f;
					}
					break;
				case afcAdjustX:
					{
						if ( this->GetFacing() )
							this->aabb.p.x -= param;
						else
							this->aabb.p.x += param;
					}
					break;
				case afcAdjustY:
					{
						this->aabb.p.y += param;
					}
					break;
				case afcStop:
					{
						if ( !this->IsPhysic() || !((ObjPhysic*)this)->IsDynamic() )
							break;
						((ObjDynamic*)this)->acc = Vector2(0,0);
						((ObjDynamic*)this)->vel = Vector2(0,0);
					}
					break;
				case afcClearTarget:
					{
						if (this->type != objEnemy)
							break;
						ObjEnemy* oe = (ObjEnemy*)this;
						if ( oe->target )
							oe->target->parentDead();
						oe->target = NULL;
						break;
					}
				case afcWaitForTarget:
					{
						//ASSERT(this->type == objEnemy);
						if (this->type != objEnemy)
							break;

						ObjEnemy* oe = (ObjEnemy*)this;
						GameObject* tg = oe->GetTarget();
						if ( tg && (tg->aabb.p - aabb.p).Length() < param )
						{
							this->SetAnimation(txt_param, true);
							return;
						}
						ObjCharacter* otarget = (ObjCharacter*)oe->GetNearestCharacter( oe->faction_hates );
						if ( (otarget) && (oe->aabb.p - otarget->aabb.p).Length() < param &&
							 (!oe->GetTarget() || oe->GetTarget() != otarget))
						{
							oe->UpdateTarget( otarget );
							this->SetAnimation(txt_param, true);
							return;
						}
						else
						{
							 otarget = (ObjCharacter*)oe->GetNearestCharacter( oe->faction_follows );
							 if ( (otarget) && (oe->aabb.p - otarget->aabb.p).Length() < param &&
								(!oe->GetTarget() || oe->GetTarget() != otarget))
							 {
								oe->UpdateTarget( otarget );
							 	this->SetAnimation("follow", true);
							 	return;
							 }
						}
					break;
					}
				case afcWaitForEnemy:
					{
						if (this->type != objEnemy)
							break;
						ObjEnemy* oe = (ObjEnemy*)this;
						ObjCharacter* otarget = (ObjCharacter*)oe->GetNearestCharacter( oe->faction_hates );
						if ( (otarget) && (oe->aabb.p - otarget->aabb.p).Length() < param &&
							(!oe->GetTarget() || otarget != oe->GetTarget() ) )
						{
							oe->UpdateTarget(otarget);
							this->SetAnimation(txt_param, true);
							return;
						}
						break;
					}
				case afcMapVarFieldAdd:
					{
						scriptApi::MapVarAdd( lua, txt_param, param );
					}
					break;
				case afcSetAnimOnTargetPos:
					{
						if ( !ps->CheckParamTypes(4, stInt, stInt, stInt, stInt) )
							break;
						ObjDynamic* target = NULL;
						if ( this->type == objEffect )
						{
							ObjDynamic* origin = (ObjDynamic*)this->parentConnection->getParent();
							if (!origin) break;
							target = ( (ObjEnemy*)origin )->GetTarget();
						}
						else if ( this->type == objEnemy ) target = ((ObjEnemy*)this)->GetTarget();
						if (!target) target = playerControl->current;
						if (target)
						{
							int dist = ps->PopInt();
							int anim3 = ps->PopInt();
							int anim2 = ps->PopInt();
							int anim1 = ps->PopInt();
							float self = this->aabb.p.y;
							float targ = target->aabb.p.y;
							if ( param & 1 )
							{
								self = this->aabb.p.x;
								targ = target->aabb.p.x;
							}
							if ( targ - self > dist )
							{
								if ( (param & 2) && this->GetFacing() )
								{
									this->sprite->JumpFrame( anim1 );
									break;
								}
								else
								{
									this->sprite->JumpFrame( anim3 );
									break;
								}
							}
							else if ( self - targ > dist )
							{
								if ( (param & 2) && this->GetFacing() )
								{
									this->sprite->JumpFrame( anim3 );
									break;
								}
								else
								{
									this->sprite->JumpFrame( anim1 );
									break;
								}
							}
							else this->sprite->JumpFrame( anim2 );
						}
					}
					break;
				case afcJumpIfTargetClose:
					{
						if ( !ps->CheckParamTypes(1, stInt) )
							break;
						int distance = ps->PopInt();
						ObjEnemy* oe = NULL;
						if ( this->type == objEnemy ) oe = (ObjEnemy*)this;
						if ( this->type == objEffect ) 
						{
							ObjDynamic* origin = (ObjDynamic*)this->parentConnection->getParent();
							if (!origin) break;
							oe = (ObjEnemy*)origin;
						}
						if ( !oe ) break;
						ObjCharacter* otarget = oe->GetTarget();
						if ( (otarget) && !(otarget->IsDead()) && (oe->aabb.p - otarget->aabb.p).Length() < distance )
							this->sprite->JumpFrame( param );
					break;
					}
				case afcJumpIfTargetCloseByX:
					{
						if ( !ps->CheckParamTypes(1, stInt) )
							break;
						int distance = ps->PopInt();
						ObjEnemy* oe = (ObjEnemy*)this;
						ObjCharacter* otarget = oe->GetTarget();
						if ( (otarget) && !(otarget->IsDead()) && abs(oe->aabb.p.x - otarget->aabb.p.x) < distance )
							this->sprite->JumpFrame( param );
					break;
					}
				case afcJumpIfTargetCloseByY:
					{
						if ( !ps->CheckParamTypes(1, stInt) )
							break;
						int distance = ps->PopInt();
						ObjEnemy* oe = (ObjEnemy*)this;
						ObjCharacter* otarget = oe->GetTarget();
						if ( otarget && abs(oe->aabb.p.y - otarget->aabb.p.y) < distance )
							this->sprite->JumpFrame( param );
					break;
					}
				case afcJumpIfOnPlane:
					{
						if (this->IsPhysic() && ((ObjPhysic*)this)->IsDynamic() && ((ObjDynamic*)this)->IsOnPlane())
							this->sprite->JumpFrame( param );
						break;
					}
				case afcStopMorphing:
					{
						if ( this->type == objPlayer )
							((ObjPlayer*)this)->activity = oatIdling;
					}
					break;
				case afcJumpIfIntEquals:
					{
					if ( !ps->CheckParamTypes(1, stIntOrNone) || !txt_param )
							break;
						int eq = ps->GetInt();
						if ( eq == param  )
						{
							SetAnimation(txt_param, false);
							return;
						}
						break;
					}
				case afcLocalJumpIfIntEquals:
					{
						if ( !ps->CheckParamTypes(2, stInt, stIntOrNone) )
							break;
						int jmp = ps->PopInt();
						int eq = ps->GetInt();
						if ( eq == param  )
							this->sprite->JumpFrame(jmp);
						break;
					}
				case afcLocalJumpIfNextIntEquals:
					{
						if ( !ps->CheckParamTypes(3, stInt, stIntOrNone, stIntOrNone) )
							break;
						int jmp = ps->PopInt();
						int eq = ps->GetInt(1);
						if ( eq == param  )
							this->sprite->JumpFrame(jmp);
						break;
					}
				case afcWait:
					{
						if ( !ps->CheckParamTypes(1, stInt) )
							break;
						this->sprite->offset = -ps->PopInt();
						return;
					}
				case afcPop:
					{
						StackElement* sd = this->sprite->stack->Pop();
						DELETESINGLE(sd);
						break;
					}
				case afcJumpIfTargetY:
					{
						if ( !ps->CheckParamTypes(1, stInt) )
							break;
						int shift = ps->PopInt();
						ObjCharacter* target = NULL;
						ObjEnemy* oe = NULL;
						if ( this->type == objEnemy ) oe = (ObjEnemy*)this;
						if ( this->type == objEffect )
						{
							ObjDynamic* origin = (ObjDynamic*)this->parentConnection->getParent();
							oe = (ObjEnemy*)origin;
						}
						if ( oe ) target = oe->GetTarget();
						if ( !target ) target = playerControl->current;

						if ( target && !target->IsDead())
						{
							if ( shift >= 0 && target->aabb.p.y + shift > this->aabb.p.y )
								this->sprite->JumpFrame( param );
							else if ( shift< 0 && target->aabb.p.y - shift < this->aabb.p.y )
								this->sprite->JumpFrame( param );
						}
						break;
					}
				case afcJumpIfTargetX:
					{
						if ( !ps->CheckParamTypes(1, stInt) )
							break;
						int shift = ps->PopInt();
						ObjCharacter* target = NULL;
						ObjEnemy* oe = NULL;
						if ( this->type == objEnemy ) oe = (ObjEnemy*)this;
						if ( this->type == objEffect )
						{
							ObjDynamic* origin = (ObjDynamic*)this->parentConnection->getParent();
							oe = (ObjEnemy*)origin;
						}
						if ( oe ) target = oe->GetTarget();
						if ( !target ) target = playerControl->current;
						
						if ( target && !target->IsDead())
						{
							if ( shift >= 0 && target->aabb.p.x + shift > this->aabb.p.x )
								this->sprite->JumpFrame( param );

							else if ( shift < 0 && target->aabb.p.x - shift < this->aabb.p.x )
								this->sprite->JumpFrame( param );
						}
						break;
					}
				case afcSetNearestWaypoint:
					{
						if ( this->type == objEnemy && Waypoints )
						{
							ObjWaypoint* wp = NULL;
							ObjWaypoint* target = NULL;
							float dist = 0.0f;
							for ( vector<GameObject*>::iterator iter = Waypoints->childrenConnection->children.begin(); 
										iter != Waypoints->childrenConnection->children.end(); ++iter)
							{
								wp = (ObjWaypoint*)(*iter);
								if ( target == NULL || (wp->aabb.p - aabb.p).Length() < dist )
								{
									target = wp;
									dist = (wp->aabb.p - aabb.p).Length();
								}
							}
							((ObjEnemy*)this)->current_waypoint = target->id;
						}
					}
					break;
				case afcFlyToWaypoint:
					if ( this->type == objEnemy )
					{
						ObjEnemy* oe = (ObjEnemy*)this;
						ps->CheckParamTypes(1, stIntOrNone);
						oe->waypoint_mode = ps->PopInt();
						
						if ( oe->waypoint_mode & 1 )
						{
							ps->CheckParamTypes(1, stIntOrNone);
							oe->waypoint_global = ps->PopInt(); 
						}
			
						if ( oe->waypoint_mode & 2 ) //Ограничиваем скорость сверху
						{
							ps->CheckParamTypes(1, stInt);
							oe->waypoint_speed_limit = ps->PopInt() / 1000.0f;
						}

						else oe->waypoint_speed_limit = 0;

						oe->waypoint_start = Vector2(this->aabb.p.x, this->aabb.p.y);
						oe->waypoint_speed = param / 1000.0f;

						oe->movement = omtMovingToWaypoint;
					}
					break;
				case afcTeleportToWaypoint:
					if ( this->type == objEnemy )
					{
						ObjEnemy* oe = (ObjEnemy*)this;
						ObjWaypoint* wp = NULL;
						wp = param == -1 ? (ObjWaypoint*)GetGameObject(oe->current_waypoint) : GetWaypoint( param );

						if (wp)
						{
							this->aabb.p = wp->aabb.p;
							oe->vel.x = oe->vel.y = 0;
							oe->acc.x = oe->acc.y = 0;
						}
					}
					break;
				case afcAdjustHomingAcc:
					if ( this->type == objBullet )
					{
						ps->CheckParamTypes(1, stIntOrNone);
						float speed_limit = ps->PopInt()/1000.0f;
						float speed = param/1000.0f;
						ObjBullet* ob = (ObjBullet*)this;
						ObjCharacter* target = NULL;
						GameObject* shooter = this->parentConnection->getParent();
						if ( shooter && shooter->type == objEnemy )
							target = ((ObjEnemy*)shooter)->GetTarget();
						if ( !target ) target = playerControl->current;
						if ( !target ) break;
						float angle = atan2( target->aabb.p.y - ob->aabb.p.y, target->aabb.p.x - ob->aabb.p.x );
						ob->acc = speed * Vector2( cos(angle), sin(angle) );
						if ( abs(ob->acc.x) >= speed_limit ) ob->acc.x = 0;
						if ( abs(ob->acc.x) >= speed_limit ) ob->acc.y = 0;
					}
					break;
				case afcMoveToTargetX:
					if ( this->type == objEnemy )
					{
						ObjEnemy* oe = (ObjEnemy*)this;
						ObjCharacter* otarget = oe->GetTarget();
						float speed = param / 100.0f;
						if ( otarget )
						{
							if ( otarget->aabb.p.x > oe->aabb.p.x )
								oe->vel.x = speed;
							else if ( otarget->aabb.p.x < oe->aabb.p.x )
								oe->vel.x = -speed;
						}
						else this->SetAnimation( "idle", false );
					}
					break;
				case afcMoveToTargetY:
					if ( this->type == objEnemy )
					{
						ObjEnemy* oe = (ObjEnemy*)this;
						ObjCharacter* otarget = oe->GetTarget();
						float speed = param / 100.0f;
						if ( otarget )
						{
							if ( otarget->aabb.p.y > oe->aabb.p.y )
								oe->vel.y = speed;
							else if ( otarget->aabb.p.y < oe->aabb.p.y )
								oe->vel.y = -speed;
						}
					}
					break;
				case afcMoveToTarget:
					if ( this->type == objEnemy )
					{
						float speed = param / 100.0f;
						if ( ((ObjEnemy*)this)->target )
						{
							ObjEnemy* oe = (ObjEnemy*)this;
							ObjCharacter* otarget = oe->GetTarget();
							float angle = atan2( otarget->aabb.p.y - this->aabb.p.y, otarget->aabb.p.x - this->aabb.p.x );
							oe->vel = speed * Vector2( cos(angle), sin(angle) );
						}
					}
					break;
				case afcMountPointSet:
					{
						if ( !ps->CheckParamTypes(2, stInt, stInt) )
							break;
						int y = ps->PopInt();
						int x = ps->PopInt();
						if ( this->sprite->mp )
						{
							if ( param >= this->sprite->mpCount )
							{
								sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Попытка установить несуществующую точку крепления номер %i в анимации %s.", this->sprite->mpCount, this->sprite->cur_anim.c_str());
								break;
							}
							if ( this->childrenConnection ) this->childrenConnection->Event( ObjectEventInfo( eventMPChanged, Vector2((scalar)x,(scalar)y)-this->sprite->mp[ param ], param ) );
							this->sprite->mp[ param ] = Vector2( (scalar)x, (scalar)y );
						}
						break;
					}
				case afcCreateParticles:
					{
						if ( !ps->CheckParamTypes(3, stIntOrNone, stIntOrNone, stIntOrNone) )
							break;
						int y = ps->PopInt();
						int x = ps->PopInt();
						int mode = ps->PopInt();
						if ( this->GetFacing() ) x *= -1;
						//CAABB* emitter = ( param & 2 ? new CAABB(this->aabb) : NULL );
						UINT emitter = param & 2 ? this->id : 0 ;
						Vector2 coord = param & 8 ? Vector2((scalar)x,(scalar)y): this->aabb.p + Vector2((scalar)x, (scalar)y) ;
						GameObject* ops = ::CreateParticleSystem(txt_param, coord, emitter, param);
						if (mode & 1)
							ps->Push( ops->id );
					}
				break;
				case afcSetShadow:
					{
						if ( !this->IsPhysic() || !((ObjPhysic*)this)->IsDynamic() )
							break;
						((ObjDynamic*)this)->drops_shadow = 1 == param;
					}
				break;
				case afcCreateEnemyBullet:
					if ( this->type == objEnemy || this->type == objSprite || this->type == objEffect || this->type == objBullet )
					{
						ObjBullet* bullet = NULL;
						if ( !ps->CheckParamTypes(2, stIntOrNone, stIntOrNone) )
							break;
						int y = ps->PopInt();
						int x = ps->PopInt();
						if ( this->GetFacing() ) x *= -1;
						WeaponDirection wd = wdDownLeft;
						if ( param == 1 )
							if ( this->GetFacing() )
								 wd = wdDownLeft;
							else
								 wd = wdDownRight;
						else
						if ( param == 2 )
							if ( this->GetFacing() )
								wd = wdUpLeft;
							else
								wd = wdUpRight;
						else
							if ( this->GetFacing() )
								wd = wdLeft;
							else
								wd = wdRight;
						ObjCharacter* shooter = (ObjCharacter*)this;
						if ( param == 3 && this->type == objEffect ) 
							shooter =  (ObjCharacter*)this->parentConnection->getParent();
						bullet = ::CreateBullet( txt_param, Vector2( this->aabb.p.x + x, this->aabb.p.y + y ),
												 shooter, wd );
						if ( param == 4 && bullet) //TODO: ...
						{
							if ( y == 0 ) y = 1;
							bullet->t_value = x * (2.0f/y)*PI;
						}
						if (bullet && this->GetFacing()) bullet->SetFacing( true );
					}
					else if(this->type == objBullet)
					{
						::CreateBullet( txt_param, Vector2(this->aabb.p.x, this->aabb.p.y), (ObjCharacter*)this->parentConnection->getParent(), wdRight);
					}
					break;
				case afcCreateEnemyRay:
					if ( this->type == objEnemy || this->type == objSprite || this->type == objEffect )
					{
						if ( !ps->CheckParamTypes(2, stIntOrNone, stIntOrNone) )
							break;
						int y = ps->PopInt();
						int x = ps->PopInt();
						if ( this->GetFacing() ) x *= -1;
						WeaponDirection wd = wdDownLeft;
						if ( param == 1 )
							if ( this->GetFacing() )
								 wd = wdDownLeft;
							else
								 wd = wdDownRight;
						else
						if ( param == 2 )
							if ( this->GetFacing() )
								wd = wdUpLeft;
							else
								wd = wdUpRight;
						else
							if ( this->GetFacing() )
								wd = wdLeft;
							else
								wd = wdRight;
						ObjRay* ray = CreateRay(txt_param, this->aabb.p + Vector2((scalar)x,(scalar)y), (ObjCharacter*)this, wd);
						if (ray && this->GetFacing()) ray->SetFacing( true );
					}
					else if(this->type == objBullet)
					{
						CreateRay( txt_param, Vector2(this->aabb.p.x, this->aabb.p.y), (ObjCharacter*)this->parentConnection->getParent(), wdRight);
					}
					break;
				case afcEnemyClean:
					{
						if ( fabs(this->aabb.p.x+this->aabb.W-CAMERA_X) >= 320+param
							/*|| fabs(this->aabb.p.y+this->aabb.H-CAMERA_Y) >= 240+param*/  )
							this->SetDead();
					}
					break;
				case afcAdjustAim:
					{
						ObjEnemy* oen = NULL;
						if ( this->type == objEnemy ) oen = (ObjEnemy*)this;
						else if ( this->type == objEffect )
						{
							ObjDynamic* origin = (ObjDynamic*)this->parentConnection->getParent();
							oen = (ObjEnemy*)origin;
						}
						if (!oen) break;
						ObjCharacter* target = oen->GetTarget();
						switch ( param )
						{
							case 1:		//Берём центр цели со смещением.
								{
									if ( !ps->CheckParamTypes(2, stInt, stInt) )
										break;
									int y = ps->PopInt();
									int x = ps->PopInt();
									if ( target )
										oen->aim = target->aabb.p + Vector2((scalar)x, (scalar)y);
									else
										oen->aim = Vector2((scalar)x, (scalar)y);
								}
								break;
							case 2:		//Смещение относительно собственного центра.
								{
									if ( !ps->CheckParamTypes(2, stInt, stInt) )
										break;
									int y = ps->PopInt();
									int x = ps->PopInt();
									oen->aim = oen->aabb.p + Vector2((scalar)x, (scalar)y);
								}
								break;
							case 3:		//Абсолютные координаты
								{
									if ( !ps->CheckParamTypes(2, stInt, stInt) )
										break;
									int y = ps->PopInt();
									int x = ps->PopInt();
									oen->aim = Vector2((scalar)x, (scalar)y);
								}
								break;
							case 4:		//Точно по игроку
								{
									if (playerControl->current) oen->aim = playerControl->current->aabb.p;
								}
								break;
							default:	//В простейшем случае просто берём координаты текущей цели
								{
									if ( target )
										oen->aim = target->aabb.p;
									else
										oen->aim = Vector2(0,0);
								}
						}
					}
					break;
				case afcAimedShot:
					//TODO: Если дойдёт до мультиплеера, то это плохо.
					if ( (this->type == objEnemy || this->type == objEffect ) )
					{
						ObjBullet* bullet = NULL;
						if ( !ps->CheckParamTypes(2, stIntOrNone, stIntOrNone) )
							break;
						int y = ps->PopInt();
						int x = ps->PopInt();
						if ( this->GetFacing() ) x *= -1;
						Vector2 aim_target;
						if (this->type == objEnemy)
						{
							aim_target = ((ObjEnemy*)this)->aim;
						}
						else
						{
							assert(this->type == objEffect);
							ObjEnemy* oe = (ObjEnemy*)this->parentConnection->getParent();
							if (oe)
								aim_target = oe->aim;
						}						
						bullet = ::CreateBullet( txt_param, Vector2( this->aabb.p.x + x, this->aabb.p.y + y ),
							(ObjCharacter*)this, aim_target, param );
						if (bullet && this->GetFacing() ) bullet->SetFacing( true );
						//if ( txt_param && 1 ) bullet->max_y_vel += 0.8; //<- WTF?
					}
					else if(this->type == objBullet)
					{
						/*ObjBullet* bul = NULL;

						bul = */::CreateBullet( txt_param, Vector2(this->aabb.p.x, this->aabb.p.y), (ObjCharacter*)this->parentConnection->getParent(), wdRight);

					}
					break;
				case afcAngledShot:
					if ( true )
					{
						ObjBullet* bullet = NULL;
						if ( !ps->CheckParamTypes(5, stInt, stInt, stInt, stInt, stInt) )
							break;
						int ani = ps->PopInt();
						int angle = ps->PopInt();
						int y = ps->PopInt();
						int x = ps->PopInt();
						int mode = ps->PopInt();
						ObjDynamic* shooter = playerControl->current;
						if ( mode & 2 )
							shooter = (ObjDynamic*)this;
						if ( this->GetFacing() )
						{
							x *= -1;
							angle *= -1;
						}
						bullet = ::CreateAngledBullet( txt_param, Vector2( this->aabb.p.x + x, this->aabb.p.y + y ),
							shooter, this->GetFacing(), angle, ani );
						if ( mode & 1 )
						{
							bullet->max_x_vel += fabs(shooter->vel.x);
							bullet->vel.x += fabs(shooter->vel.x);
						}
					}
					break;
				case afcRandomAngledSpeed:
					if ( true )
					{
						ObjDynamic* od = (ObjDynamic*)this;
						if ( !ps->CheckParamTypes(4, stInt, stInt, stInt, stInt) )
							break;
						int max_angle = ps->PopInt();
						int min_angle = ps->PopInt();
						float max_speed = ps->PopInt()/1000.0f;
						float min_speed = ps->PopInt()/1000.0f;
						float speed = Random_Float(min_speed, max_speed);
						float angle = ((float)Random_Int(min_angle, max_angle))*PI/180;
						od->vel = speed * Vector2( cos(angle), sin(angle) );
						if ( (param & 1) )
						{
							od->max_x_vel = max( od->max_x_vel, abs(od->vel.x) );
							od->max_y_vel = max( od->max_y_vel, abs(od->vel.y) );
						}
					}
					break;
				case afcFaceTarget:
					if ( this->type == objEnemy )
					{
						ObjEnemy* oe = (ObjEnemy*)this;
						ObjCharacter* otarget = oe->GetTarget();
						if ( otarget )
						{
							if ( oe->aabb.p.x > otarget->aabb.p.x )
								oe->SetFacing( true );
							if ( oe->aabb.p.x < otarget->aabb.p.x )
								oe->SetFacing( false );
						}
					}
					break;
				case afcCreateEffect:
					{
						ObjEffect* of = NULL;
						if ( !ps->CheckParamTypes(2, stIntOrNone, stIntOrNone) )
							break;
						int y = ps->PopInt();
						int x = ps->PopInt();
						//В третьем режиме смещение по x считается в направлении взгляда создателя.
						if ( (param & 4 ) && this->GetFacing() )
							x *= -1;
						
						if ( !ps->CheckParamTypes(1, stInt) )
							break;
						int mpoint = ps->PopInt();
						
						if ( this->type == objBullet )
							of = ::CreateEffect( txt_param,
							false, (ObjDynamic*)this->parentConnection->getParent(), (USHORT)mpoint, Vector2( (scalar)x, (scalar)y ) );
						else if ( this->type == objRay )
							of = ::CreateEffect( txt_param,
								false, (ObjDynamic*)this->parentConnection->getParent(), (USHORT)mpoint, Vector2( (scalar)x, (scalar)y ) );
						else
							of = ::CreateEffect( txt_param,
												  false, (ObjDynamic*)this, (USHORT)mpoint, Vector2( (scalar)x, (scalar)y ) );
						if (of)
						{
							//Если первый режим включен, то передаём IsMirrored
							if ( (param & 1 ) && this->GetFacing() )
								of->SetFacing( true );
							//Если второй режим включен, но пытаемся компенсировать разницу между центром и углом.
							if ( param & 2 )
							{
								of->aabb.p -= Vector2( ((scalar)(of->sprite->frameWidth))/2, ((scalar)(of->sprite->frameHeight))/2 );
								//Больше не передаём скорость, есть точки привязки
								//if ( this->type == objBullet && ( ((ObjPhysic*)obj)->IsDynamic() ) ) //А если это ещё и пуля и создаваемый объект подвижен - передаём скорость.
								//	((ObjDynamic*)obj)->vel = ((ObjBullet*)this)->shooter->vel;

							}
						}
						else
						{
							sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV,
								"Ошибка создания объекта %s в анимации %s:%d",
								txt_param, this->sprite->cur_anim.c_str(), a->current);
						}

					}
					break;
				case afcCreateObject:
					{
						GameObject* obj = NULL;
						if ( !ps->CheckParamTypes(2, stIntOrNone, stIntOrNone) )
							break;
						int y = ps->PopInt();
						int x = ps->PopInt();
						//В третьем режиме смещение по x считается в направлении взгляда создателя.
						if ( (param & 4 ) && this->GetFacing() )
							x *= -1;
						obj = ::CreateSprite( txt_param, Vector2(  this->aabb.p.x+x, this->aabb.p.y+y ), false, NULL);

						if (obj)
						{
							//Если первый режим включен, то передаём IsMirrored
							if ( (param & 1 ) && this->GetFacing() )
								obj->SetFacing( true );
							//Если второй режим включен, но пытаемся компенсировать разницу между центром и углом.
							if ( param & 2 )
							{
								obj->aabb.p -= Vector2( ((scalar)(obj->sprite->frameWidth)/2), ((scalar)(obj->sprite->frameHeight))/2 );
								//Больше не передаём скорость, есть точки привязки
								//if ( this->type == objBullet && ( ((ObjPhysic*)obj)->IsDynamic() ) ) //А если это ещё и пуля и создаваемый объект подвижен - передаём скорость.
								//	((ObjDynamic*)obj)->vel = ((ObjBullet*)this)->shooter->vel;

							}
						}
						else
						{
							sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV,
								"Ошибка создания объекта %s в анимации %s:%d",
								txt_param, this->sprite->cur_anim.c_str(), a->current);
						}

						break;
					}
				case afcCreateEnemy:
					{
						ObjEnemy* obj = NULL;
						if ( !ps->CheckParamTypes(2, stIntOrNone, stIntOrNone) )
							break;
						int y = ps->PopInt();
						int x = ps->PopInt();
						if ( (param & 1) && this->GetFacing() )
							x *= -1;
						obj = ::CreateEnemy( txt_param, Vector2( aabb.GetMin(0)+x, aabb.GetMin(1)+y ), "init");
						if (obj)
						{
							if ( (param & 2) && this->GetFacing() )
								obj->SetFacing( true );
							this->sprite->stack->Push(obj->id);
							if ( (param & 4) )
							{
								ObjectConnection::addChild( this, obj );
								obj->attached = true;
							}
						}
						break;
					}
				case afcSummonObject:
					{
						ObjDynamic* obj = NULL;
						if ( !ps->CheckParamTypes(3, stInt, stInt, stInt) )
							break;
						int y = ps->PopInt();
						int x = ps->PopInt();
						UINT oid = ps->PopInt();
						if ( (param & 1) && this->GetFacing() )
							x *= -1;
						obj = (ObjDynamic*)GetGameObject( oid );
						if (obj)
						{
							obj->aabb.p = Vector2( aabb.GetMin(0)+x, aabb.GetMin(1)+y );
							if ( (param & 2) && this->GetFacing() )
								obj->SetFacing( true );
							this->sprite->stack->Push(obj->id);
						}
						break;
					}
				case afcJumpIfStackIsNotEmpty:
					{
						if ( !ps->isEmpty() )
						{
							this->sprite->JumpFrame( param );
						}
					}
					break;
				case afcJumpIfWaypointClose:
					{
						if ( !ps->CheckParamTypes(2, stInt, stInt) )
							break;
						int num = ps->PopInt();
						int dist = ps->PopInt();
						ObjWaypoint* wp = GetWaypoint( num );
						if ( wp && ( wp->aabb.p - aabb.p ).Length() < dist )
						{
							this->sprite->JumpFrame( param );
						}
					}
					break;
				case afcMirror:
					{
						this->SetFacing( !this->GetFacing() );
					}
					break;
				case afcCreateItem: //Копипаста такая копипаста
					{
						ObjItem* obj = NULL;
						if ( !ps->CheckParamTypes(2, stIntOrNone, stIntOrNone) )
							break;
						int y = ps->PopInt();
						int x = ps->PopInt();
						//В третьем режиме смещение по x считается в направлении взгляда создателя.
						if ( (param & 4 ) && this->GetFacing() )
							x *= -1;
						//Иначе просто создаём объект.
						obj = ::CreateItem( txt_param, Vector2(  this->aabb.p.x+x, this->aabb.p.y+y ), NULL);

						if (obj)
						{
							//Если первый режим включен, то передаём IsMirrored
							if ( (param & 1 ) && this->GetFacing() )
								obj->SetFacing( true );
							//Если второй режим включен, но пытаемся компенсировать разницу между центром и углом.
							if ( param & 2 )
							{
								obj->aabb.p -= Vector2( ((scalar)(obj->sprite->frameWidth))/2, ((scalar)(obj->sprite->frameHeight))/2 );
							}
						}
						else
						{
							sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV,
								"Ошибка создания предмета %s в анимации %s:%d",
								txt_param, this->sprite->cur_anim.c_str(), a->current);
						}
						break;
					}
				case afcDestroyObject:
					{
						if ( !(param & 2) )
						{
							this->SetDead();
							return;
						}
						if ( param & 1 && this->childrenConnection )
						{
							for ( vector<GameObject*>::iterator iter = childrenConnection->children.begin(); iter != childrenConnection->children.end(); ++iter)
								(*iter)->SetDead();
						}
						break;
					}
				case afcJumpIfWeaponReady:
					if ( this->type == objPlayer ) //Только у игрока есть изменяемое оружие
					{
						ObjPlayer* op = (ObjPlayer*)this;
						if (op->cur_weapon && op->cur_weapon->IsReady() && op->ammo > 0 )
						{
							this->sprite->JumpFrame( param );
						}
					}
					break;
				case afcPlaySound:
					//assert(snd);
					if (soundMgr)
					{
						Vector2 origin = this->aabb.p;
						if ( param & 1 ) origin = Vector2( CAMERA_X, CAMERA_Y );
						soundMgr->PlaySnd(string(txt_param), true, origin );
					}
					break;
				case afcEnvSound:
					{
						//assert(snd);
						char * sound_name = ((ObjDynamic*)this)->env->GetSound(param);
						if (!sound_name) break;
						if (soundMgr)
							soundMgr->PlaySnd(string(sound_name), true, this->aabb.p);
					}
					break;
				case afcEnvSprite:
					{
						char * sprite_proto = ((ObjDynamic*)this)->env->GetSprite(param);
						if (!sprite_proto) break;
						GameObject* obj = NULL;
						if ( !ps->CheckParamTypes(3, stInt, stIntOrNone, stIntOrNone) )
							break;
						int sparam = ps->PopInt();
						int y = ps->PopInt();
						int x = ps->PopInt();
						//В третьем режиме смещение по x считается в направлении взгляда создателя.
						if ( (sparam & 4 ) && this->GetFacing() )
							x *= -1;
						obj = ::CreateSprite( sprite_proto, Vector2(  this->aabb.p.x+x, this->aabb.p.y+y ), false, NULL);

						if (obj)
						{
							//Если первый режим включен, то передаём IsMirrored
							if ( (sparam & 1 ) && this->GetFacing() )
								obj->SetFacing( true );
							//Если второй режим включен, но пытаемся компенсировать разницу между центром и углом.
							if ( sparam & 2 )
							{
								obj->aabb.p -= Vector2( ((scalar)(obj->sprite->frameWidth))/2, ((scalar)(obj->sprite->frameHeight))/2 );
								//Больше не передаём скорость, есть точки привязки
								//if ( this->type == objBullet && ( ((ObjPhysic*)obj)->IsDynamic() ) ) //А если это ещё и пуля и создаваемый объект подвижен - передаём скорость.
								//	((ObjDynamic*)obj)->vel = ((ObjBullet*)this)->shooter->vel;

							}
						}
						else
						{
							sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV,
								"Ошибка создания объекта %s в анимации %s:%d",
								sprite_proto, this->sprite->cur_anim.c_str(), a->current);
						}

						break;
					}
				case afcCallFunction:
					{
						if (!txt_param)
						{
							sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_ERROR_EV, 
								"Не указано имя функции. proto_name %s, анимация %s, кадр %d", 
								this->sprite->proto_name, this->sprite->cur_anim.c_str(), a->current);
							break;
						}
						lua_getglobal(lua, txt_param);
						if (!lua_isfunction(lua, -1))
						{
							sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_ERROR_EV, 
								"Тип %s - %s. proto_name %s, анимация %s, кадр %d", 
								txt_param, lua_typename(lua, lua_type(lua, -1)), this->sprite->proto_name, this->sprite->cur_anim.c_str(), a->current);
							lua_pop(lua, 1);
							break;
						}

						lua_pushinteger(lua, this->id);
						lua_pushinteger(lua, param);
						if (lua_pcall(lua, 2, 0, 0) != 0 )
						{
							sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_ERROR_EV,
								"Ошибка выполнения функции %s(%d,%d), proto_name %s, анимация %s, кадр %d: %s",
								txt_param, this->id, param, this->sprite->proto_name, this->sprite->cur_anim.c_str(), a->current, lua_tostring(lua, -1) );
							lua_pop(lua, 1);
							break;
						}

						break;
					}
				case afcCallFunctionWithStackParameter:
					{
						if ( !ps->CheckParamTypes(1, stInt) )
							break;
						if (!txt_param)
						{
							sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_ERROR_EV, 
								"Не указано имя функции. proto_name %s, анимация %s, кадр %d", 
								this->sprite->proto_name, this->sprite->cur_anim.c_str(), a->current);
							break;
						}
						lua_getglobal(lua, txt_param);
						if (!lua_isfunction(lua, -1))
						{
							sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_ERROR_EV, 
								"Тип %s - %s. proto_name %s, анимация %s, кадр %d", 
								txt_param, lua_typename(lua, lua_type(lua, -1)), this->sprite->proto_name, this->sprite->cur_anim.c_str(), a->current);
							lua_pop(lua, 1);
							break;
						}

						lua_pushinteger(lua, this->id);
						lua_pushinteger(lua, ps->PopInt());
						if (lua_pcall(lua, 2, 0, 0) != 0 )
						{
							sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_ERROR_EV,
								"Ошибка выполнения функции %s(%d,%d), proto_name %s, анимация %s, кадр %d: %s",
								txt_param, this->id, param, this->sprite->proto_name, this->sprite->cur_anim.c_str(), a->current, lua_tostring(lua, -1) );
							lua_pop(lua, 1);
							break;
						}

						break;
					}
				case afcNone:
				default:
					break;
				}
			}

		}
	}

}

void GameObject::ObjectConnection::parentDead()
{
	this->orphan = true;
	if ( this->children.size() == 0 )
		ObjectConnection::removeConnection( this );
}

GameObject* GameObject::ObjectConnection::getParent()
{
	return this->orphan ? NULL : this->parent;
}


void GameObject::ObjectConnection::childDead( UINT id )
{
	for ( vector<GameObject*>::iterator iter = children.begin(); iter != children.end(); ++iter)
	{
		if ( (*iter)->id == id )
		{
			children.erase( iter );
			break;
		}
	}

	if ( this->children.size() == 0 && this->orphan )
		GameObject::ObjectConnection::removeConnection( this );
}

void GameObject::ObjectConnection::_addChild( GameObject* child )
{
	this->children.push_back( child );
}

void GameObject::ObjectConnection::addChild( GameObject* parent, GameObject* child )
{
	if ( !parent->childrenConnection )
		parent->childrenConnection = new ObjectConnection( parent );
	parent->childrenConnection->_addChild( child );
	child->parentConnection = parent->childrenConnection;
}

void GameObject::ObjectConnection::Event( ObjectEventInfo event_info )
{
	if ( this->children.size() == 0 )
		return;
	
	for ( vector<GameObject*>::iterator iter = children.begin(); iter != children.end(); ++iter)
		(*iter)->ParentEvent( event_info );
}

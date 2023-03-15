#include "StdAfx.h"

#include "object_manager.h"
#include "objects/object_physic.h"
#include "objects/object_sprite.h"
#include "objects/object_enemy.h"
#include "objects/object_item.h"
#include "objects/object_character.h"
#include "objects/object_spawner.h"
#include "objects/object_ribbon.h"
#include "../script/api.h"

#include "editor.h"

#ifdef MAP_EDITOR
#include <set>
#include "../script/script.h"

#include "phys/sap/OPC_ArraySAP.h"

bool editor_ShowBorders = false;

extern lua_State* lua;
extern Opcode::ArraySAP *asap;

UINT editorAreaSelectorObject = 0;
bool editorAreaSelectNeeded = false;
bool editorPhysicProcessed = false;
set<UINT> editorObjectsInArea;
int editorAreaSelectProc = LUA_NOREF;


void ObjPhysic::PhysProcess()
{
	bool movement_occured = false;
	if ( old_aabb != aabb )
	{
		old_aabb = aabb;
		movement_occured = true;
	}

	// Скажи нет идеальным геометрическим фигурам
	assert(aabb.H != 0);
	assert(aabb.W != 0);



	if (aabb != old_aabb || movement_occured)
	{
		if ( this->childrenConnection )
			this->childrenConnection->Event( ObjectEventInfo( eventMoved, aabb.p - old_aabb.p ) );
		asap->UpdateObject(sap_handle, aabb.GetASAP_AABB());
	}

}

#endif // MAP_EDITOR



void EditorGetObjects(CAABB area)
{
#ifdef MAP_EDITOR
	if (editorAreaSelectNeeded)
		return;

	GameObject* obj = new ObjPhysic;
	obj->aabb = area;
	obj->SetPhysic();
	AddObject(obj);
	editorAreaSelectorObject = obj->id;
	editorAreaSelectNeeded = true;
#else
	UNUSED_ARG(area);
#endif // MAP_EDITOR
}


void ProcessEditor()
{
#ifdef MAP_EDITOR
	if (editorAreaSelectNeeded && editorPhysicProcessed)
	{

		lua_createtable(lua, 0, 2);	// Стек: t
		int i = 1;
		for (set<UINT>::iterator it = editorObjectsInArea.begin(); it != editorObjectsInArea.end(); it++, i++)
		{
			lua_pushinteger(lua, *it);
			lua_rawseti(lua, -2, i);
		}

		if (SCRIPT::ExecChunkFromReg(editorAreaSelectProc, 1))
		{
			SCRIPT::RemoveFromRegistry(editorAreaSelectProc);
		}

		GameObject* obj = GetGameObject(editorAreaSelectorObject);
		if (obj) obj->SetDead();
		editorAreaSelectorObject = 0;
		editorAreaSelectNeeded = false;
		editorObjectsInArea.clear();
		editorPhysicProcessed = false;
	}
#else
#endif // MAP_EDITOR
}

void AddObjectInArea(UINT id)
{
#ifdef MAP_EDITOR
	editorObjectsInArea.insert(id);
#else
	UNUSED_ARG(id);
#endif // MAP_EDITOR
}

void EditorRegAreaSelectProc(lua_State* L)
{
#ifdef MAP_EDITOR
	SCRIPT::RegProc(L, &editorAreaSelectProc, 1);
#else
	UNUSED_ARG(L);
#endif // MAP_EDITOR
}

int EditorCopyObject(lua_State* L)
{
#ifdef MAP_EDITOR
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "Ожидается id объекта.");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "Ожидается координата x копии объекта.");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "Ожидается координата y копии объекта.");
	GameObject* obj = GetGameObject( lua_tointeger(L, 1) );
	float x = lua_tonumber(L, 2);
	float y = lua_tonumber(L, 3);
	if ( !obj ) return 0;
	switch( obj->type )
	{
	case objSprite:
		{
			if ( !obj->proto_name )
			{
				CAABB new_box = CAABB( x - obj->aabb.W, y - obj->aabb.H, x + obj->aabb.W, y + obj->aabb.H );
				lua_pushinteger(L, CreateColorBox( new_box, obj->sprite->z, obj->sprite->color )->id );
			}
			else
			{
				if ( obj->grouped )
				{
					GameObject* group = CreateSprite( obj->proto_name, Vector2(x, y), false, NULL );
					GameObject* add = CreateSprite( obj->proto_name, Vector2(x+2*(obj->aabb.W-group->aabb.W), y+2*(obj->aabb.H-group->aabb.H)), false, NULL );
					GroupObjects( group->id, add->id );
					lua_pushinteger(L, group->id );
				}
				else
				{
					GameObject* go = CreateSprite( obj->proto_name, Vector2(x, y), false, NULL );
					lua_pushinteger(L, go->id );
				}
			}
			return 1;
		}
	case objTile:
		{
			GameObject* go = CreateSprite( obj->proto_name, Vector2(x, y), false, NULL );
			/*
			go->sprite->animsCount = 0;
			go->sprite->currentFrame = obj->sprite->currentFrame;
			go->aabb.W = obj->aabb.W;
			go->aabb.H = obj->aabb.H;
			go->type = objTile;
			if ( go->sprite ) go->sprite->z = obj->sprite->z;*/
			go->sprite->SetCurrentFrame(obj->sprite->currentFrame);
			go->sprite->animsCount = 0;
			go->aabb.W  = go->sprite->frameWidth / 2;
			go->aabb.H  = go->sprite->frameHeight / 2;
			go->sprite->z = obj->sprite->z;
			go->type = objTile;
			lua_pushinteger(L, go->id );
			return 1;
		}
	case objEnemy:
		{
			lua_pushinteger(L, CreateEnemy( obj->proto_name, Vector2(x, y), NULL )->id );
			return 1;
		}
	case objItem:
		{
			if ( obj->grouped )
			{
				GameObject* group = CreateItem( obj->proto_name, Vector2(x, y), NULL );
				GameObject* add = CreateItem( obj->proto_name, Vector2(x+2*(obj->aabb.W-group->aabb.W), y+2*(obj->aabb.H-group->aabb.H)), NULL );
				GroupObjects( group->id, add->id );
				lua_pushinteger(L, group->id );
			}
			else
				lua_pushinteger(L, CreateItem( obj->proto_name, Vector2(x, y), NULL )->id );
			return 1;
		}
	case objSpawner:
		{
			ObjSpawner* os = (ObjSpawner*)obj;
			lua_pushinteger(L, CreateSpawner(Vector2(x, y), os->proto_name, os->maximumEnemies, os->enemySpawnDelay, os->direction, os->size, os->respawn_dist )->id );
			return 1;
		}
	case objRibbon:
		{
			ObjRibbon* orb = (ObjRibbon*)obj;
			ObjRibbon* nor = CreateRibbon( orb->proto_name, orb->k, Vector2(x, y), orb->sprite->z, orb->from_proto );
			nor->setBounds( orb->bl, orb->bt, orb->br, orb->bb );
			nor->setBoundsUsage( orb->ubl, orb->ubt, orb->ubr, orb->ubb );
			nor->setRepitition( orb->repeat_x, orb->repeat_y );
			lua_pushinteger(L, nor->id);
			return 1;
		}
	default:
		break;
	}
	return 0;

#else
	UNUSED_ARG(L);
	return 0;
#endif // MAP_EDITOR
}

extern LSQ_HandleT ObjTree;

int EditorDumpMap(lua_State* L)
{
#ifdef MAP_EDITOR
	lua_createtable(L, LSQ_GetSize( ObjTree ), 0);	//table

	GameObject* obj;
	LSQ_IteratorT ObjIter = LSQ_GetFrontElement(ObjTree);
	if (ObjIter == LSQ_HandleInvalid)
		return 1;
	int obj_num = 1;

	while (!LSQ_IsIteratorPastRear(ObjIter))
	{
		obj = ((GameObject*)LSQ_DereferenceIterator(ObjIter));
		PushObject( L, obj );	//table obj
		lua_rawseti( L, -2, obj_num ); //table
		LSQ_AdvanceOneElement(ObjIter);
		obj_num++;
	}
	LSQ_DestroyIterator(ObjIter);

	return 1;
#else
	UNUSED_ARG(L);
	return 0;
#endif // MAP_EDITOR
}

int EditorResize(lua_State* L)
{
#ifdef MAP_EDITOR
	GameObject* obj = GetGameObject( lua_tointeger(L, 1) );
	int axis = lua_tointeger(L, 2);
	int align = lua_tointeger(L, 3);
	int ammount = lua_tointeger(L, 4);
	if ( axis == 0 )	//Потом, когда просплюсь, я об этом пожалею. Но это будет потом.
	{
		if ( obj->aabb.W + ammount/2.0 < 0.5f ) return 0;
		if ( align == -1 )
			obj->aabb.p.x += ammount/2.0f;
		if ( align == 1 )
		{
			ammount = -ammount;
			obj->aabb.p.x -= ammount/2.0f;
		}
		obj->aabb.W += ammount/2.0f;
	}
	else
	{
		if ( obj->aabb.H + ammount/2.0 < 0.5f ) return 0;
		if ( align == -1 )
			obj->aabb.p.y += ammount/2.0f;
		if ( align == 1 )
		{
			ammount = -ammount;
			obj->aabb.p.y -= ammount/2.0f;
		}
		obj->aabb.H += ammount/2.0f;
	}
	obj->grouped = true;
	if ( obj->sprite != NULL )
	{
		obj->sprite->renderMethod = rsmRepeatXY;
	}
	return 0;
#else
	UNUSED_ARG(L);
	return 0;
#endif //MAP_EDITOR
}
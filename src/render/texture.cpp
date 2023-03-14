#include "StdAfx.h"

#include "../misc.h"

#include "texture.h"
#include "image.h"

#include "../script/script.h"

#include "../resource_mgr.h"

extern char path_textures[MAX_PATH];

extern ResourceMgr<Texture> * textureMgr;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool GenTexCoords(Texture* tex, FrameInfo* f, lua_State* L)
{
	if (!tex && !f && !L)
		return false;

	// Стек: fname env main
	UINT i = 0;
	float x1 = 0.0f, y1 = 0.0f, x2 = 0.0f, y2 = 0.0f, w = 0.0f, h = 0.0f, ox = 0.0f, oy = 0.0f;

	lua_pushnil(L);  // Стек: fname env main nil
	FrameInfo* fi = NULL;
	while (lua_next(L, -2) != 0 && i < tex->framesCount)
	{
		fi = f + i;

		// Стек: fname env main key main[i]
		SCRIPT::GetFloatFieldByName(L, "x", &x1);
		SCRIPT::GetFloatFieldByName(L, "y", &y1);
		SCRIPT::GetFloatFieldByName(L, "w", &w);
		SCRIPT::GetFloatFieldByName(L, "h", &h);
		SCRIPT::GetFloatFieldByName(L, "ox", &ox);
		SCRIPT::GetFloatFieldByName(L, "oy", &oy);

		x2 = x1 + w;
		y2 = y1 + h;

		//if (fi->size)
		{
			fi->size.x = w;
			fi->size.y = h;
		}

		//if (fi->offset)
		{
			fi->offset.x = ox;
			fi->offset.y = oy;
		}

		float CWidth = (float)tex->width;
		float CHeight = (float)tex->height;
		float cx1 = x1 / CWidth;
		float cx2 = x2 / CWidth;
		float cy1 = 1 - y1 / CHeight;
		float cy2 = 1 - y2 / CHeight;

		if (fi->coord)
		{
			fi->coord[0].x = cx1; fi->coord[0].y = cy1;
			fi->coord[1].x = cx2; fi->coord[1].y = cy1;
			fi->coord[2].x = cx2; fi->coord[2].y = cy2;
			fi->coord[3].x = cx1; fi->coord[3].y = cy2;
		}

		if (fi->coordMir)
		{
			fi->coordMir[0].x = cx2; fi->coordMir[0].y = cy1;
			fi->coordMir[1].x = cx1; fi->coordMir[1].y = cy1;
			fi->coordMir[2].x = cx1; fi->coordMir[2].y = cy2;
			fi->coordMir[3].x = cx2; fi->coordMir[3].y = cy2;
		}


		lua_pop(L, 1);	// Стек: fname env main key
		i++;
	}

	if(i != tex->framesCount)
	{
		return false;
	}

	return true;
}

bool Texture::LoadTexFramesDescr()
{
	extern lua_State* lua;
	assert(lua);

	STACK_CHECK_INIT(lua);

	char* fname = new char[strlen(path_textures) + strlen(this->name.c_str()) + 5];
	memset(fname, '\0', strlen(path_textures) + strlen(this->name.c_str()) + 5);
	sprintf(fname, "%s%s.lua", path_textures, this->name.c_str());

	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Загружаем описание кадров текстуры: %s", fname);

	// Файл протоипа будет выполнен в защищенном окружении, чтобы не запороть что-нить глобальное.
	// Окружение создается как здесь: http://community.livejournal.com/ru_lua/402.html
	lua_newtable(lua);				// Стек: env
	lua_newtable(lua);				// Стек: env meta
	lua_getglobal(lua, "_G");			// Стек: env meta _G
	lua_setfield(lua, -2, "__index");	// Стек: env meta
	lua_setmetatable(lua, -2);		// Стек: env
	if(luaL_loadfile(lua, fname))
	{
		// Какая-то ошибка загрузки файла
		const char* err = lua_tostring(lua, -1);	// Стек: env err
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "%s", err);
		DELETEARRAY(fname);
		lua_pop(lua, 2);	// Стек: 

		STACK_CHECK(lua);

		return false;
	}

	// Стек: env loadfile
	lua_pushvalue(lua, -2);			// Стек: env loadfile env
	lua_setfenv(lua, -2);				// Стек: env loadfile

	if(lua_pcall(lua, 0, 0, 0))
	{
		// Какая-то ошибка выполнения файла
		const char* err = lua_tostring(lua, -1);	// Стек: env err
		sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, err );
		lua_pop(lua, 2);	// Стек: 
		DELETEARRAY(fname);

		STACK_CHECK(lua);

		return false;
	}
	else
	{
		// Стек: env
		SCRIPT::GetUIntFieldByName(lua, "count", (UINT*)&this->framesCount);

		if (!this->framesCount)
		{
			DELETEARRAY(fname);
			lua_pop(lua, 1);	// Стек: 

			STACK_CHECK(lua);

			return true;	// Если вдруг для текстуры создавать кадры не надо
		}

		lua_getfield(lua, -1, "main");	// Стек: env main
		if (lua_istable(lua, -1))
		{
			this->frame = new FrameInfo[this->framesCount];


			if(!GenTexCoords(this, this->frame, lua))
			{
				sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "В файле %s количество описанных кадров не соответствует значению переменной count", fname);
			}
		}
		else
			sLog(DEFAULT_SCRIPT_LOG_NAME, logLevelWarning, "В файле %s main - не таблица", fname);
		lua_pop(lua, 1);	// Стек: env

		lua_getfield(lua, -1, "overlay");	// Стек: env overlay
		if (lua_istable(lua, -1))
		{
			this->overlayCount = (UINT)lua_objlen(lua, -1);
			if (overlayCount > 0)
			{
				this->overlay = new FrameInfo[this->overlayCount*this->framesCount];
				int i = 0;
				for(lua_pushnil(lua); 
					lua_next(lua, -2) != 0 && i < (int)this->overlayCount; 
					i++, lua_pop(lua,1))
				{
					if(!GenTexCoords(this, &this->overlay[i*this->framesCount], lua))
					{
						sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_WARNING_EV, "В файле %s количество описанных оверлеев кадров не соответствует значению переменной count", fname);
					}
				}			
			}
			else
			{
				sLog(DEFAULT_SCRIPT_LOG_NAME, logLevelWarning, "В файле %s таблица overlay пустая", fname);
			}
		}
		lua_pop(lua, 1);	// Стек: env



	}

	DELETEARRAY(fname);
	lua_pop(lua, 1);	// Стек: 

	STACK_CHECK(lua);

	return true;
}

// load texture
// path - in file, texture
// out_name - name of the texture, will be used in game
bool Texture::Load()
{
	CGLImageData ImageData;
	if (ImageData.LoadTexture( this->file_name.c_str() ))
	{
		this->width = ImageData.width;
		this->height = ImageData.height;
		this->tex = ImageData.GetTexID();

		if (!LoadTexFramesDescr())
		{
			sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Ошибка загрузки описания кадров для текстуры %s. Используем стандартное.", this->name.c_str());
			this->framesCount = 1;
			this->frame = new FrameInfo[1];

			this->frame[0].size.x = (float)this->width; this->frame[0].size.y = (float)this->height;

			this->frame[0].coord[0].x = 0.0f; this->frame[0].coord[0].y = 0.0f;
			this->frame[0].coord[1].x = 1.0f; this->frame[0].coord[1].y = 0.0f;
			this->frame[0].coord[2].x = 1.0f; this->frame[0].coord[2].y = 1.0f;
			this->frame[0].coord[3].x = 0.0f; this->frame[0].coord[3].y = 1.0f;

			this->frame[0].coordMir[0].x = 1.0f; this->frame[0].coordMir[0].y = 0.0f;
			this->frame[0].coordMir[1].x = 0.0f; this->frame[0].coordMir[1].y = 0.0f;
			this->frame[0].coordMir[2].x = 0.0f; this->frame[0].coordMir[2].y = 1.0f;
			this->frame[0].coordMir[3].x = 1.0f; this->frame[0].coordMir[3].y = 1.0f;
		}


		return true;
	}

	return false;
}

bool Texture::Recover()
{
	CGLImageData ImageData;

	if (this->file_name.empty())
	{
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Unable to recover texture %s. Path is not saved.", this->name.c_str());
		return false;
	}

	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Recovering texture %s", this->name.c_str());
	if ( ImageData.LoadTexture(this->file_name.c_str()) )
	{
		this->tex = ImageData.GetTexID();
		return true;
	}

	return false;
}


#include "StdAfx.h"

#include "../render/renderer.h"
#include "../config.h"

#include "sprite.h"

#include "../resource_mgr.h"

//////////////////////////////////////////////////////////////////////////

extern UINT internal_time;
extern ResourceMgr<Texture> * textureMgr;
extern ResourceMgr<Proto> * protoMgr;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Вносит в стек строку
void ParametersStack::Push(const char* str)
{
	if (this->locked) return;
	StackElement* se = new StackElement();
	se->data.stringData = str;
	se->type = stString;
	se->next = this->top;
	this->top = se;
}

// Очевидно из названия
bool ParametersStack::isEmpty()
{
	if ( this->top )
		return false;
	else
		return true;
}

// Вносит в стек число
void ParametersStack::Push(int num)
{
	if (this->locked) return;
	StackElement* se = new StackElement();
	se->data.intData = num;
	se->type = stInt;
	se->next = this->top;
	this->top = se;
}

// Возвращает элемент стека
StackElement* ParametersStack::Pop()
{
	if (!top)
	{
		StackElement* se = new StackElement();
		se->data.intData = 0;
		se->next = NULL;
		se->type = stNone;
		return se;
	}
	else
	{
		StackElement* se = this->top;
		this->top = this->top->next;
		return se;
	}
}
//Проверяет типы элементов стека на соответствие, сверху вниз.
bool ParametersStack::CheckParamTypes(int num, StackElementType first, ...)
{
		va_list mark;
		StackElementType setp = first;
		StackElement* se = top;
		va_start( mark, first );
		for ( int i = 0; i < num; i++ )
		{
			if ( !se && setp && setp != stNone && setp != stIntOrNone )
			{
				sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Неверный набор параметров в стеке.");
				return false;
			}

			if ( se && se->type != setp && !(se->type == stInt && setp == stIntOrNone) )
			{
				sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Неверный набор параметров в стеке.");
				return false;
			}
/*
			if ( se )
			{
				if ( se->type != setp )
				{
					if ( !(se->type == stInt && setp == stIntOrNone) )
					{
						sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Неверный набор параметров в стеке.");
						return false;
					}
				}
			}
*/
			// TODO: warning: ‘StackElementType’ is promoted to ‘int’ when passed through ‘...’
			// note: (so you should pass ‘int’ not ‘StackElementType’ to ‘va_arg’)
			// note: if this code is reached, the program will abort
			setp = (StackElementType)va_arg( mark, int );
			if (se) se = se->next;
		}
		va_end( mark );
		return true;
}

//Возвращает инт с верхушки стека. Ничего не проверяет, чтобы не дублировать проверку CheckParamTypes
int ParametersStack::PopInt()
{
	StackElement* sd = this->Pop();
	int ret = sd->data.intData;
	DELETESINGLE(sd);
	return ret;
}
//Возвращает инт из стека. Ничего не проверяет, чтобы не дублировать проверку CheckParamTypes и не удаляет элемент
int ParametersStack::GetInt(int depth)
{
	if ( !this->top ) return 0;
	StackElement* sd = this->top;
	for ( int i = 0; i < depth; i++ )
	{
		sd = sd->next;
		if ( !sd ) return 0;
	}
	return sd->data.intData;
}

//Возвращает строку с верхушки стека. Ничего не проверяет, чтобы не дублировать проверку CheckParamTypes
const char* ParametersStack::PopString()
{
	StackElement* sd = this->Pop();
	const char * ret = sd->data.stringData;
	DELETESINGLE(sd);
	return ret;
}

//Возвращает строку с верхушки стека. Ничего не проверяет, чтобы не дублировать проверку CheckParamTypes
std::string ParametersStack::PopStdString()
{
	StackElement* sd = this->Pop();
	string ret = string(sd->data.stringData);
	DELETESINGLE(sd);
	return ret;
}

//Возвращает строку из стека. Ничего не проверяет, чтобы не дублировать проверку CheckParamTypes и не удаляет элемент
const char* ParametersStack::GetString(int depth)
{
	if ( !this->top ) return 0;
	StackElement* sd = this->top;
	for ( int i = 0; i < depth; i++ )
	{
		sd = sd->next;
		if ( !sd ) return 0;
	}
	return sd->data.stringData;
}

void ParametersStack::DumpToLog()
{
	sLog(DEFAULT_LOG_NAME, logLevelError, "SpriteStack Dump");
	if (!this->top) return;
	StackElement* sd = this->top;
	int i = 0;
	for (; sd; sd = sd->next, i++)
	{
		sLog(DEFAULT_LOG_NAME, logLevelError, "%d: t=%d, i=%d, s=%s", i, (int)sd->type, sd->data.intData, sd->type == stString ? sd->data.stringData : NULL);
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void Sprite::SetCurrentFrame(UINT cf)
{
	if (this->tex)
	{
		if (cf >= this->tex->framesCount)
		{
			// Если по какой-то причине код попал сюда из ChangeFrame и в a->frames[a->current] явно какая-то НЕХ,
			// значит был удален прототип со всеми списками кадров. Значит, надо искать, 
			// какого хрена он удалился пока еще существует этот спрайт.
			// Если код пришел сюда из api, значит проблемы в скриптах.Ы
			sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV,
				"Попытка указать не существующий на текстуре %s кадр %d (возможно, анимация %s)",
				this->tex->name.c_str(), cf, this->cur_anim.c_str());
			ASSERT(cf < this->tex->framesCount);	// Тут бы игру не только в дебаге рушить
			return;
		}
		else
		{
			this->frameWidth = (USHORT)this->tex->frame[cf].size.x;
			this->frameHeight = (USHORT)this->tex->frame[cf].size.y;
		}
	}
	
	this->currentFrame = cf;
}

// Устанавливает текущий кадр анимации на кадр с номером index
bool Sprite::JumpFrame( int index )
{
	Animation* a = this->GetAnimation( this->cur_anim_num );
	if ( !a || index < 0 || index > a->frameCount-1 )
	{
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Невозможный прыжок в анимации.");
		return false;
	}
	this->SetCurrentFrame(a->frames[ index ].num);
	a->current = index;
	return true;
}

// Переключает текущий кадр анимации на следующий.
// Возвращает true, если анимация завершена или необходимо ждать завершения кадра.
bool Sprite::ChangeFrame(Animation* a)
{
	if (IsAnimDone())
		return true;

	UINT now = internal_time;

	if ( a->current == MAX_ANIMFRAMES_COUNT )
	{
		a->current = 0;
		this->SetCurrentFrame(a->frames[a->current].num);
		prevAnimTick = now;
	}
	else if (now - prevAnimTick >= a->frames[a->current].duration - offset)
	{
		offset = 0;
		prevAnimTick = now;

		if ( a->current >= a->frameCount-1 )
		{
			SetAnimDone();
			return true;
		}

		a->current++;
		ASSERT(a->current < a->frameCount);

		// TODO: afcLoop в общем то не очень и нужен
		if (a->frames[a->current].command == afcLoop)
		{
			a->current = 0;
		}

		this->SetCurrentFrame(a->frames[a->current].num);
	}
	else
		return true;

	return false;

}

Animation* Sprite::GetAnimation(string name)
{
	if (this->animsCount > 0)
	{
		ASSERT(anims);
		AnimationMap::const_iterator it = this->animNames->find(name);
		if (it != this->animNames->end())
			return &anims[it->second];
	}

	return NULL;
}

Animation* Sprite::GetAnimation(size_t num)
{
	if (this->animsCount > 0 && num < this->animsCount && num < MAX_ANIMATIONS_COUNT)
	{
		ASSERT(anims);
		return &anims[num];
	}
	return NULL;
}

bool Sprite::SetAnimation(size_t num)
{
	Animation* a = GetAnimation(num);
	if (a)
	{
		this->cur_anim = string(a->name);
		this->cur_anim_num = num;
		a->current = MAX_ANIMFRAMES_COUNT;
		this->ClearAnimDone();
		//this->ProcessSprite();
		return true;
	}
	else
	{
		this->cur_anim = "";
		this->cur_anim_num = MAX_ANIMATIONS_COUNT;
	}

	return false;
}


bool Sprite::SetAnimation(const string& anim_name)
{
	if (this->animNames)
	{
		AnimationMap::const_iterator it = this->animNames->find(anim_name);
		if (it != this->animNames->end())
		{
			return SetAnimation(it->second);
		}
	}
	return false;
}

void Sprite::Draw(const CAABB &aabb)
{
	if (!this->IsVisible())
		return;

	float x = aabb.p.x - aabb.W;
	float y = aabb.p.y - aabb.H;
#ifdef DEBUG_DRAW_OBJECTS_BORDERS
	float x_ph = x;
	float y_ph = y;
#endif // DEBUG_DRAW_OBJECTS_BORDERS

	if (this->tex && this->currentFrame >= this->tex->framesCount)
	{
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV,
			"Нарисовать кадр %d текстуры %s в анимации %s невозможно. Описано только %d кадров.",
			this->currentFrame, this->tex->name.c_str(), this->cur_anim.c_str(), this->tex->framesCount);
		return;
	}

	coord2f_t* frame_coord = NULL;
	coord2f_t* overlay_coord = NULL;
	//coord2f_t* overlay_off = this->tex->overlayOffset + this->currentFrame;
	//coord2f_t* overlay_size = this->tex->overlaySize + this->currentFrame;
	//coord2f_t* frame_size = this->tex->frameSize + this->currentFrame;
	FrameInfo* f = NULL;
	FrameInfo* o = NULL;
	float over_x = 0.0f, over_y = 0.0f;

	if (!render_without_texture)
	{
		#ifndef DEBUG_RENDER_UNLOADED_TEXTURES
			if (!this->tex)
				return;
		#endif // DEBUG_RENDER_UNLOADED_TEXTURES

		f = this->tex->frame + this->currentFrame;
		

		if (this->IsMirrored())
		{
			x -= f->size.x - this->realWidth - this->realX;
			y -= this->realY;
			frame_coord = f->coordMir;
		}
		else
		{
			x -= this->realX;
			y -= this->realY;
			frame_coord = f->coord;
		}
	}

	if (this->IsFixed())
	{
		extern float CAMERA_OFF_X;
		extern float CAMERA_OFF_Y;
		x -= CAMERA_OFF_X;
		y -= CAMERA_OFF_Y;
	}

	extern float CAMERA_LEFT;
	extern float CAMERA_RIGHT;
	extern float CAMERA_TOP;
	extern float CAMERA_BOTTOM;

	if (render_without_texture)
	{
		// Если спрайт находится за пределами обзора камеры, то мы его не рисуем.
		if (x + this->frameWidth < CAMERA_LEFT || x > CAMERA_RIGHT)
			return;
		if (y + this->frameHeight < CAMERA_TOP || y > CAMERA_BOTTOM )
			return;

		coord2f_t frame_size;
		frame_size.x = aabb.W*2;
		frame_size.y = aabb.H*2;
		RenderSprite(x, y, this->z, &frame_size, NULL, NULL, this->color);
	}
	else
	{
		if (renderMethod == rsmStandart)
		{
			// Если спрайт находится за пределами обзора камеры, то мы его не рисуем.
			if (x + this->frameWidth < CAMERA_LEFT || x > CAMERA_RIGHT)
				return;
			if (y + this->frameHeight < CAMERA_TOP || y > CAMERA_BOTTOM )
				return;

			RenderSprite(x, y, this->z, &f->size, frame_coord, this->tex, this->color);
			UINT use = 0;
			for (UINT i = 0; i < this->overlayCount; i++)
			{
				use = this->overlayUse[i];
				if (use >= tex->overlayCount)
				{
					sLog(DEFAULT_LOG_NAME, logLevelError, "В прототипе %s используется оверлей %d, хотя на текстуре описано %d оверлеев",
						this->proto_name, use, tex->overlayCount);
					continue;
				}

				o = &tex->overlay[use*tex->framesCount + this->currentFrame];
				if (this->IsMirrored())
				{
					over_x = x + f->size.x - o->offset.x - o->size.x;
					over_y = y + o->offset.y;
					overlay_coord = o->coordMir;
				}
				else
				{
					over_x = x + o->offset.x;
					over_y = y + o->offset.y;
					overlay_coord = o->coord;
				}
				RenderSprite(over_x, over_y, this->z, &o->size, overlay_coord, this->tex, this->ocolor[i]);
			}
		}
		else
		{
			CAABB c = aabb;
			if (this->IsFixed())
			{
				extern float CAMERA_OFF_X;
				extern float CAMERA_OFF_Y;
				c.p.x -= CAMERA_OFF_X;
				c.p.y -= CAMERA_OFF_Y;
			}

			// Если спрайт находится за пределами обзора камеры, то мы его не рисуем.
			if (c.Right() < CAMERA_LEFT || c.Left() > CAMERA_RIGHT)
				return;
			if (c.Bottom() < CAMERA_TOP || c.Top() > CAMERA_BOTTOM )
				return;
				
			RenderSprite(x, y, this->z, c, f, this->tex, this->color, IsMirrored(), renderMethod);
			//if (this->overlayCount)
			//{
			//	RenderSprite(over_x, over_y, this->z, &o->size, overlay_coord, this->tex, this->ocolor);
			//}	
		}
	}

#ifdef DEBUG_DRAW_MP0_POINT
	if (this->mpCount > 0)
	{
		Vector2 mpos = this->IsMirrored () ? Vector2(-this->mp[0].x, this->mp[0].y) : this->mp[0];
		Vector2 pos = aabb.p + mpos;

		RenderSprite(pos.x, pos.y, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, NULL, NULL, RGBAf(DEBUG_MP0_POINT_COLOR));
	}
#endif // RENDER_MP0_POINT

}



void Sprite::LoadFromProto(const Proto* proto)
{
	if (!proto)
		return;
	
	// TODO: в принципе, запоминать можно сразу *proto, все равно он не должен быть удален
	// раньше спрайта. Но, пока это еще в состоянии отладки, пусть будет. Да и хранить указатели - плохо.
	// Хотя, все хранится эе указатель на анимации из прототипа.
	proto_name = StrDupl(proto->name.c_str());
	proto->ReserveUsage();

	this->tex = textureMgr->GetByName(proto->texture);

	this->z = proto->z;

	if (tex)
	{
		if (proto->overlayCount && tex->overlay)
		{
			this->overlayCount = proto->overlayCount;
			if (proto->overlayCount > tex->overlayCount)
				sLog(DEFAULT_LOG_NAME, logLevelWarning, "В прототипе %s используется больше оверлеев, чем на текстуре %s", proto_name, proto->texture);
		}
		this->ocolor = proto->ocolor;
		this->overlayUse = proto->overlayUsage;

		this->frameWidth = (USHORT)tex->frame[0].size.x;
		this->frameHeight = (USHORT)tex->frame[0].size.y;
	}
	else
	{
		this->overlayCount = 0;
		this->ocolor = NULL;
		this->overlayUse = NULL;

		this->frameWidth = proto->frame_widht;
		this->frameHeight = proto->frame_height;
	}

	this->color = proto->color;
	
	this->SetVisible();

	if (proto->animations && proto->animationsCount > 0)
	{
		this->animNames = &(proto->animNames);
		this->animsCount = proto->animationsCount;
		// Массив копий специально создается с помощью malloc, чтобы не потм при освобождении
		// не вызывать деструкторы.
		this->anims = (Animation*)malloc(sizeof(Animation) * proto->animationsCount);
		memcpy(this->anims, proto->animations, sizeof(Animation) * proto->animationsCount);
	}
}



Sprite::~Sprite()
{
	// Массив копий удаляем с помощью free, чтобы не вызывать деструкторы.
	free(this->anims);

	DELETESINGLE(this->stack);
	DELETEARRAY(this->mp);

	if (proto_name)
	{
		const Proto* proto = protoMgr->GetByName(proto_name);
		ASSERT(proto);
		proto->ReleseUsage();
		DELETEARRAY(proto_name);
	}
}

#include "StdAfx.h"

#include "../config.h"

#include "../render/renderer.h"

#include "objects/object_sprite.h"

#include "ribbon.h"

#include "../resource_mgr.h"

//////////////////////////////////////////////////////////////////////////

extern ResourceMgr<Proto> * protoMgr;

extern config cfg;

extern float CAMERA_OFF_Y;
extern float CAMERA_OLD_OFF_Y;
extern float CAMERA_Y;
extern float CAMERA_OFF_X;
extern float CAMERA_OLD_OFF_X;
extern float CAMERA_X;
extern float CAMERA_LEFT;
extern float CAMERA_RIGHT;

//////////////////////////////////////////////////////////////////////////

void AddRibbon(Ribbon* r);

//////////////////////////////////////////////////////////////////////////

list<Ribbon*> ribbons;

//////////////////////////////////////////////////////////////////////////
// Ribbon (лента) - объект фона, движущийся со скоростью, отличной от скорости движения камеры.
// Может быть бесконечной или иметь прямоугольные границы, за пределами которых не используется.
// Спрайт, используемый для ленты должен по ширине равняться или превосходить ширину экрана.

// Рассчет положения и отрисовка ленты. Должно вызываться после изменения положения камеры.
void Ribbon::Draw()
{
	float dx = CAMERA_OFF_X - CAMERA_OLD_OFF_X;		// Смещение камеры.
	float dy = CAMERA_OFF_Y - CAMERA_OLD_OFF_Y;
	this->aabb.p.x -= dx*speed_factor;				// Смещение ленты.
	this->aabb.p.y -= dy*speed_factor_y;

	if (use_bounds && (br < CAMERA_LEFT || bl > CAMERA_RIGHT))
		return;			// Лента за экраном, нечего рисовать.

	float left = this->aabb.Left();
	float right = this->aabb.Right();

	Sprite* s = this->sprite;

	// Пока лента движется только по горизонтали, простой рассчет координаты Y.
	if (y_attached)
	{
		this->aabb.p.y  = CAMERA_Y;
	}
	float y = this->aabb.p.y - this->aabb.H;
	float y1 = s->frameY;						// Откуда в текстуре.
	float y2 = y1 + s->frameHeight;				// Докуда в текстуре.

	// Расчет координаты X
	if ((left <= CAMERA_LEFT) &&
		(right >= CAMERA_RIGHT))
	{
		// Лента полностью помещается на экране. Нужна одна отрисовка.
		float x = CAMERA_LEFT;						// Куда рисовать на экране.
		float x1 = s->frameX + (CAMERA_LEFT - left);// Откуда в текстуре.
		float x2 = x1 + cfg.scr_width;				// Докуда в текстуре.

		if (use_bounds)
		{
			// Ограничния от границ
			if (bl > x)
			{
				// Видна левая граница
				x1 += bl - x;
				x = bl;
			}

			if (br < CAMERA_RIGHT)
			{
				// Видна правая граница
				x2 -= CAMERA_RIGHT - br;
			}
		}

		RenderSprite(x, y, s->z, x1, y1, x2, y2, s->tex, s->IsMirrored(), s->color);
	}
	else
	{
		// Лента не полностью видна на экране. Необходимо нарисовать "хвост" спрайта и "голову".
		bool rend1 = true;			// Рисовать хвост
		bool rend2 = true;			// Рисовать голову

		float x1 = CAMERA_LEFT;
		float x11 = s->frameX + CAMERA_LEFT;
		float x12 = (float)(s->frameX + s->frameWidth);

		float x2 = 0;
		float x21 = s->frameX;

		if (dx >= 0)
		{
			// Камера движется влево
			// Левая сторона спрайта всегда будет за экраном слева, а
			// конец спрайта всегда на экране.
			if (left > CAMERA_LEFT)
			{
				// Сдвигаем спрайт влево на его ширину.
				aabb.p.x -= 2*aabb.W;
				left = aabb.Left();
				right = aabb.Right();
			}

			x11 -= left;
			x2 = aabb.Right();
		}
		else
		{
			// Камера движется вправо
			// Левая сторона спрайта всегда будет на экране, а конец спрайта всегда за
			// экраном справа.
			if (right < CAMERA_RIGHT)
			{
				// Сдвигаем спрайт вправо на его ширину.
				aabb.p.x += 2*aabb.W;
				left = aabb.Left();
				right = aabb.Right();
			}

			x11 += s->frameWidth - left;
			x2 = left;
		}

		float x22 = cfg.scr_width - (x12 - x11);

		if (use_bounds)
		{
			// Ограничния от границ.
			if (bl > x1 && bl < x2)
			{
				// Левая граница на хвосте.
				x11 += bl - x1;
				x1 = bl;
			}
			else if (bl > x2)
			{
				// Левая граница на голове, хвост не рисуется.
				x21 += bl - x2;
				x2 = bl;
				rend1 = false;
			}

			if (br < CAMERA_RIGHT && br > x2)
			{
				// Правая граница на голове.
				x22 -= (x22 - x21) - (br - x2);
			}
			else if (br < x2)
			{
				// Правая граница на хвосте, голова не рисуется.
				x12 -= x2 - br;
				rend2 = false;
			}
		}

		if (rend1)
		{
			// Рисуем хвост
			RenderSprite(x1, y, s->z, x11, y1, x12, y2, s->tex, s->IsMirrored(), s->color);
		}

		if (rend2)
		{
			// Рисуем голову
			RenderSprite(x2, y, s->z, x21, y1, x22, y2, s->tex, s->IsMirrored(), s->color);
		}
	}
}

void Ribbon::SetBounds(float x1, float y1, float x2, float y2)
{
	if (x1 != x2 && y1 != y2)
	{
		// Будут использоваться границы.
		this->use_bounds = true;
		this->bl = min(x1, x2);
		this->br = max(x1, x2);
		this->bt = min(x1, x2);
		this->bb = max(x1, x2);
	}
	else
	{
		this->use_bounds = false;
	}
}

//////////////////////////////////////////////////////////////////////////

Ribbon* CreateRibbon(const Proto* proto, Vector2 coord, float speed_factor, float speed_factor_y)
{
	if (!proto)
		return NULL;

	Ribbon* r = new Ribbon();

	r->sprite = new Sprite(proto);
	r->aabb.H = r->sprite->frameHeight * 0.5f;
	r->aabb.W = r->sprite->frameWidth * 0.5f;
	r->aabb.p = coord;

	r->speed_factor = speed_factor;
	r->speed_factor_y = speed_factor_y;
	AddRibbon(r);

	return r;
}

Ribbon* CreateRibbon(const char* proto_name, Vector2 coord, float speed_factor, float speed_factor_y)
{
	if (!proto_name)
		return NULL;

	return CreateRibbon(protoMgr->GetByName(proto_name, "ribbons/"), coord, speed_factor, speed_factor_y);
}

Ribbon* GetRibbon(UINT num)
{
	Ribbon* r = NULL;
	for(list<Ribbon*>::iterator it = ribbons.begin();
		it != ribbons.end();
		it++)
	{
		r = *it;
		if (r->id == num)
			return r;
	}
	return NULL;
}

void AddRibbon(Ribbon* r)
{
	static UINT n = 1;
	r->id = n;
	n++;
	ribbons.push_back(r);
}

void DeleteRibbon(Ribbon* r)
{
	DELETESINGLE(r);
}

void DeleteAllRibbons()
{
	for(list<Ribbon*>::iterator it = ribbons.begin();
		it != ribbons.end();
		it++)
	{
		DeleteRibbon(*it);
	}
	ribbons.clear();
}

// Рассчет положений и отрисовка всех лент. Должно вызываться после изменения положения камеры.
void DrawRibbons()
{
	for(list<Ribbon*>::iterator it = ribbons.begin();
		it != ribbons.end();
		it++)
	{
		Ribbon* r = *it;
		r->Draw();
	}
}

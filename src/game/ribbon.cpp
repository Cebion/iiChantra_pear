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
// Ribbon (�����) - ������ ����, ���������� �� ���������, �������� �� �������� �������� ������.
// ����� ���� ����������� ��� ����� ������������� �������, �� ��������� ������� �� ������������.
// ������, ������������ ��� ����� ������ �� ������ ��������� ��� ������������ ������ ������.

// ������� ��������� � ��������� �����. ������ ���������� ����� ��������� ��������� ������.
void Ribbon::Draw()
{
	float dx = CAMERA_OFF_X - CAMERA_OLD_OFF_X;		// �������� ������.
	float dy = CAMERA_OFF_Y - CAMERA_OLD_OFF_Y;
	this->aabb.p.x -= dx*speed_factor;				// �������� �����.
	this->aabb.p.y -= dy*speed_factor_y;

	if (use_bounds && (br < CAMERA_LEFT || bl > CAMERA_RIGHT))
		return;			// ����� �� �������, ������ ��������.

	float left = this->aabb.Left();
	float right = this->aabb.Right();

	Sprite* s = this->sprite;

	// ���� ����� �������� ������ �� �����������, ������� ������� ���������� Y.
	if (y_attached)
	{
		this->aabb.p.y  = CAMERA_Y;
	}
	float y = this->aabb.p.y - this->aabb.H;
	float y1 = s->frameY;						// ������ � ��������.
	float y2 = y1 + s->frameHeight;				// ������ � ��������.

	// ������ ���������� X
	if ((left <= CAMERA_LEFT) &&
		(right >= CAMERA_RIGHT))
	{
		// ����� ��������� ���������� �� ������. ����� ���� ���������.
		float x = CAMERA_LEFT;						// ���� �������� �� ������.
		float x1 = s->frameX + (CAMERA_LEFT - left);// ������ � ��������.
		float x2 = x1 + cfg.scr_width;				// ������ � ��������.

		if (use_bounds)
		{
			// ���������� �� ������
			if (bl > x)
			{
				// ����� ����� �������
				x1 += bl - x;
				x = bl;
			}

			if (br < CAMERA_RIGHT)
			{
				// ����� ������ �������
				x2 -= CAMERA_RIGHT - br;
			}
		}

		RenderSprite(x, y, s->z, x1, y1, x2, y2, s->tex, s->IsMirrored(), s->color);
	}
	else
	{
		// ����� �� ��������� ����� �� ������. ���������� ���������� "�����" ������� � "������".
		bool rend1 = true;			// �������� �����
		bool rend2 = true;			// �������� ������

		float x1 = CAMERA_LEFT;
		float x11 = s->frameX + CAMERA_LEFT;
		float x12 = (float)(s->frameX + s->frameWidth);

		float x2 = 0;
		float x21 = s->frameX;

		if (dx >= 0)
		{
			// ������ �������� �����
			// ����� ������� ������� ������ ����� �� ������� �����, �
			// ����� ������� ������ �� ������.
			if (left > CAMERA_LEFT)
			{
				// �������� ������ ����� �� ��� ������.
				aabb.p.x -= 2*aabb.W;
				left = aabb.Left();
				right = aabb.Right();
			}

			x11 -= left;
			x2 = aabb.Right();
		}
		else
		{
			// ������ �������� ������
			// ����� ������� ������� ������ ����� �� ������, � ����� ������� ������ ��
			// ������� ������.
			if (right < CAMERA_RIGHT)
			{
				// �������� ������ ������ �� ��� ������.
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
			// ���������� �� ������.
			if (bl > x1 && bl < x2)
			{
				// ����� ������� �� ������.
				x11 += bl - x1;
				x1 = bl;
			}
			else if (bl > x2)
			{
				// ����� ������� �� ������, ����� �� ��������.
				x21 += bl - x2;
				x2 = bl;
				rend1 = false;
			}

			if (br < CAMERA_RIGHT && br > x2)
			{
				// ������ ������� �� ������.
				x22 -= (x22 - x21) - (br - x2);
			}
			else if (br < x2)
			{
				// ������ ������� �� ������, ������ �� ��������.
				x12 -= x2 - br;
				rend2 = false;
			}
		}

		if (rend1)
		{
			// ������ �����
			RenderSprite(x1, y, s->z, x11, y1, x12, y2, s->tex, s->IsMirrored(), s->color);
		}

		if (rend2)
		{
			// ������ ������
			RenderSprite(x2, y, s->z, x21, y1, x22, y2, s->tex, s->IsMirrored(), s->color);
		}
	}
}

void Ribbon::SetBounds(float x1, float y1, float x2, float y2)
{
	if (x1 != x2 && y1 != y2)
	{
		// ����� �������������� �������.
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

// ������� ��������� � ��������� ���� ����. ������ ���������� ����� ��������� ��������� ������.
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

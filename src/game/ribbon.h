#ifndef __RIBBON_H_
#define  __RIBBON_H_

#include "sprite.h"

class Ribbon
{
public:
	UINT id;

	Sprite* sprite;		// Спрайт ленты
	CAABB aabb;			// Положени спрайта ленты

	float speed_factor;	// Фактор скорости
	float speed_factor_y;

	bool use_bounds;	// Надо ли использовать границы

	bool y_attached;

	// Координаты сторон границ
	// left, right, top, bottom
	float bl, br, bt, bb;	

	Ribbon()
	{
		sprite = NULL;
		speed_factor = 0;
		speed_factor_y = 0;
		use_bounds = false;
		y_attached = false;
		bl = br = bt = bb = 0;
	}

	~Ribbon()
	{
		DELETESINGLE(sprite);
	}

	void SetBounds(float x, float y, float w, float h);

	void Draw();
};



void DrawRibbons();

void DeleteAllRibbons();

Ribbon* CreateRibbon(const char* proto_name, Vector2 coord, float speed_factor, float speed_factor_y);

Ribbon* GetRibbon(UINT num);

#endif // __RIBBON_H_
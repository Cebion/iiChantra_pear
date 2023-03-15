#ifndef __RENDERER_H_
#define __RENDERER_H_



#include "types.h"
#include "texture.h"
#include "../game/phys/phys_misc.h"
#include "../misc.h"

struct render_data
{
	GLuint			texture_id;			// Идентификатор текстуры OGL
	vertex3f_t*		vertices;			// Массив вершин
	coord2f_t*		coords;				// Массив координат текстур
	RGBAf*			colors;				// Массив цветов
	size_t			vert_allocated_size;		// Выделенная память под массив вершин
	size_t			coord_allocated_size;		// Выделенная память под массив кординат тектур
	size_t			colors_allocated_size;		// Выделенная память под массив цветов
	size_t			count;				// Количесвто элементов

	//render_data()
	//{
	//	texture_id = 0;
	//	vertices = NULL;
	//	coords = NULL;
	//	colors = NULL;
	//	vert_allocated_size = 0;
	//	coord_allocated_size = 0;
	//	colors_allocated_size = 0;
	//	count = 0;
	//}
};


void r_ZeroRenderData(void);				// Обнулить отрисовку
void r_ClearRenderData(void);				// Очистить отрисовку (освободить память)
//void r_RenderData(render_data* p_data);		// Отрисовать список отрисовки
void r_RenderAll(void);						// Отрисовать все


void RenderSprite(float x, float y, float z, float x1, float y1, float x2, float y2, const Texture* tex, BOOL mirrorX, const RGBAf& color);
void RenderSprite(float x, float y, float z, const coord2f_t* frame_size, const coord2f_t* tex_coords, const Texture* tex, const RGBAf& color);
void RenderSprite(float x, float y, float z, const CAABB& area, const FrameInfo* frame, const Texture* tex, const RGBAf& color, bool mirrorX, RenderSpriteMethod method);
void RenderSpritePart(float x, float y, float z, const coord2f_t* full_frame_size, const coord2f_t* part_frame_size, const coord2f_t* tex_coords, bool mirrorX, const Texture* tex, const RGBAf& color);
void RenderSpriteCyclic(const Vector2& coord, float z, const Vector2& edge, const Vector2& bs, const FrameInfo* frame, const Texture* tex, const RGBAf& color, bool mirrorX, bool repeat_x, bool repeat_y);
void RenderBox(float x, float y, float z, float w, float h, const RGBAf& color);
void RenderLine(float x1, float y1, float x2, float y2, float z, const RGBAf& color);
void RenderEllipse(float x, float y, float w, float h, float z, const RGBAf& color, UINT id, UINT segments);

#endif // __RENDERER_H_

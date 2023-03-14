#include "StdAfx.h"

#include "texture.h"
#include "renderer.h"


#include "../misc.h"

const size_t BLOCK_INIT_SIZE	= 1024;		// bytes
const size_t BLOCK_SIZE			= 512;		// bytes

#ifdef DEBUG_PRINT
extern float DBG_PRINT_VAL1;
#endif // DEBUG_PRINT

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

render_data* sprites_render_data = NULL;	// Массив списков отрисовки спрайтов
size_t sprites_render_data_size = 0;			// Количество списков отрисовки спрайтов в массиве

render_data* lines_render_data = NULL;		// Массив списков отрисовки линий
size_t lines_render_data_size = 0;			// Количество списков отрисовки линий в массиве

render_data* filed_shapes_render_data = NULL;		// Массив списков отрисовки линий
size_t filed_shapes_render_data_size = 0;			// Количество списков отрисовки линий в массиве

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Такой id текстуры уже есть в списке отрисовки?
render_data* _id_exists(render_data* p_data, size_t data_size, GLuint id)
{
	if(data_size == 0)
		return NULL;

	assert(p_data != NULL);

	for(size_t i = 0; i < data_size; i++)
	{
		if(p_data[i].texture_id == id)
			return &p_data[i];
	}

	return NULL;

}


// Освободить память из-под массивов в render_data
void _free_render_data(render_data* p_data)
{
	assert(p_data != NULL);

	if(p_data->coords)
		free(p_data->coords);

	if(p_data->vertices)
		free(p_data->vertices);

	if(p_data->colors)
		free(p_data->colors);

	p_data->vert_allocated_size = 0;
	p_data->coord_allocated_size = 0;
	p_data->colors_allocated_size = 0;
	p_data->count = 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Производит выделение памяти или расширение массива для r_GetMem
template<typename T>
void r_ReallocArray(size_t add_size, size_t count, size_t& allocated_size, T*& arr)
{
	size_t new_size = count*sizeof(T) + add_size;
	// Если нам не хватает выделенной памяти для добавления
	if(new_size > allocated_size)
	{
		// Разница между необходимым и выделенным объемом
		size_t d_size = new_size - allocated_size;

		// Выделение производится блоками по BLOCK_SIZE байт
		// Расчитываем необходимое количество блоков (как минимум 1)
		size_t blocks_to_add = (d_size / BLOCK_SIZE) + 1;

		// Новый объем выделеной памяти
		allocated_size += (blocks_to_add * BLOCK_SIZE);

		arr = (T*)realloc(arr, allocated_size);
	}
	// TODO: А что если realloc не сработает?!

}



// Выделить память под новый список отрисовки или его элементы
// Возвращает укзатаель на список, в котором можно заполнять память начиная с 
// позиции count. 
render_data* r_GetMem(render_data** render_mas, size_t* mas_size, GLuint id, size_t vert_size, size_t coord_size, size_t color_size)
{
	// Существующий список
	render_data* x_data = _id_exists(*render_mas, *mas_size, id);

	if(x_data == NULL)
	{
		// Новый список
		*render_mas = (render_data*)realloc(*render_mas, (*mas_size + 1)*sizeof(render_data));
		// TODO: А что если realloc не сработает?!
		// Списки отрисовки, возможн лучше хранить свзанным списком. Тогда отдельные списки легче будет изменять.
		// И новый список добавится с большей вероятностью. А может я и чушь порю.
		memset(&((*render_mas)[*mas_size]), 0, sizeof(render_data));
		x_data = &((*render_mas)[*mas_size]);
		x_data->texture_id = id;
		*mas_size += 1;
	}

	{
		// Расширяем имеющийся список
		r_ReallocArray(vert_size, x_data->count, x_data->vert_allocated_size, x_data->vertices);
		r_ReallocArray(color_size, x_data->count, x_data->colors_allocated_size, x_data->colors);
		if (id)
			r_ReallocArray(coord_size, x_data->count, x_data->coord_allocated_size, x_data->coords);
	}

	return x_data;
}


// Обнулить отрисовку, не освобождая память
void r_ZeroRenderData(void)
{
	for(size_t i = 0; i < sprites_render_data_size; i++)
	{
		sprites_render_data[i].count = 0;
	}

	for(size_t i = 0; i < lines_render_data_size; i++)
	{
		lines_render_data[i].count = 0;
	}

	for(size_t i = 0; i < filed_shapes_render_data_size; i++)
	{
		filed_shapes_render_data[i].count = 0;
	}


#ifdef _DEBUG_RENDER
	LogToFile("Render data was zeroized");
#endif // _DEBUG_RENDER
}

// Очистить отрисовку (освободить память)
void r_ClearRenderData(void)
{
	for(size_t i = 0; i < sprites_render_data_size; i++)
	{
		_free_render_data(&sprites_render_data[i]);
	}
	free(sprites_render_data);
	sprites_render_data = NULL;
	sprites_render_data_size = 0;

	for(size_t i = 0; i < lines_render_data_size; i++)
	{
		_free_render_data(&lines_render_data[i]);
	}
	free(lines_render_data);
	lines_render_data = NULL;
	lines_render_data_size = 0;

	for(size_t i = 0; i < filed_shapes_render_data_size; i++)
	{
		_free_render_data(&filed_shapes_render_data[i]);
	}
	free(filed_shapes_render_data);
	filed_shapes_render_data = NULL;
	filed_shapes_render_data_size = 0;
}

void RenderSprites()
{
	if(!sprites_render_data || !sprites_render_data_size)
		return;

	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	render_data* null_tex = NULL;

	for(size_t i = 0; i < sprites_render_data_size; i++)
	{
		render_data* rdata = &sprites_render_data[i];

		if (rdata->texture_id)
		{
			glBindTexture(GL_TEXTURE_2D, rdata->texture_id);
			glTexCoordPointer(2, GL_FLOAT, 0, rdata->coords);
			glVertexPointer(3, GL_FLOAT, 0, rdata->vertices);
			glColorPointer(4, GL_FLOAT, 0, rdata->colors);
			glDrawArrays(GL_QUADS, 0, rdata->count);
		}
		else
		{
			null_tex = rdata;
		}
	}

	// TODO: Это костыль, чтобы прозрачный фейд рисовался позже всего остального и правильно работал
	if (null_tex)
	{
		// Если текстуры нет, то будет нарисован просто закрашенный прямоугольник
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);

		glVertexPointer(3, GL_FLOAT, 0, null_tex->vertices);
		glColorPointer(4, GL_FLOAT, 0, null_tex->colors);
		glDrawArrays(GL_QUADS, 0, null_tex->count);
	}


	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

}

void RenderLines()
{
	if(!lines_render_data || !lines_render_data_size)
		return;

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glDisable(GL_TEXTURE_2D);

	for(size_t i = 0; i < lines_render_data_size; i++)
	{
		render_data* rdata = &lines_render_data[i];

		//glPushAttrib(GL_CURRENT_BIT);
		//glColor3f(1.0f, 0.5f, 0.25f);


		glVertexPointer(3, GL_FLOAT, 0, rdata->vertices);
		glColorPointer(4, GL_FLOAT, 0, rdata->colors);
		glDrawArrays(GL_LINES, 0, rdata->count);

		//glPopAttrib();
	}

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void RenderFilledShapes()
{
	if(!filed_shapes_render_data || !filed_shapes_render_data_size)
		return;

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glDisable(GL_TEXTURE_2D);

	for(size_t i = 0; i < filed_shapes_render_data_size; i++)
	{
		render_data* rdata = &filed_shapes_render_data[i];

		glVertexPointer(3, GL_FLOAT, 0, rdata->vertices);
		glColorPointer(4, GL_FLOAT, 0, rdata->colors);
		glDrawArrays(GL_TRIANGLE_FAN, 0, rdata->count);
	}

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

// Отрисовать все
void r_RenderAll(void)
{
	RenderSprites();
	RenderLines();
	RenderFilledShapes();
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

void RenderSprite(float x, float y, float z, float x1, float y1, float x2, float y2, const Texture* tex, BOOL mirrorX, const RGBAf& color)
{
#ifdef DEBUG_PRINT
	DBG_PRINT_VAL1++;
#endif // DEBUG_PRINT

	// Прямоугольники - по 4 координаты.
	size_t count = 4;

	render_data* rd = r_GetMem(&sprites_render_data, &sprites_render_data_size, tex ? tex->tex : 0,
		count*sizeof(vertex3f_t), tex ? count*sizeof(coord2f_t) : 0, count*sizeof(RGBAf));
	ASSERT(rd && rd->colors && rd->vertices && rd->vert_allocated_size && rd->colors_allocated_size);

	count = rd->count;
	rd->count += 4;

	rd->vertices[count].z = rd->vertices[count + 1].z = rd->vertices[count + 2].z = rd->vertices[count + 3].z = z;

	float dx = x2-x1;
	float dy = y2-y1;

	rd->vertices[count+0].x = x ;
	rd->vertices[count+0].y = y;

	rd->vertices[count+1].x = x + dx;
	rd->vertices[count+1].y = y;

	rd->vertices[count+2].x = x + dx;
	rd->vertices[count+2].y = y + dy;

	rd->vertices[count+3].x = x;
	rd->vertices[count+3].y = y + dy;

	rd->colors[count+0] = rd->colors[count+1] = rd->colors[count+2] = rd->colors[count+3] = color;

	if(tex)
	{
		ASSERT(rd->coord_allocated_size && rd->coords && rd->texture_id == tex->tex);

		float CWidth = (float)tex->width;
		float CHeight = (float)tex->height;
		float cx1 = x1 / CWidth;
		float cx2 = x2 / CWidth;
		float cy1 = 1 - y1 / CHeight;
		float cy2 = 1 - y2 / CHeight;

		if(mirrorX)
		{
			rd->coords[count+0].x = cx2;
			rd->coords[count+0].y = cy1;

			rd->coords[count+1].x = cx1;
			rd->coords[count+1].y = cy1;

			rd->coords[count+2].x = cx1;
			rd->coords[count+2].y = cy2;

			rd->coords[count+3].x = cx2;
			rd->coords[count+3].y = cy2;
		}
		else
		{
			rd->coords[count+0].x = cx1;
			rd->coords[count+0].y = cy1;

			rd->coords[count+1].x = cx2;
			rd->coords[count+1].y = cy1;

			rd->coords[count+2].x = cx2;
			rd->coords[count+2].y = cy2;

			rd->coords[count+3].x = cx1;
			rd->coords[count+3].y = cy2;
		}
	}
	else
	{
		// Случай, если текстура не загружена.
		ASSERT(!rd->coords && !rd->coord_allocated_size);
	}

#ifdef DEBUG_DRAW_SPRITES_BORDERS
	RenderBox(x, y, z, x2-x1, y2-y1, RGBAf(DEBUG_SPRITES_BORDERS_COLOR));
#endif // DEBUG_DRAW_SPRITES_BORDERS

}

void RenderSprite(float x, float y, float z, const coord2f_t* frame_size, const coord2f_t* tex_coords, const Texture* tex, const RGBAf& color)
{
#ifdef DEBUG_PRINT
	DBG_PRINT_VAL1++;
#endif // DEBUG_PRINT

	// Прямоугольники - по 4 координаты.
	size_t count = 4;

	render_data* rd = r_GetMem(&sprites_render_data, &sprites_render_data_size, tex ? tex->tex : 0,
		count*sizeof(vertex3f_t), tex ? count*sizeof(coord2f_t) : 0, count*sizeof(RGBAf));
	ASSERT(rd && rd->colors && rd->vertices && rd->vert_allocated_size && rd->colors_allocated_size);

	count = rd->count;
	rd->count += 4;

	rd->vertices[count].z = rd->vertices[count + 1].z = rd->vertices[count + 2].z = rd->vertices[count + 3].z = z;

	rd->vertices[count].x = x;
	rd->vertices[count].y = y;

	rd->vertices[count+1].x = x + frame_size->x;
	rd->vertices[count+1].y = y;

	rd->vertices[count+2].x = x + frame_size->x;
	rd->vertices[count+2].y = y + frame_size->y;

	rd->vertices[count+3].x = x;
	rd->vertices[count+3].y = y + frame_size->y;

	rd->colors[count] = rd->colors[count+1] = rd->colors[count + 2] = rd->colors[count + 3] = color;

	if(tex)
	{
		ASSERT(rd->coord_allocated_size && rd->coords && rd->texture_id == tex->tex);

		memcpy(rd->coords+count, tex_coords, 4 * sizeof(coord2f_t));
	}
	else
	{
		// Случай, если текстура не загружена.
		ASSERT(!rd->coords && !rd->coord_allocated_size);
	}


}


void RenderSpritePart(float x, float y, float z, const coord2f_t* full_frame_size, const coord2f_t* part_frame_size, const coord2f_t* tex_coords, bool mirrorX, const Texture* tex, const RGBAf& color)
{
#ifdef DEBUG_PRINT
	DBG_PRINT_VAL1++;
#endif // DEBUG_PRINT
	// Прямоугольники - по 4 координаты.
	size_t count = 4;

	render_data* rd = r_GetMem(&sprites_render_data, &sprites_render_data_size, tex ? tex->tex : 0,
		count*sizeof(vertex3f_t), tex ? count*sizeof(coord2f_t) : 0, count*sizeof(RGBAf));
	ASSERT(rd && rd->colors && rd->vertices && rd->vert_allocated_size && rd->colors_allocated_size);

	count = rd->count;
	rd->count += 4;


	for (size_t i = count; i < count+4; i++)
	{
		rd->vertices[i].z = z;
		memcpy(rd->colors + i, &color, sizeof(RGBAf));
	}

	rd->vertices[count+0].x = x;
	rd->vertices[count+0].y = y;
	rd->vertices[count+1].x = x + part_frame_size->x;
	rd->vertices[count+1].y = y;
	rd->vertices[count+2].x = x + part_frame_size->x;
	rd->vertices[count+2].y = y + part_frame_size->y;
	rd->vertices[count+3].y = y + part_frame_size->y;
	rd->vertices[count+3].x = x;

	if(tex)
	{
		ASSERT(rd->coord_allocated_size && rd->coords && rd->texture_id == tex->tex);

		float cx1;
		float cx2;
		float cy1;
		float cy2;
		if (mirrorX)
		{
			cx1 = tex_coords[1].x; 
			cx2 = tex_coords[0].x;
		}
		else
		{
			cx1 = tex_coords[0].x; 
			cx2 = tex_coords[1].x;
		}
		cy1 = tex_coords[0].y;
		cy2 = tex_coords[3].y;

		cx2 = cx1 + part_frame_size->x * (cx2 - cx1) / full_frame_size->x;
		cy2 = cy1 + part_frame_size->y * (cy2 - cy1) / full_frame_size->y;

		if(mirrorX)
		{
			rd->coords[count+0].x = cx2;
			rd->coords[count+0].y = cy1;

			rd->coords[count+1].x = cx1;
			rd->coords[count+1].y = cy1;

			rd->coords[count+2].x = cx1;
			rd->coords[count+2].y = cy2;

			rd->coords[count+3].x = cx2;
			rd->coords[count+3].y = cy2;
		}
		else
		{
			rd->coords[count+0].x = cx1;
			rd->coords[count+0].y = cy1;

			rd->coords[count+1].x = cx2;
			rd->coords[count+1].y = cy1;

			rd->coords[count+2].x = cx2;
			rd->coords[count+2].y = cy2;

			rd->coords[count+3].x = cx1;
			rd->coords[count+3].y = cy2;
		}
	}
	else
	{
		// Случай, если текстура не загружена.
		ASSERT(!rd->coords && !rd->coord_allocated_size);
	}




#ifdef DEBUG_DRAW_SPRITES_BORDERS
	RenderBox(x, y, z, part_frame_size->x, part_frame_size->y, RGBAf(DEBUG_SPRITES_BORDERS_COLOR));
	if (mirrorX)
	{
		RenderBox(x - (full_frame_size->x - part_frame_size->x), y - (full_frame_size->y - part_frame_size->y), z, part_frame_size->x, part_frame_size->y, RGBAf(DEBUG_SPRITES_BORDERS_COLOR));
	}
	else
	{
		RenderBox(x, y, z, full_frame_size->x, full_frame_size->y, RGBAf(DEBUG_SPRITES_BORDERS_COLOR));
	}
#endif // DEBUG_DRAW_SPRITES_BORDERS
}


void RenderSpriteCyclic(const Vector2& coord, float z, const Vector2& edge, const Vector2& bs, 
	const FrameInfo* frame, const Texture* tex, const RGBAf& color, bool mirrorX,
	bool repeat_x, bool repeat_y)
{
#ifdef DEBUG_PRINT
	DBG_PRINT_VAL1++;
#endif // DEBUG_PRINT

	if (!tex)
		return;

	// Прямоугольники - по 4 координаты.
	size_t count = 4, old_count, new_max_count;


	float x_count = 1, y_count = 1;
	if (repeat_x)
		x_count = (edge.x - coord.x + bs.x) / frame->size.x;
	if (repeat_y)
		y_count = (edge.y - coord.y + bs.y) / frame->size.y;

	ASSERT(x_count >= 0);
	ASSERT(y_count >= 0);

	count *= (size_t)ceil(x_count) * (size_t)ceil(y_count);

	if (count <= 0)
		return;

	render_data* rd = r_GetMem(&sprites_render_data, &sprites_render_data_size, tex ? tex->tex : 0,
		count*sizeof(vertex3f_t), tex ? count*sizeof(coord2f_t) : 0, count*sizeof(RGBAf));
	ASSERT(rd && rd->colors && rd->vertices && rd->vert_allocated_size && rd->colors_allocated_size);

	old_count = rd->count;
	rd->count += count;
	new_max_count = rd->count;
	count = old_count;

	float CWidth = (float)tex->width;
	float CHeight = (float)tex->height;

	float x1, x2, y1, y2;
	float rx, ry, rw, rh;
	bool first_x = true;
	bool first_y = true;
	for ( ry = coord.y; ry < edge.y; ry += frame->size.y )
	{
		first_x = true;
		for ( rx = coord.x; rx < edge.x; rx += frame->size.x )
		{
			x1 = frame->coord->x * CWidth;
			y1 = (1 - frame->coord->y) * CHeight;
			if ( first_x )
			{
				x1 += bs.x;
				x2 = x1 + min( frame->size.x - bs.x, edge.x - rx );
			}
			else
				x2 = x1 + min( frame->size.x, edge.x - rx );
			if ( first_y )
			{
				y1 += bs.y;
				y2 = y1 + min( frame->size.y - bs.y, edge.y - ry );
			}
			else
				y2 = y1 + min( frame->size.y, edge.y - ry );

			rw = x2 - x1;
			rh = y2 - y1;

			rd->vertices[count].x = rx;
			rd->vertices[count].y = ry;

			rd->vertices[count+1].x = rx + rw;
			rd->vertices[count+1].y = ry;

			rd->vertices[count+2].x = rx + rw;
			rd->vertices[count+2].y = ry + rh;

			rd->vertices[count+3].x = rx;
			rd->vertices[count+3].y = ry + rh;

			rd->vertices[count].z = rd->vertices[count + 1].z = rd->vertices[count + 2].z = rd->vertices[count + 3].z = z;

			rd->colors[count] = rd->colors[count+1] = rd->colors[count + 2] = rd->colors[count + 3] = color;

			
			float cx1 = x1 / CWidth;
			float cx2 = x2 / CWidth;
			float cy1 = 1 - y1 / CHeight;
			float cy2 = 1 - y2 / CHeight;

			if(mirrorX)
			{
				rd->coords[count+0].x = cx2;
				rd->coords[count+0].y = cy1;

				rd->coords[count+1].x = cx1;
				rd->coords[count+1].y = cy1;

				rd->coords[count+2].x = cx1;
				rd->coords[count+2].y = cy2;

				rd->coords[count+3].x = cx2;
				rd->coords[count+3].y = cy2;
			}
			else
			{
				rd->coords[count+0].x = cx1;
				rd->coords[count+0].y = cy1;

				rd->coords[count+1].x = cx2;
				rd->coords[count+1].y = cy1;

				rd->coords[count+2].x = cx2;
				rd->coords[count+2].y = cy2;

				rd->coords[count+3].x = cx1;
				rd->coords[count+3].y = cy2;
			}

			count += 4;

			if ( first_x )
			{
				rx -= bs.x; // На одно начальное смещение меньше.
				first_x = false;
			}
			if ( !repeat_x ) break;
		}
		if ( first_y )
		{
			ry -= bs.y; // На одно начальное смещение меньше.
			first_y = false;
		}
		if ( !repeat_y ) break;
	}

	ASSERT(count == new_max_count);

}


// method = rsmStandart: area не используется
// method = rsmStretch: area - область, на которую растягиваем
// method = rsmCrop: area - область, по которой вырезаем
// method = rsmRepeat*: area - область, по которой размножаем
void RenderSprite(float x, float y, float z, const CAABB& area, 
	const FrameInfo* frame, const Texture* tex, const RGBAf& color, bool mirrorX,
	RenderSpriteMethod method)
{
	bool repeat_x = false;
	bool repeat_y = false;
	switch(method)
	{
	case rsmStretch:
		{
			coord2f_t size;
			size.x = area.W * 2;
			size.y = area.H * 2;
			RenderSprite(x, y, z, &size, mirrorX ? frame->coord : frame->coord, tex, color);
		}
		break;
	case rsmCrop:
		{
			coord2f_t crop;
			crop.x = min(frame->size.x, area.W*2);
			crop.y = min(frame->size.y, area.H*2);
			//RenderSprite(x, y, z, &crop, mirrorX ? frame->coord : frame->coord, tex, color);
			RenderSpritePart(x, y, z, &frame->size, &crop, mirrorX ? frame->coord : frame->coord, mirrorX, tex, color);
		}
		break;
	case rsmRepeatX:
		repeat_x = true;
		goto repeat;
		break;
	case rsmRepeatY:
		repeat_y = true;
		goto repeat;
		break;
	case rsmRepeatXY:
		repeat_x = true;
		repeat_y = true;
		goto repeat;
		break;
	case rsmStandart:
	default:
		RenderSprite(x, y, z, &frame->size, mirrorX ? frame->coordMir : frame->coord, tex, color);
		break;
	}

	return;

repeat:

	Vector2 coord = Vector2(x, y);
	Vector2 bs = Vector2( max(area.Left() - coord.x - frame->size.x, 0.0f), max(area.Top() - coord.y - frame->size.y, 0.0f) );
	if ( repeat_x ) coord.x = area.Left();
	if ( repeat_y ) coord.y = area.Top();
	if ( !repeat_x && coord.x < area.Left() )
	{
		bs.x += area.Left() - coord.x;
		coord.x = area.Left();
	}

	Vector2 edge = Vector2( area.Right(), area.Bottom() );

	RenderSpriteCyclic(coord, z, edge, bs, frame, tex, color, mirrorX, repeat_x, repeat_y);

	return;
}


void RenderBox(float x, float y, float z, float w, float h, const RGBAf& color)
{
#ifdef DEBUG_PRINT
	DBG_PRINT_VAL1++;
#endif // DEBUG_PRINT

	// Прямоугольники - по 8 координаты.
	size_t count = 8;

	render_data* rd = r_GetMem(&lines_render_data, &lines_render_data_size, 0,
		count*sizeof(vertex3f_t), 0, count*sizeof(RGBAf));
	ASSERT(rd && rd->colors && rd->vertices && rd->vert_allocated_size && rd->colors_allocated_size);

	count = rd->count;
	rd->count += 8;

	ASSERT(!rd->coords && !rd->coord_allocated_size && !rd->texture_id);


	rd->vertices[count+0].z = rd->vertices[count+1].z = rd->vertices[count+2].z = rd->vertices[count+3].z =
		rd->vertices[count+4].z = rd->vertices[count+5].z = rd->vertices[count+6].z = rd->vertices[count+7].z = z;

	rd->colors[count+0] = rd->colors[count+1] = rd->colors[count+2] = rd->colors[count+3] =
		rd->colors[count+4] = rd->colors[count+5] = rd->colors[count+6] = rd->colors[count+7] = color;

	// 1 - - 2
	//glVertex2i(_x, y);
	rd->vertices[count+0].x = x;
	rd->vertices[count+0].y = y;
	//glVertex2i(x + w, y);
	rd->vertices[count+1].x = x + w;
	rd->vertices[count+1].y = y;

	// 2 - - 3
	//glVertex2i(x + w, y);
	rd->vertices[count+2].x = x + w;
	rd->vertices[count+2].y = y;
	//glVertex2i(x + w, y + h);
	rd->vertices[count+3].x = x + w;
	rd->vertices[count+3].y = y + h;

	// 3 - - 4
	//glVertex2i(x + w, y + h);
	rd->vertices[count+4].x = x + w;
	rd->vertices[count+4].y = y + h;
	//glVertex2i(x, y + h);
	rd->vertices[count+5].x = x;
	rd->vertices[count+5].y = y + h;

	// 4 - - 1
	//glVertex2i(_x, y + h);
	rd->vertices[count+6].x = x;
	rd->vertices[count+6].y = y + h;
	//glVertex2i(x, y);
	rd->vertices[count+7].x = x;
	rd->vertices[count+7].y = y;

}

void RenderLine(float x1, float y1, float x2, float y2, float z, const RGBAf& color)
{
#ifdef DEBUG_PRINT
	DBG_PRINT_VAL1++;
#endif // DEBUG_PRINT

	// Линия - 2 координаты.
	size_t count = 2;

	render_data* rd = r_GetMem(&lines_render_data, &lines_render_data_size, 0,
		count*sizeof(vertex3f_t), 0, count*sizeof(RGBAf));
	ASSERT(rd && rd->colors && rd->vertices && rd->vert_allocated_size && rd->colors_allocated_size);

	count = rd->count;
	rd->count += 2;

	ASSERT(!rd->coords && !rd->coord_allocated_size && !rd->texture_id);

	rd->vertices[count+0].z = rd->vertices[count+1].z = z;

	rd->colors[count+0] = rd->colors[count+1] = color;

	// 1 - - 2
	//glVertex2i(_x, y);
	rd->vertices[count+0].x = x1;
	rd->vertices[count+0].y = y1;
	//glVertex2i(x + w, y);
	rd->vertices[count+1].x = x2;
	rd->vertices[count+1].y = y2;

}


void RenderEllipse(float x, float y, float w, float h, float z, const RGBAf& color, UINT id, UINT segments)
{
#ifdef DEBUG_PRINT
	DBG_PRINT_VAL1++;
#endif // DEBUG_PRINT

	render_data* rd = r_GetMem(&filed_shapes_render_data, &filed_shapes_render_data_size, id,
		segments*sizeof(vertex3f_t), 0, segments*sizeof(RGBAf));
	ASSERT(rd && rd->colors && rd->vertices && rd->vert_allocated_size && rd->colors_allocated_size);

	// Тут id не для текстуры, а чтобы у разных объектов тени не смыкались.
	rd->texture_id = id;


	rd->coords = NULL;
	rd->coord_allocated_size = 0;

	size_t count = rd->count;
	for (GLfloat i = 0; i < 2*M_PI && count < rd->count + segments; i+=(2*(GLfloat)M_PI / segments), count++)
	{
		rd->vertices[count].x = x + cos(i) * w;
		rd->vertices[count].y = y + sin(i) * h;
		rd->vertices[count].z = z;
		rd->colors[count] = color;
	}
	rd->count = count;

}


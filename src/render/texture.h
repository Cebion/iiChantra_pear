#ifndef __TEXTURE_H_
#define __TEXTURE_H_

#include "types.h"

#include "../resource.h"

struct FrameInfo{
	coord2f_t size;
	coord2f_t offset;
	coord2f_t* coord;
	coord2f_t* coordMir;
	

	FrameInfo()
	{ 
		//size = new coord2f_t;
		//offset = new coord2f_t;
		coord = new coord2f_t[4];
		coordMir = new coord2f_t[4];
	}

	~FrameInfo()
	{
		//DELETESINGLE(size);
		//DELETESINGLE(offset);
		DELETEARRAY(coord);
		DELETEARRAY(coordMir);
	}
};

class Texture : public Resource
{
public:
	GLuint tex;
	UINT width;
	UINT height;
	
	UINT framesCount;
	FrameInfo* frame;
	UINT overlayCount;
	FrameInfo* overlay;

	virtual bool Load();
	virtual bool Recover();

	bool LoadTexFramesDescr();

	Texture(string file_name, string name) : Resource(file_name, name)
	{
		tex = 0;
		width = 0;
		height = 0;
		framesCount = 0;
		overlayCount = 0;
		frame = NULL;
		overlay = NULL;
	}

	~Texture()
	{
		DELETEARRAY(frame);
		DELETEARRAY(overlay);
		glDeleteTextures(1, &tex);
	}
};


#endif
#include "StdAfx.h"
#include <IL/il.h>
#include <assert.h>
#include "image.h"
#include "../misc.h"

CImageData::CImageData() : data(NULL), height(0), width(0), bpp(0)
{

}

CImageData::~CImageData()
{
	if (data != NULL)
		delete [] data;
}

bool  CImageData::MakeRGBA()
{
	if (data ==  NULL)
		return false;
	unsigned char *Rs, *Gs, *Bs, *Ap, *Rp, *Gp, *Bp;
	for(UINT i = 0;i < height*width; i++)
	{
		Rs = data + i*bpp;
		Gs = data + i*bpp + 1;
		Bs = data + i*bpp + 2;
		Rp = data + i*bpp ;
		Gp = data + i*bpp + 1;
		Bp = data + i*bpp + 2;
		Ap = data + i*bpp + 3;
		if ( *Rs == 157 && *Bs == 0 && *Gs == 236 )
			*Ap = 0;
		*Rp =*Rs;
		*Gp =*Gs;
		*Bp =*Bs;
	}
	return true;
}

bool CImageData::LoadFromFile( const char* filename )
{
	unsigned int ILID = ilGenImage();
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	ilEnable(IL_ORIGIN_SET);
	ilBindImage(ILID);
	if (!ilLoadImage(filename))
		return false;

	width = ilGetInteger(IL_IMAGE_WIDTH);
	height = ilGetInteger(IL_IMAGE_HEIGHT);
	UINT y_pad = 0;
	UINT x_pad = 0;
	if ( !(( width & (width-1)) == 0 ) || !( (height & (height-1)) == 0 )  )
	{
		x_pad = width;
		width--;
		width |= width >> 1;
		width |= width >> 2;
		width |= width >> 4;
		width |= width >> 8;
		width |= width >> 16;
		width++;
		x_pad = width - x_pad;
		y_pad = height;
		height--;
		height |= height >> 1;
		height |= height >> 2;
		height |= height >> 4;
		height |= height >> 8;
		height |= height >> 16;
		height++;
		y_pad = height - y_pad;
	}
	data = new unsigned char[width * height * 4];
	memset( data, 0, width * height * 4 );
	if ( x_pad > 0 || y_pad > 0 )
	{
		for ( UINT y = 0; y < height - y_pad; y++ )
			ilCopyPixels( 0, y, 0, width-x_pad, 1, 1, IL_RGBA, IL_UNSIGNED_BYTE, data + (y+y_pad)*width*4 );
	}
	else ilCopyPixels(0, 0, 0, width, height, 1, IL_RGBA, IL_UNSIGNED_BYTE, data);
	
	//bpp = ilGetInteger(IL_IMAGE_FORMAT);
	//bpp = ilGetInteger(IL_IMAGE_BPP);

// 	int il_error ;
// 	if (!ilSaveImage((Filename + "_ilmod.png").c_str()))
// 	{
// 		il_error = ilGetError();
// 	}
	//bpp = ilGetInteger(IL_IMAGE_BITS_PER_PIXEL)/8;
	bpp = 4;
	ilDeleteImage(ILID);
	return true;
}

CGLImageData::CGLImageData()
{
	TexID = width = height = bpp = 0;
	doCleanData = true;
	data = NULL;
}

CGLImageData::~CGLImageData()
{
// 	if(glIsTexture(TexID))
// 		glDeleteTextures(1, &TexID);
}

bool CGLImageData::MakeTexture()
{
	if (data == NULL)
		return false;
	if ((width&(width-1)) != 0)		//	Тут мы просто выходим, если ширина или высота  не является степенью двойки.
		return false;				//	Ultimate - это использовать NOT_POWER_OF_TWO екстеншон, если он доступен;
	if ((height&(height-1)) != 0)	//	Иначе - дописывать в память кусок прозрачного цвета, по размеру такой, чтобы
		return false;				//	Ширина и выстоа стали ближайшими степенями двойки. Но это потом. И это TODO.
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &TexID);
	glBindTexture(GL_TEXTURE_2D, TexID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	int MODE = (bpp == 3) ? GL_RGB : GL_RGBA;
	glTexImage2D(GL_TEXTURE_2D, 0, bpp, width, height, 0, MODE, GL_UNSIGNED_BYTE, data);
	return true;
}

bool CGLImageData::LoadTexture(const char *filename)
{
	if (!LoadFromFile(filename))
	{
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Can't load image->");
		return false;
	}
	if(!MakeRGBA())
	{
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Can't load texture.");
		return false;
	}
	if (!MakeTexture())
	{
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Can't load texture in video memory.");
		return false;
	}
	if (doCleanData)
	{
		DELETEARRAY(data);
	}
	return true;
}

GLuint CGLImageData::GetTexID()
{
	if (TexID == 0)
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "GLImageData. Trying to access TexID but it is 0");
	return TexID;
}




//! Takes a screenshot of the current OpenGL window.
// Взята из ilut_opengl.cpp
ILboolean GLScreenToIL()
{
	ILuint	ViewPort[4];

	glGetIntegerv(GL_VIEWPORT, (GLint*)ViewPort);

	if (!ilTexImage(ViewPort[2], ViewPort[3], 1, 3, IL_RGB, IL_UNSIGNED_BYTE, NULL))
		return IL_FALSE;  // Error already set.
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

	ILubyte* data = ilGetData();

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, ViewPort[2], ViewPort[3], GL_RGB, GL_UNSIGNED_BYTE, data);

	return IL_TRUE;
}


bool CGLImageData::SaveScreenshot(const char *filename)
{
	unsigned int ILID = ilGenImage();
	ilBindImage(ILID);

	// Чтобы не тянуть еще и ILUT рази одной функции ilutGLScreen, я ее взял как GLScreenToIL.
	//if (ilutGLScreen())
	if ( GLScreenToIL() )
	{
#ifdef BMP_SCREENSHOTS
		ilSave(IL_BMP, filename);
#else
		ilSave(IL_PNG, filename);
#endif
	}
	ilDeleteImage(ILID);

	return true;
}

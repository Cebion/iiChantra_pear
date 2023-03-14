#ifndef _IMAGE_H_
#define _IMAGE_H_

class CImageData
{
public:
	unsigned char* data;
	unsigned int height;
	unsigned int width;
	unsigned char bpp;

	CImageData();
	~CImageData();
	/**
	*	Checks for color key within whole image
	*/
	bool	MakeRGBA();
	bool LoadFromFile(const char* filename);
};

class CGLImageData : public CImageData
{
public:
	bool			doCleanData;
	CGLImageData();
	~CGLImageData();
	bool			MakeTexture();
	bool			LoadTexture(const char *filename);
	bool			SaveScreenshot(const char *filename); // ŸŸ»
	virtual GLuint	GetTexID();
protected:
	GLuint			TexID;
};

#endif // _IMAGE_H_

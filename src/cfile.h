#ifndef __CFILE_H_
#define __CFILE_H_


#define CFILE_MAX_STRING_LENGTH	1024



class CFile
{
public:
	enum EOpenMode
	{
		OPEN_MODE_READ,
		OPEN_MODE_WRITE
	};

	enum ESeekOrigin
	{
		SEEK_ORIGIN_BEGINNING,
		SEEK_ORIGIN_CURRENT,
		SEEK_ORIGIN_END
	};

	CFile();
	CFile(const string &AFileName, EOpenMode Mode);

	~CFile()
	{
		Close();
	}

	bool Open(const string &filename, int mode);
	bool Close();
	
	bool ReadByte(pbyte buffer);
	bool WriteByte(pbyte buffer);
	bool WriteByte(byte buffer);

	bool Read(void* buffer, size_t nbytes);
	bool Write(const void* buffer, size_t nbytes);

	bool ReadString(char* buffer);
	bool WriteString(const char* buffer);

	bool ReadString(string &buffer);
	bool WriteString(const string &buffer);

	bool WriteLine(string& buffer);
	bool ReadLine(char* &data);

	size_t Size();
	/**
	*	Reads characters until reaching 0x00
	*	data - is a pointer to a char passed by
	*	link(&) i.e. the function changes the value
	*	of data. If data is pointing to something,
	*	funtion realeses the memory using delete []
	*	then allocates new meory using new.
	*/
	// ¬верху бесполезный комментарий на ломаном английском. ¬прочем такой же бесполезный как и этот.
	
	bool Eof();
	bool Seek(unsigned int offset, ESeekOrigin origin);

private:
	FILE *file;
	string filename;
};


#endif // __CFILE_H_
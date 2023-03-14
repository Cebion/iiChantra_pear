#include "StdAfx.h"

#include "misc.h"
#include "cfile.h"

#include <sys/stat.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CFile::CFile()
{
	file = NULL;
}

CFile::CFile(const string &filename, EOpenMode mode)
{
	file = NULL;
	Open(filename, mode);
}

//////////////////////////////////////////////////////////////////////////

bool CFile::Open(const string &filename, int mode)
{
	if (filename.empty())
	{
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Can't open file. Invalid filename");
		return false;
	}

	if (file != NULL)
	{
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Can't open file %s: another file is already opened.", filename.c_str());
		return false;
	}


	switch(mode)
	{
	case OPEN_MODE_READ:
		{
			file = fopen(filename.c_str(), "rb");
			break;
		}
	case OPEN_MODE_WRITE:
		{
			file = fopen(filename.c_str(), "wb");
			break;
		}
	default:
		{
			sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Can't open file %s: invalid mode.", filename.c_str());
			return false;
		}
	}

	if ( file == NULL )
	{
		sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "Can't open file %s.", filename.c_str());
		return false;
	}

	return true;
}

bool CFile::Close()
{
	if (file == NULL)
		return false;
	fclose(file);
	file = NULL;
	return true;
}

//////////////////////////////////////////////////////////////////////////

bool CFile::ReadByte(pbyte buffer)
{
	return Read(buffer, 1);
}

bool CFile::WriteByte(pbyte buffer)
{
	return Write(buffer, 1);
}

bool CFile::WriteByte(byte buffer)
{
	return WriteByte(&buffer);
}

//////////////////////////////////////////////////////////////////////////

bool CFile::Read( void* buffer, size_t nbytes )
{
	if (buffer == NULL)
		return false;
	if (file == NULL)
		return false;
	if (nbytes == 0)
		return false;
	if (fread(buffer, 1, nbytes, file) != nbytes)
	{
		if (!Eof())
			sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "FILE IO Error. Can't read data.");
		return false;
	}
	return true;
}

bool CFile::Write( const void* buffer, size_t nbytes )
{
	if (buffer == NULL)
		return false;
	if (file == NULL)
		return false;
	if (nbytes == 0)
		return false;
	if (fwrite(buffer, 1, nbytes, file) != nbytes)
	{
		if (!Eof())
			sLog(DEFAULT_LOG_NAME, LOG_WARNING_EV, "FILE IO Error. Can't write data.");
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////

// Внимание: не следит за размерами буфера
bool CFile::ReadString(char* buffer)
{
	if (buffer == NULL)
		return false;
	if (file == NULL)
		return false;

	byte b;
	int i = 0;

	if (!Read(&b,1))
		return false;

	while (b != 0)
	{
		buffer[i] = b;
		i++;
		if (!Read(&b,1))
			return false;
	}
	return true;
}

bool CFile::WriteString(const char* buffer)
{
	if (buffer == NULL)
		return false;
	if (file == NULL)
		return false;
	byte b = 0;
	size_t buflen = strlen(buffer);
	for (size_t i = 0;i < buflen; i++)
	{
		Write(&buffer[i], 1);
	}
	Write(&b, 1);
	return true;
}

bool CFile::ReadString(string &buffer)
{
	if (file == NULL)
		return false;
	byte b;
	buffer = "";
	int i = 0;
	if (!Read(&b, 1))
		return false;
	while (b != 0)
	{
		buffer += b;
		i++;
		if (!Read(&b, 1))
			return false;
	}
	return true;
}

bool CFile::WriteString(const string &buffer)
{
	if (file == NULL)
		return false;
	byte b = 0;
	size_t buflen = buffer.length();
	for (size_t i = 0;i < buflen; i++)
	{
		Write(&buffer[i], 1);
	}
	Write(&b, 1);
	return true;
}

bool CFile::WriteLine(string &buffer)
{
	if (file==NULL)
		return false;

	byte b;
	Write((char*)buffer.data(), (size_t)buffer.length());
#ifdef _WIN32
	b = '\r';	// 0x0d = 13 - CR
	Write(&b,1);
#endif
	b = 'n';	// 0x0a = 10 - LF
	Write(&b,1);

	return true;
}

bool CFile::ReadLine(char* &data)
{
	if (file == NULL)
		return false;

	char *buffer = new char[CFILE_MAX_STRING_LENGTH];
	int count = -1;

	do
	{
		if (fread(buffer+count+1, 1, 1, file) != 1)
			break;
		count++;
	} while ((buffer[count] != 0x00) && (count < CFILE_MAX_STRING_LENGTH));
	if (data != NULL)
		delete [] data;
	data = new char[count+1];
	for(int i=0; i<count; i++)
		data[i] = buffer[i];
	data[count] = 0x00;
	delete [] buffer;
	return true;
}

size_t CFile::Size()
{
	struct stat FileStat;

	if(stat(filename.c_str(), &FileStat))
	{
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Can't get size of %s.", filename.c_str());
		return 0;
	}

	return FileStat.st_size;
}

bool CFile::Eof()
{
	return !!feof(file);
}

bool CFile::Seek( unsigned int offset, ESeekOrigin origin )
{
	if (file == NULL)
		return false;

	int origin_const;

	switch (origin)
	{
	default:
	case SEEK_ORIGIN_BEGINNING:
		origin_const = SEEK_SET;
		break;
	case SEEK_ORIGIN_CURRENT:
		origin_const = SEEK_CUR;
		break;
	case SEEK_ORIGIN_END:
		origin_const = SEEK_END;
		break;
	}

	return (fseek(file, offset, origin_const) == 0);
}

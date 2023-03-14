#include "StdAfx.h"
#include "misc.h"

#ifdef WIN32
	#include <direct.h>
	#include <io.h>
	#include "dirent/dirent.h"
#else
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <dirent.h>
#endif // WIN32

#include "config.h"
#include <unistd.h>


extern config cfg;

char path_log[MAX_PATH];
char path_config[MAX_PATH];
char path_app[MAX_PATH];
char path_textures[MAX_PATH];
char path_fonts[MAX_PATH];
char path_protos[MAX_PATH];
char path_scripts[MAX_PATH];
char path_levels[MAX_PATH];
char path_sounds[MAX_PATH];
char path_screenshots[MAX_PATH];
char path_saves[MAX_PATH];

bool IsFileExists(const char* path)
{
#ifdef WIN32
	return ( _access( path, 0 ) != -1 );
#else
	return ( access( path, F_OK ) != -1 );
#endif // WIN32

	//return (GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES);
}

// Возвращает имя файла без расширения,
// т.е. для строку "textures/sky.bmp" вернет "sky".
// Выделяет в памяти новую строку, не забывать удалить.
char* GetNameFromPath(const char* path)
{
	if(!path)
		return NULL;

	char* p = (char*)path + strlen(path);

	UINT l = 0;

	bool f = false;

	for(;;)
	{
		p--;

		if(f)
			l++;

		if(*p == '.')
			f = true;

		if(*p == '\\' || *p == '/' || p == path)
		{
			if(f)
			{
				char* name = new char[l+1];
				memset(name, '\0', l+1);
				memcpy(name, p + 1, l - 1);
				return name;
			}
			else
				return NULL;
		}
	}
	return NULL;
}

// Возращает расширение файла (ищет первую точку от конца).
// Не формирует новую строку, а возвращает указатель на подстроку в исходной.
char* ExtFromPath(const char* path)
{
	if(!path)
		return NULL;

	char* p = (char*)path + strlen(path);

	for(;;)
	{
		p--;

		if(*p == '.')
			return ++p;

		if(p == path)
			return NULL;

	}
	return NULL;
}


bool FileExists(const char* file_name)
{
#ifdef WIN32
	return ( _access( file_name, 0 ) != -1 );
#else
	return ( access( file_name, F_OK ) != -1 );
#endif // WIN32
}

int ChangeDir(const char* path)
{
#ifdef WIN32
	return _chdir(path);
#else
	return chdir(path);
#endif // WIN32
}

// Ищет файл в папке и всех подпапках
// file_name - имя файла
// path - путь к папке, в которой производится поиск
// buffer - буферная строка, в ней будет возвращен полный путь к найденном файлу
// Функция возвращает true, если файл найден. 
bool FindFile(const char * const file_name, const char * path, char * const buffer)
{
	strcpy(buffer, path);
	strcat(buffer, "/");
	strcat(buffer, file_name);

	if (IsFileExists(buffer))
	{
		return true;
	}
	else
	{
		buffer[0] = 0;

		DIR *dp;
		struct dirent *ep;
		char new_path[MAX_PATH];

		dp = opendir(path);
		if (dp != NULL)
		{
			while ( (ep = readdir(dp)) )
			{
				if (ep->d_type == DT_DIR && ep->d_name[0] != '.')
				{
					new_path[0] = 0;
					strcpy(new_path, path);
					strcat(new_path, "/");
					strcat(new_path, ep->d_name);

					if ( FindFile(file_name, new_path, buffer) )
						return true;
				}

			}
			closedir(dp);
		}
	}
	return false;
}

bool FindAllFiles(const char * path, char * const buffer, void (*callback)(const char* const) )
{
	buffer[0] = 0;

	DIR *dp;
	struct dirent *ep;
	char new_path[MAX_PATH];

	dp = opendir(path);
	if (dp != NULL)
	{
		while ( (ep = readdir(dp)) )
		{
			if (ep->d_type == DT_DIR && ep->d_name[0] != '.')
			{
				new_path[0] = 0;
				strcpy(new_path, path);
				strcat(new_path, "/");
				strcat(new_path, ep->d_name);

				FindAllFiles(new_path, buffer, callback);
			}
			else if (ep->d_name[0] != '.')
			{
				new_path[0] = 0;
				strcpy(new_path, path);
				strcat(new_path, "/");
				strcat(new_path, ep->d_name);
				
				callback(new_path);
			}

		}
		closedir(dp);
	}

	return false;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Служебные функции программы

// Очищает логи
void CleanupLogs()
{
	CreateLogFile(DEFAULT_LOG_NAME);
	CreateLogFile(DEFAULT_SCRIPT_LOG_NAME);
	CreateLogFile(DEFAULT_GUI_LOG_NAME);
	CreateLogFile(DEFAULT_NET_LOG_NAME);
}

// Вставляет в логи строку, указывающую на завершение логирования
void EndupLogs()
{
	static const char* endup_msg = "Finished logging";

	//_LogToFile(DEFAULT_LOG_NAME, endup_msg);
	//_LogToFile(DEFAULT_GUI_LOG_NAME, endup_msg);
	//_LogToFile(DEFAULT_SCRIPT_LOG_NAME, endup_msg);
	cfg.log_level = logLevelInfo;
	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "%s", endup_msg);
	sLog(DEFAULT_SCRIPT_LOG_NAME, LOG_INFO_EV, "%s", endup_msg);
	sLog(DEFAULT_NET_LOG_NAME, LOG_INFO_EV, "%s", endup_msg);
}

// Удаляет имя файла из строки, содержащей полное имя файла
void DeleteFileNameFromEndOfPathToFile(char *src)
{
	UINT i = strlen(src)-1;
	while(src[i] != '/' && src[i] != '\\' && src[i] != ':')
		i--;
	src[i+1] = 0;
}


// Задает пути
void InitPaths(void)
{
#ifdef WIN32
	// application
	GetModuleFileName(GetModuleHandle(0), path_app, MAX_PATH);
	DeleteFileNameFromEndOfPathToFile(path_app);
	ChangeDir(path_app);

	GetCurrentDirectory(MAX_PATH, path_app);
#else
	// TODO: Метод  только для linux. Поменять дл других *nix-систем.
	ssize_t count = readlink("/proc/self/exe", path_app, MAX_PATH );
	if (count > 0)
	{
		path_app[count] = 0;
		DeleteFileNameFromEndOfPathToFile(path_app);
		ChangeDir(path_app);
		printf("path_app: %s\n", path_app);
	}
	else
	{
		printf("Error getting application path");
		exit(EXIT_FAILURE);
	}

#endif // WIN32

	// log
	strcpy(path_log, path_app);
	strcat(path_log, DEFAULT_LOG_PATH);

	// config
	strcpy(path_config, path_app);
	strcat(path_config, DEFAULT_CONFIG_PATH);

	// textures
	strcpy(path_textures, path_app);
	strcat(path_textures, DEFAULT_TEXTURES_PATH);

	// fonts
	strcpy(path_fonts, path_app);
	strcat(path_fonts, DEFAULT_FONTS_PATH);

	// protos
	strcpy(path_protos, path_app);
	strcat(path_protos, DEFAULT_PROTO_PATH);

	// scripts
	strcpy(path_scripts, path_app);
	strcat(path_scripts, DEFAULT_SCRIPTS_PATH);

	// levels
	strcpy(path_levels, path_app);
	strcat(path_levels, DEFAULT_LEVELS_PATH);

	// sounds
	strcpy(path_sounds, path_app);
	strcat(path_sounds, DEFAULT_SOUNDS_PATH);

	// sounds
	strcpy(path_screenshots, path_app);
	strcat(path_screenshots, DEFAULT_SCREENSHOTS_PATH);

	// saves
	strcpy(path_saves, path_app);
	strcat(path_saves, DEFAULT_SAVES_PATH);
}


#ifndef WIN32
#define _mkdir(x) mkdir(x, S_IRWXU)
#endif // WIN32

// Процедура проверяет существует ли указанные пути.
// Если их нет, создает их.
void CheckPaths()
{
	if(!IsFileExists(path_log))
	{
		_mkdir(path_log);
	}
	if(!IsFileExists(path_config))
	{
		_mkdir(path_config);
	}
	if(!IsFileExists(path_textures))
	{
		_mkdir(path_textures);
	}
	if(!IsFileExists(path_fonts))
	{
		_mkdir(path_fonts);
	}
	if(!IsFileExists(path_protos))
	{
		_mkdir(path_protos);
	}
	if(!IsFileExists(path_scripts))
	{
		_mkdir(path_scripts);
	}
	if(!IsFileExists(path_levels))
	{
		_mkdir(path_levels);
	}
	if(!IsFileExists(path_sounds))
	{
		_mkdir(path_sounds);
	}
	if(!IsFileExists(path_screenshots))
	{
		_mkdir(path_screenshots);
	}
	if(!IsFileExists(path_saves))
	{
		_mkdir(path_saves);
	}
}


// Занесение в лог путей
void LogPaths(void)
{
#ifdef DISABLE_LOG
	return;
#endif

	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Game path:\t\t%s", path_app);
	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Log path:\t\t%s", path_log);
	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Config path:\t\t%s", path_config);
	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Textures path:\t%s", path_textures);
	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Fonts path:\t\t%s", path_fonts);
	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Prototypes path:\t%s", path_protos);
	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Scripts path:\t\t%s", path_scripts);
	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Levels path:\t\t%s", path_levels);
	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Sounds path:\t\t%s", path_sounds);
	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Screenshots path:\t\t%s", path_screenshots);
	sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Saves path:\t\t%s", path_saves);
}


// Процедура проверяет существует ли конфиг DEFAULT_CONFIG_NAME в директории path_config.
// Если конфига нет, создает его, указывая статические данные.
void CheckDefaultConfig()
{
	char path_to_cfg[MAX_PATH];

	sprintf(path_to_cfg, "%s%s", path_config, DEFAULT_CONFIG_NAME);
	if(!FileExists(path_to_cfg))
	{
		RecreateConfig();
	}


}

//////////////////////////////////////////////////////////////////////////
// Создает копию строки. Является аналогом функции strdup, но пользуется не malloc,
// а new[]. Такую строку тогда можно удалять с помощью DELETEARRAY.
char* StrDupl (const char *str)
{
	if (!str)
		return NULL;

	char *newstr = NULL;
	size_t size = strlen(str) + 1;

	newstr = new char[size];
	if (newstr)
		memcpy(newstr, str, size);
	return newstr;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

const char* LogLevelGetName( LogLevel ll )
{
	switch (ll)
	{
		case logLevelNone:
			return "logLevelNone";
		case logLevelError:
			return "logLevelError";
		case logLevelWarning:
			return "logLevelWarning";
		case logLevelInfo:
			return "logLevelInfo";
		default:
			return "logLevelError";
	}
}

// Satanlog
bool LogEnabled = true;
char LogFile[2048];				// Буфер для полного имени файла лога
CDummy InitLog = CDummy();

CDummy::CDummy(void)
{
	// DEBUG_STOP_ON_ALLOC_NUM определяется в misc.h
	// Там ее искать легче :)
#ifdef DEBUG_STOP_ON_ALLOC_NUM
	_CrtSetBreakAlloc(DEBUG_STOP_ON_ALLOC_NUM);
#endif // DEBUG_STOP_ON_ALLOC_NUM
}

CDummy::~CDummy(void)
{
}

// Создает новый файл лога (или очищает старый)
// fname - имя файла вида "\\file_name.log"
void CreateLogFile(const char *fname)
{
#ifdef DISABLE_LOG
	return;
#endif

	if (!LogEnabled)
		return;

	FILE *hf = NULL;

	memset(LogFile, '\0', strlen(path_log) + strlen(fname) + 1);
	sprintf(LogFile, "%s%s", path_log, fname);

	hf = fopen(LogFile, "w");
	if (!hf)
	{
		char buf[2048];
		sprintf(buf, "Failed to open log-file: %s", LogFile);
		#ifdef WIN32
		MessageBox(NULL, buf, NULL, NULL);
		#else
		perror(buf);
		#endif // WIN32
		return;
	}

	time_t current_time;
	time( &current_time );
	struct tm *newtime;
	newtime = localtime(&current_time);
	char buff[256];
	memset(buff,0,256);
	strcpy(buff, asctime(newtime));
	for (UINT i=0; i<strlen(buff); i++)
	{
		if (buff[i]<=13)
			buff[i] = 0;
	}

	fprintf(hf, "[%s] [%s] Log file \"%s\" created\n", buff, LogLevelGetName(LOG_INFO_EV), fname);

	fclose(hf);
}


//bool LogLevelLowerThan( const char* Event )
//{
//	/*
//	if ( strcmp(Event, LOG_INFO_EV) == 0 && cfg.log_level > logLevelInfo ) return true;
//	if ( strcmp(Event, LOG_WARNING_EV) == 0 && cfg.log_level > logLevelWarning ) return true;
//	if ( strcmp(Event, LOG_ERROR_EV) == 0 && cfg.log_level > logLevelError ) return true;*/
//	return false;
//}

const char* GetLogLevelName( LogLevel ll )
{
	switch (ll)
	{
		case logLevelError: return "ERROR";
		case logLevelInfo: return "INFO";
		case logLevelWarning: return "WARNING";
		case logLevelScript: return "SCRIPT";
		default: return "???";
	}
}

// Запись в лог
// fname - имя файла вида "\\file_name.log"
// Event - описание записи в логе
// Format - строка формата сообщения (подобно printf)
void sLog(const char* fname, LogLevel Event, const char* Format,...)
{
#ifdef DISABLE_LOG
	return;
#endif

	if (!LogEnabled)
		return;
	
	if ( Event < cfg.log_level )
		return;

	memset(LogFile, '\0', strlen(path_log) + strlen(fname) + 1);
	sprintf(LogFile, "%s%s", path_log, fname);


	FILE *hf = NULL;
	hf = fopen(LogFile, "a");
	if (!hf)
	{
		char buf[2048];
		sprintf(buf, "Failed to open log-file: %s", LogFile);
		#ifdef WIN32
		MessageBox(NULL, buf, NULL, NULL);
		#else
		perror(buf);
		#endif // WIN32
		return;
	}


	time_t current_time;
	time( &current_time );
	struct tm *newtime;
	newtime = localtime(&current_time);
	char buff[256];
	memset(buff,0,256);
	strcpy(buff,asctime(newtime));
	for (UINT i=0; i<strlen(buff); i++)
	{
		if (buff[i]<=13)
			buff[i] = 0;
	}
	fprintf(hf,"[%s] [%s] ", buff, GetLogLevelName(Event));
	//printf("[%s] [%s] ", buff, Event);
	va_list ap;
	va_start(ap, Format);
	vfprintf(hf, Format, ap);
	//vprintf(Format, ap);
	va_end(ap);
	fprintf(hf,"\n");
	//printf("\n");
	fclose(hf);

}

// Проверка, оканчивается ли строка на подстроку
// Аналог питоновского endswith
// Стырил с http://stackoverflow.com/questions/874134/find-if-string-endswith-another-string-c
bool endswith(std::string const &fullString, std::string const &ending)
{
    if (fullString.length() > ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

bool startswith(std::string const &fullString, std::string const &beginning)
{
	if (fullString.length() < beginning.length()) {
		return false;
	} else {
		return (0 == fullString.compare(0, beginning.length(), beginning));
	}
}


#ifndef __MISC_H_
#define __MISC_H_

//////////////////////////////////////////////////////////////////////////
// Для остановки на выделении памяти расскоментировать и указать номер выделения
#ifdef FIND_MEM_LEAKS
//#define DEBUG_STOP_ON_ALLOC_NUM 152893
#endif // FIND_MEM_LEAKS
//////////////////////////////////////////////////////////////////////////

char* StrDupl (const char *str);

void EndupLogs();

void CheckDefaultConfig();
char* ExtFromPath(const char* path);
char* GetNameFromPath(const char* path);
void DeleteFileNameFromEndOfPathToFile(char *src);
int ChangeDir(const char* path);
bool FindFile(const char * const file_name, const char * path, char * const buffer);
bool FindAllFiles(const char * path, char * const buffer, void (*callback)(const char* const) );

void InitPaths(void);
void CheckPaths();
void CleanupLogs();
void LogPaths(void);

const char* LogLevelGetName( LogLevel ll );
//bool LogLevelLowerThan( const char* Event );

void CreateLogFile(const char *fname);
void sLog(const char* fname, LogLevel Event, const char* Format,...);
bool FileExists(const char* file_name);
/**
*	хитрый план создавать лог. мне не нравится. TODO!
*	то то и оно проще запхать все в ф-ю Log и на первом вызове создавать логфаель.
*/

class CDummy
{
public:
	CDummy();
	~CDummy();
};


//////////////////////////////////////////////////////////////////////////

// Проверка на то, что два числа с плавующей точко близки друг к дургу с определеной точностью.
// float к примеру, может быть 1e-35, что почти 0, но не 0. НАм же такая точность иногда нафиг не нужна.
__INLINE bool IsNear(float a, float b, float accuracy)
{
	return fabs(a-b) < accuracy;
}

// Проверка, оканчивается ли строка на подстроку
// Аналог питоновского endswith
// Стырил с http://stackoverflow.com/questions/874134/find-if-string-endswith-another-string-c
bool endswith(std::string const &fullString, std::string const &ending);

bool startswith(std::string const &fullString, std::string const &beginning);
#endif // __MISC_H_

#ifndef __RESOURCE_MGR_H_
#define __RESOURCE_MGR_H_

#include <typeinfo>
#include "misc.h"


// Шаблонный класс менеджера ресурсов определенного типа.
// Ресурс должен сответствовать интерфейсу, описанному в class Resource.
template < typename T >
class ResourceMgr
{
public:
// 

	// Автоматический поис файла. Если отключен, то тогда имя ресурса должно 
	// содержать относительный путь к файлу.
	bool autoSearchFile;

	// Автозагрузка ресурсов. Если включнеа, то ресурс будет загружен по первому требованию.
	bool autoLoadResource;

	// Название типа ресурса, берется автоматически c помощью typeid(T).name() и 
	// используется в сообщениях об ошибках.
	// Из-за использования typeid(T).name(), глупая студия видит по две утечки на 
	// каждый экзепляр класса. Это еще древний баг, тянущийся со времен VC4.
	const char* typeName;

	// path - путь к папке, являющейся корнем для всех ресурсов данного типа.
	// extension - расширение файлов всех ресурсов даного типа. Автоматически 
	// присоединяется к имени, для образования имени файла.
	ResourceMgr(const char *path, const char *extension)
	{
		assert(path);
		//assert(extension);
		this->path = StrDupl(path);
		this->file_extension = StrDupl(extension);

		autoSearchFile = false;
		autoLoadResource = true;

		typeName = typeid(T).name();
	}

	virtual ~ResourceMgr()
	{
		UnloadAll();
		DELETEARRAY(path);
		DELETEARRAY(file_extension);
	}

	// Функция выгружает все ресурсы данного типа.
	void UnloadAll()
	{
		if (resMap.size() == 0)
			return;

		sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Выгружаем все %s", typeName);
		for (ResMapIter it = resMap.begin(); it != resMap.end(); it++)
		{
			DELETESINGLE(it->second);
		}
		resMap.clear();
	}

	void UnloadAllUnUsed()
	{
		if (resMap.size() == 0)
			return;

		sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Выгружаем все неиспользуемые %s", typeName);
		// TODO: оттестирвать
		ResMapIter it = resMap.begin();
		while(it != resMap.end())
		{
			if (it->second->IsUnUsed())
			{
				DELETESINGLE(it->second);
				resMap.erase(it++);
			}
			else
			{
				sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Нельзя выгрузить %s по имени %s, он все еще используется", typeName, it->second->name.c_str());
				++it;
			}
		}
	}

	T* GetByName(const string& name, const string& possible_prefix)
	{
		if (!name.empty())
		{
			const string full_name = possible_prefix + name;
			// Поиск в map
			ResMapConstIter it = resMap.find(name);
			if (it != resMap.end())
			{
				return it->second;
			}
			else if (autoLoadResource)
			{
				// Попытка загрузить
				T* res = Load(name, possible_prefix);
				if (!res) res = Load(name);
				if (res)
				{
					// Загрузили, теперь вносим в map
					resMap[name] = res;
					return res;
				}
			}
		}
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Попытка получить %s по имени %s не увенчалась успехом", typeName, name.c_str());
		return NULL;
	}

	// Возвращает указатель на загруженный ресурс.
	T* GetByName(const string& name)
	{
		if (!name.empty())
		{
			// Поиск в map
			ResMapConstIter it = resMap.find(name);
			if (it != resMap.end())
			{
				return it->second;
			}
			else if (autoLoadResource)
			{
				// Попытка загрузить
				T* const res = Load(name);
				if (res)
				{
					// Загрузили, теперь вносим в map
					resMap[name] = res;
					return res;
				}
			}
		}
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Попытка получить %s по имени %s не увенчалась успехом", typeName, name.c_str());
		return NULL;
	}

	T* GetByName(const char* name, const char* possible_prefix)
	{
		//assert(name);
		return name ? GetByName((string)name, (string)possible_prefix) : NULL;
	}

	T* GetByName(const char* name)
	{
		//assert(name);
		return name ? GetByName((string)name) : NULL;
	}

	// Выгружает ресурс с заданным именем.
	void Unload(const string& name)
	{
		if (!name.empty() && resMap.size())
		{
			ResMapIter it = resMap.find(name);
			if (it != resMap.end())
			{
				if (it->second->IsUnUsed())
				{
					DELETESINGLE(it->second);
					resMap.erase(it);
				}
			}
		}
	}

	// Вызывает для всех ресуров функцию Recover() - восстановление/перезагрузка ресурса.
	void RecoverAll()
	{
		if (resMap.size() == 0)
			return;

		for (ResMapConstIter it = resMap.begin(); it != resMap.end(); it++)
		{
			it->second->Recover();
		}
	}
	
private:
	// Строит имя и путь к файлу и выполняет загрузку.
	T* Load(const string& name)
	{
		string file_name = name;
		if (this->file_extension)
			file_name += string(".") + this->file_extension;

		char full_file_name[MAX_PATH];
		
		if (autoSearchFile)
		{
			if( !FindFile(file_name.c_str(), this->path, full_file_name) )
			{
				sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Файл %s не найден", file_name.c_str());
				return NULL;
			}
		}
		else
		{
			strcpy(full_file_name, this->path);
#ifndef WIN32
			strcat(full_file_name, "/");
#endif //WIN32
			strcat(full_file_name, file_name.c_str());
		}

		sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Загружаем %s", file_name.c_str());
		T* t = new T(full_file_name, name);
		if (!t->Load())
		{
			DELETESINGLE(t);
			return NULL;
		}			

		return t;
	}

	// Строит имя и путь к файлу и выполняет загрузку c именем, которое может отличаться от пути.
	T* Load(const string& name, const string& path)
	{
		string file_name = name;
		if (this->file_extension)
			file_name += string(".") + this->file_extension;

		char full_file_name[MAX_PATH];
		
		if (autoSearchFile)
		{
			if( !FindFile(file_name.c_str(), this->path, full_file_name) )
			{
				sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Файл %s не найден", file_name.c_str());
				return NULL;
			}
		}
		else
		{
			strcpy(full_file_name, this->path);
			strcat(full_file_name, "/");
			strcat(full_file_name, path.c_str());
			strcat(full_file_name, file_name.c_str());
		}

		sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "Загружаем %s", file_name.c_str());
		T* t = new T(full_file_name, name);
		if (!t->Load())
		{
			DELETESINGLE(t);
			return NULL;
		}			

		return t;
	}

	char* path;
	char* file_extension;

protected:
	
	typedef typename std::map<string, T*> ResMap;
	typedef typename ResMap::iterator ResMapIter;
	typedef typename ResMap::const_iterator ResMapConstIter;

	ResMap resMap;
};


void InitResourceManagers();
void DestroyResourceManagers();
//void TestLoading();
#endif // __RESOURCE_MGR_H_

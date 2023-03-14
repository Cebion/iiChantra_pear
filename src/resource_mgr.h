#ifndef __RESOURCE_MGR_H_
#define __RESOURCE_MGR_H_

#include <typeinfo>
#include "misc.h"


// ��������� ����� ��������� �������� ������������� ����.
// ������ ������ �������������� ����������, ���������� � class Resource.
template < typename T >
class ResourceMgr
{
public:
// 

	// �������������� ���� �����. ���� ��������, �� ����� ��� ������� ������ 
	// ��������� ������������� ���� � �����.
	bool autoSearchFile;

	// ������������ ��������. ���� ��������, �� ������ ����� �������� �� ������� ����������.
	bool autoLoadResource;

	// �������� ���� �������, ������� ������������� c ������� typeid(T).name() � 
	// ������������ � ���������� �� �������.
	// ��-�� ������������� typeid(T).name(), ������ ������ ����� �� ��� ������ �� 
	// ������ �������� ������. ��� ��� ������� ���, ��������� �� ������ VC4.
	const char* typeName;

	// path - ���� � �����, ���������� ������ ��� ���� �������� ������� ����.
	// extension - ���������� ������ ���� �������� ������ ����. ������������� 
	// �������������� � �����, ��� ����������� ����� �����.
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

	// ������� ��������� ��� ������� ������� ����.
	void UnloadAll()
	{
		if (resMap.size() == 0)
			return;

		sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "��������� ��� %s", typeName);
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

		sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "��������� ��� �������������� %s", typeName);
		// TODO: ������������
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
				sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "������ ��������� %s �� ����� %s, �� ��� ��� ������������", typeName, it->second->name.c_str());
				++it;
			}
		}
	}

	T* GetByName(const string& name, const string& possible_prefix)
	{
		if (!name.empty())
		{
			const string full_name = possible_prefix + name;
			// ����� � map
			ResMapConstIter it = resMap.find(name);
			if (it != resMap.end())
			{
				return it->second;
			}
			else if (autoLoadResource)
			{
				// ������� ���������
				T* res = Load(name, possible_prefix);
				if (!res) res = Load(name);
				if (res)
				{
					// ���������, ������ ������ � map
					resMap[name] = res;
					return res;
				}
			}
		}
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "������� �������� %s �� ����� %s �� ���������� �������", typeName, name.c_str());
		return NULL;
	}

	// ���������� ��������� �� ����������� ������.
	T* GetByName(const string& name)
	{
		if (!name.empty())
		{
			// ����� � map
			ResMapConstIter it = resMap.find(name);
			if (it != resMap.end())
			{
				return it->second;
			}
			else if (autoLoadResource)
			{
				// ������� ���������
				T* const res = Load(name);
				if (res)
				{
					// ���������, ������ ������ � map
					resMap[name] = res;
					return res;
				}
			}
		}
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "������� �������� %s �� ����� %s �� ���������� �������", typeName, name.c_str());
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

	// ��������� ������ � �������� ������.
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

	// �������� ��� ���� ������� ������� Recover() - ��������������/������������ �������.
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
	// ������ ��� � ���� � ����� � ��������� ��������.
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
				sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "���� %s �� ������", file_name.c_str());
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

		sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "��������� %s", file_name.c_str());
		T* t = new T(full_file_name, name);
		if (!t->Load())
		{
			DELETESINGLE(t);
			return NULL;
		}			

		return t;
	}

	// ������ ��� � ���� � ����� � ��������� �������� c ������, ������� ����� ���������� �� ����.
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
				sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "���� %s �� ������", file_name.c_str());
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

		sLog(DEFAULT_LOG_NAME, LOG_INFO_EV, "��������� %s", file_name.c_str());
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

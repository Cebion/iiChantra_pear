#ifndef __RESOURCE_H_
#define __RESOURCE_H_

class Resource
{
public:
	string name;
	string file_name;

	mutable int usageCounter;

	Resource(string _file_name, string _name) 
		: name(_name), file_name(_file_name)
	{
		usageCounter = 0;
	}

	virtual ~Resource()
	{
		//assert(usageCounter == 0);
	}

	virtual bool Load() = 0;
	//virtual bool Unload() = 0;
	virtual bool Recover() { return false; }

	void ReserveUsage()	const	{ usageCounter++; }			// Тут const - это ложь. На самом деле это метод-мутатор. Просто так проще.
	void ReleseUsage()	const	{ assert(usageCounter > 0); usageCounter--; }
	bool IsUnUsed()		const	{ return usageCounter == 0; }
};


#endif // __RESOURCE_H_
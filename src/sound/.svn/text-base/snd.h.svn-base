#ifndef _SND_H_
#define _SND_H_

#include "../resource.h"
#include "../resource_mgr.h"
#include "bass.h"
#include "../game/phys/phys_misc.h" //Там Vector2. Или передвинуть или убрать отсюда.

typedef HMUSIC BASSHANDLE;

enum AudioType { atSound, atMusic };

class Sound : public Resource
{
public:
	Sound(string file_name, string name) : Resource(file_name, name)
	{
		bassHandle = 0;
		type = atSound;
	}

	virtual ~Sound();

	bool Play(bool restart) const;
	bool Pause() const;
	bool Stop() const;

	// TODO: Используя BASS_ChannelSlideAttribute, можно плавно менять громкость.

	float GetVolume() const;
	bool SetVolume(float value);
	
	bool SetLooped(bool loop);

	virtual bool Load();

private:
	BASSHANDLE bassHandle;
	AudioType type;
};

class SoundMgr : public ResourceMgr<Sound>
{
public:
	SoundMgr(const char* path, const char* extension);
	~SoundMgr();
	bool Initialize();
	void Destroy();

	bool PlaySnd(const string& name, bool looped);
	bool PlaySnd(const string& name, bool restart, Vector2 origin);
	bool PauseSnd(const string& name);
	bool StopSnd(const string& name);

	void StopAll();
	bool PauseAll();
	bool ResumeAll();

	float GetVolume();
	bool SetVolume(float vol);
	float GetSoundVolume();
	float GetMusicVolume();
	void SetSoundVolume(float value);
	void SetMusicVolume(float value);
	float GetSndVolume(string name);
	bool SetSndVolume(string name, float vol);
	bool SetMusVolume(float vol);

	bool PlayBackMusic(string name);
	bool PauseBackMusic();
	bool StopBackMusic();

	const string& GetCurrentBackMusic() const;
private:
	void SetCurrentBackMusic(string newMusic);



	string currBackMusic;
	int err;
	bool initialized;
};

#endif 

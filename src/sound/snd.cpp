#include "StdAfx.h"

#include "bass.h"
#include "snd.h"

#include "../misc.h"
#include "../config.h"

extern config cfg;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


const char * GetBassErrorText(int code)
{
	switch (code)
	{
	case BASS_OK:				return "all is OK"; break;
	case BASS_ERROR_MEM:		return "memory error"; break;
	case BASS_ERROR_FILEOPEN:	return "can't open the file"; break;
	case BASS_ERROR_DRIVER:		return "can't find a free/valid driver"; break;
	case BASS_ERROR_BUFLOST:	return "the sample buffer was lost"; break;
	case BASS_ERROR_HANDLE:		return "invalid handle"; break;
	case BASS_ERROR_FORMAT:		return "unsupported sample format"; break;
	case BASS_ERROR_POSITION:	return "invalid position"; break;
	case BASS_ERROR_INIT:		return "BASS_Init has not been successfully called"; break;
	case BASS_ERROR_START:		return "BASS_Start has not been successfully called"; break;
	case BASS_ERROR_ALREADY:	return "already initialized/paused/whatever"; break;
	case BASS_ERROR_NOCHAN:		return "can't get a free channel"; break;
	case BASS_ERROR_ILLTYPE:	return "an illegal type was specified"; break;
	case BASS_ERROR_ILLPARAM:	return "an illegal parameter was specified"; break;
	case BASS_ERROR_NO3D:		return "no 3D support"; break;
	case BASS_ERROR_NOEAX:		return "no EAX support"; break;
	case BASS_ERROR_DEVICE:		return "illegal device number"; break;
	case BASS_ERROR_NOPLAY:		return "not playing"; break;
	case BASS_ERROR_FREQ:		return "illegal sample rate"; break;
	case BASS_ERROR_NOTFILE: 	return "the stream is not a file stream"; break;
	case BASS_ERROR_NOHW:		return "no hardware voices available"; break;
	case BASS_ERROR_EMPTY:		return "the MOD music has no sequence data"; break;
	case BASS_ERROR_NONET:		return "no internet connection could be opened"; break;
	case BASS_ERROR_CREATE:		return "couldn't create the file"; break;
	case BASS_ERROR_NOFX:		return "effects are not available"; break;
	case BASS_ERROR_NOTAVAIL:	return "requested data is not available"; break;
	case BASS_ERROR_DECODE:		return "the channel is a \"decoding channel\""; break;
	case BASS_ERROR_DX:			return "a sufficient DirectX version is not installed"; break;
	case BASS_ERROR_TIMEOUT:	return "connection timedout"; break;
	case BASS_ERROR_FILEFORM:	return "unsupported file format"; break;
	case BASS_ERROR_SPEAKER:	return "unavailable speaker"; break;
	case BASS_ERROR_VERSION:	return "invalid BASS version (used by add-ons)"; break;
	case BASS_ERROR_CODEC:		return "codec is not available/supported"; break;
	case BASS_ERROR_ENDED:		return "the channel/file has ended"; break;
	case BASS_ERROR_UNKNOWN:
	default:					return "some other mystery problem"; break;
	}
}

__INLINE void LogBassError(const char* const funcName)
{
	int err = BASS_ErrorGetCode();
	sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "BASS error %d in function %s: %s", err, funcName, GetBassErrorText(err));
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

Sound::~Sound()
{
	// TOOD: Выгружать звук из bass или чего там еще использоваться будет.
}

bool Sound::Play(bool restart) const
{
#ifdef NOSOUND_BASS
	UNUSED_ARG(restart);
#else
	if (BASS_ChannelPlay(this->bassHandle, restart))
		return true;
	else
		LogBassError(__FUNCTION__);
#endif // NOSOUND_BASS
	return false;
}

bool Sound::Pause() const
{
#ifndef NOSOUND_BASS
	if (BASS_ChannelPause(this->bassHandle))
		return true;
	else
		LogBassError(__FUNCTION__);
#endif // NOSOUND_BASS
	return false;
}

bool Sound::Stop() const
{
#ifndef NOSOUND_BASS
	if (BASS_ChannelStop(this->bassHandle))
		return true;
	else
		LogBassError(__FUNCTION__);
#endif // NOSOUND_BASS
	return false;
}

float Sound::GetVolume() const
{
#ifndef NOSOUND_BASS
	float bufVolume;
	DWORD attrib = BASS_ATTRIB_VOL;
	if ( type == atMusic ) attrib = BASS_ATTRIB_MUSIC_VOL_CHAN;
	if (BASS_ChannelGetAttribute(this->bassHandle, attrib, &bufVolume))
		return bufVolume;
	else
		LogBassError(__FUNCTION__);
#endif // NOSOUND_BASS
	return 0.0f;
}

bool Sound::SetVolume(float vol)
{
	if (vol < 0.0f) vol = 0.0f;
	if (vol > 1.0f) vol = 1.0f;
#ifndef NOSOUND_BASS
	DWORD attrib = BASS_ATTRIB_VOL;
	if ( type == atMusic ) attrib = BASS_ATTRIB_MUSIC_VOL_CHAN;
	if (BASS_ChannelSetAttribute(this->bassHandle, attrib, vol))
		return true;
	else
		LogBassError(__FUNCTION__);
#endif // NOSOUND_BASS
	return false;
}

bool Sound::SetLooped(bool loop)
{
#ifdef NOSOUND_BASS
	UNUSED_ARG(loop);
#else
	DWORD flag = loop ? BASS_SAMPLE_LOOP : 0;
	if ( BASS_ChannelFlags(this->bassHandle, flag, BASS_SAMPLE_LOOP) )
		return true;
	else
		LogBassError(__FUNCTION__);
#endif // NOSOUND_BASS
	return false;
}

bool Sound::Load()
{
#ifndef NOSOUND_BASS
	bassHandle = BASS_MusicLoad(FALSE, file_name.c_str(), 0, 0, 0, 0);
	type = atMusic;
	if (!bassHandle)
		bassHandle = BASS_StreamCreateFile(FALSE, file_name.c_str(), 0, 0, 0);	
		type = atSound;
	if (bassHandle)
		return true;
	else
		LogBassError(__FUNCTION__);
#endif // NOSOUND_BASS

	return false;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


SoundMgr::SoundMgr(const char* path, const char* extension) : ResourceMgr<Sound>(path, extension)
{
	initialized = false;
	Initialize();
}

//SND* snd = NULL;
SoundMgr::~SoundMgr()
{
	Destroy();
}

bool SoundMgr::Initialize()
{
	if (initialized)
		Destroy();

#ifndef NOSOUND_BASS
	if (BASS_Init(-1, 44100, 0, 0, NULL))
	{
		return initialized = true;
	}
	else
	{
		LogBassError(__FUNCTION__);
		initialized = false;		
	}
#endif // NOSOUND_BASS
	return false;
}

void SoundMgr::Destroy()
{
#ifndef NOSOUND_BASS
	if (initialized)
	{
		if (!BASS_Free())
			LogBassError(__FUNCTION__);
		initialized = false;
	}
#endif // NOSOUND_BASS
}

bool SoundMgr::PlaySnd(const string& name, bool restart)
{
	Sound* snd = GetByName(name);
	snd->SetVolume( cfg.volume_sound );
	if (snd)
		return snd->Play(restart);
	return false;
}

extern float CAMERA_X;
extern float CAMERA_Y;

bool SoundMgr::PlaySnd(const string& name, bool restart, Vector2 origin)
{
	Sound* snd = GetByName(name);
	if (snd)
	{
		origin -= Vector2(CAMERA_X, CAMERA_Y);
		float dist = origin.Length();
		if ( dist > MAXIMUM_SOUND_DISTANCE )
			return true;
		dist = 1 - dist/MAXIMUM_SOUND_DISTANCE;
		dist *= cfg.volume_sound;
		snd->SetVolume( dist );
		return snd->Play(restart);
	}
	return false;
}

bool SoundMgr::StopSnd(const string& name)
{
	if (name.empty())
		return false;
	const Sound* snd = GetByName(name);
	if (snd)
		return snd->Stop();
	return false;
}

bool SoundMgr::PauseSnd(const string& name)
{
	const Sound* snd = GetByName(name);
	if (snd)
		return snd->Pause();
	return false;
}

float SoundMgr::GetVolume()
{
#ifndef NOSOUND_BASS
	if (initialized)
	{

		unsigned long vol = BASS_GetConfig(BASS_CONFIG_GVOL_MUSIC);
		//if (vol != -1)
		if (!BASS_ErrorGetCode())
			return (float)vol / 10000;
		else
			LogBassError(__FUNCTION__);
	}
#endif // NOSOUND_BASS
	return 0.0f;
}

float SoundMgr::GetSoundVolume()
{
	return cfg.volume_sound;
}

float SoundMgr::GetMusicVolume()
{
	return cfg.volume_music;
}

void SoundMgr::SetSoundVolume(float value)
{
	cfg.volume_sound = floor((value*10)+0.5f)/10.0f;
}

void SoundMgr::SetMusicVolume(float value)
{
	cfg.volume_music = floor((value*10)+0.5f)/10.0f;
	SetMusVolume( cfg.volume_music );
}

bool SoundMgr::SetVolume(float vol)
{
	if (!this->initialized)
		return false;

	if (vol < 0.0f) vol = 0.0f;
	if (vol > 1.0f) vol = 1.0f;
	cfg.volume = vol;
#ifndef NOSOUND_BASS
	if (BASS_SetConfig(BASS_CONFIG_GVOL_MUSIC, (DWORD)(vol*10000)) && 
		BASS_SetConfig(BASS_CONFIG_GVOL_SAMPLE, (DWORD)(vol*10000)) &&
		BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, (DWORD)(vol*10000)) )		
		return true;
	else
		LogBassError(__FUNCTION__);	
#endif // NOSOUND_BASS
	return false;
}

float SoundMgr::GetSndVolume(string name)
{
	const Sound* snd = GetByName(name);
	if (snd)
		return snd->GetVolume();
	return 0.0f;
}

bool SoundMgr::SetSndVolume(string name, float vol)
{
	Sound* snd = GetByName(name);
	if (snd)
		return snd->SetVolume(vol);
	return false;
}

bool SoundMgr::SetMusVolume(float vol)
{
	std::string mus = GetCurrentBackMusic();
	if ( mus == "" ) return true;
	Sound* snd = GetByName(mus);
	if (snd)
		return snd->SetVolume( vol );
	return false;
}

bool SoundMgr::PlayBackMusic(string name)
{
	this->StopBackMusic();
	Sound* snd = GetByName(name);

	if (snd && snd->SetVolume( cfg.volume_music ) && snd->Play(true) && snd->SetLooped(true))
	{
		SetCurrentBackMusic(name);
		return true;
	}
	return false;
}

bool SoundMgr::PauseBackMusic()
{
	return PauseSnd(GetCurrentBackMusic());
}

bool SoundMgr::StopBackMusic()
{
	bool ret = StopSnd(GetCurrentBackMusic());
	SetCurrentBackMusic(string(""));
	return ret;
}

void SoundMgr::SetCurrentBackMusic(string newMusic)
{
	currBackMusic = newMusic;
}

const string& SoundMgr::GetCurrentBackMusic() const
{
	return currBackMusic;
}

void SoundMgr::StopAll()
{
	if (resMap.size() == 0)
		return;

	for (ResMapIter it = resMap.begin(); it != resMap.end(); it++)
	{
		it->second->Stop();
	}
}

bool SoundMgr::PauseAll()
{
#ifndef NOSOUND_BASS
	if (this->initialized)
	{
		if (BASS_Pause())
			return true;
		else
			LogBassError(__FUNCTION__);
	}
#endif // NOSOUND_BASS
	return false;
}

bool SoundMgr::ResumeAll()
{
#ifndef NOSOUND_BASS
	if (this->initialized)
	{
		if (BASS_Start())
			return true;
		else
			LogBassError(__FUNCTION__);
	}
#endif // NOSOUND_BASS
	return false;
}




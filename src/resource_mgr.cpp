#include "StdAfx.h"

#include "resource_mgr.h"

#include "render/texture.h"
#include "render/font.h"
#include "game/proto.h"
#include "sound/snd.h"





extern char path_app[MAX_PATH];
extern char path_textures[MAX_PATH];
extern char path_fonts[MAX_PATH];
extern char path_protos[MAX_PATH];
extern char path_sounds[MAX_PATH];


ResourceMgr<Proto> * protoMgr = NULL;
ResourceMgr<Texture> * textureMgr = NULL;
SoundMgr * soundMgr = NULL;

void InitResourceManagers()
{
	protoMgr = new ResourceMgr<Proto>(path_protos, "lua");
	textureMgr = new ResourceMgr<Texture>(path_textures, "png");
	soundMgr = new SoundMgr(path_sounds, NULL);
}

void DestroyResourceManagers()
{
	DELETESINGLE(protoMgr);
	DELETESINGLE(textureMgr);
	DELETESINGLE(soundMgr);
}

//void DoLoad(const char* const name)
//{
//	char* n1 = GetNameFromPath(name);
//	protoMgr->GetByName(n1);
//	DELETEARRAY(n1);
//}
//
//
//void TestLoading()
//{
//	char full_file_name[MAX_PATH];
//	FindAllFiles(path_protos, full_file_name, DoLoad);
//}
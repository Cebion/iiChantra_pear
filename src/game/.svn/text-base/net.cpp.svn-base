#include "StdAfx.h"
#include "net.h"
#include "game.h"
//#include "objects/object_remote_player.h"
#include "net.h"
#include "objects/object_player.h"
#include "player.h"
#include "../scene.h"
#include "../render/font.h"
#include <vector>
#include "SDL/SDL_net.h"

extern bool netgame;
bool server;

bool Net::Init()
{
	if ( SDLNet_Init() == -1 )
	{
		sLog(DEFAULT_NET_LOG_NAME, LOG_ERROR_EV, "SDL_net initialization failed: %s", SDLNet_GetError());
		return false;
	}

	/*message_buffer = new char[ 255 ];
	for ( int i = 0; i < 255; i++ )
		message_buffer[i] = '\0';*/

	return true;
}

bool Net::Close()
{
	//DELETEARRAY( message_buffer );
	SDLNet_Quit();
	return true;
}


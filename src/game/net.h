#ifndef __NET_H
#define __NET_H

//#include "objects/object_remote_player.h"
//#include "SDL_net.h"

namespace Net
{
	bool Init();
	bool Close();
	
	//bool StartServer( UINT16 port );
	//bool Connect( const char* host, UINT16 port );
	
	//void Update();
}


enum NetPacketPriority{ nppNormal, nppImportant, nppCritical };
//Normal - ��������� � ������.
//Important - ����������, ���� �� ������� �������������
//Critical - ���������� �� ������ ����������, ���� �� ������� �������������.

#endif //__NET_H
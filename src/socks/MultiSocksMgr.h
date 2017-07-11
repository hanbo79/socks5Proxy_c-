#ifndef _MULTISENDERMGR_H_
#define _MULTISENDERMGR_H_

#include "../common/public.h"
#include "../common/Socket.h"

typedef struct
{
	int s;
	int port;
	std::string ip;
}SOCKSTATE;

typedef std::vector<SOCKSTATE> VEC_NetCardInfo;

typedef struct  
{
	int listenport;
	VEC_NetCardInfo vecNetCards;
}SERVERCONFIG;

class CMultiSocksMgr
{	
public:
	CMultiSocksMgr();
	~CMultiSocksMgr();

	int Init();
	int Connect(sockaddr_in serveraddr);
	int ReConnect();
	int Close();

	int GetCurrentSocket();
private:
	int _ConnectOnce(std::string localIP,int localPort);
	int _Connect();
	int _Get4GNetCardConfig();

private:
	typedef std::vector<SOCKSTATE> VEC_LOCALSOCKHANDLE;
	VEC_LOCALSOCKHANDLE m_vecSockets;

	sockaddr_in m_serverAddr;
	int m_iCurrentSocket;
};

#endif //_MULTISENDERMGR_H_
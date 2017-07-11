#include "stdafx.h"
#include "MultiSocksMgr.h"
#include <string>
#include <fstream>

extern SERVERCONFIG g_serverconfig;

CMultiSocksMgr::CMultiSocksMgr()
{
	m_iCurrentSocket = -1;
}

CMultiSocksMgr::~CMultiSocksMgr()
{

}

int CMultiSocksMgr::Init()
{	
	m_iCurrentSocket = -1;
	
	//get local network ip config
	return _Get4GNetCardConfig();
}

int CMultiSocksMgr::Connect( sockaddr_in serveraddr )
{
	m_serverAddr = serveraddr;

	return _Connect();
}

int CMultiSocksMgr::ReConnect()
{
	return _Connect();
}

int CMultiSocksMgr::Close()
{
	m_iCurrentSocket = -1;

	for( VEC_LOCALSOCKHANDLE::iterator iter = m_vecSockets.begin();
		iter != m_vecSockets.end();
		++iter )
	{
		if( iter->s >= 0 )
			Socket::Close(iter->s);
		iter->s = -1;
	}

	return 0;
}

int CMultiSocksMgr::_ConnectOnce(std::string localIP,int localPort)
{
	int sock = Socket::Create();
	Socket::Bind(sock, Socket::IPPort2Addr(localIP.c_str(), localPort));
	if( !Socket::Connect(sock, m_serverAddr) )
	{
		Socket::Close(sock);
		return -1;
	}
	return sock;
}

int CMultiSocksMgr::_Connect()
{
	m_iCurrentSocket = -1;

	for( VEC_LOCALSOCKHANDLE::iterator iter = m_vecSockets.begin();
		iter != m_vecSockets.end();
		++iter )
	{
		if( iter->s >= 0 )
		{
			printf("CMultiSocksMgr::_Connect 关闭连接 socket=%d \n",iter->s);

			Socket::Close(iter->s);
			iter->s = -1;
		}
		else
		{
			int iRet = _ConnectOnce(iter->ip,0);//iter->port);
			if( iRet < 0 )
			{
				printf("CMultiSocksMgr::_Connect 连接失败 本地ip=%s, port=%d \n",iter->ip.c_str(), iter->port);
				continue;
			}

			printf("CMultiSocksMgr::_Connect 连接成功 本地ip=%s, port=%d \n",iter->ip.c_str(), iter->port);
			iter->s = iRet;
			m_iCurrentSocket = iRet;
			return 0;
		}
	}

	return -1;
}

int CMultiSocksMgr::_Get4GNetCardConfig()
{
	m_vecSockets.clear();

	if( g_serverconfig.vecNetCards.empty() )
		return -1;

	m_vecSockets = g_serverconfig.vecNetCards;
	return 0;
}

int CMultiSocksMgr::GetCurrentSocket()
{
	return m_iCurrentSocket;
}

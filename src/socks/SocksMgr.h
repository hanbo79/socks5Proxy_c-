﻿#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <set>
#include "SocksParser.h"

#include "../common/public.h"

#pragma comment(lib,"ws2_32.lib")

typedef struct 
{
	std::string ip;
	std::string pwd;
	std::string user;
	int port;
}CONNECT_INFO,*PCONNECT_INFO;

typedef std::set<int> SOCKET_SET;

class CSocksMgr
{	
	DECLARE_SINGLETON(CSocksMgr)

public:
	bool Begin( LPCSTR ip1, int port1,LPCSTR ip2,int port2);
	bool Begin( LPCSTR ip, int port);
	bool Begin( int port );

	void Wait();

	void Close();

	void SetAuth(LPCTSTR user,LPCTSTR pwd);

private:

	bool Proxy(int s,LPSTR user , LPSTR pwd);

	static DWORD WINAPI TCP_C2S(void* lpParameter);
	static DWORD WINAPI TCP_S2C(void* lpParameter);
	static DWORD WINAPI TCP_WorkerProc(void* lpParameter);

	static DWORD WINAPI TCPTunnel( LPVOID lpParameter );
	DWORD WINAPI TCPTunnelProc( LPVOID lpParameter );

	static DWORD WINAPI UDP_C2S_S2C(void* lpParameter);
	static DWORD WINAPI UDPTunnel( LPVOID lpParameter );
	DWORD WINAPI UDPTunnelProc( LPVOID lpParameter );

	static DWORD WINAPI Reverse(void* lpParameter);
	DWORD WINAPI ReverseProc(void* lpParameter);

	static DWORD WINAPI Forward(void* lpParameter);
	DWORD WINAPI ForwardProc(void* lpParameter);

	static DWORD WINAPI Redirect( LPVOID lpParameter );
	DWORD WINAPI RedirectProc( LPVOID lpParameter );
	std::string m_user;
	std::string m_pwd;
	std::string m_rIp;
	int m_rPort;

	bool m_NeedAuth;
};


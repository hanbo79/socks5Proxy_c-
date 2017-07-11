// libsocksclient.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "libsocksclient.h"
#include "socks5.h"
#include "AutoCleanup.h"

using proxy::socks5::Ipv4;
using proxy::socks5::Ipv4Address;
using proxy::socks5::ClientTcp;
using proxy::socks5::ErrorGeneric;

int SOCKSCLIENT_NewInstance( char proxyIP[4], int proxyPort, char targetIP[4], int targetPort, SOCKSCLIENTHANDLE *handle)//, RecvCallback callback, void *usrData )
{
	ClientTcp *socks5 = new ClientTcp;
	if( !socks5 ) return -1;

	const auto proxy(Ipv4(proxyIP[0],proxyIP[1],proxyIP[2],proxyIP[3],proxyPort));
	const auto target(Ipv4(targetIP[0],targetIP[1],targetIP[2],targetIP[3],targetPort));
	Ipv4Address binded = {};

	if( socks5->Auth(proxy) != 0 )
	{
		socks5->Disconnect();
		delete socks5;
		return -1;
	}

	if( socks5->Connect( target, binded) != 0 )
	{
		socks5->Disconnect();
		delete socks5;
		return -1;
	}

	*handle = socks5;
	return 0;
}

int SOCKSCLIENT_SendOverTcp( SOCKSCLIENTHANDLE handle, char* buffer, int len )
{
	if( !handle || !buffer || len <= 0 ) return -1;

	ClientTcp *socks5 = (ClientTcp*)handle;
	return socks5->Send(buffer,len);
}

int SOCKSCLIENT_RecvOverTcp( SOCKSCLIENTHANDLE handle, char* buffer, int len, int WaitMSeconds )
{
	if( !handle || !buffer || len <= 0 ) return -1;

	ClientTcp *socks5 = (ClientTcp*)handle;
	return socks5->SelectRecv(buffer,len, WaitMSeconds);
}

int SOCKSCLIENT_DeleteInstance( SOCKSCLIENTHANDLE handle )
{
	if( !handle ) return -1;

	ClientTcp *socks5 = (ClientTcp*)handle;
	socks5->Disconnect();
	delete socks5;

	return 0;
}

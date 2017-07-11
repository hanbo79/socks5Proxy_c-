//#include "socks5.h"
#include "libsocksclient.h"
#include <iostream>

using namespace std;
int main(int argc, char *argv[])
{
	char proxyIP[] = {10,130,68,113};
	char targetIP[] = {10,130,68,113};

	SOCKSCLIENTHANDLE handle;
	int ret = SOCKSCLIENT_NewInstance(proxyIP,1080,targetIP,8952,&handle);
	if( ret != 0 )
	{
		printf("SOCKSCLIENT_NewInstance error,ret=%d\n", ret);
		return -1;
	}

	char buffer[1024] = {0};
	while( gets(buffer) )
	{
		if( strcmp("quit",buffer ) == 0 )
			break;

		int ret = SOCKSCLIENT_SendOverTcp(handle,buffer,strlen(buffer));
		if( ret <= 0 )
			printf("SOCKSCLIENT_SendOverTcp error,len=%d\n", ret);

		memset(buffer,0,sizeof(buffer));
		int len = SOCKSCLIENT_RecvOverTcp(handle, buffer, sizeof(buffer));
		printf("recv buffer =%s,len=%d\n",buffer,len);

		memset(buffer,0,sizeof(buffer));
	}

	SOCKSCLIENT_DeleteInstance(handle);
	return 0;
}

int test()
{
// 	using proxy::socks5::Ipv4;
// 	using proxy::socks5::Ipv4Address;
// 	using proxy::socks5::ClientTcp;
// 	using proxy::socks5::ErrorGeneric;
// 
// 	try
// 	{
// 		const auto proxy(Ipv4(10,130,68,113,1080));
// 		const auto target(Ipv4(10,130,68,113,8951));
// 		Ipv4Address binded = {};
// 		ClientTcp socks5;
// 
// 		socks5.Auth( proxy, "admin", "admin" );
// 		socks5.Connect( target, binded);
// 		//    socks5.Send("\n\n === Hello, Valdemar! === \n\n");
// 
// 		char buffer[1024] = {0};
// 		while( gets(buffer) )
// 		{
// 			if( strcmp("quit",buffer ) == 0 )
// 				break;
// 
// 			socks5.Send(buffer);
// 			memset(buffer,0,sizeof(buffer));
// 		}
// 
// 		socks5.Disconnect();
// 
// 		while( true )
// 		{
// 			Sleep(100);
// 		}
// 	}
// 	catch (const ErrorGeneric &ex)
// 	{
// 		cout << "Error: " << ex.what() << endl;
// 		return 1;
// 	}
	return 0;
}
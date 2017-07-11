#ifndef _LIBSOCKSCLIENT_H_
#define _LIBSOCKSCLIENT_H_

#ifdef WIN32
#ifndef LIBSOCKSCLIENT_EXPORTS
#define RCC_DLL_EXPORT __declspec(dllimport)
	#pragma comment( lib, "libsocksclient" )
#else
#define RCC_DLL_EXPORT __declspec(dllexport)
#endif//DLLIMPORT
#else
#define RCC_DLL_EXPORT
#endif

#define SOCKSCLIENTHANDLE void*
// typedef DWORD (*RecvCallback)( char* buffer, int length, void* usrData );

extern "C" 
{
	RCC_DLL_EXPORT int SOCKSCLIENT_NewInstance(
		char proxyIP[4], 
		int proxyPort, 
		char targetIP[4],
		int targetPort,
		SOCKSCLIENTHANDLE *handle
// 		,RecvCallback callback = NULL
// 		,void *usrData = NULL
		);

	RCC_DLL_EXPORT int SOCKSCLIENT_SendOverTcp(SOCKSCLIENTHANDLE handle, char* buffer, int len);

	RCC_DLL_EXPORT int SOCKSCLIENT_RecvOverTcp(SOCKSCLIENTHANDLE handle, char* buffer, int len, int WaitMSeconds = 0);

	RCC_DLL_EXPORT int SOCKSCLIENT_DeleteInstance(SOCKSCLIENTHANDLE handle);
} //extern "C" 

#endif //#ifndef _LIBSOCKSCLIENT_H_

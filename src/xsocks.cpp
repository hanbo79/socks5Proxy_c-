#include "stdafx.h"
#include <map>
#include <string>
#include "Tunnel.h"
#include "th3rd/dns.h"
#include "socks/SocksMgr.h"
#include <vector>
#include <string>

using namespace std;

#ifdef LINUX
typedef map<string,string> CMD_MAP;
#else
typedef map<wstring,wstring> CMD_MAP;
#endif


SERVERCONFIG g_serverconfig = {-1};

void Version()
{
	return;
}

void Usage()
{
	printf("\nUsage    : xsocks  [-l port] [-t] [-p1 port] [-p2 port] [-s ip:port] \n");
	printf("                   [-r ip:port] [-u username] [-p password]          \n\n");

	printf("Options  : -l  Set forward mode.						               \n");
	printf("           -r  Set reverse mode.							           \n");
	printf("           -t  Build tunnel on socks5.                                 \n");
	printf("           -s  Redirect another socks server.                          \n");
	printf("           -u  Login username.				                           \n");
	printf("           -p  Login password.					                       \n");
	printf("           -p1 Accept of XSOCKS client's port.                         \n");
	printf("           -p2 Accept of proxy  client's port.                       \n\n");

	printf("Examples : xsocks -l 8085 -u root -p 123456                            \n");
	printf("           xsocks -t -p1 8085 -p2 8086                                 \n");
	printf("           xsocks -r 192.168.1.10:8085 -u root -p 123456               \n");
	printf("           xsocks -s 192.168.1.11:8085 -r 192.168.1.10:8086          \n\n");
}

#ifdef LINUX
void LoadCommand(int argc, char* argv[] ,CMD_MAP& map)
#else
void LoadCommand(int argc, _TCHAR* argv[] ,CMD_MAP& map)
#endif
{
	for(int i = 1 ; i < argc ; i++)
	{
		if (argv[i][0] == '-')
		{
			if (i+1 != argc && argv[i+1][0] != '-')
			{
				map[argv[i]] = argv[i+1];
			}
			else
			{
				map[argv[i]] = _T("");
			}
		}
	}
}

int LoadConfig()
{
	g_serverconfig.listenport = -1;

	std::fstream fs;
	fs.open("xsocks.conf");
	if( !fs.is_open() )
		return -1;

	char buffer[1024] = {0};
	fs.getline(buffer,sizeof(buffer));
	std::string str = buffer;
	int pos = str.find('=');
	g_serverconfig.listenport = atoi(str.substr(pos+1).c_str());

	while( fs.getline(buffer,sizeof(buffer)) )
	{
		str = buffer;
		int pos = str.find('=');
		str = str.substr(pos+1);

		SOCKSTATE state;
		state.s = -1;
		int pos1 = str.find(":",pos);
		state.ip = str.substr(0,pos1);
		state.port = atoi(str.substr(pos1+1).c_str());
		g_serverconfig.vecNetCards.push_back(state);
	}
	fs.close();

	return 0;
}

#ifdef LINUX
int main(int argc, char* argv[])
{
	string user = _T("");
	string pwd = _T("");
#else
int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2),&wsaData);
	wstring user = _T("");
	wstring pwd = _T("");
#endif

	CMD_MAP cmd;

	if( LoadConfig() < 0 )
	{
		Usage();
		return -1;
	}

	//没有参数
	if ( g_serverconfig.listenport < 0 || 
		g_serverconfig.vecNetCards.size() == 0)
	{
		Usage();
		return -1;
	}

	//获取验证账号和密码
	if (user.length() != 0 && pwd.length() != 0)
		CSocksMgr::GetInstanceRef().SetAuth(user.c_str(),pwd.c_str());

	//正向模式
	Version();
	CSocksMgr::GetInstanceRef().Begin(g_serverconfig.listenport);

	return 0;
}


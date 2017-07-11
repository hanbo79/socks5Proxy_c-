#include "stdafx.h"
#include "socks5.h"
#include "Socket.h"

namespace proxy {
namespace socks5 {


///
/// \brief The fmt class
///
class fmt
{
  std::ostringstream stream_;
public:
  template<typename T> fmt(const T& s) : stream_() { stream_ << s; }
  template<typename T> fmt& operator << (const T& val) { stream_ << val; return *this; }
  fmt& operator << (const Ipv4Address& val);
  string str() const { return stream_.str(); }
  operator string() const { return stream_.str(); }
};


ClientTcp::ClientTcp()
  : socket_(INVALID_SOCKET),m_bExit(false)
{
  const WORD requested_version = MAKEWORD(2, 2);
  WSADATA wsa_data = {};
  int err = WSAStartup(requested_version, &wsa_data);
  if (err != 0)
	  return;
//    throw ErrorSocksFailure(fmt("WSAStartup failed with error: ") << err);
  if (wsa_data.wVersion != requested_version) {
    WSACleanup();
	return;
//    throw ErrorSocksFailure("Could not find a usable version of Winsock.dll");
  }
  socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socket_ == INVALID_SOCKET)
  {
    WSACleanup();
	return;
//    throw ErrorSocksFailure("Could not find a usable version of Winsock.dll");
  }
}

ClientTcp::~ClientTcp()
{
  if (socket_ != INVALID_SOCKET) {
    closesocket(socket_);
    socket_ = INVALID_SOCKET;
  }
  WSACleanup();
}

int ClientTcp::Auth( const Ipv4Address& proxy, const std::string username/*=""*/,const std::string password/*=""*/ )
{
	sockaddr_in addr      = {};
	addr.sin_family       = AF_INET;
	addr.sin_port         = htons(proxy.port);
	addr.sin_addr.s_addr  = htonl(proxy.addr);
	int ret = connect(socket_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
	if (ret == SOCKET_ERROR)
		return -1;
//		throw ErrorSocksFailure(fmt("Could not connect to proxy server ") << proxy << " failed: " << WSAGetLastError());

	if( username == "" && password == "" )
	{
		client::AuthRequest ca(AuthMethod_Unauthorized);
		Send(ca, ca.sendlen());

		server::AuthResponse sa;
		Recv(sa, sa.recvlen());
		if (! sa.validate())
			return -1;
//			throw ErrorSocksFailure("Receive server::AuthResponse failed: Invalid data recieved.");
	}
	else
	{
		client::AuthRequest ca(AuthMethod_UsernamePassword);
		Send(ca, ca.sendlen());

		server::AuthResponse sa;
		Recv(sa, sa.recvlen());
		if (! sa.validate())
			return -1;
//			throw ErrorSocksFailure("Receive server::AuthResponse failed: Invalid data recieved.");

		client::LoginRequest la(username, password);
		Send(la, la.sendlen());

		server::LoginResponse ls;
		Recv(ls, ls.recvlen());
		if (! ls.validate())
			return -1;
//			throw ErrorSocksFailure("Receive server::LoginResponse failed: Invalid data recieved.");
	}
	return 0;
}

int ClientTcp::Connect( const Ipv4Address& host, Ipv4Address& binded)
{
  {
    client::Request req(Command_Connect, host);
    Send(req, req.sendlen());
  }
  {
    server::Response res;
    Recv(res, res.recvlen());
    if (! res.validate())
		return -1;
//      throw ErrorSocksFailure("Receive server::Response failed: Invalid data recieved.");
    switch(res.address_type)
    {
      case AddressType_Ipv4:
      {
        Recv(res.bind_addr.ipv4);
        res.bind_addr.ipv4.addr = htonl(res.bind_addr.ipv4.addr);
        res.bind_addr.ipv4.port = htons(res.bind_addr.ipv4.port);
        binded = res.bind_addr.ipv4;
        break;
      }
      case AddressType_Ipv6:
      {
        Recv(res.bind_addr.ipv6);
        //TODO:
        // res.bind_addr.ipv6.addr = htonl(res.bind_addr.ipv6.addr);
        // res.bind_addr.ipv6.port = htons(res.bind_addr.ipv6.port);
        break;
      }
      case AddressType_DomainName:
      {
        memset(res.bind_addr.domain.name, 0, sizeof(res.bind_addr.domain.name));

        Recv(res.bind_addr.domain.length);
        Recv(res.bind_addr.domain.name, res.bind_addr.domain.length);
        Recv(res.bind_addr.domain.port);
        res.bind_addr.domain.port = htons(res.bind_addr.domain.port);
        break;
      }
      default: return -1;//throw ErrorGeneric(fmt("Unsupported address type: ") << (int)res.address_type);
    }
  }
  return 0;
}

void ClientTcp::Disconnect()
{
  if (socket_ != INVALID_SOCKET) {
    closesocket(socket_);
    socket_ = INVALID_SOCKET;
  }
}

int ClientTcp::Recv(void* data, const size_t size)
{
  int len = static_cast<int>(size);
  char *buf = reinterpret_cast<char*>(data);
  int ret = recv(socket_, buf, len, 0);
  if (ret == SOCKET_ERROR)
	  return -1;
//    throw ErrorSocksFailure(fmt("Receive from proxy failed: ") << WSAGetLastError());
  return ret;
}

int ClientTcp::SelectRecv(void* data, const size_t size, int mSeconds)
{
	int len = static_cast<int>(size);
	char *buf = reinterpret_cast<char*>(data);
	
	std::vector<int> vecIn,vecOut;
	vecIn.push_back(socket_);

	if( !Socket::Select(vecIn, vecOut, mSeconds) ) return 0;

	int ret = recv(socket_, buf, len, 0);
	if (ret == SOCKET_ERROR)
		return -1;
	//    throw ErrorSocksFailure(fmt("Receive from proxy failed: ") << WSAGetLastError());
	return ret;
}

int ClientTcp::Send(const void* data, const size_t size)
{
  int len = static_cast<int>(size);
  const char *buf = reinterpret_cast<const char*>(data);
  do
  {
    int ret = send(socket_, buf, len, 0);
    if (ret == SOCKET_ERROR)
		return -1;
//      throw ErrorSocksFailure(fmt("Send to proxy failed: ") << WSAGetLastError());
    len -= ret;
  }
  while(len > 0);
  return static_cast<int>(size);
}

int ClientTcp::Recv(string& data)
{
  return Recv(&data[0], data.length());
}

int ClientTcp::Send(const string& data)
{
  return Send(&data[0], data.length());
}

namespace client {

Request::Request(const Command cmd, const Ipv4Address& ipv4)
  : version(kSocksVersion)
  , command(static_cast<CommandField>(cmd))
  , reserved(0)
  , address_type(AddressType_Ipv4)
  , dest_addr()
{
  dest_addr.ipv4.addr = htonl(ipv4.addr);
  dest_addr.ipv4.port = htons(ipv4.port);
}

int Request::sendlen() const
{
  size_t len = offsetof(Request, dest_addr);
  switch (address_type) {
    case AddressType_Ipv4:        return static_cast<int>(len + sizeof(dest_addr.ipv4));
    case AddressType_Ipv6:        return static_cast<int>(len + sizeof(dest_addr.ipv6));
    case AddressType_DomainName:  return static_cast<int>(len + sizeof(dest_addr.domain.length) + dest_addr.domain.length);
    default: return -1;//throw ErrorGeneric(fmt("Unsupported address type: ") << (int)address_type);
  }
}

AuthRequest::AuthRequest(AuthMethod method)
  : version(kSocksVersion)
  , auth_method_count(1)
{
  memset(auth_methods, 0, sizeof(auth_methods));
  auth_methods[0] = static_cast<AuthMethodField>(method);
}


LoginRequest::LoginRequest( std::string username,std::string password )
	: version(kSocksVersion)
{
	int offset = 0;
	buf[offset++] = (char)version;

	if (username.length() > MAX_USR_LEN) return;
	this->username = username;
	buf[offset++] = (char)username.size();
	memcpy(&buf[offset], username.c_str(), username.length());
	offset += username.length();

	if (password.length() > MAX_PWD_LEN) return;
	this->password = password;
	buf[offset++] = (char)password.size();
	memcpy(&buf[offset], password.c_str(), password.length());
	offset += password.length();

//	return offset;
}

}  // client

namespace server {

Response::Response()
  : version(Version_Undefined)
  , reply_type(ReplyType_Undefined)
  , reserved(0x00)
  , address_type(AddressType_Undefined)
  , bind_addr()
{}

bool Response::validate() const
{
  if (version == Version_Undefined) return false;
  if (version != kSocksVersion) return -1;//throw ErrorGeneric(fmt("Unsupported version: ") << (int)version);
  if (reply_type == ReplyType_Undefined)  return false;
  if (reply_type != ReplyType_Succeeded) {
#define case_reply_type(err) \
  case ReplyType_ ## err: return -1;//throw Error ## err("ReplyType is " # err);
    switch (reply_type) {
      case_reply_type(SocksFailure        );
      case_reply_type(NotAllowed          );
      case_reply_type(NetUnreachable      );
      case_reply_type(HostUnreachable     );
      case_reply_type(ConnRefused         );
      case_reply_type(TtlExpired          );
      case_reply_type(UnsupportedCommand  );
      case_reply_type(UnsupportedAddrType );
      default: return -1;//throw ErrorGeneric(fmt("Unsuported reply type ") << (int)reply_type);
    }
#undef case_reply_type
  }
  if (address_type == AddressType_Undefined) return false;
  if (address_type != AddressType_Ipv4 &&
      address_type != AddressType_Ipv6 &&
      address_type != AddressType_DomainName) return -1;//throw ErrorGeneric(fmt("Unsuported address type ") << (int)address_type);
  return true;
}

AuthResponse::AuthResponse()
  : version(Version_Undefined)
  , auth_method(AuthMethod_Undefined)
{}

bool AuthResponse::validate() const {
  if (version == Version_Undefined) return false;
  if (version != kSocksVersion) return -1;//throw ErrorGeneric(fmt("Unsupported version: ") << (int)version);
  if (auth_method == AuthMethod_Undefined)  return false;
  if (auth_method == AuthMethod_NoAcceptableMethods) return -1;//throw ErrorNoAcceptableMethodsFound("No acceptable methods was found.");
  return true;
}

LoginResponse::LoginResponse()
{

}

bool LoginResponse::validate() const
{
	if (version == Version_Undefined) return false;
	if (version != kSocksVersion) return -1;//throw ErrorGeneric(fmt("Unsupported version: ") << (int)version);
	if (status != LOGIN_OK)  return false;
	return true;
}

}

Ipv4Address Ipv4(uint32_t _addr, uint16_t _port)
{
  Ipv4Address a = {};
  a.addr = _addr;
  a.port = _port;
  return a;
}

Ipv4Address Ipv4(uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4, uint16_t _port)
{
  Ipv4Address a = {};
  a.part1 = p1;
  a.part2 = p2;
  a.part3 = p3;
  a.part4 = p4;
  a.port  = _port;
  return a;
}

fmt& fmt::operator <<(const Ipv4Address& val)
{
  stream_ << static_cast<int>(val.part1) << "."
          << static_cast<int>(val.part2) << "."
          << static_cast<int>(val.part3) << "."
          << static_cast<int>(val.part4) << ":"
          << static_cast<int>(val.port);
  return *this;
}

// server

}  // socks5

}  // proxy

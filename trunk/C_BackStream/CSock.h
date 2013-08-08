#ifndef _H_KHS_CSOCK_20120217
#define _H_KHS_CSOCK_20120217

#include <winsock2.h>
#include <string>



class CSock{

private:
	enum {MAX_BUF=2048};
	WSADATA m_wsaData;
	bool m_IsInitWsaData;

	SOCKET m_hServerSock;
	SOCKET m_hClientSock;
	SOCKADDR_IN m_hServerAddr;
	SOCKADDR_IN m_hClientAddr;
	unsigned m_port;

public:
	char m_buf[MAX_BUF];

public:
	CSock();
	bool BindListen(unsigned int port);
	bool WaitProcess();
	void CloseClient();
	void CloseServer();
	void Close();

	void SendPacket(std::string & sPacket);
	void SendFile(const char * sFile);
	void RecvFile(const char * sFile);

	void Error(const char *sMessage);
	
	~CSock();

};

#endif
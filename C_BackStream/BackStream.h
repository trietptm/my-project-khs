#ifndef _H_KHS_BACKSTREAM
#define _H_KHS_BACKSTREAM

#include "CSock.h"
#include <windows.h>
#include <vector>
#include <string>


typedef struct _Files{
	
	std::string files;
	bool IsFile;
	unsigned int filesize;
}TFiles;

typedef struct _Process{

	std::string process;

}TProcess;

typedef std::vector<TFiles> Files;
typedef std::vector<TProcess> Process;

typedef enum {DIRLIST = 1,PROCESS_LIST, FILE_SEND, FILE_RECV} TYPE;

class CBackStream{

public:
	CSock m_CSock;
	
public:
	std::string m_NowPath;
	std::string m_PacketString;

	Files m_files;
	Process m_process;

public:

	void FillDirFiles(const char *spath);
	void GetDirListPacket();

	void FillProcessList();
	void GetProcessListPacket();

	void CmdProcess();
	bool GetCmdArg(WORD &cmd, std::string &arg);
	

	void SendPacket(WORD type);

	void SendFile(const char *sFile);
	void RecvFile(const char *sFile);

	CBackStream();
	~CBackStream();
	void Error(const char *mes);

};

#endif 
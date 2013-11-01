#ifndef _H_KHS_CVBOXANTITEST
#define _H_KHS_CVBOXANTITEST


#include <Windows.h>

#define VIRTUAL_DRIVER_NAME	"VBOX"
#define IDT_ADDRESS_LIMIT	0x0d000000


#define SIZE_DRIVER_NAME	128

class CVboxAnti	
{
private:

public:
	char m_pszModel[SIZE_DRIVER_NAME];
public:
	CVboxAnti() {}
		
	/********** Start = Utils Func **********/
	void AllToUpper(unsigned char* str,unsigned long len);
	unsigned char* ScanDataForString(unsigned char* data,unsigned long data_length,unsigned char* string2);
	/********** End = Utils func **********/

	/********** Start = VirtualCheck Func **********/
	PSTR GetFirstPhysicalDriveModelNames();
	unsigned int GetIDT();
	bool CheckLDT(unsigned int *a_pLdtBase);
	unsigned long CheckRDTSC();
	bool CheckTss(unsigned int *pAddrTss);
	bool QueryRegedit();
	bool CheckNICMacInfo();
	bool CheckRegSMBios();
	bool CheckBiosWMI();
	bool CheckDXDiagSysInfo();
	bool CheckWindowSetupLog();
	/********** End = VirtualCheck Func **********/


	/********** Start = Virtual TestCase Func **********/
	bool TestCase1();	// # 서명된 드라이버 문자열 탐지
	bool TestCase2();	// # IDT 주소가 위치한 곳을 비교
	bool TestCase3();	// # LDT 주소가 위치한 곳을 비교
	bool TestCase4();	// # RDTSC 시간차이를 비교 탐지
	bool TestCase5();	// # Tss의 값의 차이를 비교 탐지
	bool TestCase6();	// # 등록된 레지스터의 VBOX 문자열 비교 탐지
	bool TestCase7();	// # NIC의 Mac Address 범위를 비교 탐지
	bool TestCase8();	// # Reg에 등록된 SMBios의 Type을 비교 탐지 
	bool TestCase9();	// # Bios Brand Version 비교 탐지 (WMI)
	bool TestCase10();	// # DirectX SysInfo의 문자열 비교 탐지
	bool TestCase11();	// # c:\\windows\\setuplog.txt 이 VBOX 문자열 비교 탐지

	/********** End = Virtual TestCase Func **********/
};

#endif 
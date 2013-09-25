#ifndef _H_KHS_CVBOXANTITEST
#define _H_KHS_CVBOXANTITEST


#include <Windows.h>

#define VIRTUAL_DRIVER_NAME	"VBOX"
//#define VIRTUAL_DRIVER_NAME "APPLE SSD SM0128F"
#define IDT_ADDRESS_LIMIT	0x0d000000


#define SIZE_DRIVER_NAME	128

class CVboxAnti	
{
private:

public:
	char m_pszModel[SIZE_DRIVER_NAME];
public:
	CVboxAnti() {}
	PSTR GetFirstPhysicalDriveModelNames();
	unsigned int GetIDT();
	bool CheckLDT();
	unsigned long CheckRDTSC();
	bool CheckTss();
	bool QueryRegedit();
	bool CheckNICMacInfo();


	bool TestCase1();	// # 서명된 드라이버 문자열 탐지
	bool TestCase2();	// # IDT 주소가 위치한 곳을 비교
	bool TestCase3();	// # LDT 주소가 위치한 곳을 비교
	bool TestCase4();	// # RDTSC 시간차이를 비교 탐지
	bool TestCase5();	// # Tss의 값의 차이를 비교 탐지
	bool TestCase6();	// # 등록된 레지스터의 VBOX 문자열 비교 탐지
	bool TestCase7();	// # NIC의 Mac Address 범위를 비교 탐지
};

#endif 
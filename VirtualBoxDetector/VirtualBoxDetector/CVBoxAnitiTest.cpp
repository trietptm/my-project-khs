#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>

#include "CVBoxAntiTest.h"
#include "VBoxTestDefine.h"
#include <stdio.h>


/********** Start = Utils Func **********/

void CVboxAnti::AllToUpper(unsigned char* str,unsigned long len)
{
        for(unsigned long c=0;c<len;c++)
        {
                if(str[c]>='a' && str[c]<='z')
                {
                        str[c]-=32;
                }
        }
}

unsigned char* CVboxAnti::ScanDataForString(unsigned char* data,unsigned long data_length,unsigned char* string2)
{
        unsigned long string_length=(unsigned long)strlen((char*)string2);
        for(unsigned long i=0;i<=(data_length-string_length);i++)
        {
                if(strncmp((char*)(&data[i]),(char*)string2,string_length)==0) return &data[i];
        }
        return 0;
}
/********** End = Utils Func **********/




/********** Start = VirtualCheck Func **********/

// Code was originally written by Lynn McGuire
// http://www.winsim.com/diskid32/winio/diskid32.cpp

PSTR CVboxAnti::GetFirstPhysicalDriveModelNames()
{
    HANDLE hDrive;
    DWORD dwBytesReturned = 0;
    DWORD dwCnt;
    BYTE bIDInCmd = 0;  
    SENDCMDINPARAMS  Scip;
    BYTE pIDOutCmd[sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1];
    PIDSECTOR pIdSector;
    GETVERSIONOUTPARAMS VersionParams;

    hDrive = CreateFileA("\\\\.\\PhysicalDrive0", 
                         GENERIC_READ | GENERIC_WRITE, 
                         FILE_SHARE_READ | FILE_SHARE_WRITE, 
                         NULL,
                         OPEN_EXISTING, 
                         0, 
                          NULL);
 
    if (hDrive == INVALID_HANDLE_VALUE)
    {
        printf ("Unable to open physical drive 0, error code: 0x%lX\n", GetLastError ());
        goto Error;
    }
 
    memset (&VersionParams, 0, sizeof(VersionParams));
 
    // Get the version and co, of previousliy opened physical drive driver
    if (!DeviceIoControl(hDrive, 
        DFP_GET_VERSION,
        NULL, 
        0,
        &VersionParams,
        sizeof(VersionParams),
        &dwBytesReturned, 
        NULL))
    {         
        printf ("DFP_GET_VERSION failed error code: 0x%lX\n", GetLastError ());
        goto Error;
    }
 
    // Is there bit map of IDE devices?
    if (VersionParams.bIDEDeviceMap > 0)
    {
        // Get ID sector and decide if its a ATAPI or ATA disk
        bIDInCmd = (VersionParams.bIDEDeviceMap & 0x10) ? IDE_ATAPI_IDENTIFY : IDE_ATA_IDENTIFY;
 
        memset (&Scip, 0, sizeof(Scip));
        memset (pIDOutCmd, 0, sizeof(pIDOutCmd));
 
        // Set up data structures for IDENTIFY command
        Scip.cBufferSize = IDENTIFY_BUFFER_SIZE;
        Scip.irDriveRegs.bFeaturesReg = 0;
        Scip.irDriveRegs.bSectorCountReg = 1;
        Scip.irDriveRegs.bSectorNumberReg = 1;
        Scip.irDriveRegs.bCylLowReg = 0;
        Scip.irDriveRegs.bCylHighReg = 0;
 
        // Compute the drive number
        Scip.irDriveRegs.bDriveHeadReg = 0xA0 | (1 << 4);
 
        // The command can either be IDE identify or ATAPI identify
        Scip.irDriveRegs.bCommandReg = bIDInCmd;
        Scip.bDriveNumber = 0;
        Scip.cBufferSize = IDENTIFY_BUFFER_SIZE;
 
        // Get drive data
        if(!DeviceIoControl(hDrive, 
                            DFP_RECEIVE_DRIVE_DATA,
                            (LPVOID) &Scip,
                            sizeof(SENDCMDINPARAMS) - 1,
                            (LPVOID) pIDOutCmd,
                            sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1,
                            &dwBytesReturned, 
                            NULL))
        {
            printf ("DFP_RECEIVE_DRIVE_DATA failed error code: 0x%lX\n", GetLastError ());
            goto Error;
        }
 
        // Point to IDSECTOR
        pIdSector = (PIDSECTOR)((PSENDCMDOUTPARAMS)pIDOutCmd)->bBuffer;
 
        // Exchange every two bytes of sModelNumber 
        for(dwCnt = 0; dwCnt < sizeof(pIdSector->sModelNumber); dwCnt+=2)
        {
            m_pszModel[dwCnt+1] = pIdSector->sModelNumber[dwCnt];
            m_pszModel[dwCnt] = pIdSector->sModelNumber[dwCnt+1];
        }
 
        // Add ending
        m_pszModel[dwCnt] = '\0';
 
        return CharUpperA(m_pszModel);
    }
 
Error:
    //free(pszModel);
    if(hDrive)
        CloseHandle(hDrive);
    return NULL;
}


unsigned int CVboxAnti::GetIDT()
{
	unsigned char idtr[6] = {0};
	unsigned int retval = 0;

	_asm
		sidt idtr

	memcpy(&retval,&idtr[2],4);

	return retval;
}

bool CVboxAnti::CheckLDT()
{
	unsigned char m[4];

	__asm
		sldt m;	

	if(m[0] == 0x00 && m[1] == 0x00)
		return false;
	else
		return true;
}

unsigned long CVboxAnti::CheckRDTSC()
{
	unsigned long a,b,c;
	__asm
	{
		push eax
		push ebx

		rdtsc
		mov b, eax
		mov ebx, eax
		rdtsc
		mov c, eax
		sub eax, ebx
		mov a, eax	

		pop eax
		pop ebx
		
	}
	
	return a;
}


bool CVboxAnti::CheckTss()
{
	unsigned char mem[4] = {0,0,0,0};

	__asm
		str mem;

	if((mem[0] == 0x40) && (mem[1] == 0x00))
		return false;
	else
		return true;
}

bool CVboxAnti::QueryRegedit()
{
	HKEY hKey = NULL;
	LONG retu = 0;
	bool bResult = false;

	retu = RegOpenKeyExA(HKEY_LOCAL_MACHINE,"HARDWARE\\ACPI\\DSDT\\VBOX__",0,KEY_READ,&hKey);
	

	if(retu == ERROR_SUCCESS)	
	{
		//printf("Detector RegKey : HARDWARE\\ACPI\\DSDT\\VBOX__\n");
		bResult = true;
	}

	RegCloseKey(hKey);

	hKey = NULL;

	retu = RegOpenKeyExA(HKEY_LOCAL_MACHINE,"HARDWARE\\DESCRIPTION\\System",0,KEY_READ,&hKey);

	char sData[128] = {0};
	DWORD dwSize = sizeof(sData);

	if(retu == ERROR_SUCCESS)
	{
		retu = RegQueryValueExA(hKey,"SystemBiosVersion",NULL,NULL,(LPBYTE)sData,&dwSize);
		
		if(retu == ERROR_SUCCESS)
		{
			if(strstr(sData,"VBOX"))
			{
				//printf("Detector RegKey : HARDWARE\\DESCRIPTION\\System - SystemBiosVersion\n");
				bResult = true;
			}
		}


		memset(sData,0,sizeof(sData));
		dwSize = sizeof(sData);

		retu = RegQueryValueExA(hKey,"VideoBiosVersion",NULL,NULL,(LPBYTE)sData,&dwSize);

		if(retu == ERROR_SUCCESS)
		{
			if(strstr(sData,"VirtualBox"))
				{
					//printf("Detector RegKey : HARDWARE\\DESCRIPTION\\System - VideoBiosVersion\n");
					bResult = true;
				}
		}
	}

	RegCloseKey(hKey);

	memset(sData,0,sizeof(sData));
	dwSize = sizeof(sData);

	retu = RegOpenKeyExA(HKEY_LOCAL_MACHINE,"HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 0\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0",0,KEY_READ,&hKey);

	if(retu == ERROR_SUCCESS)
	{
		retu = RegQueryValueExA(hKey,"Identifier",NULL,NULL,(LPBYTE)sData,&dwSize);
		if(retu == ERROR_SUCCESS)
		{
			if(strstr(sData,"VBOX"))
			{
				//printf("Detector RegKey : HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 0\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0\n");
				bResult = true;
			}
		}
	}

	retu = RegOpenKeyExA(HKEY_LOCAL_MACHINE,"HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 1\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0",0,KEY_READ,&hKey);

	memset(sData,0,sizeof(sData));
	dwSize = sizeof(sData);

	if(retu == ERROR_SUCCESS)
	{
		
		retu = RegQueryValueExA(hKey,"Identifier",NULL,NULL,(LPBYTE)sData,&dwSize);

		if(retu == ERROR_SUCCESS)
		{
			if(strstr(sData,"VBOX"))
			{
				//printf("Detector RegKey : HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 1\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0\n");
				bResult = true;
			}
		}
	}
	

	RegCloseKey(hKey);

	return bResult;
}

#pragma comment(lib,"IPHLPAPI.lib")
#pragma comment(lib,"ws2_32.lib")

//VirtualBox Adapters (Host and Guests) always have their MAC addresses in the form of 08-00-27-??-??-??. This range was originally assigned to Cadmus Computer Systems.
bool CVboxAnti::CheckNICMacInfo()
{
	bool bResult = false;

        WSADATA WSD;
        if(!WSAStartup(MAKEWORD(2,2),&WSD))
        {
                unsigned long tot_size=0;
                int ret=GetAdaptersAddresses(AF_UNSPEC,GAA_FLAG_INCLUDE_PREFIX,0,0,&tot_size);
                if(ret==ERROR_BUFFER_OVERFLOW)
                {
                        IP_ADAPTER_ADDRESSES* px=(IP_ADAPTER_ADDRESSES*)LocalAlloc(LMEM_ZEROINIT,tot_size);
                        if(px)
                        {
                                ret=GetAdaptersAddresses(AF_UNSPEC,GAA_FLAG_INCLUDE_PREFIX,0,px,&tot_size);
                                IP_ADAPTER_ADDRESSES* pxx=px;
                                //Traverse a singly-linked list
                                for(pxx;pxx;pxx=pxx->Next)
                                {
                                        if(pxx->PhysicalAddressLength==0x6)
                                        {
                                                if(wcsicmp(pxx->FriendlyName,L"VirtualBox Host-Only Network"))  //We don't want to detect the HOST OS
                                                {
                                                    char xx[0x6]={0};
                                                    memcpy(xx,pxx->PhysicalAddress,0x6);
                                                        if(xx[0]==0x08&& xx[1]==0x00 && xx[2]==0x27) //Cadmus Computer Systems Mac address
                                                        {
																bResult = true;
                                                               // MessageBox(0,L"VirtualBox detected",L"waliedassar",0);
                                                        }
                                                }
                                        }
                                }
                                LocalFree(px);
                        }
                }
                WSACleanup();
        }

	return bResult;
}

bool CVboxAnti::CheckRegSMBios()
{
	bool bResult = false;
    HKEY hk=0;
    int ret=RegOpenKeyExA(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Services\\mssmbios\\data",0,KEY_ALL_ACCESS,&hk);

    if(ret==ERROR_SUCCESS)
    {
            unsigned long type=0;
            unsigned long length=0;
            ret=RegQueryValueExA(hk,"SMBiosData",0,&type,0,&length);
            if(ret==ERROR_SUCCESS)
            {
                    if(length)
                    {
                            char* p=(char*)LocalAlloc(LMEM_ZEROINIT,length);
                            if(p)
                            {
                                    ret=RegQueryValueExA(hk,"SMBiosData",0,&type,(unsigned char*)p,&length);
                                    if(ret==ERROR_SUCCESS)
                                    {
                                            //--------------------------Only when parsing SMBiosData retrieved from Registry------------------
                                            unsigned long new_length=((BIOS_DATA_HEAD*)p)->length;  //length-8
                                            p+=0x8;
                                            //printf("Length is: %X\r\n",new_length);
                                            //------------------------------------------------------------------------------------------------
                                            unsigned long i=0;
                                            while(i<new_length)
                                            {
                                                    unsigned char type=((HeadER*)(p+i))->Type;
                                                    //PrintType(type);
                                                    unsigned char section_size=((HeadER*)(p+i))->section_length;
                                                    //printf("Section length is: %X\r\n",section_size);
                                                    unsigned short handles=((HeadER*)(p+i))->handles;
                                                    //printf("Handle is: %X\r\n",handles);
 
                                                    if(type==0x7F) break; //End-Of-Table
 
                                                    if(type==TYPE_INACTIVE) //0x7E
                                                    {
                                                        //PrintString(p+i+section_size,*(p+i+4));   //print Brand
                                                        //PrintString(p+i+section_size,*(p+i+5));   //print Version
                                                            //MessageBoxA(0,"VirtualBox detected","waliedassar",0);
														bResult = true;
														return bResult;
                                                    }
                                                    //---Get End of Structure--------------
                                                    unsigned char* pxp=(unsigned char*)p+i+section_size;
                                                    while(*(unsigned short*)pxp!=0) pxp++;
                                                    pxp++;
                                                    pxp++;
                                                    //-------------------------------------
                                                    i=(pxp-((unsigned char*)p));
                                            }
                                    }
                                    LocalFree(p);
                            }
                    }
            }
            RegCloseKey(hk);
    }
    

	return bResult;
}

#include <comdef.h>
#include <Wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")

bool CVboxAnti::CheckBiosWMI()
{
	
    BSTR rootwmi=SysAllocString(L"root\\wmi");
    BSTR tables=SysAllocString(L"MSSmBios_RawSMBiosTables");
	BSTR biosdata=SysAllocString(L"SMBiosData");
 
    HRESULT hr=CoInitializeEx(0, COINIT_MULTITHREADED);
    if(!SUCCEEDED(hr)) return false;
    IWbemLocator* pLoc=0;
    hr=CoCreateInstance(CLSID_WbemLocator,0,CLSCTX_INPROC_SERVER,IID_IWbemLocator,(void**)&pLoc);
    if(!SUCCEEDED(hr))
    {
		CoUninitialize();
		return false;
    }

    IWbemServices* pSvc=0;
    hr=pLoc->ConnectServer(rootwmi,0 ,0 ,0 ,0,0,0,&pSvc);

    if(!SUCCEEDED(hr))
	{
            pLoc->Release();    
			CoUninitialize();
			return false;
	}

	hr=CoSetProxyBlanket(pSvc,RPC_C_AUTHN_WINNT,RPC_C_AUTHZ_NONE,0,RPC_C_AUTHN_LEVEL_CALL,RPC_C_IMP_LEVEL_IMPERSONATE,0,EOAC_NONE);
    if(!SUCCEEDED(hr))
    {
		
        pSvc->Release();
		pLoc->Release();    
		CoUninitialize();
		return false;
    }
 
	IEnumWbemClassObject* pEnum=0;	
	hr=pSvc->CreateInstanceEnum(tables,0,0, &pEnum);
    
	if(!SUCCEEDED(hr))
    {
        pSvc->Release();
		pLoc->Release();    
		CoUninitialize();
		return false;
    }
 
    IWbemClassObject* pInstance=0;
    unsigned long Count=0;
    hr=pEnum->Next(WBEM_INFINITE,1,&pInstance,&Count);
        
	if(SUCCEEDED(hr))
    {              
			VARIANT BiosData;
			VariantInit(&BiosData);
			CIMTYPE type;
			hr=pInstance->Get(biosdata,0,&BiosData,&type,NULL);
			if(SUCCEEDED(hr))
			{
				SAFEARRAY* p_array = NULL;
				p_array = V_ARRAY(&BiosData);
				unsigned char* p_data=(unsigned char *)p_array->pvData;
				unsigned long length=p_array->rgsabound[0].cElements;
				AllToUpper(p_data,length);
				unsigned char* x1=ScanDataForString((unsigned char*)p_data,length,(unsigned char*)"INNOTEK GMBH");
				unsigned char* x2=ScanDataForString((unsigned char*)p_data,length,(unsigned char*)"VIRTUALBOX");
				unsigned char* x3=ScanDataForString((unsigned char*)p_data,length,(unsigned char*)"SUN MICROSYSTEMS");
				unsigned char* x4=ScanDataForString((unsigned char*)p_data,length,(unsigned char*)"VIRTUAL MACHINE");
				unsigned char* x5=ScanDataForString((unsigned char*)p_data,length,(unsigned char*)"VBOXVER");
				
				if(x1 || x2 || x3 || x4 || x5)
				{
					pSvc->Release();
					pLoc->Release();    
					CoUninitialize();
					return true;
					//printf("VirtualBox detected\r\n");
					//printf("Some Strings found:\r\n");
					if(x1) printf("%s\r\n",x1);
					if(x2) printf("%s\r\n",x2);
					if(x3) printf("%s\r\n",x3);
					if(x4) printf("%s\r\n",x4);
					if(x5) printf("%s\r\n",x5);
				}
			}
			VariantClear(&BiosData);
			pInstance->Release();
        }
    pSvc->Release();
    pLoc->Release();    
    CoUninitialize();

out:
	return false;
}
#include <initguid.h>
#include "DXDIAG\\INCLUDE\\dxdiag.h"

bool CVboxAnti::CheckDXDiagSysInfo()
{
    HRESULT hr=CoInitialize(0);

    if(!SUCCEEDED(hr)) 
	{
		printf("CoInitializeEx Error\n");
		return false;
	}
	IDxDiagProvider* pProvider = NULL;
    hr=CoCreateInstance(CLSID_DxDiagProvider,0,CLSCTX_INPROC_SERVER,IID_IDxDiagProvider,(void**)&pProvider );
	
	if(!SUCCEEDED(hr))
     {
        CoUninitialize();
        return false;
     }
    DXDIAG_INIT_PARAMS InitParams={0};
    InitParams.dwSize=sizeof(DXDIAG_INIT_PARAMS);
    InitParams.dwDxDiagHeaderVersion=DXDIAG_DX9_SDK_VERSION;
    InitParams.bAllowWHQLChecks=false;
    hr=pProvider->Initialize(&InitParams);

    if(SUCCEEDED(hr))
        {
          IDxDiagContainer* pDxDiagRoot=0;
          IDxDiagContainer* pDxDiagSystemInfo=0;
          hr=pProvider->GetRootContainer(&pDxDiagRoot );
          if(SUCCEEDED(hr))
          {
                hr=pDxDiagRoot->GetChildContainer( L"DxDiag_SystemInfo", &pDxDiagSystemInfo );
                if(SUCCEEDED(hr) )
                {
                       VARIANT varX;
                                           hr=pDxDiagSystemInfo->GetProp( L"szSystemManufacturerEnglish",&varX);
                       if( SUCCEEDED(hr)&&varX.vt==VT_BSTR && SysStringLen(varX.bstrVal)!=0)
                       {
                                                   wchar_t* pMan=varX.bstrVal;
                           //wprintf(L"System Manufacturer is %s\r\n",pMan);
                                                   if(!_wcsicmp(pMan,L"innotek GmbH"))
                                                   {
                                                           //printf("VirtualBox detected\r\n");
															pDxDiagRoot->Release();
															pProvider->Release();
															CoUninitialize();
															return true;
                                                   }
                                       }
                       VariantClear(&varX);
                                           hr=pDxDiagSystemInfo->GetProp( L"szSystemModelEnglish",&varX);
                       if( SUCCEEDED(hr)&&varX.vt==VT_BSTR && SysStringLen(varX.bstrVal)!=0)
                       {
                                                   wchar_t* pMan=varX.bstrVal;
                          // wprintf(L"System Model is %s\r\n",pMan);
                                                   if(!_wcsicmp(pMan,L"VirtualBox"))
                                                   {
                                                          // printf("VirtualBox detected\r\n");
														pDxDiagRoot->Release();
														pProvider->Release();
														CoUninitialize();
													   return true;
                                                   }
                                       }
                       VariantClear(&varX);
                
                }
                
        }
       
    }

	//pDxDiagRoot->Release();
	pProvider->Release();
    CoUninitialize();
    return false;
}

#include <string>

bool CVboxAnti::CheckWindowSetupLog()
{

	using namespace std;
	FILE *fp = NULL;

	fp = fopen("c:\\windows\\setuplog.txt","rb");

	if(!fp)		return false;
	else
	{
		long lSize = 0;
		char *buffer = NULL;
		size_t result = 0;

		fseek(fp,0,SEEK_END);
		lSize = ftell(fp);
		fseek(fp,0,SEEK_SET);

		buffer = (char*)malloc(lSize);

		if(buffer == NULL)
		{
			printf("CheckWidnowSetupLog - malloc Func Error\n");
			fclose(fp);
			return false;
		}
		result = fread(buffer,1,lSize,fp);

		
		string s = buffer;


		if(s.find("VBOX") != string::npos)
		{
			free(buffer);
			fclose(fp);
			return true;
		}
		else{
			free(buffer);
			fclose(fp);
			return false;
		}
	}
	
	return false;
}
/********** End = VirtualCheck Func **********/



/********** Start = Virtual TestCase Func **********/

bool CVboxAnti::TestCase1()
{
	bool bResult;
	GetFirstPhysicalDriveModelNames();

	if(strstr(m_pszModel,VIRTUAL_DRIVER_NAME))
	{
		printf("[*] TestCase1 - Detect VirtualBox - DriverName =  [ YES ]\n");
		bResult = true;
	}
	else
	{
		printf("[*] TestCase1 - Detect VirtualBox - DriverName =  [ NO ]\n");
		bResult = false;
	}

	return bResult;
}

bool CVboxAnti::TestCase2()
{
	unsigned int iAddrIDT = 0;
	bool bResult;

	iAddrIDT = GetIDT();

	if(iAddrIDT > IDT_ADDRESS_LIMIT)
	{
		printf("[*] TestCase2 - Detect VirtualBox - IDTAddress =  [ YES ]\n");
		bResult = true;
	}
	else
	{
		printf("[*] TestCase2 - Detect VirtualBox - IDTAddress =  [ NO ]\n");
		bResult = false;
	}

	return bResult;
}

bool CVboxAnti::TestCase3()
{
	bool bResult = CheckLDT();

	if(bResult)
	{
		printf("[*] TestCase3 - Detect VirtualBox - LDTAddress =  [ YES ]\n");
		bResult = true;
	}
	else
	{
		printf("[*] TestCase3 - Detect VirtualBox - LDTAddress =  [ NO ]\n");
		bResult = false;
	}
	return bResult;
}

bool CVboxAnti::TestCase4()
{
	unsigned long ctime = 0;
	bool bResult;
	
	ctime = CheckRDTSC();

	if(ctime > 512)
	{
		printf("[*] TestCase4 - Detect VirtualBox - RDTSC Diff =  [ YES ]\n");
		bResult = true;
	}
	else
	{
		printf("[*] TestCase4 - Detect VirtualBox - RDTSC Diff =  [ NO ]\n");
		bResult = false;
	}

	return bResult;
}

bool CVboxAnti::TestCase5()
{
	bool bResult;

	bResult = CheckTss();

	if(bResult)
	{
		printf("[*] TestCase5 - Detect VirtualBox - TSSAddress =  [ YES ]\n");
		bResult = true;
	}
	else
	{
		printf("[*] TestCase5 - Detect VirtualBox - TSSAddress =  [ NO ]\n");
		bResult = false;
	}

	return bResult;
}

bool CVboxAnti::TestCase6()
{
	bool bResult;

	bResult = QueryRegedit();

	if(bResult)
	{
		printf("[*] TestCase6 - Detect VirtualBox - RegString  =  [ YES ]\n");
		bResult = true;
	}	
	else
	{
		printf("[*] TestCase6 - Detect VirtualBox - RegString  =  [ NO ]\n");
		bResult = false;
	}

	return bResult;
}

bool CVboxAnti::TestCase7()
{
	bool bResult;

	bResult = CheckNICMacInfo();

	if(bResult)
	{
		printf("[*] TestCase7 - Detect VirtualBox - NICMacInfo =  [ YES ]\n");
		bResult = true;
	}
	else
	{
		printf("[*] TestCase7 - Detect VirtualBox - NICMacInfo =  [ NO ]\n");
		bResult = false;
	}

	return bResult;
}

bool CVboxAnti::TestCase8()
{
	bool bResult;

	bResult = CheckRegSMBios();

	if(bResult)
	{
		printf("[*] TestCase8 - Detect VirtualBox - RegSMBiosType =  [ YES ]\n");
		bResult = true;
	}
	else
	{
		printf("[*] TestCase8 - Detect VirtualBox - RegSMBiosType =  [ NO ]\n");
		bResult = false;
	}

	return bResult;
}

bool CVboxAnti::TestCase9()
{
	bool bResult;

	bResult = CheckBiosWMI();

	if(bResult)
	{
		printf("[*] TestCase9 - Detect VirtualBox - BiosVersionWMI =  [ YES ]\n");
		bResult = true;
	}
	else
	{
		printf("[*] TestCase9 - Detect VirtualBox - BiosVersionWMI =  [ NO ]\n");
		bResult = false;
	}

	return bResult;
}

bool CVboxAnti::TestCase10()
{
	bool bResult;

	bResult = CheckDXDiagSysInfo();

	if(bResult)
	{
		printf("[*] TestCase10 - Detect VirtualBox - DXDiagSysInfo =  [ YES ]\n");
		bResult = true;
	}
	else
	{
		printf("[*] TestCase10 - Detect VirtualBox - DXDiagSysInfo =  [ NO ]\n");
		bResult = false;
	}

	return bResult;
}

bool CVboxAnti::TestCase11()
{
	bool bResult = false;

	bResult = CheckWindowSetupLog();	

	if(bResult)
	{
		printf("[*] TestCase11 - Detect VirtualBox - setuplog.txt =  [ YES ]\n");
		bResult = true;
	}
	else
	{
		printf("[*] TestCase11 - Detect VirtualBox - setuplog.txt =  [ NO ]\n");
		bResult = false;
	}
	return bResult;
}

/********** End = Virtual TestCase Func **********/
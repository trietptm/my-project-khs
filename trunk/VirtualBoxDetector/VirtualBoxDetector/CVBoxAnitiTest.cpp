#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>

#include "CVBoxAntiTest.h"
#include "VBoxTestDefine.h"
#include <stdio.h>


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
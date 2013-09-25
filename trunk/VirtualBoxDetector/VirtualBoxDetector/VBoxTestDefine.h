#ifndef _H_KHS_VBOXTESTDEFINE
#define _H_KHS_VBOXTESTDEFINE


/*************** Start = GetFirstPhysicalDriveModelNames ********************************/

// Valid values for the bCommandReg member of IDEREGS.    
#define  IDE_ATAPI_IDENTIFY  0xA1  // Returns ID sector for ATAPI.    
#define  IDE_ATA_IDENTIFY    0xEC  // Returns ID sector for ATA.    


// IOCTL commands    
#define  DFP_GET_VERSION          0x00074080    
#define  DFP_SEND_DRIVE_COMMAND   0x0007c084    
#define  DFP_RECEIVE_DRIVE_DATA   0x0007c088    

// GETVERSIONOUTPARAMS contains the data returned from the    
// Get Driver Version function.    
typedef struct _GETVERSIONOUTPARAMS    
{    
    BYTE bVersion;      // Binary driver version.    
    BYTE bRevision;     // Binary driver revision.    
    BYTE bReserved;     // Not used.    
    BYTE bIDEDeviceMap; // Bit map of IDE devices.    
    DWORD fCapabilities; // Bit mask of driver capabilities.    
    DWORD dwReserved[4]; // For future use.    
}GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS; 

typedef struct _IDSECTOR    
{    
    USHORT  wGenConfig;    
    USHORT  wNumCyls;    
    USHORT  wReserved;    
    USHORT  wNumHeads;    
    USHORT  wBytesPerTrack;    
    USHORT  wBytesPerSector;    
    USHORT  wSectorsPerTrack;    
    USHORT  wVendorUnique[3];    
    CHAR    sSerialNumber[20];    
    USHORT  wBufferType;    
    USHORT  wBufferSize;    
    USHORT  wECCSize;    
    CHAR    sFirmwareRev[8];    
    CHAR    sModelNumber[40];    
    USHORT  wMoreVendorUnique;    
    USHORT  wDoubleWordIO;    
    USHORT  wCapabilities;    
    USHORT  wReserved1;    
    USHORT  wPIOTiming;    
    USHORT  wDMATiming;    
    USHORT wBS;    
    USHORT  wNumCurrentCyls;    
    USHORT  wNumCurrentHeads;    
    USHORT  wNumCurrentSectorsPerTrack;    
    ULONG   ulCurrentSectorCapacity;    
    USHORT  wMultSectorStuff;    
    ULONG   ulTotalAddressableSectors;    
    USHORT  wSingleWordDMA;    
    USHORT  wMultiWordDMA;    
    BYTE    bReserved[128];    
}IDSECTOR, *PIDSECTOR;

/*************** End = GetFirstPhysicalDriveModelNames ***************/

#endif
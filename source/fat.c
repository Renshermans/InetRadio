/****************************************************************************
*  This file is part of the AVRIDE device driver.
*
*  Copyright (c) 2002-2004 by Michael Fischer. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without 
*  modification, are permitted provided that the following conditions 
*  are met:
*  
*  1. Redistributions of source code must retain the above copyright 
*     notice, this list of conditions and the following disclaimer.
*  2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the 
*     documentation and/or other materials provided with the distribution.
*  3. Neither the name of the author nor the names of its contributors may 
*     be used to endorse or promote products derived from this software 
*     without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
*  THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
*  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
*  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF 
*  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
*  SUCH DAMAGE.
*
****************************************************************************
*  History:
*
*  14.12.02  mifi   First Version 
*  23.12.02  mifi   Add FileOpen, FileClose, FileError, FileSize,
*                   FileSeek and FileRead. But the FileSeek function.
*                   does not work in the moment, later...
*  28.12.02  mifi   Now support FAT16 AND FAT32.
*  01.01.03  mifi   Support long directory entries, but without to 
*                   check the checksum of the short entry.
*                   Change FAT32FileSize return value from int to long.
*                   The max size of a long filename segment is
*                   (FAT_LONG_NAME_LEN-1). But the complete filename can
*                   be longer. 
*
*                   segment1/segment2/segment3/index.html
*
*                   segmentX max length = (FAT_LONG_NAME_LEN-1)
*
*  04.01.03  mifi   Take a look at the return values...
*  18.01.03  mifi   Change Licence from GPL to BSD.
*  25.01.03  mifi   Implement a new FindFile function.
*                   I have some trouble with short file names under
*                   Win98. Win98 store a short name like "enlogo.gif"
*                   as a long name, nasty OS. 
*                   Remove FAT32_MAX_FILES and the array aFileHandle,
*                   a file handle will now be allocated by NutHeapAlloc, 
*                   therefore we have no restrictions about the count of 
*                   the open file handle. (Only by available memory)
*  27.01.03  mifi   Rename all FAT32xxx function to FATxxx.
*
*  28.01.03  mifi   Start porting to Nut/OS 3.X.X
*  19.06.03  mifi   Change the call of IDEInit, now we use the BaseAddress
*                   of 0. Because the fat module does not need to know the
*                   address. It will be handled in ide.c.
*  29.06.03  mifi   First ATAPI-Version
*                   Now we can read files from a CD-ROM. But there exist
*                   some open points:
*                   - only first session from a multisession CD is supported
*                   - only iso9660, no Joliet support now, later
*  24.07.04  mifi   Some changes to support more than one drive (datanut)
*  25.07.04  mifi   Add support for the PCMCIA_11 hardware.
*  27.07.04  mifi   Start to add functionality for write function.
*  03.08.04  mifi   Add format function.
****************************************************************************/
#define __FAT_C__

#define LOG_MODULE  LOG_FAT_MODULE

#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <time.h>

#include <sys/heap.h>
#include <sys/event.h>
#include <sys/thread.h>

#include <sys/device.h>

#include "typedefs.h"

#include "fat.h"
#include "fatdrv.h"
#include "portio.h"
#include "log.h"


/*==========================================================*/
/*  DEFINE: All Structures and Common Constants             */
/*==========================================================*/
//
// FAT return codes
//
#define FAT_OK                0
#define FAT_ERROR             -1
#define FAT_ERROR_EOF         -2
#define FAT_ERROR_IDE         -3

//
// Define for correct return values Nut/OS
//
#define NUTDEV_OK                       0
#define NUTDEV_ERROR                    -1
#define NUTDEV_WRONG_HW                 -2

#define FAT_MAX_DRIVE                   3

//
// Some defines for the FAT structures
//
#define ZIP_DRIVE_BR_SECTOR             32

#define BPB_RsvdSecCnt                  32
#define BPB_NumFATs                     2
#define BPB_HiddSec                     63

#define FAT32_MEDIA                     0xf8
#define FAT32_SECTOR_SIZE               HW_SECTOR_SIZE

#define FAT32_OFFSET_FSINFO             1
#define FAT32_OFFSET_BACKUP_BOOT        6

#define FAT16_CLUSTER_EOF               0x0000FFFF
#define FAT16_CLUSTER_ERROR             0x0000FFF7
#define FAT16_CLUSTER_MASK              0x0000FFFF

#define FAT32_CLUSTER_EOF               0x0FFFFFFF
#define FAT32_CLUSTER_ERROR             0x0FFFFFF7
#define FAT32_CLUSTER_MASK              0x0FFFFFFF

#define FAT_SIGNATURE                   0xAA55

#define MBR_SIGNATURE                   FAT_SIGNATURE
#define MBR_FAT32                       0x0B

#define FSINFO_FIRSTSIGNATURE           0x41615252
#define FSINFO_FSINFOSIGNATURE          0x61417272
#define FSINFO_SIGNATURE                FAT_SIGNATURE

#define DIRECTORY_ATTRIBUTE_READ_ONLY   0x01
#define DIRECTORY_ATTRIBUTE_HIDDEN      0x02
#define DIRECTORY_ATTRIBUTE_SYSTEM_FILE 0x04
#define DIRECTORY_ATTRIBUTE_VOLUME_ID   0x08
#define DIRECTORY_ATTRIBUTE_DIRECTORY   0x10
#define DIRECTORY_ATTRIBUTE_ARCHIVE     0x20

//
// DIRECTORY_ATTRIBUTE_READ_ONLY   |
// DIRECTORY_ATTRIBUTE_HIDDEN      |
// DIRECTORY_ATTRIBUTE_SYSTEM_FILE |
// DIRECTORY_ATTRIBUTE_VOLUME_ID
//
#define DIRECTORY_ATTRIBUTE_LONG_NAME   0x0F

//
// DIRECTORY_ATTRIBUTE_READ_ONLY   |
// DIRECTORY_ATTRIBUTE_HIDDEN      |
// DIRECTORY_ATTRIBUTE_SYSTEM_FILE |
// DIRECTORY_ATTRIBUTE_VOLUME_ID   |
// DIRECTORY_ATTRIBUTE_DIRECTORY   |
// DIRECTORY_ATTRIBUTE_ARCHIVE
// 
#define DIRECTORY_ATTRIBUTE_LONG_NAME_MASK  0x3F

#define FAT_NAME_LEN                    8
#define FAT_EXT_LEN                     3

//
// FAT_SHORT_NAME_LEN name len = 
// name + ext + 1 for the point
//
#define FAT_SHORT_NAME_LEN              (FAT_NAME_LEN+FAT_EXT_LEN+1)
#define FAT_LONG_NAME_LEN               64


//
// Some stuff for HD and CD, DRIVE_INFO Flags
// 
//
#define FLAG_FAT_IS_CDROM               0x0001
#define FLAG_FAT_IS_ZIP                 0x0002

//
//  DiskSize to SectorPerCluster table
//
typedef struct
{
    DWORD DiskSize;
    BYTE  SecPerClusVal;
} DSKSZTOSECPERCLUS;

typedef struct _FAT32FileDataTime
{
    unsigned Seconds:5;
    unsigned Minute:6;
    unsigned Hour:5;
    unsigned Day:5;
    unsigned Month:4;
    unsigned Year:7;
} FAT32_FILEDATETIME, *PFAT32_FILEDATETIME;

typedef struct _FAT32DirectoryEntry
{
    BYTE               Name[FAT_NAME_LEN];
    BYTE               Extension[FAT_EXT_LEN];
    BYTE               Attribute;
    BYTE               Reserved[8];
    WORD               HighCluster;
    FAT32_FILEDATETIME Date;
    WORD               LowCluster;
    DWORD              FileSize;
} FAT32_DIRECTORY_ENTRY;

typedef struct _FAT32DirectoryEntryLong
{
    BYTE Order;
    WORD Name1[5];
    BYTE Attribute;
    BYTE Type;
    BYTE Chksum;
    WORD Name2[6];
    WORD LowCluster;
    WORD Name3[2];
} FAT32_DIRECTORY_ENTRY_LONG;

typedef struct _FAT32FileSystemInformation
{
    DWORD FirstSignature;
    BYTE  Reserved1[480];
    DWORD FSInfoSignature;
    DWORD NumberOfFreeClusters;
    DWORD MostRecentlyAllocatedCluster;
    BYTE  Reserved2[12];
    BYTE  Reserved3[2];
    WORD  Signature;
} FAT32_FSINFO;

typedef struct _FAT32PartitionEntry
{
    BYTE  BootInd;
    BYTE  FirstHead;
    BYTE  FirstSector;
    BYTE  FirstTrack;
    BYTE  FileSystem;
    BYTE  LastHead;
    BYTE  LastSector;
    BYTE  LastTrack;
    DWORD StartSectors;
    DWORD NumSectors;
} FAT32_PARTITION_ENTRY;

typedef struct _FAT32PartionTable
{
    BYTE                  LoadInstruction[446];
    FAT32_PARTITION_ENTRY Partition[4];
    WORD                  Signature;               /* AA55 */
} FAT32_PARTITION_TABLE;

typedef struct _bpbfat16
{
    BYTE  DrvNum;
    BYTE  Reserved1;
    BYTE  BootSig;
    DWORD VollID;
    BYTE  VolLab[11];
    BYTE  FilSysType[8];
    BYTE  Reserved2[28];
} BPBFAT16;

typedef struct _bpbfat32
{
    DWORD FATSz32;          // xxx
    WORD  ExtFlags;         // 0
    WORD  FSVer;            // must 0
    DWORD RootClus;         // 
    WORD  FSInfo;           // typically 1
    WORD  BkBootSec;        // typically 6
    BYTE  Reserved[12];     // set all to zero
    BYTE  DrvNum;           // must 0x80
    BYTE  Reserved1;        // set all to zero
    BYTE  BootSig;          // must 0x29 
    DWORD VollID;           // xxx
    BYTE  VolLab[11];       // "abcdefghijk"
    BYTE  FilSysType[8];    // "FAT32   "
} BPBFAT32;

typedef union _bpboffset36
{
    BPBFAT16 FAT16;
    BPBFAT32 FAT32;
} BPBOFFSET36;

typedef struct _FAT32BootRecord
{
    BYTE        JumpBoot[3];    // 0xeb, 0x58, 0x90
    BYTE        OEMName[8];     // "MSWIN4.1"
    WORD        BytsPerSec;     // must 512
    BYTE        SecPerClus;     // 8 for 4K cluster
    WORD        RsvdSecCnt;     // typically 32 for FAT32
    BYTE        NumFATs;        // always 2
    WORD        RootEntCnt;     // must 0 for FAT32
    WORD        TotSec16;       // must 0 for FAT32
    BYTE        Media;          // must 0xf8

    WORD        FATSz16;        // 0   for FAT32
    WORD        SecPerTrk;      // 63  for FAT32
    WORD        NumHeads;       // 255 for FAT32
    DWORD       HiddSec;        // 63  for FAT32

    DWORD       TotSec32;       // xxx

    BPBOFFSET36 Off36;

    BYTE        Reserved[420];
    WORD        Signature;      // must 0xAA55
} FAT32_BOOT_RECORD, *PFAT32_BOOT_RECORD;

typedef struct _fat_entry_table16
{
    WORD aEntry[256];
} FAT_ENTRY_TABLE16;

typedef struct _fat_entry_table32
{
    DWORD aEntry[128];
} FAT_ENTRY_TABLE32;

typedef union _fat_dir_table
{
    FAT32_DIRECTORY_ENTRY      aShort[16];
    FAT32_DIRECTORY_ENTRY_LONG aLong[16];
} FAT_DIR_TABLE;

typedef struct _drive_info
{
    BYTE  bIsFAT32;
    BYTE  bDevice;
    BYTE  bSectorsPerCluster;
    BYTE  bFlags;

    WORD  wSectorSize;

    DWORD dwRootDirSectors;
    DWORD dwFirstRootDirSector;

    DWORD dwRootCluster;
    DWORD dwFAT1StartSector;
    DWORD dwFAT2StartSector;
    DWORD dwCluster2StartSector;

    DWORD dwClusterSize;
} DRIVE_INFO;

typedef struct _fhandle
{
    DWORD      dwFileSize;
    DWORD      dwStartCluster;
    DWORD      dwReadCluster;
    DWORD      dwFilePointer;          /* total file pointer   */
    DWORD      dwClusterPointer;       /* cluster read pointer */

    int         nLastError;
    int         nEOF;

    DRIVE_INFO *pDrive;
} FHANDLE;

static int QuickFormat(NUTDEVICE *dev, DRIVE_INFO *pDrive);

/*==========================================================*/
/*  DEFINE: Definition of all local Data                    */
/*==========================================================*/
static int        nIsInit = FALSE;

static BYTE      *pSectorBuffer = NULL;
static char      *pLongName1 = NULL;
static char      *pLongName2 = NULL;
static DRIVE_INFO sDriveInfo[FAT_MAX_DRIVE];

static HANDLE hFATSemaphore;

static DSKSZTOSECPERCLUS DskTableFAT32[] = {
    {      66600,  0}, /* disks up to 32.5MB, the 0 value for SecPerClusVal trips an error */
    {     532480,  1}, /* disks up to 260 MB, 0.5k cluster */
    {   16777216,  8}, /* disks up to   8 GB,   4k cluster */
    {   33554432, 16}, /* disks up to  16 GB,   8k cluster */
    {   67108864, 32}, /* disks up to  32 GB,  16k cluster */
    { 0xFFFFFFFF, 64}  /* disks greather than 32GB, 32k cluster */
};

#define DSK_TABLE_FAT32_ENTRY_COUNT (sizeof(DskTableFAT32) / sizeof(DSKSZTOSECPERCLUS))
/*==========================================================*/
/*  DEFINE: Definition of all local Procedures              */
/*==========================================================*/

void FATRelease()
{
    nIsInit=FALSE;
}
/************************************************************/
/*  FATLock                                                 */
/************************************************************/
void FATLock(void)
{
    NutEventWait(&hFATSemaphore, 0);
}

/************************************************************/
/*  FATFree                                                 */
/************************************************************/
void FATFree(void)
{
    NutEventPost(&hFATSemaphore);
}

/************************************************************/
/*  FATSemaInit                                             */
/************************************************************/
void FATSemaInit(void)
{
    NutEventPost(&hFATSemaphore);
}

/************************************************************/
/*  GetFirstSectorOfCluster                                 */
/************************************************************/
static DWORD GetFirstSectorOfCluster(DRIVE_INFO *pDrive, DWORD dwCluster)
{
    DWORD dwSector;

    if (pDrive->bFlags & FLAG_FAT_IS_CDROM)
    {
        dwSector  = dwCluster;
    }
    else
    {
        dwSector  = (dwCluster - 2) * pDrive->bSectorsPerCluster;
        dwSector += pDrive->dwCluster2StartSector;
    }

    return(dwSector);
}

/************************************************************/
/*  GetNextCluster                                          */
/************************************************************/
static DWORD GetNextCluster(DRIVE_INFO *pDrive, DWORD dwCluster)
{
    DWORD             dwNextCluster;
    DWORD             dwSector;
    DWORD             dwIndex;
    FAT_ENTRY_TABLE16 *pFatTable16;
    FAT_ENTRY_TABLE32 *pFatTable32;

    if (pDrive->bFlags & FLAG_FAT_IS_CDROM)
    {
        dwNextCluster = dwCluster + 1;
    }
    else
    {
        if (pDrive->bIsFAT32 == TRUE)
        {
            //
            //  (HW_SECTOR_SIZE / sizeof(long)) == 128
            // 
            dwSector = (dwCluster / 128) + pDrive->dwFAT1StartSector;
            dwIndex  = dwCluster % 128;

            HWReadSectors(pDrive->bDevice, pSectorBuffer, dwSector, 1);
            pFatTable32 = (FAT_ENTRY_TABLE32 *) pSectorBuffer;

            dwNextCluster = (pFatTable32->aEntry[dwIndex] & FAT32_CLUSTER_MASK);
            if ((dwNextCluster == FAT32_CLUSTER_EOF) || (dwNextCluster == FAT32_CLUSTER_ERROR))
            {
                dwNextCluster = 0;
            }

        }
        else
        {  /* FAT16 */
            //
            //  (HW_SECTOR_SIZE / sizeof(word)) == 256
            // 
            dwSector = (dwCluster / 256) + pDrive->dwFAT1StartSector;
            dwIndex  = dwCluster % 256;

            HWReadSectors(pDrive->bDevice, pSectorBuffer, dwSector, 1);
            pFatTable16 = (FAT_ENTRY_TABLE16 *) pSectorBuffer;

            dwNextCluster = (pFatTable16->aEntry[dwIndex] & FAT16_CLUSTER_MASK);
            if ((dwNextCluster == FAT16_CLUSTER_EOF) || (dwNextCluster == FAT16_CLUSTER_ERROR))
            {
                dwNextCluster = 0;
            }

        } /* endif pDrive->bIsFAT32 */
    }

    return(dwNextCluster);
}

/************************************************************/
/*  GetLongChar                                             */
/************************************************************/
static char GetLongChar(WORD wValue)
{
    BYTE Value;

    Value = (BYTE)(wValue & 0x00FF);
    if (Value == 0xFF)
    {
        Value = 0;
    }

    if (Value != 0)
    {
        Value = toupper(Value);
    }

    return((char)Value);
}

/************************************************************/
/*  FindFile                                                */
/*                                                          */
/*  Find a file by a given name pLongName.                  */
/*                                                          */
/*  It is possible that a SHORT name like "enlogo.gif"      */
/*  is stored as a LONG name. I have seen this              */
/*  nasty behaviour by Win98. Therefore I will check        */
/*  the long name too, even if nIsLongName is FALSE.        */
/************************************************************/
static DWORD FindFile(DRIVE_INFO            *pDrive,
                      FAT32_DIRECTORY_ENTRY *pSearchEntry, 
                      char                  *pLongName,
                      DWORD                 dwDirCluster, 
                      DWORD                 *pFileSize, 
                      int                    nIsLongName)
{
    int                       i, x;
    BYTE                      bError;
    int                       nNameLen;
    int                       nMaxLen;
    BYTE                      bFound;
    BYTE                      bEndLoop;
    BYTE                      bOrder;
    BYTE                      bMaxOrder;
    int                       nDirMaxSector;
    DWORD                     dwSector;
    DWORD                     dwNewCluster;
    FAT32_DIRECTORY_ENTRY      *pDirEntryShort;
    FAT32_DIRECTORY_ENTRY_LONG *pDirEntryLong;
    char                       *pDirName = 0;
    FAT_DIR_TABLE              *pDirTable;

    bError       = FALSE;
    *pFileSize   = 0;
    dwNewCluster = 0;

    nNameLen  = strlen(pLongName);

    bMaxOrder = (BYTE) ((nNameLen + 12) / 13);
    nMaxLen   = (int) (bMaxOrder * 13);
    if (nMaxLen >= (FAT_LONG_NAME_LEN - 1))
    {
        bError = TRUE;
    }

    bOrder = (BYTE) (0x40 | bMaxOrder);
    if (bOrder == 0xE5)
    {
        //
        // I do not know what should I do if the bOrder is 0xe5.
        // This is a sign for a "empty" entry. 
        //
        bError = TRUE;
    }

    if (bError == FALSE)
    {
        bFound   = FALSE;
        bEndLoop = FALSE;
        while ((bEndLoop == FALSE) && (dwDirCluster != 0))
        {
            dwSector = GetFirstSectorOfCluster(pDrive, dwDirCluster);
            nDirMaxSector = (int) pDrive->bSectorsPerCluster;

            //
            // Test for special case dwDirCluster and FAT16.
            //
            if ((dwDirCluster == 1) && (pDrive->bIsFAT32 == FALSE))
            {
                dwSector = pDrive->dwFirstRootDirSector;
                nDirMaxSector = (int) pDrive->dwRootDirSectors;
            }
            //
            // One cluster has SecPerCluster sectors.
            //
            for (i = 0; i < nDirMaxSector; i++)
            {
                HWReadSectors(pDrive->bDevice, pSectorBuffer, dwSector + i, 1);
                pDirTable = (FAT_DIR_TABLE *) pSectorBuffer;

                //
                // And one sector has 16 entries.
                //
                // HW_SECTOR_SIZE / sizeof(FAT32_DIRECTORY_ENTRY) = 16
                //
                for (x = 0; x < 16; x++)
                {
                    if (bFound == TRUE)
                    {
                        pDirEntryShort = (FAT32_DIRECTORY_ENTRY *) & pDirTable->aShort[x];
                        dwNewCluster   = pDirEntryShort->HighCluster;
                        dwNewCluster   = (dwNewCluster << 16) | (DWORD) pDirEntryShort->LowCluster;
                        *pFileSize     = pDirEntryShort->FileSize;
                        bEndLoop       = TRUE;
                        break;
                    }
                    //
                    // Check for valid entry.
                    //
                    pDirEntryShort = (FAT32_DIRECTORY_ENTRY *) & pDirTable->aShort[x];
                    pDirEntryLong = (FAT32_DIRECTORY_ENTRY_LONG *) & pDirTable->aLong[x];

                    if (nIsLongName == FALSE)
                    {
                        //
                        // Check if it could be a short name. If Win2000 tell us it
                        // is a short name, it is true. But with Win98 we must
                        // test both, short and long...
                        //
                        if ((pDirEntryShort->Name[0] != 0xE5) && (pDirEntryShort->Name[0] != 0x00))
                        {
                            if (memcmp(pDirEntryShort, pSearchEntry, 11) == 0)
                            {
                                //
                                // Check for the correct attribute, this is done with
                                // the '&' and not with the memcmp.
                                // With the '&' it is possible to find a hidden archive too :-)
                                //
                                if ((pDirEntryShort->Attribute & pSearchEntry->Attribute) ==
                                    pSearchEntry->Attribute)
                                {

                                    dwNewCluster = pDirEntryShort->HighCluster;
                                    dwNewCluster = (dwNewCluster << 16) | (DWORD) pDirEntryShort->LowCluster;
                                    *pFileSize   = pDirEntryShort->FileSize;

                                    bEndLoop = TRUE;
                                    break;

                                } /* endif Attribute */
                            } /* endif test Name+Ext */
                        } /* endif Name[0] != 0xe5, 0x00 */
                    }
                    /*
                     * endif nIsLongName == FALSE 
                     */
                    if ((pDirEntryLong->Attribute == DIRECTORY_ATTRIBUTE_LONG_NAME)
                        && (pDirEntryLong->Order == bOrder))
                    {
                        //
                        // Next bOrder is bOrder--
                        //  
                        if (bOrder & 0x40)
                        {
                            bOrder &= ~0x40;

                            //
                            // Get the space for the name.
                            //
                            pDirName = pLongName2;

                            //
                            // Set the end of string.
                            //
                            pDirName[nMaxLen--] = 0;
                        }

                        pDirName[nMaxLen--] = GetLongChar(pDirEntryLong->Name3[1]);
                        pDirName[nMaxLen--] = GetLongChar(pDirEntryLong->Name3[0]);

                        pDirName[nMaxLen--] = GetLongChar(pDirEntryLong->Name2[5]);
                        pDirName[nMaxLen--] = GetLongChar(pDirEntryLong->Name2[4]);
                        pDirName[nMaxLen--] = GetLongChar(pDirEntryLong->Name2[3]);
                        pDirName[nMaxLen--] = GetLongChar(pDirEntryLong->Name2[2]);
                        pDirName[nMaxLen--] = GetLongChar(pDirEntryLong->Name2[1]);
                        pDirName[nMaxLen--] = GetLongChar(pDirEntryLong->Name2[0]);

                        pDirName[nMaxLen--] = GetLongChar(pDirEntryLong->Name1[4]);
                        pDirName[nMaxLen--] = GetLongChar(pDirEntryLong->Name1[3]);
                        pDirName[nMaxLen--] = GetLongChar(pDirEntryLong->Name1[2]);
                        pDirName[nMaxLen--] = GetLongChar(pDirEntryLong->Name1[1]);
                        pDirName[nMaxLen--] = GetLongChar(pDirEntryLong->Name1[0]);

                        bOrder--;
                        if (bOrder == 0)
                        {
                            //
                            // Now compare the name.
                            //
                            bOrder = (BYTE) (0x40 | bMaxOrder);
                            nMaxLen = (int) (bMaxOrder * 13);
                            if (memcmp(pLongName, pDirName, strlen(pLongName)) == 0)
                            {
                                //
                                // The next entry will be the correct entry.
                                //
                                bFound = TRUE;
                            }
                        }
                    }
                    /*
                     * pDirEntryLong->Order == bOrder 
                     */
                } /* endfor x<16 */

                if (bEndLoop == TRUE)
                {
                    break;
                }
            }

            if (bEndLoop == FALSE)
            {
                //
                // No file found in this cluster, get the next one.
                //
                dwDirCluster = GetNextCluster(pDrive, dwDirCluster);
            }
        } /* endwhile */

    }
    /*
     * endif bError == FALSE 
     */
    return(dwNewCluster);
}

/************************************************************/
/*  MountHW                                                 */
/************************************************************/
static int MountHW(int nDrive)
{
    int                    nError;
    int                    i;
    DWORD                 dwSector;
    DWORD                 dwFATSz;
    DWORD                 dwRootDirSectors;
    FAT32_PARTITION_TABLE *pPartitionTable;
    FAT32_BOOT_RECORD     *pBootRecord;
    DRIVE_INFO            *pDrive;

    nError   = HW_OK;
    i        = nDrive;
    pDrive   = &sDriveInfo[nDrive];
    dwSector = 0;

    if (pDrive->bFlags & FLAG_FAT_IS_ZIP)
    {
        dwSector = ZIP_DRIVE_BR_SECTOR;
    }
    else
    {
        //
        // Try to find a PartitionTable.
        // 
        nError = HWReadSectors(nDrive, pSectorBuffer, 0, 1);
        if (nError == HW_OK)
        {
            pPartitionTable = (FAT32_PARTITION_TABLE *) pSectorBuffer;

            if (pPartitionTable->Signature == FAT_SIGNATURE)
            {
                if (pPartitionTable->Partition[0].NumSectors)
                {
                    //
                    // We found a PartitionTable, read BootRecord.
                    // 
                    dwSector = pPartitionTable->Partition[0].StartSectors;
                }
            }
        }
    }

    if (dwSector != 0)
    {
        HWReadSectors(i, pSectorBuffer, dwSector, 1);
        pBootRecord = (FAT32_BOOT_RECORD *) pSectorBuffer;

        //
        // Test valid BootRecord.
        //
        if (pBootRecord->Signature == FAT_SIGNATURE)
        {
            pDrive->bSectorsPerCluster = pBootRecord->SecPerClus;

            if (pBootRecord->FATSz16 != 0)
            {
                dwFATSz = pBootRecord->FATSz16;
                pDrive->bIsFAT32 = FALSE;
                pDrive->dwRootCluster = 1;  /* special value, see */
                                            /* FindFile           */
            }
            else
            {
                dwFATSz               = pBootRecord->Off36.FAT32.FATSz32;
                pDrive->bIsFAT32      = TRUE;
                pDrive->dwRootCluster = pBootRecord->Off36.FAT32.RootClus;
            }

            dwRootDirSectors =
            ((pBootRecord->RootEntCnt * 32) +
             (pBootRecord->BytsPerSec - 1)) / pBootRecord->BytsPerSec;

            pDrive->dwFAT1StartSector = pBootRecord->HiddSec + pBootRecord->RsvdSecCnt;
            pDrive->dwFAT2StartSector = pDrive->dwFAT1StartSector + dwFATSz;

            pDrive->dwCluster2StartSector =
            pBootRecord->HiddSec + pBootRecord->RsvdSecCnt +
            (pBootRecord->NumFATs * dwFATSz) + dwRootDirSectors;

            pDrive->dwClusterSize = pBootRecord->SecPerClus * pDrive->wSectorSize;

            pDrive->dwRootDirSectors = dwRootDirSectors;
            pDrive->dwFirstRootDirSector = pDrive->dwFAT2StartSector + dwFATSz;

        } /* endif pBootRecord->Signature */
    }
    /*
     * endif dwSector != 0 
     */
    return(nError);
}

/************************************************************/
/*  MountAllDrives                                          */
/************************************************************/
static int MountDrive(BYTE bDrive)
{
    int nError;

    nError = HW_OK;

    FATLock();

    if (pLongName1 == NULL)
    {
        pLongName1 = (char *)NutHeapAlloc(FAT_LONG_NAME_LEN);
    }
    if (pLongName2 == NULL)
    {
        pLongName2 = (char *)NutHeapAlloc(FAT_LONG_NAME_LEN);
    }
    if (pSectorBuffer == NULL)
    {
        pSectorBuffer = (BYTE *)NutHeapAlloc(MAX_SECTOR_SIZE);
    }

    if ((pSectorBuffer != NULL) && (pLongName1 != NULL) && (pLongName2 != NULL))
    {
        memset((BYTE *) & sDriveInfo[bDrive], 0x00, sizeof(DRIVE_INFO));

        sDriveInfo[bDrive].bDevice     = bDrive;
        sDriveInfo[bDrive].wSectorSize = HWGetSectorSize(bDrive);

        if (HWIsCDROMDevice(bDrive) == TRUE)
        {
            sDriveInfo[bDrive].bFlags |= FLAG_FAT_IS_CDROM;
        }

        if (HWIsZIPDevice(bDrive) == TRUE)
        {
            sDriveInfo[bDrive].bFlags |= FLAG_FAT_IS_ZIP;
        }

        switch (sDriveInfo[bDrive].wSectorSize)
        {
            case HW_SECTOR_SIZE:{
                    nError = MountHW(bDrive);
                    break;
                }
            default:{
                    nError = HW_ERROR;
                    break;
                }
        }
    }
    /*
     * endif pSectorBuffer != NULL 
     */
    FATFree();

    return(nError);
}

/************************************************************/
/*  MountAllDrives                                          */
/************************************************************/
static int MountAllDrives(void)
{
    BYTE  i;
    int  nError = HW_OK;

    for (i = 0; i < FAT_MAX_DRIVE; i++)
    {
        MountDrive(i);
    }

    return(nError);
}

/*==========================================================*/
/*  DEFINE: All code exported by the NUTDEVICE              */
/*==========================================================*/
/************************************************************/
/*  FATUnMountDrive                                         */
/************************************************************/
static int FATUnMountDrive(int nDrive)
{
    int         nError;
    DRIVE_INFO *pDrive;

    FATLock();

    nError = FAT_OK;

    if ((nDrive >= HW_DRIVE_C) && (nDrive <= HW_DRIVE_D))
    {
        pDrive = &sDriveInfo[nDrive];
        pDrive->bSectorsPerCluster = 0;
    }
    else
    {
        nError = FAT_ERROR;
    }

    FATFree();

    return(nError);
}

/************************************************************/
/*  CFMount                                                 */
/************************************************************/
static void CFMount(int nDrive)
{
    BYTE *pSectorBuffer;

    pSectorBuffer = (BYTE *) NutHeapAlloc(MAX_SECTOR_SIZE);
    if (pSectorBuffer != NULL)
    {
        MountDrive(nDrive);
        NutHeapFree(pSectorBuffer);
    }
}

/************************************************************/
/*  CFUnMount                                               */
/************************************************************/
static void CFUnMount(int nDrive)
{
    FATUnMountDrive(nDrive);
}

/************************************************************/
/*  FATInit                                                 */
/************************************************************/
static int FATInit(NUTDEVICE * pDevice)
{
    int   nError;
    int   nHWMode;  
    BYTE *pSectorBuffer;

    nError        = NUTDEV_ERROR;
    nHWMode       = 0;
    pSectorBuffer = NULL;

    if ((nIsInit              == FALSE) && 
        (pDevice->dev_name[0] == 'F')   &&
        (pDevice->dev_name[1] == 'A')   &&
        (pDevice->dev_name[2] == 'T'))
    {
        nError = NUTDEV_OK;      

        //
        // Get the mode.
        //    
        switch (pDevice->dev_base)
        {

#if (FAT_USE_MMC_INTERFACE >= 1)
            case FAT_MODE_MMC:
                nHWMode = 0;   
                break; 
#endif        
            default:
                nError = NUTDEV_ERROR;
                break;
        }

        if (nError == NUTDEV_OK)
        {
            //
            // Init my semaphore.
            //
            FATSemaInit();

            pSectorBuffer = (BYTE *) NutHeapAlloc(MAX_SECTOR_SIZE);
            if (pSectorBuffer != NULL)
            {
                /*
                 * HW init
                 */
                nError = HWInit(nHWMode, CFMount, CFUnMount);
                if (nError == HW_OK)
                {
                    HWMountAllDevices(nHWMode, pSectorBuffer);

                    /*
                     * FAT init
                     */
                    MountAllDrives();

                    nError  = NUTDEV_OK;
                    nIsInit = TRUE;    
                }
                else
                {
                    nError  = NUTDEV_WRONG_HW;
                }  

                NutHeapFree(pSectorBuffer);
            } /* endif pSectorBuffer != NULL */
        } /* endif nError == NUTDEV_OK */
    }
    else
    {
        if (nIsInit == TRUE)
        {
#if (FAT_USE_MMC_INTERFACE >= 1)
            if ((pDevice->dev_name[0] == 'F')   &&
                (pDevice->dev_name[1] == 'M')   &&
                (pDevice->dev_name[2] == '0'))
            {
                /*
                 * Always OK, because the MMC card
                 * can be inserted later.
                 */
                nError = NUTDEV_OK;  
            }
#endif /* (FAT_USE_MMC_INTERFACE >= 0) */      

        } /* endif (nIsInit == TRUE) */
    } /* endif nIsInit == FALSE */

    return(nError);
}

/************************************************************/
/*  GetDriveByDevice                                        */
/************************************************************/
static DRIVE_INFO* GetDriveByDevice (NUTDEVICE *pDevice)
{
    DRIVE_INFO* pDrive = NULL;

    if (pDevice != NULL)
    {
        switch (pDevice->dev_name[2])
        {
            case 'F':{
                    pDrive = &sDriveInfo[HW_DRIVE_C];
                    break;
                }
            case '0':{
                    pDrive = &sDriveInfo[HW_DRIVE_D];
                    break;
                }
            case '1':{
                    pDrive = &sDriveInfo[HW_DRIVE_E];
                    break;        
                }      
        }
    }

    return(pDrive);
}  

/************************************************************/
/*  FATFileOpen                                             */
/*                                                          */
/*  Opens an existing file for reading.                     */
/*                                                          */
/*  Parameters: pName points to a string that specifies the */
/*              name of the file to open. The name must     */
/*              exactly match the full pathname of the file.*/
/*                                                          */
/*  Returns:    A pointer to a FILE structure that can be   */
/*              used to read the file and retrieve          */
/*              information about the file.                 */
/*                                                          */
/*              A return value of -1 indicates an error.    */
/************************************************************/
static NUTFILE *FATFileOpen(NUTDEVICE *pDevice, CONST char *pName, int nMode,
                            int nAccess)
{
    int                    i, x;
    int                    nError;
    int                    nEndWhile;
    DWORD                 dwFileSize;
    DWORD                 dwCluster;
    FHANDLE               *hFile;
    DRIVE_INFO            *pDrive;
    FAT32_DIRECTORY_ENTRY  sDirEntry;
    NUTFILE               *hNUTFile;
    int                    nLongName;
    char                  *pLongName;
    char                  *pShortName;
    char                  *pExtension;

    //
    // If the user has forgotten to call NUTDeviceOpen,
    // we must call FATInit.
    //
    if (nIsInit == FALSE)
    {
        FATInit(pDevice);
    }

    FATLock();

    pDrive    = NULL;
    nError    = TRUE;
    nLongName = FALSE;
    pLongName = (char *)pLongName1;

    //
    // hFile is our FAT-Handle.
    //
    hFile = NULL;

    //
    // hNUTFile is the.... correct, NUT handle.
    //   
    hNUTFile = (NUTFILE *) NUTDEV_ERROR;

    pDrive = GetDriveByDevice(pDevice);

    if ((pDrive != NULL) && (pDrive->bSectorsPerCluster != 0) && (pName[0] != 0))
    {

        //
        // Create a new file handle.
        //    
        hFile = (FHANDLE *) NutHeapAlloc(sizeof(FHANDLE));

        if ((pDrive->dwFAT1StartSector) && (hFile != NULL) && (*pName != '.'))
        {

            memset(hFile, 0x00, sizeof(FHANDLE));

            //
            // Start by the ROOT dir...
            //
            dwCluster = pDrive->dwRootCluster;

            //
            // If the first char a "/", jump over, e.g. "/index.html"
            // 
            //
            if (*pName == '/')
            {
                pName++;
            }

            nEndWhile = FALSE;
            while (nEndWhile == FALSE)
            {  /* master loop */

                nLongName = FALSE;

                //
                // Get Name
                //
                i = 0;
                while ((*pName != '/') && (*pName != '\\') && (*pName != 0))
                {

                    if (i >= (FAT_LONG_NAME_LEN - 1))
                    {
                        nEndWhile = TRUE;
                        break;
                    }

                    pLongName[i] = toupper(*pName);

                    i++;
                    pName++;
                } /* endwhile Name */

                if (nEndWhile == FALSE)
                {
                    pLongName[i] = 0;

                    //
                    // Check if it is a Long Directory Entry.
                    // Yes, I know that 'i' is the length of the string.
                    // But the code is easier to read with the next strlen.
                    //
                    if (strlen(pLongName) <= FAT_SHORT_NAME_LEN)
                    {
                        //
                        // It could be a ShortName, but "abc.defg" is possible
                        // and this is a long name too. Therfore we need some tests.
                        //
                        pExtension = strchr(pLongName, '.');
                        if (pExtension == NULL)
                        {
                            if (strlen(pLongName) > FAT_NAME_LEN)
                            {
                                nLongName = TRUE;
                            }
                            else
                            {
                                nLongName = FALSE;
                            }
                        }
                        else
                        {
                            //
                            // Check the length of the extensions.
                            //
                            pExtension++; /* jump over the '.' */
                            if (strlen(pExtension) > 3)
                            {
                                nLongName = TRUE;
                            }
                        }
                    }
                    else
                    {  /* Len > FAT_SHORT_NAME_LEN */
                        //
                        // Now we have a LongName, sure.
                        // See the "nasty Win98" in FindFile :-)
                        //
                        nLongName = TRUE;
                    }

                    //
                    // Here we knows, if we have a LongName or ShortName.
                    //
                    if (nLongName == FALSE)
                    {
                        //
                        // ShortName
                        //
                        pShortName = pLongName;
                        memset(&sDirEntry, 0x00, sizeof(FAT32_DIRECTORY_ENTRY));
                        memset(sDirEntry.Name, 0x20, FAT_NAME_LEN);
                        memset(sDirEntry.Extension, 0x20, FAT_EXT_LEN);

                        //
                        // Get the name
                        //
                        i = 0;
                        while ((pShortName[i] != '.') && (pShortName[i] != 0))
                        {
                            sDirEntry.Name[i] = pShortName[i];
                            i++;
                        }
                        //
                        // And the extension
                        //
                        if (pShortName[i] == '.')
                        {
                            i++;  /* jump over the '.' */
                            x = 0;
                            while (pShortName[i] != 0)
                            {
                                sDirEntry.Extension[x] = pShortName[i];
                                i++;
                                x++;
                            }
                        }
                    }
                    //
                    // The file could be a long or short one.
                    // I have seen that Win98 store the short filename
                    // in a long one :-(
                    //
                    switch (*pName)
                    {
                        //
                        // The file is an ARCHIVE
                        //
                        case 0:{
                                nEndWhile = TRUE;
                                sDirEntry.Attribute = DIRECTORY_ATTRIBUTE_ARCHIVE;

                                if (pDrive->bFlags & FLAG_FAT_IS_CDROM)
                                {
                                    dwCluster = 0;
                                }
                                else
                                {
                                    dwCluster =
                                    FindFile(pDrive, &sDirEntry, pLongName, dwCluster, &dwFileSize,
                                             nLongName);
                                }
                                if (dwCluster != 0)
                                {
                                    hFile->dwFileSize       = dwFileSize;
                                    hFile->dwStartCluster   = dwCluster;
                                    hFile->dwReadCluster    = dwCluster;
                                    hFile->dwFilePointer    = 0;
                                    hFile->dwClusterPointer = 0;
                                    hFile->pDrive           = pDrive;
                                    hFile->nLastError       = FAT_OK;
                                    hFile->nEOF             = FALSE;

                                    nError                  = FALSE;
                                }
                                break;
                            } /* endcase 0 */

                            //
                            // The file is a DIRECTORY
                            //
                        case '/':
                        case '\\':{
                                pName++;  /* jump over the char */

                                sDirEntry.Attribute = DIRECTORY_ATTRIBUTE_DIRECTORY;

                                if (pDrive->bFlags & FLAG_FAT_IS_CDROM)
                                {
                                    dwCluster = 0;
                                }
                                else
                                {
                                    dwCluster =
                                    FindFile(pDrive, &sDirEntry, pLongName, dwCluster, &dwFileSize,
                                             nLongName);
                                }
                                if (dwCluster != 0)
                                {

                                    //
                                    // The new Cluster is the Cluster of the directory
                                    //

                                }
                                else
                                {
                                    nEndWhile = TRUE;
                                }
                                break;
                            } /* endcase / \ */

                        default:{
                                nEndWhile = TRUE;
                                break;
                            }
                    } /* end switch */

                }
                /*
                 * endif nEndWhile == FALSE 
                 */
            } /* end while */

        }
        /*
         * endif pDrive->dwFAT1StartSector 
         */
        if (nError == TRUE)
        {
            //
            // We found no file, therefore we can delete our FAT-Handle
            //
            if (hFile != NULL)
            {
                NutHeapFree(hFile);
            }
        }
        else
        {
            //
            // We have found a FILE and can create a NUT-Handle.
            //
            hNUTFile = NutHeapAlloc(sizeof(NUTFILE));
            if (hNUTFile != NULL)
            {
                hNUTFile->nf_next = 0;
                hNUTFile->nf_dev  = pDevice;
                hNUTFile->nf_fcb  = hFile;
            }
            else
            {
                //
                // Error, no mem for the NUT-Handle, therefore we 
                // can delete our FAT-Handle too.
                //
                NutHeapFree(hFile);
            }
        }

    }
    /*
     * endif pName[0] != 0 
     */
    FATFree();

    return(hNUTFile);
}

/************************************************************/
/*  FATFileClose                                            */
/*                                                          */
/*  Close a previously opened file.                         */
/*                                                          */
/*  Parameters: hNUTFile Identifies the file to close.      */
/*              This pointer must have been created by      */
/*              calling FAT32FileOpen().                    */
/*                                                          */
/*  Returns:    0 if the function is successfully closed,   */
/*              -1 otherwise.                               */
/************************************************************/
static int FATFileClose(NUTFILE * hNUTFile)
{
    int      nError;
    FHANDLE *hFile;

    nError = NUTDEV_ERROR;

    FATLock();

    if (hNUTFile != NULL)
    {
        hFile = (FHANDLE *) hNUTFile->nf_fcb;
        if (hFile != NULL)
        {
            //
            // Clear our FAT-Handle
            //
            NutHeapFree(hFile);
        }
        //
        // Clear the NUT-Handle
        //
        NutHeapFree(hNUTFile);

        nError = NUTDEV_OK;
    }

    FATFree();

    return(nError);
}

/************************************************************/
/*  FATFileSize                                             */
/*                                                          */
/*  Retrieve the size of a file.                            */
/*                                                          */
/*  Parameters: pFile Identifies the file to query.         */
/*              This pointer must have been created by      */
/*              calling FAT32FileOpen().                    */
/*                                                          */
/*  Returns:    The number of bytes in this file or         */
/*              -1 if an error occured                      */
/************************************************************/
long FATFileSize(NUTFILE * hNUTFile)
{
    long     lSize;
    FHANDLE *hFile;

    FATLock();

    lSize = NUTDEV_ERROR;

    if (hNUTFile != NULL)
    {
        hFile = (FHANDLE *) hNUTFile->nf_fcb;
        if (hFile != NULL)
        {
            lSize = hFile->dwFileSize;
        }
    }

    FATFree();

    return(lSize);
}

#if 0
/************************************************************/
/*  FATFileSeek                                             */
/*                                                          */
/*  Move the file pointer to a new position.                */
/*  It points to the next byte to be read from a file.      */
/*  The file pointer is automatically incremented for       */
/*  each byte read. When the file is opened, it is at       */
/*  position 0, the beginning of the file.                  */
/*                                                          */
/*  Parameters: pFile Identifies the file to seek.          */
/*              This pointer must have been created by      */
/*              calling FAT32FileOpen().                    */
/*                                                          */
/*              lPos Specifies the new absolute position    */
/*              of the file pointer.                        */
/*                                                          */
/*  Returns:    0 if the function is successful,            */
/*              -1 otherwise.                               */
/************************************************************/
int FATFileSeek(FILE * pFile, long lPos)
{
    int      nError = NUTDEV_ERROR;
    FHANDLE *hFile;

    FATLock();

    hFile = (FHANDLE *) pFile;

    //
    // We must do some work here...
    //
    nError = FAT32_ERROR;

    FATFree();

    return(nError);
}
#endif

/************************************************************/
/*  FATFileRead                                             */
/*                                                          */
/*  Read data from a file.                                  */
/*                                                          */
/*  Parameters: pFile Identifies the file to read from.     */
/*              This pointer must have been created by      */
/*              calling FAT32FileOpen().                    */
/*                                                          */
/*              pData Points to the buffer that receives    */
/*              the data.                                   */
/*                                                          */
/*              nSize Specifies the number of bytes to      */
/*              read from the file.                         */
/*                                                          */
/*  Returns:    The number of bytes read from the file or   */
/*               -1 if an error occured.                    */
/************************************************************/
int FATFileRead(NUTFILE * hNUTFile, void *pData, int nSize)
{
    int         nError;
    int         nBytesRead;
    int         nBytesToRead;
    FHANDLE    *hFile;
    DRIVE_INFO *pDrive;
    BYTE       *pByte;
    DWORD      dwReadSector;
    DWORD      dwSector;
    int         nSectorCount;
    int         nSectorOffset;
    WORD        wSectorSize;

    nBytesRead = 0;

    FATLock();

    hFile  = NULL;
    nError = NUTDEV_ERROR;

    if (hNUTFile != NULL)
    {
        hFile = (FHANDLE *) hNUTFile->nf_fcb;
    }

    if ((hFile != NULL) && (nSize != 0))
    {
        if (hFile->dwFilePointer < hFile->dwFileSize)
        {

            if ((hFile->dwFilePointer + nSize) > hFile->dwFileSize)
            {
                nSize = (int) (hFile->dwFileSize - hFile->dwFilePointer);
            }

            pDrive = (DRIVE_INFO *) hFile->pDrive;
            pByte  = (BYTE *) pData;

            nBytesRead  = nSize;
            wSectorSize = pDrive->wSectorSize;

            while (nSize)
            {
                dwSector = GetFirstSectorOfCluster(pDrive, hFile->dwReadCluster);
                nSectorCount = hFile->dwClusterPointer / wSectorSize;
                nSectorOffset = hFile->dwClusterPointer % wSectorSize;

                //
                // (Sector + SectorCount) is the sector to read
                // SectorOffset is the start position in the sector itself
                //
                dwReadSector = dwSector + nSectorCount;

                nError = HWReadSectors(pDrive->bDevice, pSectorBuffer, dwReadSector, 1);
                if (nError == HW_OK)
                {
                    //
                    // Find the size we can read from ONE sector
                    //
                    if (nSize > (int) wSectorSize)
                    {
                        nBytesToRead = wSectorSize;
                    }
                    else
                    {
                        nBytesToRead = nSize;
                    }

                    //
                    // Test inside a sector
                    //
                    if ((nSectorOffset + nBytesToRead) > (int) wSectorSize)
                    {
                        nBytesToRead = wSectorSize - nSectorOffset;
                    }

                    memcpy(pByte, &pSectorBuffer[nSectorOffset], nBytesToRead);
                    pByte += nBytesToRead;

                    hFile->dwFilePointer    += nBytesToRead;
                    hFile->dwClusterPointer += nBytesToRead;

                    //
                    // Check for EOF
                    if (hFile->dwFilePointer >= hFile->dwFileSize)
                    {
                        hFile->nEOF = TRUE;
                    }

                    if (hFile->dwClusterPointer >= pDrive->dwClusterSize)
                    {
                        //
                        // We must switch to the next cluster
                        //
                        hFile->dwReadCluster = GetNextCluster(pDrive, hFile->dwReadCluster);
                        hFile->dwClusterPointer = 0;
                    }

                    nSize -= nBytesToRead;

                }
                else
                {  /* HWReadSectors Error */

                    nBytesRead = 0;
                    hFile->nLastError = FAT_ERROR_IDE;
                    break;
                } /* endif nError == HW_OK */

            } /* endwhile */

        }
        else
        {  /* reached the EOF */
            hFile->nLastError = FAT_ERROR_EOF;
        } /* endif hFile->dwFilePointer < hFile->dwFileSize */

    }
    /*
     * endif (hFile != NULL) && (nSize != 0) 
     */
    FATFree();

    return(nBytesRead);
}

/************************************************************/
/*  FATFileWrite                                            */
/*                                                          */
/*  Write data to a file.                                   */
/*                                                          */
/*  Parameters: pFile Identifies the file to write to.      */
/*              This pointer must have been created by      */
/*              calling FAT32FileOpen().                    */
/*                                                          */
/*              pData Points to the buffer that holds       */
/*              the data.                                   */
/*                                                          */
/*              nSize Specifies the number of bytes to      */
/*              write to the file.                          */
/*                                                          */
/*  Returns:    The number of bytes written to the file or  */
/*               -1 if an error occured.                    */
/************************************************************/
static int FATFileWrite(NUTFILE * hNUTFile, CONST void *pData, int nSize)
{
    int nError;

    nError = NUTDEV_ERROR;

    return(nError);
}

#ifdef __HARVARD_ARCH__
static int FATFileWriteP(NUTFILE * hNUTFile, PGM_P pData, int nSize)
{
    int nError;

    nError = NUTDEV_ERROR;

    return(nError);
}
#endif

/************************************************************/
/*  FATIOCtl                                                */
/*                                                          */
/*  Perform IOCTL control functions.                        */
/*                                                          */
/*  reg May be set to one of the following constants:       */
/*                                                          */
/*  Parameters: dev  Identifies the device to use.          */
/*              reg  Requested control function.            */
/*              conf Points to a buffer that contains any   */
/*                   data required for the given control    */
/*                   function or receives data from that    */
/*                   function.                              */
/*  Returns:    0 on success, or -1 if an error occured.    */
/************************************************************/
int FATIOCtl(NUTDEVICE *dev, int req, void *conf)
{
    int         nError = NUTDEV_ERROR;
    DRIVE_INFO *pDrive = GetDriveByDevice(dev);

    if (pDrive != NULL)
    {
        switch (req)
        {

#if (FAT_SUPPORT_FORMAT >= 1)    
            case FAT_IOCTL_QUICK_FORMAT: {
                    nError = QuickFormat(dev, pDrive);
                    break;
                }
#endif

            default: {
                    nError = NUTDEV_ERROR;
                    break; 
                }

        }  
    }

    return(nError);
}

//
// FAT Device information structure.
// Is mapped to the FAT-Device.
// The user MUST first register the FAT hardware.
//
NUTDEVICE devFAT = {
    0,              /* Pointer to next device. */

    /*
     * Unique device name.     
     */
    {'F', 'A', 'T', 0, 0, 0, 0, 0, 0},

    IFTYP_STREAM,   /* Type of device.                   */
    2,              /* Base address.                     */
    0,              /* First interrupt number.           */
    0,              /* Interface control block.          */
    0,              /* Driver control block.             */
    FATInit,        /* Driver initialization routine.    */
    0,              /* Driver specific control function. */
    0,              /* Driver specific read function.    */
    0,              /* Driver specific write function.   */

#ifdef __HARVARD_ARCH__
    0,              /* Driver specific write_p function. */
#endif

    0,              /* Driver specific open function.    */
    0,              /* Driver specific close function.   */
    0
};

#if (FAT_USE_MMC_INTERFACE >= 1)
//
// FATUSB Device information structure.
// Is mapped to the first USB drive
//
NUTDEVICE devFATMMC0 = {
    0,              /* Pointer to next device. */

    /*
     * Unique device name.     
     */
    {'F', 'M', '0', 0, 0, 0, 0, 0, 0},

    IFTYP_STREAM,   /* Type of device.                   */
    2,              /* Base address.                     */
    0,              /* First interrupt number.           */
    0,              /* Interface control block.          */
    0,              /* Driver control block.             */
    FATInit,        /* Driver initialization routine.    */
    FATIOCtl,       /* Driver specific control function. */
    FATFileRead,    /* Driver specific read function.    */
    FATFileWrite,   /* Driver specific write function.   */

#ifdef __HARVARD_ARCH__
    FATFileWriteP,  /* Driver specific write_p function. */
#endif

    FATFileOpen,    /* Driver specific open function.    */
    FATFileClose,   /* Driver specific close function.   */
    FATFileSize
};
#endif /* (FAT_USE_MMC_INTERFACE >= 1) */


#if (FAT_SUPPORT_FORMAT >= 1)
/************************************************************/
/*  GetClusterSize                                          */
/************************************************************/
static BYTE GetClusterSize (DWORD dTotalSectors)
{
    BYTE bIndex;
    BYTE bSecPerClus = 0;

    for (bIndex=0; bIndex<DSK_TABLE_FAT32_ENTRY_COUNT; bIndex++)
    {
        if (dTotalSectors <= DskTableFAT32[bIndex].DiskSize)
        {
            bSecPerClus = DskTableFAT32[bIndex].SecPerClusVal;
            break;      
        }
    }

    return(bSecPerClus);
}

/************************************************************/
/*  QuickFormat                                             */
/************************************************************/
static int QuickFormat(NUTDEVICE *dev, DRIVE_INFO *pDrive)
{
    DWORD                   i;
    int                    nError = NUTDEV_ERROR;
    BYTE                  *pBuffer;
    BYTE                   bDevice;
    BYTE                   bSecPerClus;  
    DWORD                  dSector;
    DWORD                  dFirstDataSector;
    DWORD                  dLastDataSector;
    DWORD                  dMaxClusterCount;
    DWORD                  dFAT1StartSector=0;
    DWORD                  dFAT2StartSector=0;
    DWORD                  dCluster2StartSector;
    DWORD                  dClearCount;
    DWORD                  dTotalSectors;
    DWORD                  dPossibleClusterCount;
    DWORD                  dFATSz32;
    FAT32_PARTITION_TABLE *pPartitionTable;  
    FAT32_BOOT_RECORD     *pBootRecord;
    FAT32_FSINFO          *pFSInfo;
    FAT32_DIRECTORY_ENTRY *pDirEntry;
    LONG                  *pLong;
    DWORD                  dVollID;

    FATLock();

    /*
     * Get the sector buffer
     */  
    pBuffer = (u_char *)NutHeapAlloc(pDrive->wSectorSize);
    if (pBuffer != NULL)
    {

        bDevice        = pDrive->bDevice;    
        dTotalSectors  = HWGetTotalSectors(bDevice);
        dTotalSectors -= BPB_RsvdSecCnt;

        bSecPerClus = GetClusterSize(dTotalSectors);     
        if (bSecPerClus == 0)
        {
            /*
             * Sorry, to small for FAT32,
             * the lib will support FAT16 later....
             */
            NutHeapFree(pBuffer);  
            return(NUTDEV_ERROR);    
        }

        dPossibleClusterCount = (dTotalSectors / (LONG)bSecPerClus);

        dFATSz32              = (dPossibleClusterCount * sizeof(long) + 
                                 (FAT32_SECTOR_SIZE -1) ) / FAT32_SECTOR_SIZE; 

        /*
         * First we will clear the sectors which we will use
         * for the MBR, BPB and the FA's.
         */
        dClearCount  = BPB_HiddSec + BPB_RsvdSecCnt;
        dClearCount += (BPB_NumFATs * dFATSz32) + bSecPerClus;

        memset(pBuffer, 0x00, pDrive->wSectorSize);        
        for (i=0; i<dClearCount; i++)
        {
            nError = HWWriteSectors(bDevice, pBuffer, i, 1);    
            if (nError != HW_OK)
            {
                break;
            }
        }

        /*
         * Create the Volume serial number,
         * we have no random value, therefore
         * use the dTotalSectors and time.
         */
        dVollID = (DWORD)time(NULL) * dTotalSectors;

        /*
         * Create MBR (MasterBootRecord)
         */
        memset(pBuffer, 0x00, pDrive->wSectorSize);
        pPartitionTable = (FAT32_PARTITION_TABLE *)pBuffer;

        pPartitionTable->Partition[0].BootInd      = 0x00;
        pPartitionTable->Partition[0].FirstHead    = 1;
        pPartitionTable->Partition[0].FirstSector  = 1;
        pPartitionTable->Partition[0].FirstTrack   = 0;
        pPartitionTable->Partition[0].FileSystem   = MBR_FAT32;
        pPartitionTable->Partition[0].LastHead     = 0;
        pPartitionTable->Partition[0].LastSector   = 0;
        pPartitionTable->Partition[0].StartSectors = BPB_HiddSec;
        pPartitionTable->Partition[0].NumSectors   = dTotalSectors;
        pPartitionTable->Signature                 = MBR_SIGNATURE;

        /*
         * Write the MBR to the drive
         */
        nError = HWWriteSectors(bDevice, pBuffer, 0, 1);    
        if (nError == HW_OK)
        {
            /*
             * Create the BPB
             */
            memset(pBuffer, 0x00, pDrive->wSectorSize);
            pBootRecord = (FAT32_BOOT_RECORD *)pBuffer;

            pBootRecord->JumpBoot[0] = 0xEB;
            pBootRecord->JumpBoot[1] = 0x58;
            pBootRecord->JumpBoot[2] = 0x90;

            pBootRecord->OEMName[0]  = 'M';
            pBootRecord->OEMName[1]  = 'S';
            pBootRecord->OEMName[2]  = 'D';
            pBootRecord->OEMName[3]  = 'O';
            pBootRecord->OEMName[4]  = 'S';
            pBootRecord->OEMName[5]  = '5';
            pBootRecord->OEMName[6]  = '.';
            pBootRecord->OEMName[7]  = '0';

            pBootRecord->BytsPerSec  = FAT32_SECTOR_SIZE;
            pBootRecord->SecPerClus  = bSecPerClus;
            pBootRecord->RsvdSecCnt  = BPB_RsvdSecCnt;
            pBootRecord->NumFATs     = BPB_NumFATs;
            pBootRecord->RootEntCnt  = 0;
            pBootRecord->TotSec16    = 0;
            pBootRecord->Media       = FAT32_MEDIA;      

            pBootRecord->FATSz16     = 0;
            pBootRecord->SecPerTrk   = 63;
            pBootRecord->NumHeads    = 255;

            pBootRecord->HiddSec     = BPB_HiddSec;
            pBootRecord->TotSec32    = dTotalSectors;

            pBootRecord->Off36.FAT32.FATSz32       = dFATSz32;
            pBootRecord->Off36.FAT32.ExtFlags      = 0;
            pBootRecord->Off36.FAT32.FSVer         = 0;
            pBootRecord->Off36.FAT32.RootClus      = 2;
            pBootRecord->Off36.FAT32.FSInfo        = FAT32_OFFSET_FSINFO;
            pBootRecord->Off36.FAT32.BkBootSec     = FAT32_OFFSET_BACKUP_BOOT;

            pBootRecord->Off36.FAT32.DrvNum        = 0x00; 
            pBootRecord->Off36.FAT32.BootSig       = 0x29;
            pBootRecord->Off36.FAT32.VollID        = dVollID;
            pBootRecord->Off36.FAT32.VolLab[0]     = 'N';
            pBootRecord->Off36.FAT32.VolLab[1]     = 'O';
            pBootRecord->Off36.FAT32.VolLab[2]     = ' ';
            pBootRecord->Off36.FAT32.VolLab[3]     = 'N';
            pBootRecord->Off36.FAT32.VolLab[4]     = 'A'; 
            pBootRecord->Off36.FAT32.VolLab[5]     = 'M'; 
            pBootRecord->Off36.FAT32.VolLab[6]     = 'E'; 
            pBootRecord->Off36.FAT32.VolLab[7]     = ' '; 
            pBootRecord->Off36.FAT32.VolLab[8]     = ' '; 
            pBootRecord->Off36.FAT32.VolLab[9]     = ' '; 
            pBootRecord->Off36.FAT32.VolLab[10]    = ' '; 

            pBootRecord->Off36.FAT32.FilSysType[0] = 'F';
            pBootRecord->Off36.FAT32.FilSysType[1] = 'A';
            pBootRecord->Off36.FAT32.FilSysType[2] = 'T';
            pBootRecord->Off36.FAT32.FilSysType[3] = '3';
            pBootRecord->Off36.FAT32.FilSysType[4] = '2';
            pBootRecord->Off36.FAT32.FilSysType[5] = ' ';
            pBootRecord->Off36.FAT32.FilSysType[6] = ' ';
            pBootRecord->Off36.FAT32.FilSysType[7] = ' ';

            pBootRecord->Signature                 = FAT_SIGNATURE;

            /*
             * Write the BPB to the drive
             */
            dSector = BPB_HiddSec;
            nError  = HWWriteSectors(bDevice, pBuffer, dSector, 1);    
            if (nError == HW_OK)
            {
                dSector += FAT32_OFFSET_BACKUP_BOOT;
                nError   = HWWriteSectors(bDevice, pBuffer, dSector, 1);            
            }
        }

        /*
         * Create the FSINFO
         */
        if (nError == HW_OK)
        {
            memset(pBuffer, 0x00, pDrive->wSectorSize);
            pFSInfo = (FAT32_FSINFO *)pBuffer;

            dFirstDataSector = BPB_HiddSec + BPB_RsvdSecCnt;
            dLastDataSector  = dTotalSectors;
            dMaxClusterCount = dLastDataSector - dFirstDataSector;
            dMaxClusterCount = dMaxClusterCount / (LONG)bSecPerClus;

            pFSInfo->FirstSignature               = FSINFO_FIRSTSIGNATURE;
            pFSInfo->FSInfoSignature              = FSINFO_FSINFOSIGNATURE;
            pFSInfo->NumberOfFreeClusters         = dMaxClusterCount;
            pFSInfo->MostRecentlyAllocatedCluster = 2;
            pFSInfo->Signature                    = FSINFO_SIGNATURE;

            /*
             * Write the FSINFO to the drive
             */      
            dSector = BPB_HiddSec + FAT32_OFFSET_FSINFO;
            nError  = HWWriteSectors(bDevice, pBuffer, dSector, 1);        
        }

        /*
         * Write first FAT entry after format.
         */
        if (nError == HW_OK)
        {
            dFAT1StartSector = BPB_HiddSec + BPB_RsvdSecCnt;    
            dFAT2StartSector = dFAT1StartSector + dFATSz32; 

            memset(pBuffer, 0x00, pDrive->wSectorSize);
            pLong = (LONG *)pBuffer;

            /*
             * Set the two reserved cluster after the format,
             * take a look in the MS Hardware White Paper
             * Version 1.03, December 6, 2000 for more information.
             */
            *pLong++ = 0x0FFFFFF8;
            *pLong++ = FAT32_CLUSTER_EOF;
            *pLong++ = FAT32_CLUSTER_EOF;

            nError = HWWriteSectors(bDevice, pBuffer, dFAT1StartSector, 1);        
            if (nError == HW_OK)
            {
                nError = HWWriteSectors(bDevice, pBuffer, dFAT2StartSector, 1);              
            }
        }

        /*
         * Create RootDir
         */
        if (nError == HW_OK)
        {
            memset(pBuffer, 0x00, pDrive->wSectorSize);
            pDirEntry = (FAT32_DIRECTORY_ENTRY *)pBuffer;

            pDirEntry->Name[0]      = 'E'; 
            pDirEntry->Name[1]      = 'T';
            pDirEntry->Name[2]      = 'H';
            pDirEntry->Name[3]      = 'E';
            pDirEntry->Name[4]      = 'R';
            pDirEntry->Name[5]      = 'N';
            pDirEntry->Name[6]      = 'U';
            pDirEntry->Name[7]      = 'T';

            pDirEntry->Extension[0] = ' ';
            pDirEntry->Extension[1] = ' ';
            pDirEntry->Extension[2] = ' ';

            pDirEntry->Attribute    = DIRECTORY_ATTRIBUTE_VOLUME_ID;
            pDirEntry->HighCluster  = 0;
            pDirEntry->LowCluster   = 0;
            pDirEntry->FileSize     = 0;

            dCluster2StartSector = dFAT2StartSector + dFATSz32;
            nError = HWWriteSectors(bDevice, pBuffer, dCluster2StartSector, 1);                    
        }

        NutHeapFree(pBuffer);  

        FATFree();


        /*
         * Don't forget to mount the drive
         */   
        if (nError == HW_OK)
        {
            nError = MountDrive(bDevice);
        }

        if (nError == HW_OK)
        {
            nError = NUTDEV_OK;        
        }
        else
        {
            nError = NUTDEV_ERROR;    
        }    
    }

    return(nError);
}
#endif /* (FAT_SUPPORT_FORMAT >= 1) */


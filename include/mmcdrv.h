/****************************************************************************
*  This file is part of the MMC device driver.
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
*  10.10.04  mifi   First Version
****************************************************************************/
#ifndef __MMCDRV_H__
#define __MMCDRV_H__

#include "typedefs.h"

/*-------------------------------------------------------------------------*/
/* global defines                                                          */
/*-------------------------------------------------------------------------*/
//
// Here we can switch on/off some
// feature of the software
//
#define MMC_SUPPORT_WRITE               1

#define MMC_OK                          0x00
#define MMC_ERROR                       0x01
#define MMC_DRIVE_NOT_FOUND             0x02
#define MMC_PARAM_ERROR                 0x03
#define MMC_BUSY                        0x04
#define MMC_NOT_SUPPORTED               0x08

#define MMC_DRIVE_C                     0

//
// Sector size
//
#define MMC_SECTOR_SIZE                 512
#define MAX_SECTOR_SIZE                 MMC_SECTOR_SIZE

/*-------------------------------------------------------------------------*/
/* global types                                                            */
/*-------------------------------------------------------------------------*/
typedef void MMC_MOUNT_FUNC(int nDevice);

/*-------------------------------------------------------------------------*/
/* global macros                                                           */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* Prototypes                                                              */
/*-------------------------------------------------------------------------*/
int MMCInit(int nMMCMode, MMC_MOUNT_FUNC * pMountFunc, MMC_MOUNT_FUNC * pUnMountFunc);

int MMCMountAllDevices(int nMMCMode, BYTE *pSectorBuffer);

int MMCGetSectorSize(BYTE bDevice);

int MMCIsCDROMDevice(BYTE bDevice);

int MMCIsZIPDevice(BYTE bDevice);

int MMCUnMountDevice(BYTE bDevice);

DWORD MMCGetTotalSectors(BYTE bDevice);

int MMCReadSectors(BYTE bDevice, void *pData, DWORD dwStartSector, WORD wSectorCount);

#if (MMC_SUPPORT_WRITE == 1)

int MMCWriteSectors(BYTE bDevice, void *pData, DWORD dwStartSector, WORD wSectorCount);

#endif

#endif /* !__MMCDRV_H__ */

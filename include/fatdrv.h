/****************************************************************************
*  This file is part of the FAT device driver.
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
*  03.08.04  mifi   First Version
****************************************************************************/
#ifndef __FATDRV_H__
#define __FATDRV_H__

#include "typedefs.h"

/*-------------------------------------------------------------------------*/
/* global defines                                                          */
/*-------------------------------------------------------------------------*/

/*
 * Define the hardware here
 */
#define FAT_USE_IDE_INTERFACE     0 
#define FAT_USE_USB_INTERFACE     0
#define FAT_USE_MMC_INTERFACE     1 


#if (FAT_USE_IDE_INTERFACE >= 1)
#include "ide.h"

#define HW_SUPPORT_WRITE  IDE_SUPPORT_WRITE
#define HW_SUPPORT_ATAPI  IDE_SUPPORT_ATAPI

#define HW_OK             IDE_OK
#define HW_ERROR          IDE_ERROR

#define HW_DRIVE_C        IDE_DRIVE_C
#define HW_DRIVE_D        IDE_DRIVE_D
#define HW_DRIVE_E        IDE_DRIVE_E

#define HW_SECTOR_SIZE    IDE_SECTOR_SIZE
#endif /* (FAT_USE_IDE_INTERFACE >= 1) */ 


#if (FAT_USE_USB_INTERFACE >= 1)
#include "usbdrv.h"

#define HW_SUPPORT_WRITE  USB_SUPPORT_WRITE
#define HW_SUPPORT_ATAPI  USB_SUPPORT_ATAPI

#define HW_OK             USB_OK
#define HW_ERROR          USB_ERROR

#define HW_DRIVE_C        USB_DRIVE_C
#define HW_DRIVE_D        USB_DRIVE_D
#define HW_DRIVE_E        USB_DRIVE_E

#define HW_SECTOR_SIZE    USB_SECTOR_SIZE

#endif /* (FAT_USE_USB_INTERFACE >= 1) */


#if (FAT_USE_MMC_INTERFACE >= 1)
#include "mmcdrv.h"

#define HW_SUPPORT_WRITE  MMC_SUPPORT_WRITE
#define HW_SUPPORT_ATAPI  MMC_SUPPORT_ATAPI

#define HW_OK             MMC_OK
#define HW_ERROR          MMC_ERROR

#define HW_DRIVE_C        MMC_DRIVE_C
#define HW_DRIVE_D        MMC_DRIVE_C
#define HW_DRIVE_E        MMC_DRIVE_C

#define HW_SECTOR_SIZE    MMC_SECTOR_SIZE

#endif /* (FAT_USE_MMC_INTERFACE >= 1) */

/*-------------------------------------------------------------------------*/
/* global types                                                            */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* global macros                                                           */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* Prototypes                                                              */
/*-------------------------------------------------------------------------*/
#if (FAT_USE_IDE_INTERFACE >= 1)
#define HWInit              IDEInit
#define HWMountAllDevices   IDEMountAllDevices
#define HWGetSectorSize     IDEGetSectorSize
#define HWIsCDROMDevice     IDEIsCDROMDevice
#define HWIsZIPDevice       IDEIsZIPDevice
#define HWUnMountDevice     IDEUnMountDevice
#define HWGetTotalSectors   IDEGetTotalSectors
#define HWReadSectors       IDEReadSectors  
#define HWWriteSectors      IDEWriteSectors
#endif /* (FAT_USE_IDE_INTERFACE >= 1) */ 

#if (FAT_USE_USB_INTERFACE >= 1)
#define HWInit              USBInit
#define HWMountAllDevices   USBMountAllDevices
#define HWGetSectorSize     USBGetSectorSize
#define HWIsCDROMDevice     USBIsCDROMDevice
#define HWIsZIPDevice       USBIsZIPDevice
#define HWUnMountDevice     USBUnMountDevice
#define HWGetTotalSectors   USBGetTotalSectors
#define HWReadSectors       USBReadSectors  
#define HWWriteSectors      USBWriteSectors
#endif /* (FAT_USE_USB_INTERFACE == 1) */

#if (FAT_USE_MMC_INTERFACE >= 1)
#define HWInit              MMCInit
#define HWMountAllDevices   MMCMountAllDevices
#define HWGetSectorSize     MMCGetSectorSize
#define HWIsCDROMDevice     MMCIsCDROMDevice
#define HWIsZIPDevice       MMCIsZIPDevice
#define HWUnMountDevice     MMCUnMountDevice
#define HWGetTotalSectors   MMCGetTotalSectors
#define HWReadSectors       MMCReadSectors  
#define HWWriteSectors      MMCWriteSectors
#endif /* (FAT_USE_MMC_INTERFACE == 1) */

#endif /* !__FATDRV_H__ */

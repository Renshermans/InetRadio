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
*  18.01.03  mifi   Change Licence from GPL to BSD
*  25.01.03  mifi   Change FAT32Init 
*                   New function FAT32MountDrive and FAT32UnMountDrive
*                   Remove FAT32_MAX_FILES, a file handle will be 
*                   allocated by NutHeapAlloc, therefore we have no
*                   restrictions about the count of the open file handle.
*                   (Only by available memory)
*  27.01.03  mifi   Rename all FAT32xxx function to FATxxx.
*
*  28.01.03  mifi   Start porting to Nut/OS 3.X.X
*  29.06.03  mifi   First ATAPI-Version
*  24.07.04  mifi   Some changes to support more than one drive (datanut)
*  25.07.04  mifi   Add support for the PCMCIA_11 hardware.
*  27.07.04  mifi   Add support for IOCTL function
****************************************************************************/
#ifndef __FAT_H__
#define __FAT_H__

#include <sys/device.h>
#include "fatdrv.h"

/*-------------------------------------------------------------------------*/
/* global defines                                                          */
/*-------------------------------------------------------------------------*/

/*
 * Device mode
 */
#define FAT_MODE_IDE_HD           0x0000
#define FAT_MODE_IDE_CF           0x0001
#define FAT_MODE_MEM_CF           0x0002
#define FAT_MODE_DATANUT_CF       0x0003
#define FAT_MODE_DATANUT_IDECF    0x0004
#define FAT_MODE_DATANUT_IDEWLAN  0x0005
#define FAT_MODE_USB              0x0006
#define FAT_MODE_MMC              0x0007


/*
 * If the function should be 
 * supported, set it to 1
 */
#define FAT_SUPPORT_FORMAT        1

/*
 * IOCTL-Function
 */
#define FAT_IOCTL_QUICK_FORMAT    0x1000

/*-------------------------------------------------------------------------*/
/* global types                                                            */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* global macros                                                           */
/*-------------------------------------------------------------------------*/
#define FAT_IOCTL(_a,_b,_c)   ((NUTDEVICE *)_a)->dev_ioctl((_a), (_b), (_c))

#define FATQuickFormat(_a)    FAT_IOCTL(_a, FAT_IOCTL_QUICK_FORMAT, NULL)
 

/*-------------------------------------------------------------------------*/
/* Prototypes                                                              */
/*-------------------------------------------------------------------------*/
extern NUTDEVICE devFAT;

#if (FAT_USE_IDE_INTERFACE >= 1)
extern NUTDEVICE devFATCF;
extern NUTDEVICE devFATIDE0;
extern NUTDEVICE devFATIDE1;
#endif

#if (FAT_USE_USB_INTERFACE >= 1)
extern NUTDEVICE devFATUSB0;
#endif

#if (FAT_USE_MMC_INTERFACE >= 1)
extern NUTDEVICE devFATMMC0;
extern void FATRelease(void);

#endif


#endif                          /* !__FAT_H__ */

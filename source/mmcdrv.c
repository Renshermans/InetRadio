/****************************************************************************
*  This file is part of the MMC device driver.
*
*  Copyright (c) 2004 by Michael Fischer. All rights reserved.
*
*  Thanks to Sylvain Bissonnette for some of his low level functions.
*  Take a look at www.microsyl.com (Led Sign with MMC MemoryCard)
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
#define __MMCDRV_C__

#define LOG_MODULE  LOG_MMCDRV_MODULE

#include <stdio.h>
#include <string.h>

#include <sys/timer.h>
#include <sys/thread.h>
#include <sys/event.h>
#include <sys/heap.h>

#include "typedefs.h"
#include "portio.h"
#include "mmcdrv.h"
#include "vs10xx.h"
#include "led.h"
#include "log.h"
#include "spidrv.h"

/*==========================================================*/
/*  DEFINE: All Structures and Common Constants             */
/*==========================================================*/
#define MMC_MAX_SUPPORTED_DEVICE    1

/*
 * Drive Flags
 */
#define MMC_SUPPORT_LBA             0x0001
#define MMC_SUPPORT_LBA48           0x0002

#define MMC_READ_ONLY               0x4000
#define MMC_READY                   0x8000

#define Delay_1ms(_x) NutDelay(_x)

#define SPIDDR        DDRB
#define SPIPORT       PORTB
#define SPIPIN        PINB

/*
    PragmaLab: disable PIN-defines (already defined in 'portio.h'
#define SCLK          0x02
#define MOSI          0x04
#define MISO          0x08
#define CS            0x20
#define ENABLE        0x40
    end PragmaLab
*/

#define MMC_RESET         0
#define MMC_INIT          1
#define MMC_READ_CSD    9
#define MMC_READ_CID    10

typedef struct _drive
{
    /*
     * Interface values
     */
    WORD  wFlags;
    BYTE  bDevice;

    /*
     * LBA value
     */
    DWORD dTotalSectors;
    WORD  wSectorSize;
} DRIVE;

/*==========================================================*/
/*  DEFINE: Definition of all local Data                    */
/*==========================================================*/
static HANDLE          hMMCSemaphore;
static DRIVE           sDrive[MMC_MAX_SUPPORTED_DEVICE];

static MMC_MOUNT_FUNC *pUserMountFunc;
static MMC_MOUNT_FUNC *pUserUnMountFunc;

/*==========================================================*/
/*  DEFINE: Definition of all local Procedures              */
/*==========================================================*/


/************************************************************/
/*  MMCLock                                                 */
/************************************************************/
static void MMCLock(void)
{
    NutEventWait(&hMMCSemaphore, 0);
} /* MMCLock */

/************************************************************/
/*  MMCFree                                                 */
/************************************************************/
static void MMCFree(void)
{
    NutEventPost(&hMMCSemaphore);
} /* MMCFree */

/************************************************************/
/*  MMCSemaInit                                             */
/************************************************************/
static void MMCSemaInit(void)
{
    NutEventPost(&hMMCSemaphore);
} /* MMCSemaInit */


/************************************************************
 * int MMCDataToken(void)
 *
 * - pings the card until it gets data token
 * - returns one byte of read info (data token)
 ************************************************************/
static BYTE MMCDataToken(void)
{
    WORD i = 0xffff;
    BYTE Byte = 0xff;

    while ((Byte != 0xfe) && (--i))
    {
        Byte = SPIgetByte();
    }
    return(Byte);
} /* MMCDataToken */

/************************************************************
 * unsigned char MMCGet(void)
 *
 * - pings the card until it gets a non-0xff value
 * - returns one byte of read info
 ************************************************************/
static BYTE MMCGet(void)
{
    WORD i = 0xffff;
    BYTE Byte = 0xff;

    while ((Byte == 0xff) && (--i))
    {
        Byte = SPIgetByte();
    }

    return(Byte);
} /* MMCGet */

/************************************************************
 * void MMCCommand(unsigned char command, unsigned int px, unsigned int py)
 *
 * - send one byte of 0xff, then issue command + params + (fake) crc
 * - eat up the one command of nothing after the CRC
 ************************************************************/
static void MMCCommand(unsigned char command, unsigned int px, unsigned int py)
{
    SPIselect(SPI_DEV_MMC);

    SPIputByte(0xff);
    SPIputByte(command | 0x40);
    SPIputByte((unsigned char)((px >> 8)&0x0ff)); /* high byte of param y */
    SPIputByte((unsigned char)(px & 0x00ff));     /* low byte of param y */
    SPIputByte((unsigned char)((py >> 8)&0x0ff)); /* high byte of param x */
    SPIputByte((unsigned char)(py & 0x00ff));     /* low byte of param x */
    SPIputByte(0x95);            /* correct CRC for first command in SPI          */
                              /* after that CRC is ignored, so no problem with */
                              /* always sending 0x95                           */
    SPIputByte(0xff);
} /* MMCCommand */

/************************************************************/
/* GetCSD                                                   */
/************************************************************/
static int GetCSD (DRIVE *pDrive)
{
    int   i;
    int  nError = MMC_ERROR;
    BYTE bData[16];
    WORD wREAD_BL_LEN;
    WORD wC_SIZE;
    WORD wC_SIZE_MULT;
    WORD wDummy;
    DWORD dTotalSectors = 0;

    MMCCommand(MMC_READ_CSD, 0, 0);
    if (MMCDataToken() != 0xfe)
    {
        LogMsg_P(LOG_ERR, PSTR("error during CSD read"));
    }
    else
    {
        for (i=0; i<16; i++)
        {
            bData[i] = SPIgetByte();
        }

        SPIputByte(0xff);    /* checksum -> don't care about it for now */
        SPIputByte(0xff);    /* checksum -> don't care about it for now */

        SPIdeselect();

        /*
         * Get the READ_BL_LEN
         */
        wREAD_BL_LEN = (1 << (bData[5] & 0x0F));

        /*
         * Get the C_SIZE
         */
        wC_SIZE  = (bData[6] & 0x03);
        wC_SIZE  = wC_SIZE << 10;

        wDummy   = bData[7];
        wDummy   = wDummy << 2;
        wC_SIZE |= wDummy;

        wDummy   = (bData[8] & 0xC0);
        wDummy   = wDummy >> 6;
        wC_SIZE |= wDummy;

        /*
         * Get the wC_SIZE_MULT
         */
        wC_SIZE_MULT  = (bData[9] & 0x03);
        wC_SIZE_MULT |= wC_SIZE_MULT << 1;
        wDummy        = (bData[10] & 0x80);
        wDummy        = wDummy >> 7;
        wC_SIZE_MULT |= wDummy;
        wC_SIZE_MULT  = (1 << (wC_SIZE_MULT+2));

        dTotalSectors  = wC_SIZE+1;
        dTotalSectors *= wC_SIZE_MULT;

        pDrive->dTotalSectors = dTotalSectors;
        pDrive->wSectorSize   = wREAD_BL_LEN;

        nError = MMC_OK;
    }

    return(nError);
} /* GetCSD */

#if 0
/************************************************************/
/* GetCID                                                   */
/************************************************************/
static void GetCID(void)
{
    int i;
    BYTE bData[16];

    MMCCommand(MMC_READ_CID, 0, 0);
    if (MMCDataToken() != 0xfe)
    {
        printf("MMC: error during CID read\n");
    }
    else
    {
        printf("MMC: CID read\n");
    }

    for (i=0; i<16; i++)
    {
        bData[i] = SPIgetByte();
    }

    SPIputByte(0xff);    /* checksum -> don't care about it for now */
    SPIputByte(0xff);    /* checksum -> don't care about it for now */

    SPIdeselect();

    printf("MMC: Product Name: %c%c%c%c%c%c\n",
           bData[3], bData[4], bData[5],
           bData[6], bData[7], bData[8]);
} /* GetCID */
#endif

/************************************************************/
/*  InitMMCCard                                             */
/*                                                          */
/* - flushes card receive buffer                            */
/* - selects card                                           */
/* - sends the reset command                                */
/* - sends the initialization command, waits for card ready */
/************************************************************/
static int InitMMCCard(void)
{
    WORD i;

    /* PragmaLab: disable initit of PINS and SPI, already done in 'SystemInitIO()'
    SPIDDR = SCLK + MOSI + CS;
    SPIPORT = 0x00;
    Delay_1ms(250);
    Delay_1ms(250);
    SPIPORT |= CS;
    SPCR = (1 << SPE) | (1 << MSTR);  // enable SPI as master, set clk divider
                                      // set to max speed
    Delay_1ms(250);


    SPIdeselect();

    // start off with 80 bits of high data with card deselected

    PragmaLab: why send dummy bytes with card DEselected? This messes up the VS10XX init */
    for (i = 0; i < 10; i++)
    {
        SPIputByte(0xff);
    }

    /*end PragmaLab */

    /* send CMD0 - go to idle state */
    MMCCommand(MMC_RESET, 0, 0);

    if (MMCGet() != 1)
    {
        SPIdeselect();
        return(MMC_ERROR);  // MMC Not detected
    }

    /* send CMD1 until we get a 0 back, indicating card is done initializing */
    i = 0xffff;
    while ((SPIgetByte() != 0) && (--i))
    {
        MMCCommand(MMC_INIT, 0, 0);
    }
    if (i == 0)
    {
        SPIdeselect();
        return(MMC_ERROR);  // Init Fail
    }

    SPIdeselect();
    return(MMC_OK);
} /* InitMMCCard */

/************************************************************/
/*  ReadSectors                                             */
/************************************************************/
static int ReadSectors(DRIVE *pDrive, BYTE *pBuffer, DWORD dStartSector, WORD wSectorCount)
{
    int   nError = MMC_OK;
    int   nSector;
    WORD  wDataCount;
    DWORD dReadSector;

    pDrive = pDrive;

    for (nSector=0; nSector<wSectorCount; nSector++)
    {
        dReadSector = dStartSector + nSector;

        MMCCommand(17,(dReadSector>>7) & 0xffff, (dReadSector<<9) & 0xffff);
        if (MMCDataToken() != 0xfe)
        {
            nError = MMC_ERROR;
            SPIdeselect();
            break;
        }

        for (wDataCount=0; wDataCount<512; wDataCount++)
        { /* read the sector */
            *pBuffer = SPIgetByte();
            pBuffer++;
        }

        SPIputByte(0xff);    /* checksum -> don't care about it for now */
        SPIputByte(0xff);    /* checksum -> don't care about it for now */
        SPIdeselect();
    }

    return(nError);
} /* ReadSectors */

#if (MMC_SUPPORT_WRITE == 1)
/************************************************************/
/*  WriteSectors                                            */
/************************************************************/
static BYTE WriteSectors(DRIVE *pDrive, BYTE *pBuffer, DWORD dStartSector, WORD wSectorCount)
{
    int   nError = MMC_OK;
    int   nSector;
    WORD  wDataCount;
    DWORD dWriteSector;

    pDrive = pDrive;

    for (nSector=0; nSector<wSectorCount; nSector++)
    {
        dWriteSector = dStartSector + nSector;

        MMCCommand(24, (dWriteSector>>7)& 0xffff, (dWriteSector<<9)& 0xffff);
        if (MMCGet() == 0xff)
        {
            nError = MMC_ERROR;
            SPIdeselect();
            break;
        }

        SPIputByte(0xfe);  // Send Start Byte

        for (wDataCount=0; wDataCount<512; wDataCount++)
        { /* read the sector */
            SPIputByte(*pBuffer);
            pBuffer++;
        }

        SPIputByte(0xff);  /* checksum -> don't care about it for now */
        SPIputByte(0xff);  /* checksum -> don't care about it for now */
        SPIputByte(0xff);  /* Read "data response byte"               */

        wDataCount = 0xffff;
        while ((SPIgetByte() == 0x00) && (--wDataCount)); /* wait for write finish */
        if (wDataCount == 0)
        {
            nError = MMC_ERROR;
            SPIdeselect();
            break;
        }

        SPIdeselect();
    }

    return(nError);
}
#endif /* WriteSectors */

/*==========================================================*/
/*  DEFINE: All code exported                               */
/*==========================================================*/
/************************************************************/
/*  MMCInit                                                 */
/************************************************************/
int MMCInit(int nMMCMode, MMC_MOUNT_FUNC *pMountFunc,
            MMC_MOUNT_FUNC *pUnMountFunc)
{
    int  nError = MMC_OK;
    BYTE bIndex;

    nMMCMode         = nMMCMode;
    pUserMountFunc   = pMountFunc;
    pUserUnMountFunc = pUnMountFunc;

    for (bIndex=0; bIndex<MMC_MAX_SUPPORTED_DEVICE; bIndex++)
    {
        memset((BYTE *) & sDrive[bIndex], 0x00, sizeof(DRIVE));

        sDrive[bIndex].bDevice  = bIndex;
    }

    MMCSemaInit();

    nError = InitMMCCard();
    if (nError == MMC_OK)
    {
        sDrive[MMC_DRIVE_C].wFlags = MMC_READY;
        //GetCID();
    }

    return(nError);
} /* MMCInit */

/************************************************************/
/*  MMCMountAllDevices                                      */
/************************************************************/
int MMCMountAllDevices(int nMMCMode, BYTE *pSectorBuffer)
{
    int    nError = MMC_ERROR;
    DRIVE *pDrive;

    nMMCMode = nMMCMode;

    pDrive = NULL;

    MMCLock();

    pDrive = &sDrive[MMC_DRIVE_C];
    if (pDrive->wFlags & MMC_READY)
    {
        nError = GetCSD(pDrive);
    }

    MMCFree();

    if (nError == MMC_OK)
    {
        nError = MMCReadSectors(MMC_DRIVE_C, pSectorBuffer, 0, 1);
    }

    return(nError);
} /* MMCMountDevice */

/************************************************************/
/*  MMCGetSectorSize                                        */
/************************************************************/
int MMCGetSectorSize(BYTE bDevice)
{
    int    nSectorSize;
    DRIVE *pDrive;

    nSectorSize = 0;

    MMCLock();

    if (bDevice >= MMC_MAX_SUPPORTED_DEVICE)
    {
        nSectorSize = 0;
    }
    else
    {
        pDrive = &sDrive[bDevice];
        nSectorSize = pDrive->wSectorSize;
    }

    MMCFree();

    return(nSectorSize);
} /* MMCGetSectorSize */

/************************************************************/
/*  MMCIsCDROMDevice                                        */
/************************************************************/
int MMCIsCDROMDevice(BYTE bDevice)
{
    return(FALSE);
} /* MMCIsCDROMDevice */

/************************************************************/
/*  MMCIsZIPDevice                                          */
/************************************************************/
int MMCIsZIPDevice(BYTE bDevice)
{
    return(FALSE);
} /* MMCIsZIPDevice */

/************************************************************/
/*  MMCUnMountDevice                                        */
/************************************************************/
int MMCUnMountDevice(BYTE bDevice)
{
    return(MMC_OK);
} /* MMCUnMountDevice */

/************************************************************/
/*  MMCGetTotalSectors                                      */
/************************************************************/
DWORD MMCGetTotalSectors(BYTE bDevice)
{
    DWORD  dwTotalSectors;
    DRIVE *pDrive;

    dwTotalSectors = 0;

    MMCLock();

    if (bDevice >= MMC_MAX_SUPPORTED_DEVICE)
    {
        dwTotalSectors = 0;
    }
    else
    {
        pDrive = &sDrive[bDevice];
        dwTotalSectors = pDrive->dTotalSectors;

        //dwTotalSectors -= 64;
    }

    MMCFree();

    return(dwTotalSectors);
} /* MMCGetTotalSectors */

/************************************************************/
/*  MMCReadSectors                                          */
/************************************************************/
int MMCReadSectors(BYTE bDevice, void *pData, DWORD dwStartSector, WORD wSectorCount)
{
    int    nError;
    WORD   wReadCount;
    DRIVE *pDrive = 0;
    BYTE  *pByte;

    nError = MMC_OK;

    MMCLock();

    if (bDevice >= MMC_MAX_SUPPORTED_DEVICE)
    {
        nError = MMC_DRIVE_NOT_FOUND;
    }
    else
    {
        pDrive = &sDrive[bDevice];
        if ((pDrive->wFlags & MMC_READY) == 0)
        {
            nError = MMC_DRIVE_NOT_FOUND;
        }
        else
        {
            if ((dwStartSector + wSectorCount) > pDrive->dTotalSectors)
            {
                nError = MMC_PARAM_ERROR;
            }
        }
    }

    if (nError == MMC_OK)
    {
        pByte = (BYTE *)pData;

        if (wSectorCount != 1)
        {
            while (wSectorCount > 0)
            {

                if (wSectorCount < 256)
                {
                    wReadCount = wSectorCount;
                }
                else
                {
                    wReadCount = 256;
                }

                nError = ReadSectors(pDrive, pByte, dwStartSector, wReadCount);
                if (nError != MMC_OK)
                {
                    break;
                }

                dwStartSector += wReadCount;
                wSectorCount -= wReadCount;
                pByte += (wReadCount * pDrive->wSectorSize);
            }
        }
        else
        {
            nError = ReadSectors(pDrive, pByte, dwStartSector, 1);
        }
    }

    MMCFree();

    return(nError);
} /* MMCReadSectors */

#if (MMC_SUPPORT_WRITE == 1)
/************************************************************/
/*  MMCWriteSectors                                         */
/************************************************************/
int MMCWriteSectors(BYTE   bDevice,      void *pData,
                    DWORD dwStartSector, WORD  wSectorCount)
{
    int    nError;
    WORD   wWriteCount;
    DRIVE *pDrive = 0;
    BYTE  *pByte;

    nError = MMC_OK;

    MMCLock();

    if (bDevice >= MMC_MAX_SUPPORTED_DEVICE)
    {
        nError = MMC_DRIVE_NOT_FOUND;
    }
    else
    {
        pDrive = &sDrive[bDevice];

        if ((dwStartSector + wSectorCount) > pDrive->dTotalSectors)
        {
            nError = MMC_PARAM_ERROR;
        }
        if ((pDrive->wFlags & MMC_READY) == 0)
        {
            nError = MMC_DRIVE_NOT_FOUND;
        }
        if (pDrive->wFlags & MMC_READ_ONLY)
        {
            nError = MMC_NOT_SUPPORTED;
        }
    }

    if (nError == MMC_OK)
    {
        pByte = (BYTE *) pData;
        while (wSectorCount > 0)
        {

            if (wSectorCount < 256)
            {
                wWriteCount = wSectorCount;
            }
            else
            {
                wWriteCount = 256;
            }

            nError = WriteSectors(pDrive, pByte, dwStartSector, wWriteCount);
            if (nError != MMC_OK)
            {
                break;
            }

            dwStartSector += wWriteCount;
            wSectorCount  -= wWriteCount;
            pByte         += (wWriteCount * MMC_SECTOR_SIZE);
        }
    }

    MMCFree();

    return(nError);
} /* MMCWriteSectors */
#endif





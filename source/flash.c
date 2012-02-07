/* ========================================================================
 * [PROJECT]    SIR
 * [MODULE]     Flash
 * [TITLE]      Routines for Atmel AT45 serial dataflash memory chips.
 * [FILE]       flash.c
 * [VSN]        1.0
 * [CREATED]    11042007
 * [LASTCHNGD]  11042007
 * [COPYRIGHT]  Copyright (C) STREAMIT BV 2010
 * [PURPOSE]    contains all interface- and low-level routines to
 *              read/write/delete blocks in the serial DataFlash (AT45DBXX)
 * ======================================================================== */

#define LOG_MODULE  LOG_FLASH_MODULE

//#include <stdio.h>
#include <cfg/os.h>
#include <cfg/memory.h>

#include <sys/timer.h>

#include <string.h>
#include <stdlib.h>

#include "typedefs.h"
#include "flash.h"
#include "portio.h"
#include "log.h"
#include "spidrv.h"


/*-------------------------------------------------------------------------*/
/* local defines                                                           */
/*-------------------------------------------------------------------------*/
#ifndef MAX_AT45_CMDLEN
#define MAX_AT45_CMDLEN         8
#endif

#ifndef AT45_ERASE_WAIT
#define AT45_ERASE_WAIT         3000
#endif

#ifndef AT45_CHIP_ERASE_WAIT
#define AT45_CHIP_ERASE_WAIT    50000
#endif

#ifndef AT45_WRITE_POLLS
#define AT45_WRITE_POLLS        1000
#endif

#define DFCMD_READ_PAGE         0xD2    /* Read main memory page. */
#define DFCMD_READ_STATUS       0xD7    /* Read status register. */
#define DFCMD_CONT_READ         0xE8    /* Continuos read. */
#define DFCMD_PAGE_ERASE        0x81    /* Page erase. */
#define DFCMD_BUF1_WRITE        0x84    /* Buffer 1 write. */
#define DFCMD_BUF1_FLASH        0x83    /* Buffer 1 flash with page erase. */

/*
 *  \brief last page of flash (264 bytes) can be dedicated for parameter storage
 *   Special routines are provided for that goal but can be disabled here to save
 *   codespace (about 360 bytes of code for GCC)
 */

//#define USE_FLASH_PARAM_PAGE

/*-------------------------------------------------------------------------*/
/* typedefs & structs                                                      */
/*-------------------------------------------------------------------------*/
/*!
 * \brief Known device type entry.
 */
typedef struct _AT45_DEVTAB
{
    u_long devt_pages;
    u_int devt_pagsiz;
    u_int devt_offs;
    u_char devt_srmsk;
    u_char devt_srval;
} AT45_DEVTAB;

/*!
 * \brief Active device entry.
 */
typedef struct _AT45DB_DCB
{
    AT45_DEVTAB *dcb_devt;
    u_char dcb_cmdbuf[MAX_AT45_CMDLEN];
} AT45DB_DCB;

/*!
 * \brief Table of known Dataflash types.
 */
AT45_DEVTAB at45_devt[] = {
    {512,  264,  9,  0x3C, 0x0C},   // AT45DB011B - 128kB
    {1025, 264,  9,  0x3C, 0x14},   // AT45DB021B - 256kB
    {2048, 264,  9,  0x3C, 0x1C},   // AT45DB041B - 512kB
    {4096, 264,  9,  0x3C, 0x24},   // AT45DB081B - 1MB
    {4096, 528,  10, 0x3C, 0x2C},   // AT45DB0161B - 2MB
    {8192, 528,  10, 0x3C, 0x34},   // AT45DB0321B - 4MB
    {8192, 1056, 11, 0x38, 0x38},   // AT45DB0642 - 8MB
    {0,    0,    0,  0,    0}       // End of table
};

/*-------------------------------------------------------------------------*/
/* local variable definitions                                              */
/*-------------------------------------------------------------------------*/
/*!
 * \brief Table of active devices.
 */
static AT45DB_DCB dcbtab;

/*-------------------------------------------------------------------------*/
/* local routines (prototyping)                                            */
/*-------------------------------------------------------------------------*/
static int At45dbTransfer(CONST void *txbuf, void *rxbuf, int xlen, CONST void *txnbuf, void *rxnbuf, int xnlen);

/*!
 * \addtogroup SerialFlash
 */

/*@{*/

/*-------------------------------------------------------------------------*/
/*                         start of code                                   */
/*-------------------------------------------------------------------------*/

/*!
 * \brief mid-level SPI-interface routine
 *
 *  This routine handles sending a command an reading back the reply in one routine
 *  It will perform 'xlen' + 'xnlen' SPI-byte cycles. During the 'xlen' cycles, data in 'txbuf'
 *  is sent using the SPI, and each resulting byte is stored in 'rxbuf'. Then it starts with
 *  the 'xnlen' cycles whereby the contents of the 'txnbuf' are sent using the SPI.
 *  Each resulting byte then is stored in 'rxnbuf'
 */
//                                        cb,          cb,        len,             tdata,         rdata,      datalen
static int At45dbTransfer(CONST void *txbuf, void *rxbuf, int xlen, CONST void *txnbuf, void *rxnbuf, int xnlen)
{   int i;
    u_char *ptTxbuf, *ptRxbuf;

    SPIselect(SPI_DEV_FLASH);

    ptTxbuf=(u_char*)txbuf;
    ptRxbuf=(u_char*)rxbuf;
    /*
     *  send command, store the bytes that were read the same time
     */
    for (i=0; i<xlen; ++i)
    {
        *ptRxbuf++=SPItransferByte(*ptTxbuf++);
    }

    ptTxbuf=(u_char*)txnbuf;
    ptRxbuf=(u_char*)rxnbuf;
    /*
     *  send dummy data, store the bytes that were read the same time
     */
    for (i=0; i<xnlen; ++i)
    {
        *ptRxbuf++=SPItransferByte(*ptTxbuf++);
    }

    SPIdeselect();

    return(0);  // always...

}

/*!
 * \brief send a command to the AT45dbXX
 *
 */
int At45dbSendCmd(u_char op, u_long parm, int len, CONST void *tdata, void *rdata, int datalen)
{
    u_char *cb = dcbtab.dcb_cmdbuf;

    if (len > MAX_AT45_CMDLEN)
    {
        return (-1);
    }
    memset(cb, 0, len);
    cb[0] = op;
    if (parm)
    {
        cb[1] = (u_char) (parm >> 16);
        cb[2] = (u_char) (parm >> 8);
        cb[3] = (u_char) parm;
    }
    return (At45dbTransfer(cb, cb, len, tdata, rdata, datalen));
}

/*!
 * \brief read status
 *
 */
u_char At45dbGetStatus()
{
    u_char buf[2] = { DFCMD_READ_STATUS, 0xFF};

    if (At45dbTransfer(buf, buf, 2, NULL, NULL, 0))
    {
        return(u_char) - 1;
    }
    return (buf[1]);
}

/*!
 * \brief Wait until flash memory cycle finished.
 *
 * \return 0 on success or -1 in case of an error.
 */
int At45dbWaitReady(u_long tmo, int poll)
{
    u_char sr;

    while (((sr = At45dbGetStatus()) & 0x80) == 0)
    {
        if (!poll)
        {
            NutSleep(1);
        }
        if (tmo-- == 0)
        {
            return (-1);
        }
    }
    return (0);
}

/*!
 * \brief runtime detection of serial flash device
 */
int At45dbInit()
{
    u_char sr;
    u_char i;

    At45dbGetStatus();
    sr = At45dbGetStatus();

    for (i=0; at45_devt[i].devt_pages; i++)
    {
        if ((sr & at45_devt[i].devt_srmsk) == at45_devt[i].devt_srval)
        {
            dcbtab.dcb_devt = &at45_devt[i];
            break;
        }
    }
    return (i);
}

/*!
 * \brief Erase sector at the specified offset.
 */
int At45dbPageErase(u_int pgn)
{
    return (At45dbSendCmd(DFCMD_PAGE_ERASE, pgn, 4, NULL, NULL, 0));
}

/*!
 * \brief Erase entire flash memory chip.
 */
int At45dbChipErase(void)
{
    return (-1);
}

/*!
 * \brief Read data from flash memory.
 *
 * \param pgn  Page number to read, starting at 0.
 * \param data Points to a buffer that receives the data.
 * \param len  Number of bytes to read.
 *
 * \return 0 on success or -1 in case of an error.
 */
int At45dbPageRead(u_long pgn, void *data, u_int len)
{
    pgn <<= dcbtab.dcb_devt->devt_offs;
    return (At45dbSendCmd(DFCMD_CONT_READ, pgn, 8, data, data, len));
}

/*!
 * \brief Write data into flash memory.
 *
 * The related sector must have been erased before calling this function.
 *
 * \param pgn  Start location within the chip, starting at 0.
 * \param data Points to a buffer that contains the bytes to be written.
 * \param len  Number of bytes to write.
 *
 * \return 0 on success or -1 in case of an error.
 */
int At45dbPageWrite(u_long pgn, CONST void *data, u_int len)
{
    int rc = -1;
    void *rp;

    if ((rp = malloc(len)) != NULL)
    {
        /* Copy data to dataflash RAM buffer. */
        if (At45dbSendCmd(DFCMD_BUF1_WRITE, 0, 4, data, rp, len) == 0)
        {
            /* Flash RAM buffer. */
            pgn <<= dcbtab.dcb_devt->devt_offs;
            if (At45dbSendCmd(DFCMD_BUF1_FLASH, pgn, 4, NULL, NULL, 0) == 0)
            {
                rc = At45dbWaitReady(AT45_WRITE_POLLS, 1);
            }
        }
        free(rp);
    }
    return (rc);
}

#ifdef USE_FLASH_PARAM_PAGE

u_long At45dbParamPage(void)
{
#ifdef AT45_CONF_PAGE
    return (AT45_CONF_PAGE);
#else
    return (dcbtab.dcb_devt->devt_pages - 1);
#endif
}

int At45dbParamSize(void)
{
    int rc;

#ifdef AT45_CONF_SIZE
    rc = AT45_CONF_SIZE;
#else
    rc = dcbtab.dcb_devt->devt_pagsiz;
#endif
    return (rc);
}

/*!
 * \brief Load configuration parameters from flash memory.
 *
 * \param pos  Start location within configuration sector.
 * \param data Points to a buffer that receives the contents.
 * \param len  Number of bytes to read.
 *
 * \return Always 0.
 */
int At45dbParamRead(u_int pos, void *data, u_int len)
{
    int rc = -1;
    u_char *buff;
    int csize = At45dbParamSize();
    u_long cpage = At45dbParamPage();

    /* Load the complete configuration area. */
    if (csize > len && (buff = malloc(csize)) != NULL)
    {
        rc = At45dbPageRead(cpage, buff, csize);
        /* Copy requested contents to caller's buffer. */
        memcpy(data, buff + pos, len);
        free(buff);
    }
    return (rc);
}

/*!
 * \brief Store configuration parameters in flash memory.
 *
 * \param pos   Start location within configuration sector.
 * \param data  Points to a buffer that contains the bytes to store.
 * \param len   Number of bytes to store.
 *
 * \return 0 on success or -1 in case of an error.
 */
int At45dbParamWrite(u_int pos, CONST void *data, u_int len)
{
    int rc = -1;
    u_char *buff;
    int csize = At45dbParamSize();
    u_long cpage = At45dbParamPage();

    /* Load the complete configuration area. */
    if (csize > len && (buff = malloc(csize)) != NULL)
    {
        rc = At45dbPageRead(cpage, buff, csize);
        /* Compare old with new contents. */
        if (memcmp(buff + pos, data, len))
        {
            /* New contents differs. Copy it into the sector buffer. */
            memcpy(buff + pos, data, len);
            /* Erase sector and write new data. */
            rc = At45dbPageWrite(cpage, buff, csize);
        }
        free(buff);
    }
    return (rc);
}

#endif // USE_FLASH_PARAM_PAGE

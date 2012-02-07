/* ========================================================================
 * [PROJECT]    SIR100
 * [MODULE]     SPI
 * [TITLE]      SPI source file
 * [FILE]       spi.c
 * [VSN]        1.0
 * [CREATED]    06102006
 * [LASTCHNGD]  06102006
 * [COPYRIGHT]  Copyright (C) STREAMIT BV 2010
 * [PURPOSE]    contains all interface- and low-level routines to
 *              control audiofunctions of the VS1003//AT45DBXX/MMC-SD(HC)card
 * ======================================================================== */

#define LOG_MODULE  LOG_SPIDRV_MODULE

/*-------------------------------------------------------------------------*/
/* includes                                                                */
/*-------------------------------------------------------------------------*/
#include <stdio.h>

#include "system.h"
#include "spidrv.h"
#include "portio.h"
#include "log.h"

#include "vs10xx.h"

#include <sys/timer.h>

/*-------------------------------------------------------------------------*/
/* local defines                                                           */
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/* typedefs & structs                                                      */
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/* local variable definitions                                              */
/*-------------------------------------------------------------------------*/
static u_char g_Speedmode;

/*-------------------------------------------------------------------------*/
/* local routines (prototyping)                                            */
/*-------------------------------------------------------------------------*/

/*!
 * \addtogroup Drivers
 */

/*@{*/

/*-------------------------------------------------------------------------*/
/*                         start of code                                   */
/*-------------------------------------------------------------------------*/
/*!
 * \brief enable SPI-logic for given device
 *
 * SPI bits:
 * SPSR:  SPI2X         double speed as set in SPR0, SPR1
 * SPCR:  SPIE:         SPI interrupt enable
 *        SPE:          SPI enable
 *        SPR1, SPR0:   clockrate
 *        MSTR:         set to master
 *
 * SPI2X SPR1 SPR0 for a ATMEGA @  14.7456 MHz
 *
 * 1     0    0:  fosc/2  = 136 ns  -> 7.37 MHz
 * 0     0    0:  fosc/4  = 271 ns  -> 3.6864 MHz
 * 1     0    1:  fosc/8  = 542 ns  -> 1.8432 MHz
 * 0     0    1:  fosc/16 = 1085 ns -> 0.9216 MHz
 *
 */

void SPIselect(TSPIDevice Device)
{

    // set SPI-speed for selected device
    if (Device==SPI_DEV_VS10XX)
    {
        if (g_Speedmode==SPEED_SLOW)
        {
            // set speed to Fosc/8
            outb(SPSR, BV(SPI2X));
            outb(SPCR, BV(MSTR) | BV(SPE) | BV(SPR0));
        }
        else if (g_Speedmode==SPEED_FAST)
        {
            // set speed to Fosc/4
            outb(SPSR, 0);
            outb(SPCR, BV(MSTR) | BV(SPE));
        }
        else if (g_Speedmode==SPEED_ULTRA_FAST)
        {
            // set speed to Fosc/2
            outb(SPSR, BV(SPI2X));
            outb(SPCR, BV(MSTR) | BV(SPE));
        }
        else
        {
            LogMsg_P(LOG_ERR,PSTR("invalid Speed"));
        }
    }
    else if (Device==SPI_DEV_FLASH)
    {
        // set speed for flash to Fosc/2
        outb(SPSR, BV(SPI2X));
        outb(SPCR, BV(MSTR) | BV(SPE));
    }
    else
    {
        // set speed for flash to Fosc/2
//        outb(SPSR, BV(SPI2X));
//        outb(SPCR, BV(MSTR) | BV(SPE));

        // set speed for card to Fosc/4
        outb(SPSR, 0);
        outb(SPCR, BV(MSTR) | BV(SPE));

        // set speed for card to Fosc/8
//        outb(SPSR, BV(SPI2X));
//        outb(SPCR, BV(MSTR) | BV(SPE) | BV(SPR0));

        // set speed for card to Fosc/16
//        outb(SPSR, 0);
//        outb(SPCR, BV(MSTR) | BV(SPE) | BV(SPR0));
    }

    // enable selected device
    switch (Device)
    {
        case SPI_DEV_VS10XX:
            {
                sbi(FLASH_OUT_WRITE, FLASH_ENABLE);    // disable serial Flash
                sbi(MMCVS_OUT_WRITE, MMC_ENABLE);      // disable MMC/SDHC
                sbi(MMCVS_OUT_WRITE, VS_ENABLE);       // enable VS10XX
                break;
            }
        case SPI_DEV_FLASH:
            {
                sbi(MMCVS_OUT_WRITE, MMC_ENABLE);      // disable MMC/SDHC
                cbi(MMCVS_OUT_WRITE, VS_ENABLE);       // disable VS10XX
                cbi(FLASH_OUT_WRITE, FLASH_ENABLE);    // enable serial Flash
                break;
            }
        case SPI_DEV_MMC:
            {
                sbi(FLASH_OUT_WRITE, FLASH_ENABLE);    // disable serial Flash
                cbi(MMCVS_OUT_WRITE, VS_ENABLE);       // disable VS10XX
                cbi(MMCVS_OUT_WRITE, MMC_ENABLE);      // enable MMC/SDHC
                break;
            }
        default: break;
    }
}

/*!
 * \brief disable SPI-logic for ALL devices
 *
 */
void SPIdeselect()
{
    sbi(FLASH_OUT_WRITE, FLASH_ENABLE);    // disable serial Flash
    cbi(MMCVS_OUT_WRITE, VS_ENABLE);       // disable VS10XX
    sbi(MMCVS_OUT_WRITE, MMC_ENABLE);      // disable MMC/SDHC
}

/*!
 * \brief not all devices can operate always on maximum speed. This routine determines the several speed modes.
 *
 */
void SPImode(u_char data)
{
    g_Speedmode = data;
}

u_char SPIgetmode(void)
{
    return(g_Speedmode);
}
/*!
 * \brief send a byte using SPI, ignore result
 *
 */
void SPIputByte(u_char data)
{
    SPDR = data;
    while (!(SPSR & (1<<SPIF)));     // wait for completion
}

/*!
 * \brief read byte using SPI, don't use any input
 *
 */
u_char SPIgetByte()
{
    SPDR = 0xFF;                     // dummy
    while (!(SPSR & (1<<SPIF)));     // wait for completion
    return(SPDR);                    // return with byte shifted in from receiver
}

/*!
 * \brief send byte using SPI, return result
 *
 */
u_char SPItransferByte(u_char data)
{
    SPDR = data;
    while (!(SPSR & (1<<SPIF)));     // wait for completion
    return(SPDR);                    // return with byte shifted in from receiver
}

/*!
 * \brief Initialise SPI registers (speed)
 *
 *  Note that the IO-lines (SCK, SI, SO) are already set in 'SysInitIO()'
 *  in the main-module
 *
 */
void SPIinit()
{
    sbi(FLASH_OUT_WRITE, FLASH_ENABLE);    // disable serial Flash
    cbi(MMCVS_OUT_WRITE, VS_ENABLE);       // disable VS10XX
    sbi(MMCVS_OUT_WRITE, MMC_ENABLE);      // disable MMC/SDHC
}


/*  様様  End Of File  様様様様 様様様様様様様様様様様様様様様様様様様様様様 */



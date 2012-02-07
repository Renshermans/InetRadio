/* ========================================================================
 * [PROJECT]    SIR100
 * [MODULE]     SPI
 * [TITLE]      spi header file
 * [FILE]       spi.h
 * [VSN]        1.0
 * [CREATED]    12052007
 * [LASTCHNGD]  12052007
 * [COPYRIGHT]  Copyright (C) STREAMIT BV 2010
 * [PURPOSE]    API and global defines for SPI module
 * ======================================================================== */

#ifndef _SPI_H
#define _SPI_H


/*-------------------------------------------------------------------------*/
/* global defines                                                          */
/*-------------------------------------------------------------------------*/
#define SPEED_SLOW         0
#define SPEED_FAST         1
#define SPEED_ULTRA_FAST   2

/*-------------------------------------------------------------------------*/
/* typedefs & structs                                                      */
/*-------------------------------------------------------------------------*/
typedef enum
{
    SPI_DEV_VS10XX=0,
    SPI_DEV_FLASH,
    SPI_DEV_MMC,

    SPI_NROF_DEVICES        // keep last
}TSPIDevice;
/*--------------------------------------------------------------------------*/
/*  Global variables                                                        */
/*--------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* export global routines (interface)                                      */
/*-------------------------------------------------------------------------*/
extern void SPIselect(TSPIDevice Device);
extern void SPIdeselect(void);
extern void SPIputByte(u_char bByte);       // send byte using SPI, ignore result
extern u_char SPIgetByte(void);             // read byte using SPI, don't use any input
extern u_char SPItransferByte(u_char);      // send byte using SPI, return result
extern void SPIinit(void);                  // initialise SPI-registers (speed, mode)
extern void SPImode(u_char data);
extern u_char SPIgetmode(void);

#endif /* _SPI_H */
/*  様様  End Of File  様様様様 様様様様様様様様様様様様様様様様様様様様様様 */


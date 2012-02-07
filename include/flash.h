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
#ifndef _Flash_H
#define _Flash_H


#include <sys/types.h>

/*-------------------------------------------------------------------------*/
/* global defines                                                          */
/*-------------------------------------------------------------------------*/

/*
 *  next defines have a 1-1 relationship to the index in the 'at45_devt'
 *  array, so do not change the values
 */
#define AT45DB011B  0
#define AT45DB021B  1
#define AT45DB041B  2
#define AT45DB081B  3
#define AT45DB0161B 4
#define AT45DB0321B 5
#define AT45DB0642  6

/*-------------------------------------------------------------------------*/
/* typedefs & structs                                                      */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/*  Global variables                                                       */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* export global routines (interface)                                      */
/*-------------------------------------------------------------------------*/

extern int At45dbSendCmd(u_char op, u_long parm, int len, CONST void *tdata, void *rdata, int datalen);
extern u_char At45dbGetStatus(void);
extern int At45dbWaitReady(u_long tmo, int poll);
extern int At45dbInit(void);
extern int At45dbPageErase(u_int off);
extern int At45dbChipErase(void);
extern int At45dbPageRead(u_long pgn, void *data, u_int len);
extern int At45dbPageWrite(u_long pgn, CONST void *data, u_int len);

#ifdef USE_FLASH_PARAM_PAGE
extern int At45dbParamRead(u_int pos, void *data, u_int len);
extern int At45dbParamWrite(u_int pos, CONST void *data, u_int len);
#endif // USE_FLASH_PARAM_PAGE

#endif /* _Flash_H */
/*  様様  End Of File  様様様様 様様様様様様様様様様様様様様様様様様様様様様 */







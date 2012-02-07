/* ========================================================================
 * [PROJECT]    SIR
 * [MODULE]     Remote Control
 * [TITLE]      remote control header file
 * [FILE]       remcon.h
 * [VSN]        1.0
 * [CREATED]    1 july 2003
 * [LASTCHNGD]  1 july 2003
 * [COPYRIGHT]  Copyright (C) STREAMIT BV 2010
 * [PURPOSE]    remote control routines for SIR
 * ======================================================================== */

/*-------------------------------------------------------------------------*/
/* global defines                                                          */
/*-------------------------------------------------------------------------*/
#define RC_OK                 0x00
#define RC_ERROR              0x01
#define RC_BUSY               0x04

#define RCST_IDLE             0x00
#define RCST_WAITFORLEADER    0x01
#define RCST_SCANADDRESS      0x02
#define RCST_SCANDATA         0x03

#define RC_INT_SENS_MASK      0x03
#define RC_INT_FALLING_EDGE   0x02
#define RC_INT_RISING_EDGE    0x03

#define IR_RECEIVE            4
#define IR_BUFFER_SIZE        1


/*-------------------------------------------------------------------------*/
/* export global routines (interface)                                      */
/*-------------------------------------------------------------------------*/
void    RcInit(void);


/*  様様  End Of File  様様様様 様様様様様様様様様様様様様様様様様様様様様様 */














/* ========================================================================
 * [PROJECT]    SIR
 * [MODULE]     LED
 * [TITLE]      LED control header file
 * [FILE]       led.h
 * [VSN]        1.0
 * [CREATED]    1 augustus 2003
 * [LASTCHNGD]  23 augustus 2003
 * [COPYRIGHT]  Copyright (C) STREAMIT BV 2010
 * [PURPOSE]    LED control routines for SIR
 * ======================================================================== */

/*-------------------------------------------------------------------------*/
/* global defines                                                          */
/*-------------------------------------------------------------------------*/

#define LED_OFF         0
#define LED_ON          1
#define LED_POWER_ON    2
#define LED_POWER_OFF   3
#define LED_TOGGLE      4
#define LED_FLASH_ON    5
#define LED_FLASH_OFF   6

/*-------------------------------------------------------------------------*/
/* typedefs & structs                                                      */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* export global routines (interface)                                      */
/*-------------------------------------------------------------------------*/
void LedInit(void);
void LedControl(u_char);
u_char LedGetStatus(void);

/*  様様  End Of File  様様様様 様様様様様様様様様様様様様様様様様様様様様様 */














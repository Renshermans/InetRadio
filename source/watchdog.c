/*
 *  Copyright STREAMIT BV, 2010.
 *
 *  Project             : SIR
 *  Module              : watchdog
 *  File name  $Workfile: watchdog.c  $
 *       Last Save $Date: 2007/01/16 11:17:24  $
 *             $Revision: 0.1  $
 *  Creation Date       : 2007/01/16 11:17:25
 *
 *  Description         : Watchdog routines for the ATMega128/256
 *                        Inspired by the arch/xxx/dev/watchdog routines in
 *                        NutOS 4.2.1 and higher.
 *
 */

#define LOG_MODULE  LOG_WATCHDOG_MODULE
/*--------------------------------------------------------------------------*/
/*  Include files                                                           */
/*--------------------------------------------------------------------------*/
#include <stdio.h>

#ifdef __AVR__
#ifdef __GNUC__
    #include <avr/wdt.h>
#endif /* #ifdef __GNUC__ */
#endif /* #ifdef __AVR__ */

#include "watchdog.h"

/*--------------------------------------------------------------------------*/
/*  Constant definitions                                                    */
/*--------------------------------------------------------------------------*/
/*!
 * \brief Watchdog oscillator frequency.
 */
#ifndef NUT_WDT_FREQ
#define NUT_WDT_FREQ    128000UL
#endif

#ifndef WDTO_15MS
#define WDTO_15MS   0x00
#define WDTO_30MS   0x01
#define WDTO_60MS   0x02
#define WDTO_120MS  0x03
#define WDTO_250MS  0x04
#define WDTO_500MS  0x05
#define WDTO_1S     0x06
#define WDTO_2S     0x07
#define WDTO_4S     0x20
#define WDTO_8S     0x21
#endif /* #ifndef WDTO_15MS */

#define WD_CONTROL      WDTCSR

/*--------------------------------------------------------------------------*/
/*  Type declarations                                                       */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*  Local variables                                                         */
/*--------------------------------------------------------------------------*/
/*!\brief Counts nested calls */
static unsigned char g_byNested;

/*!\brief Watchdog Timeout constant */
static unsigned char g_byWdtDivider;

/*--------------------------------------------------------------------------*/
/*  Global variables                                                        */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*  Local functions                                                         */
/*--------------------------------------------------------------------------*/

#ifndef wdt_enable
#define wdt_enable(tmo) \
{ \
    register u_char s = _BV(WDCE) | _BV(WDE); \
    register u_char r = tmo | _BV(WDE); \
    asm("in R0, 0x3F\n"     \
        "cli\n"             \
        "wdr\n"             \
        "sts 0x60, %s\n"    \
        "sts 0x60, %r\n"    \
        "out 0x3F, R0\n");  \
}
#endif /* #ifndef wdt_enable */

#ifndef wdt_disable
#define wdt_disable() \
{ \
    asm("in R0, $3F\n"      \
        "cli\n"                 \
        "wdr\n"                 \
        "in      r24,0x34\n"    \
        "andi    r24,0xF7\n"    \
        "out     0x34,r24\n"    \
        "lds     r24,0x0060\n"  \
        "ori     r24,0x18\n"    \
        "sts     0x0060,r24\n"  \
        "clr     r2\n"          \
        "sts     0x0060,r2\n"   \
        "out 0x3F, R0\n");      \
}
#endif /* #ifndef wdt_disable */

#ifndef wdt_reset
#define wdt_reset() \
{ \
    _WDR(); \
}
#endif /* #ifndef wdt_reset */

/*--------------------------------------------------------------------------*/
/*  Global functions                                                        */
/*--------------------------------------------------------------------------*/

void WatchDogStart(unsigned long ms)
{
    const unsigned long unPrescaleFactor = 2048UL;       // PragmaLab: check datasheet Mega2561

    unsigned char byPrescale = 1;

    wdt_reset();

    /*
     * Calculate the needed prescaling for the timer ticks
     */
    while (((unPrescaleFactor << byPrescale) / (NUT_WDT_FREQ / 1000UL)) < ms)
    {
        byPrescale++;
    }

    g_byWdtDivider = byPrescale;

    if (g_byWdtDivider > WDTO_8S)
    {
        // restrict timeout since HW only can handle 8 seconds for Mega256
        g_byWdtDivider &= WDTO_8S;
    }

    wdt_enable(g_byWdtDivider);

    g_byNested = 1;
}

void WatchDogRestart(void)
{
    wdt_reset();
}

void WatchDogDisable(void)
{
    if (g_byNested != 0)
    {
        g_byNested++;
    }
    wdt_disable();
}

void WatchDogEnable(void)
{
    if (g_byNested > 1 && --g_byNested == 1)
    {
        wdt_enable(g_byWdtDivider);
    }
}

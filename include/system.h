/* ========================================================================
 * [PROJECT]    SIR
 * [MODULE]     global module
 * [TITLE]      system header file
 * [FILE]       system.h
 * [VSN]        1.0
 * [CREATED]    18 august 2003
 * [LASTCHNGD]  18 august 2003
 * [COPYRIGHT]  Copyright (C) STREAMIT BV 2010
 * [PURPOSE]    global defines and prototypes for Lucas project
 * ======================================================================== */

/*--------------------------------------------------------------------------*/
/*  Include files                                                           */
/*--------------------------------------------------------------------------*/
#include "typedefs.h"
/*-------------------------------------------------------------------------*/
/* global defines                                                          */
/*-------------------------------------------------------------------------*/
#define ON    (1)
#define OFF   (0)

#define NO_CHANNEL      200     // make sure this is > SETTINGS_NROF_CHANNELS
#define NO_PLAYLIST     200     // make sure this is > SETTINGS_NROF_PLAYLISTS


/*!\brief System status bit definitions */
#define STATUS_STARTING_UP              0x01        // true during startup and Setup Wizzard
#define STATUS_IP_MODE                  0x02        // do we use DHCP or Static IP?
#define STATUS_POWER                    0x04        // if set, we are on
#define STATUS_SELFTEST                 0x08        // switch to re-direct keys from Menu to Selftest module
#define STATUS_CONFIGURED               0x10        // if set, we are configured
#define STATUS_GPB_INVALID              0x20        // if set, we cannot rely on Productcode, Remote Updatemode, etc
#define STATUS_FALLBACK_ACTIVE          0x40        // if set, we cannot play from the internet and if a card is present, we play from card

#define STATUS_STARTING_UP_MASK         0x01
#define STATUS_IP_MODE_MASK             0x02
#define STATUS_POWER_MASK               0x04
#define STATUS_SELFTEST_MASK            0x08
#define STATUS_CONFIGURED_MASK          0x10
#define STATUS_GPB_INVALID_MASK         0x20
#define STATUS_FALLBACK_ACTIVE_MASK     0x40

#define ONE_SECOND      225             // 225 times 4,44 msecs makes 1 second
#define ONE_MINUTE      60
#define ONE_HOUR        60


/*!\brief define possible languages */
#define LANGUAGE_NL     0
#define LANGUAGE_EN     1
#define LANGUAGE_DE     2

/*!\brief define possible IP_Modes */
#define IP_MODE_DHCP    0
#define IP_MODE_STATIC  1

/*!\brief define possible countrycodes (mainly used for the modem) */
#define COUNTRY_US      0
#define COUNTRY_UK      1
#define COUNTRY_SA      2
#define COUNTRY_NL_V92  3
#define COUNTRY_NL_V34  4
#define COUNTRY_NROF    5

/*!\brief Maximum size to use for threadnames */
#define THREADNAME_SIZE 9

enum
{
        IRQ_INT0,
        IRQ_INT1,
        IRQ_INT2,
        IRQ_INT3,
        IRQ_INT4,
        IRQ_INT5,
        IRQ_INT6,
        IRQ_INT7,
        IRQ_TIMER2_COMP,
        IRQ_TIMER2_OVF,
        IRQ_TIMER1_CAPT,
        IRQ_TIMER1_COMPA,
        IRQ_TIMER1_COMPB,
        IRQ_TIMER1_OVF,
        IRQ_TIMER0_COMP,
        IRQ_TIMER0_OVF,
        IRQ_SPI_STC,
        IRQ_UART_RX,
        IRQ_UART_UDRE,
        IRQ_UART_TX,
        IRQ_ADC,
        IRQ_EE_RDY,
        IRQ_ANA_COMP,
        IRQ_TIMER1_COMPC,
        IRQ_TIMER3_CAP,
        IRQ_TIMER3_COMPA,
        IRQ_TIMER3_COMPB,
        IRQ_TIMER3_COMPC,
        IRQ_TIMER3_OVF,
        IRQ_UART1_RX,
        IRQ_UART1_UDRE,
        IRQ_UART1_TX,
        IRQ_I2C,
        IRQ_SPM_RDY,
        IRQ_MAX
};


/*-------------------------------------------------------------------------*/
/* typedefs & structs                                                      */
/*-------------------------------------------------------------------------*/
typedef struct
{
    u_char KeyLockStatus;               // overall lock, if ACTIVE, keyboard is locked
    u_char KeyLockEditStatus;           // used when user wants to change the status of the Keylock
    u_char KeyLockTempStatus;           // after entering a valid PIN, this goes to INACTIVE for 30 secs
    u_short KeyLockPIN;                 // range = [0]..[5555]
} TKeyLock;

typedef u_char TIPMode;
typedef u_char TLanguage;

/*-------------------------------------------------------------------------*/
/* export global variables                                                 */
/*-------------------------------------------------------------------------*/
extern u_char           SystemStatus;

extern TLanguage        SystemLanguage;
extern TIPMode          SystemIPMode;
extern TKeyLock         SystemKeyLock;

/*-------------------------------------------------------------------------*/
/* export global routines (interface)                                      */
/*-------------------------------------------------------------------------*/
extern void SysInitVars(void);
extern void SysPowerOn(void);
extern void SysPowerOff(void);
extern void SysInitIO(void);
extern void SystemShowSysInfo(void);


/*  様様  End Of File  様様様様 様様様様様様様様様様様様様様様様様様様様様様 */



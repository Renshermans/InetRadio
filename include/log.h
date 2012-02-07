#ifndef _Log_H
#define _Log_H
/*
 *  Copyright STREAMIT BV, 2010.
 *
 *  Project             : SIR
 *  Module              : Log
 *  File name  $Workfile: Log.h  $
 *       Last Save $Date: 2003/08/16 15:01:36  $
 *             $Revision: 0.1  $
 *  Creation Date       : 2003/08/16 15:01:36
 *
 *  Description         :
 *
 */

/*!
 *\ brief Define this to output raw unformatted output.
 *
 * Advantage of RAW is you can use fprintf(stdout, "format", ...) anywhere.
 * Disadvantage is that there is no selection on loglevel and that there
 * is no indication whether it is a warning, error, etc.
 */
//#define LOG_RAW

/*--------------------------------------------------------------------------*/
/*  Include files                                                           */
/*--------------------------------------------------------------------------*/
#include <stdio.h>

/*--------------------------------------------------------------------------*/
/*  Constant definitions                                                    */
/*--------------------------------------------------------------------------*/
#ifdef LOG_RAW
#define LogMsg_P            fputc('\n', stdout);fprintf_P
#endif

/*--------------------------------------------------------------------------*/
/*  Type declarations                                                       */
/*--------------------------------------------------------------------------*/
/*! \brief The log priority levels
 * The first entry has the highest priority
 */
#ifdef LOG_RAW
#define LOG_EMERG stdout
#define LOG_ALERT stdout
#define LOG_CRIT stdout
#define LOG_ERR stdout
#define LOG_WARNING stdout
#define LOG_NOTICE stdout
#define LOG_INFO stdout
#define LOG_DEBUG stdout
typedef FILE * TLogLevel;
#else
typedef u_char TLogLevel;
#endif

// bit 0..2 for LEVEL
#define LOG_EMERG_LEV   0x00
#define LOG_ALERT_LEV   0x01
#define LOG_CRIT_LEV    0x02
#define LOG_ERR_LEV     0x03
#define LOG_WARNING_LEV 0x04
#define LOG_NOTICE_LEV  0x05
#define LOG_INFO_LEV    0x06
#define LOG_DEBUG_LEV   0x07

// bit 3..7 for MODULE
#define LOG_AUDIO_MODULE        0x10
#define LOG_CHANNEL_MODULE      0x20
#define LOG_COMAND_MODULE       0x30
#define LOG_DISPLAY_MODULE      0x40
#define LOG_FAT_MODULE          0x50
#define LOG_FLASH_MODULE        0x60
#define LOG_HTTP_MODULE         0x70
#define LOG_INET_MODULE         0x80
#define LOG_KEYBOARD_MODULE     0x90
#define LOG_LED_MODULE          0xA0
#define LOG_LOG_MODULE          0xB0
#define LOG_MAIN_MODULE         0xC0
#define LOG_MENU_MODULE         0xD0
#define LOG_MMC_MODULE          0xE0
#define LOG_MMCDRV_MODULE       0xF0
#define LOG_PARSE_MODULE        0x08
#define LOG_PLAYER_MODULE       0x18
#define LOG_REMCON_MODULE       0x28
#define LOG_RTC_MODULE          0x38
#define LOG_SELFTEST_MODULE     0x48
#define LOG_SESSION_MODULE      0x58
#define LOG_SETTINGS_MODULE     0x68
#define LOG_SPIDRV_MODULE       0x78
#define LOG_STREAMER_MODULE     0x88
#define LOG_UART0DRIVER_MODULE  0x98
#define LOG_UPDATE_MODULE       0xA8
#define LOG_UTIL_MODULE         0xB8
#define LOG_VERSION_MODULE      0xC8
#define LOG_VS10XX_MODULE       0xD8
#define LOG_WATCHDOG_MODULE     0xE8

// note that LOG_MODULE must be defined before including this "log.h"
#define LOG_EMERG       (TLogLevel)(LOG_EMERG_LEV   | LOG_MODULE)
#define LOG_ALERT       (TLogLevel)(LOG_ALERT_LEV   | LOG_MODULE)
#define LOG_CRIT        (TLogLevel)(LOG_CRIT_LEV    | LOG_MODULE)
#define LOG_ERR         (TLogLevel)(LOG_ERR_LEV     | LOG_MODULE)
#define LOG_WARNING     (TLogLevel)(LOG_WARNING_LEV | LOG_MODULE)
#define LOG_NOTICE      (TLogLevel)(LOG_NOTICE_LEV  | LOG_MODULE)
#define LOG_INFO        (TLogLevel)(LOG_INFO_LEV    | LOG_MODULE)
#define LOG_DEBUG       (TLogLevel)(LOG_DEBUG_LEV   | LOG_MODULE)

/*--------------------------------------------------------------------------*/
/*  Global variables                                                        */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*  Global functions                                                        */
/*--------------------------------------------------------------------------*/
extern void LogInit(void);

extern void LogOpen(void);
extern void LogClose(void);

#ifndef LOG_RAW
extern void LogMsg_P(TLogLevel tLevel, PGM_P szMsg, ...);
extern void LogChar_P(const char bChar);
#endif

extern TLogLevel LogSetLevel(TLogLevel tLevel);

extern void HexDump(FILE *stream, CONST unsigned char *cp, size_t length);

#endif /* _Log_H */

/*
 *  Copyright STREAMIT BV, 2010.
 *
 *  Project             : SIR
 *  Module              : Log
 *  File name  $Workfile:   Log.c  $
 *       Last Save $Date:   2003/08/16 15:01:19  $
 *             $Revision:   0.1  $
 *  Creation Date       : 2003/08/16 15:01:19
 *
 *  Description         : Keeps track of log messages
 *                        As an initial implementation this module
 *                        outputs messages to the serial port and uses
 *                        no buffering. (As a result logging delays execution
 *                        as long as the serial write takes)
 *                        At a later stage this module will have a logging
 *                        queue. This will not have much impact on execution
 *                        time. It will output messages to either a serial port
 *                        or telnet client.
 *
 */

/*--------------------------------------------------------------------------*/
/*  Include files                                                           */
/*--------------------------------------------------------------------------*/
#define LOG_MODULE  LOG_LOG_MODULE

#include <stdio.h>
#include <string.h>

#include <sys/thread.h>
#include <sys/heap.h>
#include <sys/device.h>
#include <sys/osdebug.h>

//#pragma text:appcode

#include "uart0driver.h"
//#include "settings.h"
#include "log.h"

/*--------------------------------------------------------------------------*/
/*  Constant definitions                                                    */
/*--------------------------------------------------------------------------*/
/*!\brief Max length of address */
#define MAX_OFFSET_LEN  8

/*!\brief Max byte values printed on a line */
#define BYTES_PER_LINE  16

/*!\brief Max number of characters hex dump takes: 2 digits plus trailing blank */
#define HEX_DUMP_LEN    (BYTES_PER_LINE*3)

/*!\brief Number of characters hex dump + ascii take: 3 chars, 2 blanks, 1 char */
#define DATA_DUMP_LEN   (HEX_DUMP_LEN + 2 + BYTES_PER_LINE)

/*!\brief Number of characters per line: address, 2 blanks, data dump */
#define MAX_LINE_LEN    (MAX_OFFSET_LEN + 2 + DATA_DUMP_LEN)

#define LEVEL_MASK      0x07        // b0...b2
#define NAME_MASK       0xF8        // b3..b7


/*--------------------------------------------------------------------------*/
/*  Type declarations                                                       */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*  Local variables                                                         */
/*--------------------------------------------------------------------------*/
/*!\brief The current log level */
static TLogLevel    g_tLevel;

/*!\brief Stream to output the log data to */
static FILE         *g_tStream;

/*--------------------------------------------------------------------------*/
/*  Global variables                                                        */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*  Local functions                                                         */
/*--------------------------------------------------------------------------*/

/*!
 * \brief Return the prefix-level for the given log level.
 *
 * \param   tLevel [in] The log level.
 *
 * \return  Pointer to a string in program space.
 */
static PGM_P LogPrefixLevel_P(TLogLevel tLevel)
{

    switch (tLevel)
    {
        case LOG_EMERG_LEV   :return(PSTR("\n#Emerg "));
        case LOG_ALERT_LEV   :return(PSTR("\n#Alert "));
        case LOG_CRIT_LEV    :return(PSTR("\n#Crit  "));
        case LOG_ERR_LEV     :return(PSTR("\n#Err   "));
        case LOG_WARNING_LEV :return(PSTR("\n#Warn  "));
        case LOG_NOTICE_LEV  :return(PSTR("\n#Notic "));
        case LOG_INFO_LEV    :return(PSTR("\n#Info  "));
        case LOG_DEBUG_LEV   :return(PSTR("\n#Debug "));
        default          :return(PSTR("\n"));
    }
}

/*!
 * \brief Return the prefix-name for the given log module.
 *
 * \param   tLevel [in] The log module.
 *
 * \return  Pointer to a string in program space.
 */
static PGM_P LogPrefixName_P(TLogLevel tLevel)
{

    switch (tLevel)
    {
        case LOG_AUDIO_MODULE        :return(PSTR("AU: "));
        case LOG_CHANNEL_MODULE      :return(PSTR("CH: "));
        case LOG_COMAND_MODULE       :return(PSTR("CM: "));
        case LOG_DISPLAY_MODULE      :return(PSTR("DP: "));
        case LOG_FAT_MODULE          :return(PSTR("FA: "));
        case LOG_FLASH_MODULE        :return(PSTR("FL: "));
        case LOG_HTTP_MODULE         :return(PSTR("HT: "));
        case LOG_INET_MODULE         :return(PSTR("IN: "));
        case LOG_KEYBOARD_MODULE     :return(PSTR("KB: "));
        case LOG_LED_MODULE          :return(PSTR("LE: "));
        case LOG_LOG_MODULE          :return(PSTR("LG: "));
        case LOG_MAIN_MODULE         :return(PSTR("SY: "));
        case LOG_MENU_MODULE         :return(PSTR("ME: "));
        case LOG_MMC_MODULE          :return(PSTR("MM: "));
        case LOG_MMCDRV_MODULE       :return(PSTR("MD: "));
        case LOG_PARSE_MODULE        :return(PSTR("PA: "));
        case LOG_PLAYER_MODULE       :return(PSTR("PL: "));
        case LOG_REMCON_MODULE       :return(PSTR("RC: "));
        case LOG_RTC_MODULE          :return(PSTR("RT: "));
        case LOG_SELFTEST_MODULE     :return(PSTR("ST: "));
        case LOG_SESSION_MODULE      :return(PSTR("SE: "));
        case LOG_SETTINGS_MODULE     :return(PSTR("SG: "));
        case LOG_SPIDRV_MODULE       :return(PSTR("SP: "));
        case LOG_STREAMER_MODULE     :return(PSTR("SR: "));
        case LOG_UART0DRIVER_MODULE  :return(PSTR("UA: "));
        case LOG_UPDATE_MODULE       :return(PSTR("UD: "));
        case LOG_UTIL_MODULE         :return(PSTR("UT: "));
        case LOG_VERSION_MODULE      :return(PSTR("VE: "));
        case LOG_VS10XX_MODULE       :return(PSTR("VS: "));
        case LOG_WATCHDOG_MODULE     :return(PSTR("WD: "));
        default          :return(PSTR("?? <DMK> "));
    }
}

/*--------------------------------------------------------------------------*/
/*  Global functions                                                        */
/*--------------------------------------------------------------------------*/

/*!
 * \brief Initialises this module
 *
 * \param   -
 *
 * \return  -
 */
void LogInit(void)
{
    /* Set default level */
    g_tLevel = LOG_DEBUG_LEV;

    LogOpen();
}

/*!
 * \brief Opens the module for use.
 *
 * \param   -
 *
 * \return  -
 */
void LogOpen(void)
{
    /* Associate our stream with a device */
    g_tStream = Uart0DriverGetStream();
}

/*!
 * \brief Closes the module.
 *
 * All interface functions from this module will result in void
 * operations.
 *
 * \param   -
 *
 * \return  -
 */
void LogClose(void)
{
    FILE *tPrevStream = g_tStream;

    /* Don't allow adding of new output. */
    g_tStream = NULL;

    /* Finish all current output. */
    fflush(tPrevStream);
}

/*!
 * \brief Log a message to the log medium using a fixed string.
 *
 * The fixed string must reside in program space. It is parsed
 * using the rules of (s)printf.
 *
 * \param tLevel priority level of the message.
 * \param szMsg  format string of the message.
 * \param ...    arguments to the format string.
 */
void LogMsg_P(TLogLevel tLevel, PGM_P szMsg, ...)
{
    va_list ap;

    if (g_tStream)
    {
        /* Log the string if the message is more important than the current level */
        if ((tLevel&LEVEL_MASK) <= g_tLevel)
        {
            fputs_P(LogPrefixLevel_P(tLevel&LEVEL_MASK), g_tStream);
            fputs_P(LogPrefixName_P(tLevel&NAME_MASK), g_tStream);
            va_start(ap, szMsg);
            vfprintf_P(g_tStream, szMsg, ap);
            va_end(ap);
        }
    }
}

void LogChar_P(const char bChar)
{
    if (g_tStream)
    {
        fputc(bChar, g_tStream);
    }
}

/*!
 * \brief Set the priority level
 *
 * \param tNewLevel New priority level
 *
 * \return The previous priority level
 */
TLogLevel LogSetLevel(TLogLevel tNewLevel)
{
    TLogLevel tPrevLevel = g_tLevel;

    if (tNewLevel <= LOG_DEBUG_LEV)
    {
        g_tLevel = tNewLevel;
    }
    return(tPrevLevel);
}

/*!
 * \brief Print a block of memory
 *
 * Prints out 16 bytes of data per line
 * Every line starts with the address (offset),
 * the data in hex and that same data in ascii.
 *
 * \param tStream The stream to print to
 * \param cp The data
 * \param length The length of the data
 *
 * \return none
 */
void HexDump(FILE *tStream, CONST u_char *cp, size_t length)
{
    register unsigned int address, i, hex_pos, ascii_pos, l;
    unsigned int address_len;
    unsigned char c;
    char line[MAX_LINE_LEN + 1];
    static CONST char binhex[16] =
    {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

    if (tStream != NULL)
    {
        /*
         * How many digits do we need for the address?
         * We use at least 4 digits or more as needed
         */
        if (((length - 1) & 0xF0000000) != 0)
        {
            address_len = 8;
        }
        else if (((length - 1) & 0x0F000000) != 0)
        {
            address_len = 7;
        }
        else if (((length - 1) & 0x00F00000) != 0)
        {
            address_len = 6;
        }
        else if (((length - 1) & 0x000F0000) != 0)
        {
            address_len = 5;
        }
        else
        {
            address_len = 4;
        }

        address = 0;
        i = 0;
        hex_pos = 0;
        ascii_pos = 0;
        while (i < length)
        {
            if ((i & 15) == 0)
            {
                /*
                 * Start of a new line.
                 */
                memset(line, ' ', sizeof(line));
                hex_pos = 0;
                ascii_pos = 0;
                l = address_len;
                do
                {
                    l--;
                    c = (address >> (l*4)) & 0xF;
                    line[hex_pos++] = binhex[c];
                } while (l != 0);

                /* 2 spaces */
                hex_pos += 2;

                /*
                 * Offset in line of ASCII dump.
                 */
                ascii_pos = hex_pos + HEX_DUMP_LEN + 2;
            }
            c = *cp++;

            /* Dump the hex value */
            line[hex_pos++] = binhex[c >> 4];
            line[hex_pos++] = binhex[c & 0xF];
            hex_pos++;

            /* Print the ascii value */
            line[ascii_pos++] = (c >= ' ' && c < 127) ? c : '.';

            i++;
            if ((i & 15) == 0 || i == length)
            {
                /*
                 * We'll be starting a new line, or
                 * we're finished printing this buffer;
                 * dump out the line we've constructed,
                 * and advance the offset.
                 */
                line[ascii_pos] = '\0';
                fputs(line, tStream);
                fputc('\n', tStream);
                address += BYTES_PER_LINE;
            }
        }
    }
}


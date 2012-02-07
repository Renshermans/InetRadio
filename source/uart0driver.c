/*!
 * Copyright (C) 2003 by Streamit All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgement:
 *
  *    This product includes software developed by Streamit
 *    and its contributors.
 *
 * THIS SOFTWARE IS PROVIDED BY STREAMIT AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL CALL DIRECT
 * CELLULAR SOLUTIONS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * For additional information see http://www.streamit.nl/
 */

/*!
 *
 * COPYRIGHT STREAMIT BV 2010
 * Tested except for the DTR code. This will be done when the final hardware is available
 *
 *
 * [Note] this UART0 is used for serial communication with LTP (or a PC in general).
 */


#define LOG_MODULE  LOG_UART0DRIVER_MODULE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>

#include <dev/uartavr.h>
//#include <sys/heap.h>
#include <sys/thread.h>
#include <sys/timer.h>

//#pragma text:appcode

#include "system.h"
#include "uart0driver.h"


//----------------------------------------------------------

static FILE *stream=NULL;

//----------------------------------------------------------

/*!
 * \brief Handle input.
 */
// THREAD(Uart0KeyEvents, arg)
// {
    // NutThreadSetPriority(254);  // low prio
    // for (;;)
    // {
        // if (stream==NULL)
        // {
            // NutSleep(2000);           //Mhe
            // continue;
        // }

       // CommandHandler(stream);
    // }
// }

//----------------------------------------------------------

/*!
 * \brief return stream that is connected to terminal program (serial or TCP/IP)
 */
FILE *Uart0DriverGetStream(void)
{
    return(stream);
}

/*!
 * \brief Uart0 process initialisation. Uses stdout in combination with Uart0.
 */
void Uart0DriverInit(void)
{
    stream = NULL;

    // register Uart0
    NutRegisterDevice(&devUart0, 0, 0);
}

/*!
 * \brief Creates a thread to handle incoming data from User.
 */
void Uart0DriverStart(void)
{
    u_long baud = 115200;
    char DeviceNameBuffer[6];
    char FileModeBuffer[3];

    strcpy_P(DeviceNameBuffer, PSTR("uart0"));
    strcpy_P(FileModeBuffer, PSTR("w"));

    /* Open the stream, connect to stdout */
    if ((stream=freopen(DeviceNameBuffer, FileModeBuffer, stdout)) != NULL)
    {
        _ioctl(_fileno(stream), UART_SETSPEED, &baud);
    }

    // if (stream != NULL)
    // {
        // if (GetThreadByName(DeviceNameBuffer) == NULL)
        // {
            // NutThreadCreate(DeviceNameBuffer, Uart0KeyEvents, 0, 512);
        // }
    // }
}

void Uart0DriverSetCookedMode(u_long CookedMode)
{
    _ioctl(_fileno(stream), UART_SETCOOKEDMODE, &CookedMode);
}

/*!
 * \brief Close the stream.
 */
void Uart0DriverStop(void)
{
    if (stream)
    {
        // needed? (void)_close(_fileno(stream));
        (void)fclose(stream);
        stream = NULL;
    }
}

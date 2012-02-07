#ifndef _UART0DRIVER_H_
#define _UART0DRIVER_H_
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
 * For additional information see http://www.streamit.eu/
 */
/*!
 *
 * COPYRIGHT STREAMIT BV 2010
 * Created
 *
 *
 */
/*--------------------------------------------------------------------------*/
/*  Include files                                                           */
/*--------------------------------------------------------------------------*/
#include <stdio.h>

#define UART_COOKEDMODE_RAW    0L
#define UART_COOKEDMODE_EOL    1L

extern void  Uart0DriverInit(void);
extern void  Uart0DriverStart(void);
extern void  Uart0DriverStop(void);
extern FILE *Uart0DriverGetStream(void);
extern int   Uart0DriverGetNumber(PGM_P prompt, int dflt);
extern int   Uart0DriverKey(void);
extern void  Uart0DriverSetCookedMode(u_long Mode);


#endif

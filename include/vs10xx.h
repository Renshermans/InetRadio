/*
 * Copyright (C) 2001-2003 by egnite Software GmbH. All rights reserved.
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
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY EGNITE SOFTWARE GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL EGNITE
 * SOFTWARE GMBH OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * For additional information see http://www.ethernut.de/
 *
 * -
 * Portions Copyright (C) 2001 Jesper Hansen <jesperh@telia.com>.
 *
 * This file is part of the yampp system.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
 *
 * Revision 2.0  2004/09/30 19:37:39  johan van der stoel
 * Fully rewritten to support VS1011 / VS1003. For VS1001: use driver from
 * Ethernut distribution
 *
 * Revision 1.2  2003/07/13 19:37:39  haraldkipp
 * Enable application to control decoder interrupts.
 *
 *
 */

#include <sys/nutconfig.h>
#include <sys/types.h>

/*-------------------------------------------------------------------------*/
/* global defines                                                          */
/*-------------------------------------------------------------------------*/

// Instruction opcodes
#define VS_OPCODE_READ      3
#define VS_OPCODE_WRITE     2

// Decoder registers VS1011/VS1003
#define VS_MODE_REG         0
#define VS_STATUS_REG       1
#define VS_BASS_REG         2
#define VS_CLOCKF_REG       3
#define VS_DECODE_TIME_REG  4
#define VS_AUDATA_REG       5
#define VS_WRAM_REG         6
#define VS_WRAMADDR_REG     7
#define VS_HDAT0_REG        8
#define VS_HDAT1_REG        9
#define VS_AIADDR_REG      10
#define VS_VOL_REG         11
#define VS_AICTRL0_REG     12
#define VS_AICTRL1_REG     13
#define VS_AICTRL2_REG     14
#define VS_AICTRL3_REG     15

#define NROF_VS_REGS       (VS_AICTRL3_REG+1)

// SCI Mode register bits VS1011/VS1003
#define VS_SM_DIFF          0x0001
#define VS_SM_JUMP          0x0002 /* VS1011 */
#define VS_SM_SETTOZERO     0x0002 /* VS1003 */
#define VS_SM_RESET         0x0004
#define VS_SM_OUTOFWAV      0x0008
#define VS_SM_PDOWN         0x0010
#define VS_SM_TESTS         0x0020
#define VS_SM_STREAM        0x0040
#define VS_SM_SETTOZERO1    0x0080 /* VS1011 */
#define VS_SM_PLUSV         0x0080 /* VS1003 */
#define VS_SM_DACT          0x0100
#define VS_SM_SDIORD        0x0200 /* VS1011 */
#define VS_SM_SDISHARE      0x0400 /* VS1011 */
#define VS_SM_SDINEW        0x0800 /* VS1011 */
#define VS_SM_SETTOZERO2    0x1000 /* VS1011 */
#define VS_SM_ADPCM         0x1000 /* VS1003 */
#define VS_SM_SETTOZERO3    0x2000 /* VS1011 */
#define VS_SM_ADPCM_HP      0x2000 /* VS1003 */
#define VS_SM_LINE_IN       0x4000 /* VS1003 */

// SCI Status register bits 6-4 contains the ID number of the chip
#define VS_VS1001           0   /* not supported by this driver */
#define VS_VS1011           1
#define VS_VS1011e          2   /* VS1002 also returns this, but is not supported by this driver */
#define VS_VS1003           3
#define VS_VS1053           4
#define VS_VS1033           5


// Status of the decoder
#define VS_STATUS_STOPPED   0
#define VS_STATUS_RUNNING   1
#define VS_STATUS_EOF       2
#define VS_STATUS_EMPTY     4

/*-------------------------------------------------------------------------*/
/* typedefs & structs                                                      */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* export global variables                                                 */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* export global routines (interface)                                      */
/*-------------------------------------------------------------------------*/
extern int VsPlayerInit(void);
extern int VsPlayerReset(u_short mode);
extern int VsPlayerSetMode(u_short mode);
extern int VsPlayerKick(void);
extern int VsPlayerStop(void);
extern u_char VsPlayerInterrupts(u_char enable);

extern u_char VsGetStatus(void);
extern u_short VsMemoryTest(void);
extern u_short VsGetType(void);
extern u_short VsGetTypeHex(void);
extern int VsSetVolume(u_char left, u_char right);
extern u_short VsGetVolume(void);
extern int VsBeep(u_char fsin, u_short ms);
extern int VsBeepStart(u_char fsin);
extern int VsBeepStartRaw(u_char Raw);
extern int VsBeepStop(void);
extern u_short VsRegInfo(u_char reg);
extern void VsRegWrite(u_char reg, u_short data);
extern u_short VsStreamValid(void);


/*@}*/

#ifndef _Platform_H
#define _Platform_H
/*
 *  Copyright STREAMIT BV, 2010.
 *
 *  Project             : SIR
 *  Module              : Platform
 *  File name  $Workfile: Platform.h  $
 *       Last Save $Date: 2003/08/18 10:09:48  $
 *             $Revision: 0.1  $
 *  Creation Date       : 2003/08/18 10:09:48
 *
 *  Description         : Definitions which are dependent on the compiler and
 *                        or processor.
 */

/*--------------------------------------------------------------------------*/
/*  Include files                                                           */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*  Constant definitions                                                    */
/*--------------------------------------------------------------------------*/
#define LOBYTE(w)           ((u_char) ((w) & 0xFF))
#define HIBYTE(w)           ((u_char) (((u_short) (w) >> 8) & 0xFF))
#define LOWORD(l)           ((u_short) ((l) & 0xFFFF))
#define HIWORD(l)           ((u_short) (((u_long) (l) >> 16) & 0xFFFF))

#define MAKEWORD(bLow, bHigh) \
    ((u_short) (((u_char) (bLow)) | ((u_short) ((u_char) (bHigh))) << 8))
#define MAKEULONG(wLow, wHigh) \
    ((u_long) (((u_short) (wLow)) | ((u_long) ((u_short) (wHigh))) << 16))

/*--------------------------------------------------------------------------*/
/*  Type declarations                                                       */
/*--------------------------------------------------------------------------*/

#ifndef prog_int
#define prog_int        prog_int16_t
#endif /* #ifndef prog_int */

/*--------------------------------------------------------------------------*/
/*  Global variables                                                        */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*  Global functions                                                        */
/*--------------------------------------------------------------------------*/
#define PRG_RDW(addr)   pgm_read_word(addr)

#endif /* _Platform_H */

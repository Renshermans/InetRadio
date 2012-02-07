#ifndef _watchdog_H
#define _watchdog_H
/*
 *  Copyright STREAMIT BV, 2010.
 *
 *  Project             : SIR
 *  Module              : watchdog
 *  File name  $Workfile: watchdog.h  $
 *       Last Save $Date: 2007/01/16 11:13:53  $
 *             $Revision: 0.1  $
 *  Creation Date       : 2007/01/16 11:13:53
 *
 *  Description         : Watchdog routines for the ATMega128/256
 *                        Inspired by the arch/xxx/dev/watchdog routines in
 *                        NutOS 4.2.1 and higher.
 *
 */

/*--------------------------------------------------------------------------*/
/*  Include files                                                           */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*  Constant definitions                                                    */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*  Type declarations                                                       */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*  Global variables                                                        */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*  Global functions                                                        */
/*--------------------------------------------------------------------------*/

/*!
 * \brief Start the watch dog timer.
 *
 * This function can be used by applications to prevent hang-ups.
 * The watch dog timer will be automatically enabled on return.
 *
 * \param   ms [in] Watch dog time out in milliseconds.
 *
 * \return  -
 *
 * The following code fragment starts the watch timer with a time out
 * of 550 milliseconds and restarts it every 500 milliseconds.
 *
 * \code
 * #include <sys/timer.h>
 * #include "watchdog.h"
 *
 * WatchDogStart(550);
 * for(;;)
 * {
 *     WatchDogRestart();
 *     NutSleep(500);
 * }
 *
 * \endcode
 */
extern void WatchDogStart(unsigned long ms);


/*!
 * \brief Restart the watch dog timer.
 */
extern void WatchDogRestart(void);

/*!
 * \brief Disables the watch dog timer.
 *
 * Applications should call this function to temporarily disable the
 * watch dog timer. To re-enable it, call WatchDogEnable().
 *
 * \code
 * #include <dev/watchdog.h>
 *
 * WatchDogStart(100, 0);
 *
 * //Some code here.
 *
 * WatchDogRestart();
 *
 * //Some code here.
 *
 * WatchDogDisable();
 *
 * //Some lengthy code here, like writing to flash memory.
 *
 * WatchDogEnable();
 *
 * \endcode
 */
extern void WatchDogDisable(void);

/*!
 * \brief Enables the watch dog timer.
 *
 * The function can be safely used within nested subroutines.
 * The watch dog will be enabled only, if this function is called
 * the same number of times as WatchDogDisable(). If enabled,
 * the watch dog timer will also have been re-started and the
 * full time out value is available before another WatchDogRestart()
 * is required.
 *
 * If the watch has not been started by WatchDogStart(), then this
 * function does nothing.
 */
extern void WatchDogEnable(void);

#endif /* _watchdog_H */

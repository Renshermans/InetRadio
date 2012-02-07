/*! \file
 * Led.c contains all interface- and low-level routines to
 * control the LED
 *  Copyright STREAMIT BV 2010
 *  \version 1.0
 *  \date 26 september 2003
 */


#define LOG_MODULE  LOG_LED_MODULE

#include <sys/timer.h>
#include <sys/thread.h>
#include <sys/event.h>

#include "system.h"
#include "portio.h"
#include "led.h"

static u_char LedStatus;

/*-------------------------------------------------------------------------*/
/* local variable definitions                                              */
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/* local routines (prototyping)                                            */
/*-------------------------------------------------------------------------*/


/*!
 * \addtogroup Led
 */

/*@{*/

/*-------------------------------------------------------------------------*/
/*                         start of code                                   */
/*-------------------------------------------------------------------------*/


/* อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ */
/*!
 * \brief Interface routine to control the SIR LED
 *
 * Using this routine, the LED can be set to 'ON', 'OFF" or it's state can
 * be toggeld
 *
 * \param LedMode can be eiter 'ON', 'OFF' or 'TOGGLE'
 */
/* อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ */
/************************************************************/
/*  Control the LED: ON, OFF or TOGGLE                      */
/*                   in case Lucas is 'OFF' -> return       */
/************************************************************/
void LedControl(u_char LedMode)
{
   switch (LedMode)
    {
        case LED_OFF:
        case LED_POWER_OFF:
        case LED_FLASH_OFF:
            cbi (LED_OUT_WRITE, LED_PIN);
            LedStatus = LedMode;
            break;
        case LED_ON:
        case LED_POWER_ON:
        case LED_FLASH_ON:
            sbi (LED_OUT_WRITE, LED_PIN);
            LedStatus = LedMode;
            break;
        case LED_TOGGLE:
            if (LedStatus == LED_ON)
            {
                cbi (LED_OUT_WRITE, LED_PIN);
                LedStatus = LED_OFF;
            }
            else
            {
                sbi (LED_OUT_WRITE, LED_PIN);
                LedStatus = LED_ON;
            }
            break;
    }
}

/* อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ */
/*!
 * \brief Initialise the Led module
 *
 * clear LED and update LedStatus
 *
 */
/* อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ */
void LedInit()
{
    cbi (LED_OUT_WRITE, LED_PIN);
    LedStatus = LED_OFF;
}

/*!
 * \brief Get LedStatus for external use
 *
 */
u_char LedGetStatus()
{
  return(LedStatus);
}

/* ---------- end of module ------------------------------------------------ */

/*@}*/

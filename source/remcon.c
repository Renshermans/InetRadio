/*! \file
 * remcon.c contains all interface- and low-level routines that
 * perform handling of the infrared bitstream
 * [COPYRIGHT]  Copyright (C) STREAMIT BV
 *  \version 1.0
 *  \date 26 september 2003
 */



#define LOG_MODULE  LOG_REMCON_MODULE

#include <stdlib.h>
#include <fs/typedefs.h>
#include <sys/heap.h>

#include <sys/event.h>
#include <sys/atom.h>
#include <sys/types.h>
#include <dev/irqreg.h>

#include "system.h"
#include "portio.h"
#include "remcon.h"
#include "display.h"
#include "keyboard.h"
#include "led.h"


/*-------------------------------------------------------------------------*/
/* local variable definitions                                              */
/*-------------------------------------------------------------------------*/
static HANDLE  hRCEvent;

/*-------------------------------------------------------------------------*/
/* local routines (prototyping)                                            */
/*-------------------------------------------------------------------------*/
static void RcInterrupt(void*);
static void RcClearEvent(HANDLE*);


/*!
 * \addtogroup RemoteControl
 */

/*@{*/

/*-------------------------------------------------------------------------*/
/*                         start of code                                   */
/*-------------------------------------------------------------------------*/



/* อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ */
/*!
 * \brief ISR Remote Control Interrupt (ISR called by Nut/OS)
 *
 *  NEC-code consists of 5 parts:
 *
 *  - leader (9 msec high, 4,5 msec low)
 *  - address (8 bits)
 *  - inverted address (8 bits)
 *  - data (8 bits)
 *  - inverted data (8 bits)
 *
 *  The first sequence contains these 5 parts, next
 *  sequences only contain the leader + 1 '0' bit as long
 *  as the user holds down the button
 *  repetition time is 108 msec in that case
 *
 *  Resolution of the 16-bit timer we use here is 4,3 usec
 *
 *  13,5 msecs are 3109 ticks
 *  '0' is 1,25 msecs (260 ticks)
 *  '1' is 2,25 msecs (517 ticks)
 *
 * \param *p not used (might be used to pass parms from the ISR)
 */
/* อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ */
static void RcInterrupt(void *p)
{
	// Hier ISR implementeren voor bijv. NEC protocol
}

/* อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ */
/*!
 * \brief Clear the eventbuffer of this module
 *
 * This routine is called during module initialization.
 *
 * \param *pEvent pointer to the event queue
 */
/* อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ */
static void RcClearEvent(HANDLE *pEvent)
{
    NutEnterCritical();

    *pEvent = 0;

    NutExitCritical();
}



/* อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ */
/*!
 * \brief Initialise the Remote Control module
 *
 * - register the ISR in NutOS
 * - initialise the HW-timer that is used for this module (Timer1)
 * - initialise the external interrupt that inputs the infrared data
 * - flush the remote control buffer
 * - flush the eventqueue for this module
 */
/* อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ */
void RcInit()
{
    int nError = 0;

    EICRB &= ~RC_INT_SENS_MASK;    // clear b0, b1 of EICRB

    // Install Remote Control interrupt
    nError = NutRegisterIrqHandler(&sig_INTERRUPT4, RcInterrupt, NULL);
    if (nError == FALSE)
    {
/*
 *  ToDo: control External Interrupt following NutOS calls
#if (NUTOS_VERSION >= 421)
        NutIrqSetMode(&sig_INTERRUPT4, NUT_IRQMODE_FALLINGEDGE);
#else
        EICRB |= RC_INT_FALLING_EDGE;
#endif
        EIMSK |= 1<<IRQ_INT4;         // enable interrupt
 */
        EICRB |= RC_INT_FALLING_EDGE;
        EIMSK |= 1<<IRQ_INT4;         // enable interrupt
    }

    // Initialise 16-bit Timer (Timer1)
    TCCR1B |= (1<<CS11) | (1<<CS10); // clockdivider = 64
    TIFR   |= 1<<ICF1;
    //TIMSK = 1<<TICIE1;

    RcClearEvent(&hRCEvent);


}

/* ---------- end of module ------------------------------------------------ */

/*@}*/

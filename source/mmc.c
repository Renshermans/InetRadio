/* ========================================================================
 * [PROJECT]    SIR100
 * [MODULE]     MMC driver
 * [TITLE]      Media Card driver
 * [FILE]       mmc.c
 * [VSN]        1.0
 * [CREATED]    02 october 2006
 * [LASTCHNGD]  20 may 2007
 * [COPYRIGHT]  Copyright (C) STREAMIT BV 2010
 * [PURPOSE]    routines and API to support MMC-application
 * ======================================================================== */

#define LOG_MODULE  LOG_MMC_MODULE

#include <string.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#include <sys/event.h>
#include <sys/thread.h>
#include <sys/timer.h>
#include <sys/device.h>
#include <sys/bankmem.h>
#include <sys/heap.h>

//#pragma text:appcode

#include "system.h"
#include "mmc.h"
#include "portio.h"
#include "vs10xx.h"
#include "display.h"
#include "log.h"
#include "fat.h"
#include "mmcdrv.h"
#include "led.h"
#include "keyboard.h"

#ifdef DEBUG
//#define MMC__DEBUG
#endif /* #ifdef DEBUG */
/*-------------------------------------------------------------------------*/
/* local defines                                                           */
/*-------------------------------------------------------------------------*/

#define CARD_PRESENT_COUNTER_OK         30
#define CARD_NOT_PRESENT_COUNTER_OK     20


/*--------------------------------------------------------------------------*/
/*  Type declarations                                                       */
/*--------------------------------------------------------------------------*/
/*!\brief Statemachine for card-detection */
typedef enum T_CARD_STATE
{
    CARD_IDLE,                      /* nothing to do */
    CARD_PRESENT,                   /* card seen at least one time */
    CARD_VALID,                     /* card seen at least <valid> times */
    CARD_NOT_PRESENT                /* card not seen at least (valid> times */
}TCardState;


/*-------------------------------------------------------------------------*/
/* local variable definitions                                              */
/*-------------------------------------------------------------------------*/
static u_char CardPresentFlag;
static u_char ValidateCounter;

/*!\brief state-variable for Card-statemachine */
static TCardState CardState;

/*!\brief Status of this module */
static TError g_tStatus;

/*-------------------------------------------------------------------------*/
/* local routines (prototyping)                                            */
/*-------------------------------------------------------------------------*/



/*!
 * \addtogroup Card
 */

/*@{*/

/*-------------------------------------------------------------------------*/
/*                         start of code                                   */
/*-------------------------------------------------------------------------*/

/*!
 * \brief check if MM-Card is inserted or removed.
 *
 * \Note: this routine is called from an ISR !
 *
 */
u_char CardCheckCard(void)
{
    u_char RetValue=CARD_NO_CHANGE;

    switch (CardState)
    {
        case CARD_IDLE:
            {
                if (bit_is_clear(MMC_IN_READ, MMC_CDETECT))
                {
                    ValidateCounter=1;
                    CardState = CARD_PRESENT;
                }
            }
            break;
        case CARD_PRESENT:
            {
                if (bit_is_clear(MMC_IN_READ, MMC_CDETECT))
                {
                    if (++ValidateCounter==CARD_PRESENT_COUNTER_OK)
                    {
                        CardPresentFlag=CARD_IS_PRESENT;
                        CardState=CARD_VALID;
                        RetValue=CARD_IS_PRESENT;
                    }
                }
                else
                {
                    CardState=CARD_IDLE;                  // false alarm,start over again
                }
            }
            break;
        case CARD_VALID:
            {
                if (bit_is_set(MMC_IN_READ, MMC_CDETECT))
                {
                    ValidateCounter=1;
                    CardState=CARD_NOT_PRESENT;         // Card removed
                }
            }
            break;
        case CARD_NOT_PRESENT:
            {
                if (++ValidateCounter==CARD_NOT_PRESENT_COUNTER_OK)
                {
                    CardPresentFlag=CARD_IS_NOT_PRESENT;
                    CardState=CARD_IDLE;
                    RetValue=CARD_IS_NOT_PRESENT;
                }
            }
            break;
    }
    return(RetValue);
}

/*!
 * \brief return status of "Card is Present"
 *
 */
u_char CardCheckPresent()
{
    return(CardPresentFlag);
}

/*!
 * \brief initialise the card by reading card contents (.pls files)
 *
 * We initialse the card by registering the card and the filesystem
 * that is on the card.
 *
 * Then we start checking if a number of playlists are
 * present on the card. The names of these playlists are hardcoded
 * (1.pls, 2.pls, to 20.pls). We 'search' the card for these list
 * of playlists by trying to open them. If succesfull, we read the
 * number of songs present (int) in that list
 * Finally we update some administration (global) variables
 *
 */
int CardInitCard()
{
    int iResult=-1;
    int fid;        // current file descriptor
    char szFileName[10];
    //u_char i;
	u_char ief;

    /*
     * Register our device for the file system (if not done already.....)
     */
    if (NutDeviceLookup(devFAT.dev_name) == 0)
    {
        ief = VsPlayerInterrupts(0);
        if ((iResult=NutRegisterDevice(&devFAT, FAT_MODE_MMC, 0)) == 0)
        {
            iResult=NutRegisterDevice(&devFATMMC0, FAT_MODE_MMC, 0);
        }
        VsPlayerInterrupts(ief);
    }
    else
    {
        NUTDEVICE * dev;

        /*
         *  we must call 'FatInit' here to initialise and mount the filesystem (again)
         */

        FATRelease();
        ief = VsPlayerInterrupts(0);
        dev=&devFAT;
        if (dev->dev_init == 0 || (*dev->dev_init)(dev) == 0)
        {
            dev=&devFATMMC0;
            if (dev->dev_init == 0 || (*dev->dev_init)(dev) == 0)
            {
                iResult=0;
            }
        }
        VsPlayerInterrupts(ief);
    }

    if (iResult==0)
    {
        LogMsg_P(LOG_INFO, PSTR("Card mounted"));
        /*
         *  try to open the playlists. If an error is returned, we assume the
         *  playlist does not exist and we do not check any further lists
         */

		 /* Kroeske: onderstaande code ter illustratie om file op card te openen */
		 
 //       for (i=1; i<SETTINGS_NROF_PLAYLISTST; ++i)
 //       {
            // compose name to open
            //sprintf_P(szFileName, PSTR("FM0:%d.pls"), i);
            if ((fid = _open(szFileName, _O_RDONLY)) != -1)
            {
                _close(fid);
            }
            else
            {

                //g_NrofPlayLists=i-1;
                //LogMsg_P(LOG_INFO, PSTR("Found %d Playlists on the Card"), i-1);
 //               break;
            }
//        }
    }
    else
    {
        LogMsg_P(LOG_ERR, PSTR("Error initialising File system and Card-driver"));
    }

    return(iResult);
}

/*!
 * \brief The CardPresent thread.
 *
 * execute code when card is inserted or redrawn
 *
 * \param   -
 *
 * \return  -
 */
THREAD(CardPresent, pArg)
{
    static u_char OldCardStatus;

    OldCardStatus=CardPresentFlag;

    for (;;)
    {
        if ((CardPresentFlag==CARD_IS_PRESENT) && (OldCardStatus==CARD_IS_NOT_PRESENT))
        {
            LogMsg_P(LOG_INFO, PSTR("Card inserted"));
            if (CardInitCard()==0)
            {
                KbInjectKey(KEY_MMC_IN);
            }
            OldCardStatus=CardPresentFlag;
        }
        else if ((CardPresentFlag==CARD_IS_NOT_PRESENT) && (OldCardStatus==CARD_IS_PRESENT))
        {
            LogMsg_P(LOG_INFO, PSTR("Card removed"));
            CardClose();
            OldCardStatus=CardPresentFlag;
        }
        else
        {
            NutSleep(500);
        }
    }
}


/*!
 * \brief return global variable that indicates the status of this module
 *
 */
TError CardStatus(void)
{
    return(g_tStatus);
}

/*!
 * \brief Stop playing.
 *
 * \param   -
 *
 * \return  -
 */
void CardClose(void)
{

}


/*!
 * \brief initialise this module
 *
 */
void CardInit()
{
    char ThreadName[10];

    CardState=CARD_IDLE;
    CardPresentFlag=CARD_IS_NOT_PRESENT;

    /*
     * Create a CardPresent thread
     */
    strcpy_P(ThreadName, PSTR("CardPres"));

    if (GetThreadByName((char *)ThreadName) == NULL)
    {
        if (NutThreadCreate((char *)ThreadName, CardPresent, 0, 768) == 0)
        {
            LogMsg_P(LOG_EMERG, PSTR("Thread failed"));
        }
    }

}

/* ---------- end of module ------------------------------------------------ */

/*@}*/



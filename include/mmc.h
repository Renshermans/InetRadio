/* ========================================================================
 * [PROJECT]    SIR100
 * [MODULE]     MMC driver
 * [TITLE]      Media Card driver include file
 * [FILE]       mmc.h
 * [VSN]        1.0
 * [CREATED]    02 october 2006
 * [LASTCHNGD]  02 october 2006
 * [COPYRIGHT]  Copyright (C) STREAMIT BV 2010
 * [PURPOSE]    routines and API to support MMC-application
 * ======================================================================== */

/*-------------------------------------------------------------------------*/
/* global defines                                                          */
/*-------------------------------------------------------------------------*/
#define CARD_IS_NOT_PRESENT           0
#define CARD_IS_PRESENT               1
#define CARD_NO_CHANGE                2       // no change since last event occured

/*-------------------------------------------------------------------------*/
/* export global routines (interface)                                      */
/*-------------------------------------------------------------------------*/
extern void CardInit(void);
extern int CardInitCard(void);
extern TError CardOpen(u_char);
extern void CardClose(void);
extern TError CardStatus(void);
extern unsigned int CardGetCurrentSong(void);
extern char* CardGetCurrentSongName(unsigned int *punLength);

extern u_char CardCheckCard(void);          // check by examining physical PIN
extern u_char CardCheckPresent(void);       // check by examining administration
extern TError CardPlayMp3File(char *path);
extern void CardStopMp3File(void);
extern void CardUpdateTicks(void);
extern u_char CardGetNumberOfPlayLists(void);

       //
/*  様様  End Of File  様様様様 様様様様様様様様様様様様様様様様様様様様様様 */



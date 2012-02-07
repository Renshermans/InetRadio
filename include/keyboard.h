/* ========================================================================
 * [PROJECT]    SIR
 * [MODULE]     Keyboard module
 * [TITLE]      keyboard module source file
 * [FILE]       keyboard.h
 * [VSN]        1.0
 * [CREATED]    28 july 2003
 * [LASTCHNGD]  18 august 2003
 * [COPYRIGHT]  Copyright (C) STREAMIT BV 2010
 * [PURPOSE]    Keyboard routines
 * ======================================================================== */

/*-------------------------------------------------------------------------*/
/* global defines                                                          */
/*-------------------------------------------------------------------------*/
#define KB_COL_0       3
#define KB_COL_1       4
#define KB_COL_2       3
#define KB_COL_3       2

#define KB_ROW_0       0
#define KB_ROW_1       1
#define KB_ROW_2       2
#define KB_ROW_3       3

#define KB_ROW_MASK    0xF0

#define KB_OK          0x00
#define KB_ERROR       0x01

/* state machine defines -------------------------------------------------- */
#define KB_IDLE        0x00
#define KB_KEY         0x01
#define KB_VALID       0x02
#define KB_RELEASE     0x03

#define KB_COUNTER_OK  0x03    // # a key must be seen before declared 'valid'
#define KB_LONG_HOLD_TIME 500  // 500 x 4.4 msec = 2200 msec
#define KB_BUFFER_SIZE 1


/*
 *  below are the keys after they where remapped to 8-bit values
 *  These definitions are used by the application
 */

#define KEY_SPEC       0
#define KEY_01         1
#define KEY_02         2
#define KEY_03         3
#define KEY_04         4
#define KEY_05         5
#define KEY_ALT        6

#define KEY_ESC        7
#define KEY_UP         8
#define KEY_OK         9
#define KEY_LEFT       10
#define KEY_DOWN       11
#define KEY_RIGHT      12

#define KEY_POWER      13
#define KEY_SETUP      14
#define KEY_LCD        15       // virtual key, generated when '1' is pressed and hold for > 2 secs

#define KEY_07         18       // only on RC and only used for selftest

#define KEY_NROF_KEYS  16

// next 2 'keys' are simulated when inserting or removing a MMC
#define KEY_MMC_IN     16
#define KEY_MMC_OUT    17


// remove these, not available on SIR100
#define KEY_00         0xFC

#define IS_IR_KEY(key)  (((key>=KEY_01) && (key<=KEY_09)) || (key==KEY_00))

/* definition of virtual special keys ------------------------------------- */
#define KEY_UNDEFINED  0x88
#define KEY_TIMEOUT    0xAA
#define KEY_NO_KEY     0xFFFF   // yes, indeed no u_char....

#define KEY_REPEAT_TIME     100 // 100 * 4.48 = about half a second
#define KEY_REPEAT          1
#define KEY_NO_REPEAT       2

/*-------------------------------------------------------------------------*/
/* export global routines (interface)                                      */
/*-------------------------------------------------------------------------*/
void    KbInit(void);
void    KbScan(void);
int     KbWaitForKeyEvent(u_long);
u_char  KbGetKey(void);
void    KbSetKeyRepeating(u_char, u_char);
void    KbInjectKey(u_char VirtualKey);

/*  様様  End Of File  様様様様 様様様様様様様様様様様様様様様様様様様様様様 */














/* ========================================================================
 * [PROJECT]    SIR
 * [MODULE]     PortIO
 * [TITLE]      Port IO header file
 * [FILE]       portio.h
 * [VSN]        2.0
 * [CREATED]    030414
 * [LASTCHNGD]  072405
 * [COPYRIGHT]  Copyright (C) STREAMIT BV 2010
 * [PURPOSE]    support for low-level pinning
 * ======================================================================== */

/*-------------------------------------------------------------------------*/
/* global defines                                                          */
/*-------------------------------------------------------------------------*/

//      ********* PORT B ************

//     7   6   5   4   3   2   1   0
//   �������������������������������Ŀ
//  �RES�CEN�CSC�XCS�SO � SI�CLK�CWP�
// ���������������������������������

#define MMC_ENABLE      5           // active LOW!
#define VS_ENABLE       6           // note that logic has changed since SIR80 V1

#define MMCVS_OUT_DDR   DDRB        // data-direction of MMC/VS functions
#define MMCVS_OUT_WRITE PORTB       // write PORTB (B as output)

//      ********* PORT D ************

//     7   6   5   4   3   2   1   0
//   �������������������������������Ŀ
//  �LD7�LD6�LD5�LD4�KC2�KC3� EN� - �
// ���������������������������������

#define LCD_EN          2           // LCD chip Enable (or: chip select)

#define LCD_DATA_4      4
#define LCD_DATA_5      5
#define LCD_DATA_6      6
#define LCD_DATA_7      7

#define LCD_DATA_DDR    DDRD        // data-direction of LCD-data port
#define LCD_DATA_PORT   PORTD       // port to write LCD-data
#define LCD_IN_PORT     PIND        // port to read LCD-data

#define LCD_EN_DDR      DDRE        // data-direction of LCD-control port
#define LCD_EN_PORT     PORTE       // data of LCD-control port

#define KB_OUT_DDR_B    DDRD        // data-direction of KB_OUT (second part)
#define KB_OUT_WRITE_B  PORTD       // write PORTD (D as output)

//      ********* PORT E ************

//     7   6   5   4   3   2   1   0
//   �������������������������������Ŀ
//  �CSF�REQ�ETH�IR �BL � - �TXD�RXD�
// ���������������������������������

#define FLASH_ENABLE    7       // active LOW!
#define LCD_BL_BIT      3       // Backlight LCD
#define IR_PIN          4       // Infrared receive PIN

#define FLASH_OUT_WRITE PORTE   // write PORTE for Flash CS
#define LCD_BL_PORT     PORTE



//      ********* PORT F ************

//     7   6   5   4   3   2   1   0
//   �������������������������������Ŀ
//  �KR3�KR2�KR1�KR0�LED�LRS�LRW�CRD�        // note TDI ( JTAG-pin) is shared with KR3
// ���������������������������������

#define MMC_CDETECT         0       // signal card inserted/removed
#define LCD_RW              1       // R/W LCD
#define LCD_RS              2       // Register select LCD
#define LED_PIN             3       // LED


#define LCD_RS_DDR          DDRF    // data-direction of LCD-control port
#define LCD_RS_PORT         PORTF   // data of LCD-control port
#define LCD_RW_DDR          DDRF    // data-direction of LCD-control port
#define LCD_RW_PORT         PORTF   // data of LCD-control port

#define LED_OUT_DDR         DDRF    // data-direction of LED control pin
#define LED_OUT_WRITE       PORTF   // write PORTF

#define KB_IN_DDR           DDRF    // data-direction of KB_IN
#define KB_IN_WRITE         PORTF   // write PORTF (F as output)
#define KB_IN_READ          PINF    // read PINF (F as input)

#define MMC_IN_DDR          DDRF    // data-direction of MMC_CDETECT
#define MMC_IN_READ         PINF    // read PINF (F as input)


//     7   6   5   4   3   2   1   0
//   �������������������������������Ŀ
//  � - � - � - �KC1�KC0� - � - � - �
// ���������������������������������

#define KB_OUT_DDR_A    DDRG        // data-direction of KB_OUT (first part)
#define KB_OUT_WRITE_A  PORTG       // write PORTG (G as output)


#define JTAG_REG            MCUCR
#define OVERFLOW_SIGNAL     sig_OVERFLOW0

#define init_8_bit_timer()  \
{                           \
    TCCR0B |= (1<<CS02);    \
    TIFR0 |= 1<<TOV0;       \
    TIMSK0 |= 1<<TOIE0;     \
}

#define disable_8_bit_timer_ovfl_int()  \
{                                       \
    TIMSK0 &= ~(1<<TOIE0);              \
}

//      ********* PORT G ************

/*-------------------------------------------------------------------------*/
/* export global routines (interface)                                      */
/*-------------------------------------------------------------------------*/

/*  ����  End Of File  �������� �������������������������������������������� */














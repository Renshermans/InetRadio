#ifndef _RTC_H_
#define	_RTC_H_

/* ========================================================================
 * [PROJECT]    SIR
 * [MODULE]     Real Time Clock
 * [TITLE]      High- and low level Routines for INtersil X1205 RTC chip
 * [FILE]       rtc.c
 * [VSN]        1.0
 * [CREATED]    13042007
 * [LASTCHNGD]  131042007
 * [COPYRIGHT]  Copyright (C) STREAMIT BV 2010
 * [PURPOSE]    contains all interface- and low-level routines to
 *              read/write date/time/status strings from the X1205
 * ======================================================================== */
#include <time.h>           // for tm-struct
                            //
/*! \name Non-Volatile Alarm Registers */
/*@{*/
#define X12RTC_SCA0         0x00    /*!< \brief Alarm 0 second. */
#define X12RTC_SCA1         0x08    /*!< \brief Alarm 1 second. */
#define X12RTC_SCA_ESC      0x80    /*!< \brief Second alarm enabled. */

#define X12RTC_MNA0         0x01    /*!< \brief Alarm 0 minute. */
#define X12RTC_MNA1         0x09    /*!< \brief Alarm 1 minute. */
#define X12RTC_MNA_EMN      0x80    /*!< \brief Minute alarm enabled. */

#define X12RTC_HRA0         0x02    /*!< \brief Alarm 0 hour. */
#define X12RTC_HRA1         0x0A    /*!< \brief Alarm 1 hour. */
#define X12RTC_HRA_EHR      0x80    /*!< \brief Hour alarm enabled. */

#define X12RTC_DTA0         0x03    /*!< \brief Alarm 0 day of month. */
#define X12RTC_DTA1         0x0B    /*!< \brief Alarm 1 day of month. */
#define X12RTC_DTA_EDT      0x80    /*!< \brief Day of month alarm enabled. */

#define X12RTC_MOA0         0x04    /*!< \brief Alarm 0 month. */
#define X12RTC_MOA1         0x0C    /*!< \brief Alarm 1 month. */
#define X12RTC_MOA_EMO      0x80    /*!< \brief Month alarm enabled. */

#define X12RTC_YRA0         0x05    /*!< \brief Currently unused alarm 0 register. */
#define X12RTC_YRA1         0x0D    /*!< \brief Currently unused alarm 1 register. */

#define X12RTC_DWA0         0x06    /*!< \brief Alarm 0 weekday. */
#define X12RTC_DWA1         0x0E    /*!< \brief Alarm 1 weekday. */
#define X12RTC_DWA_EDW      0x80    /*!< \brief Weekday alarm enabled. */

#define X12RTC_Y2K0         0x07    /*!< \brief Alarm 0 . */
#define X12RTC_Y2K1         0x0F    /*!< \brief Alarm 1 . */
/*@}*/

/*! \name Non-Volatile Control Registers */
/*@{*/
#define X12RTC_BL           0x10    /*!< \brief Block protection and watchdog register. */

#define X12RTC_BL_WD        0x14    /*!< \brief Watchdog configuration. */
#define X12RTC_BL_WD_1750   0x00    /*!< \brief Timeout after 1.75 seconds. */
#define X12RTC_BL_WD_750    0x04    /*!< \brief Timeout after 750 milliseconds. */
#define X12RTC_BL_WD_250    0x10    /*!< \brief Timeout after 250 milliseconds. */
#define X12RTC_BL_WD_OFF    0x14    /*!< \brief Disabled. */

#define X12RTC_BL_BP        0xE0    /*!< \brief Block protection. */
#define X12RTC_BL_BP_NONE   0x00    /*!< \brief No protection. */
#define X12RTC_BL_BP_UQUAD  0x20    /*!< \brief Upper quarter protected. */
#define X12RTC_BL_BP_UHALF  0x40    /*!< \brief Upper half protected. */
#define X12RTC_BL_BP_FULL   0x60    /*!< \brief Full array protected. */
#define X12RTC_BL_BP_FIRST1 0x80    /*!< \brief First page protected. */
#define X12RTC_BL_BP_FIRST2 0xA0    /*!< \brief First 2 pages protected. */
#define X12RTC_BL_BP_FIRST3 0xC0    /*!< \brief First 4 pages protected. */
#define X12RTC_BL_BP_FIRST8 0xE0    /*!< \brief First 8 pages protected. */

#define X12RTC_INT          0x11    /*!< \brief Interrupt control and freq. output register. */

#define X12RTC_INT_FO       0x14    /*!< \brief Programmable frequency output bits. */
#define X12RTC_INT_FO_IRQ   0x00    /*!< \brief Alarm interrupt. */
#define X12RTC_INT_FO_32KHZ 0x04    /*!< \brief 32.768kHz. */
#define X12RTC_INT_FO_100HZ 0x10    /*!< \brief 100Hz. */
#define X12RTC_INT_FO_1HZ   0x14    /*!< \brief 1Hz. */

#define X12RTC_INT_AL0E     0x20    /*!< \brief Alarm 0 interrupt enable. */
#define X12RTC_INT_AL1E     0x40    /*!< \brief Alarm 1 interrupt enable. */
#define X12RTC_INT_IM       0x80    /*!< \brief Repetitive alarm. */

#define X12RTC_ATR          0x12    /*!< \brief Analog trimming register. */

#define X12RTC_DTR          0x13    /*!< \brief Digital trimming register. */
#define X12RTC_DTR_NONE     0x00    /*!< \brief 0 PPM. */
#define X12RTC_DTR_PLUS10   0x02    /*!< \brief +10 PPM. */
#define X12RTC_DTR_PLUS20   0x01    /*!< \brief +20 PPM. */
#define X12RTC_DTR_PLUS30   0x03    /*!< \brief +30 PPM. */
#define X12RTC_DTR_MINUS10  0x06    /*!< \brief -10 PPM. */
#define X12RTC_DTR_MINUS20  0x05    /*!< \brief -20 PPM. */
#define X12RTC_DTR_MINUS30  0x07    /*!< \brief -30 PPM. */
/*@}*/

/*! \name Volatile Date and Time Registers */
/*@{*/
#define X12RTC_SC           0x30    /*!< Seconds register, 0 - 59. */
#define X12RTC_MN           0x31    /*!< Minutes register, 0 - 59. */
#define X12RTC_HR           0x32    /*!< Hours register, 0 - 23. */
#define X12RTC_HR_MIL       0x80    /*!< Use 24h format. */
#define X12RTC_DT           0x33    /*!< Day register, 1 - 31. */
#define X12RTC_MO           0x34    /*!< Month register, 1 - 12. */
#define X12RTC_YR           0x35    /*!< Year register, 0 - 99. */
#define X12RTC_DW           0x36    /*!< Day of the weeks register, 0 - 6. */
#define X128xRTC_SSEC       0x37    /*!< X1286 1/100 second register, 0 - 99 (read only). */
#define X122xRTC_Y2K        0x37    /*!< X1226 epoch register, 19 or 20. */
/*@}*/

/*! \name Volatile Status Register */
/*@{*/
#define X12RTC_SR           0x3F    /*!< Status register. */
#define X12RTC_SR_RTCF      0x01    /*!< Power failure. */
#define X12RTC_SR_WEL       0x02    /*!< Memory write enable. */
#define X12RTC_SR_RWEL      0x04    /*!< Register write enable. */
#define X12RTC_SR_AL0       0x20    /*!< Alarm 0 indicator. */
#define X12RTC_SR_AL1       0x40    /*!< Alarm 1 indicator. */
#define X12RTC_SR_BAT       0x80    /*!< Operating from battery. */
/*@}*/

#define RTC_STATUS_PF       0x00000001
#define RTC_STATUS_AL0      0x00000020
#define RTC_STATUS_AL1      0x00000040

#define RTC_ALARM_SECOND    0x00000001
#define RTC_ALARM_MINUTE    0x00000002
#define RTC_ALARM_HOUR      0x00000004
#define RTC_ALARM_MDAY      0x00000008
#define RTC_ALARM_MONTH     0x00000010
#define RTC_ALARM_WDAY      0x00000080

/*!
 * \brief Convert binary coded decimal to binary value.
 */
#define BCD2BIN(x) ((((u_char)(x)) >> 4) * 10 + ((x) & 0x0F))

/*!
 * \brief Convert binary to binary coded decimal value.
 */
#define BIN2BCD(x) (((((u_char)(x)) / 10) << 4) + (x) % 10)


/* Prototypes */
extern int X12Init(void);

extern int X12RtcGetClock(tm *tm);
extern int X12RtcSetClock(CONST tm *tm);
extern int X12RtcGetAlarm(int idx, tm *tm, int *aflgs);
extern int X12RtcSetAlarm(int idx, CONST tm *tm, int aflgs);
extern int X12RtcGetStatus(u_long *sflgs);
extern int X12RtcClearStatus(u_long sflgs);
extern int X12RtcReadRegs(u_char addr, u_char *buff, size_t len);
extern int X12RtcWrite(int nv, CONST u_char *buff, size_t len);

extern int X12EepromRead(u_int addr, void *buff, size_t len);
extern int X12EepromWrite(u_int addr, CONST void *buff, size_t len);

/* End of prototypes */
#endif

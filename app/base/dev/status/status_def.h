/***************************************************************************************
 file:         status_def.h
 description:  the source file of status definition
 date:         2017/11/1
 author        liuzhongwen
 ***************************************************************************************/
#include "status.h"
ST_TABLE_BEGIN()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_POW_VOLTAGE)
ST_MEMBER_BEGIN()
ST_MEMBER("Power Voltage", ST_DATA_USHORT, sizeof(unsigned short)) /* KL30/31 Power Voltage */
ST_MEMBER_END()
ST_ITEM_END()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_KL15_SIG)
ST_MEMBER_BEGIN()
ST_MEMBER("KL15 SIG", ST_DATA_UCHAR, sizeof(unsigned char)) /* KL15 SIG */
ST_MEMBER_END()
ST_ITEM_END()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_KL75_SIG)
ST_MEMBER_BEGIN()
ST_MEMBER("KL75 SIG", ST_DATA_UCHAR, sizeof(unsigned char)) /* KL75 SIG */
ST_MEMBER_END()
ST_ITEM_END()


ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_BCALL_SIG)
ST_MEMBER_BEGIN()
ST_MEMBER("BCALL SIG", ST_DATA_UCHAR, sizeof(unsigned char)) /* BCALL SIG */
ST_MEMBER_END()
ST_ITEM_END()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_ICALL_SIG)
ST_MEMBER_BEGIN()
ST_MEMBER("ICALL SIG", ST_DATA_UCHAR, sizeof(unsigned char)) /* ICALL SIG */
ST_MEMBER_END()
ST_ITEM_END()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_ECALL_SIG)
ST_MEMBER_BEGIN()
ST_MEMBER("ECALL SIG", ST_DATA_UCHAR, sizeof(unsigned char)) /* ECALL SIG */
ST_MEMBER_END()
ST_ITEM_END()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_SLOW_CHG_SIG)
ST_MEMBER_BEGIN()
ST_MEMBER("SLOW CHARGE", ST_DATA_UCHAR, sizeof(unsigned char)) /* SLOW CHARGE SIG */
ST_MEMBER_END()
ST_ITEM_END()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_QUICK_CHG_SIG)
ST_MEMBER_BEGIN()
ST_MEMBER("QUICK CHARGE", ST_DATA_UCHAR, sizeof(unsigned char)) /* QUICK CHARGE SIG */
ST_MEMBER_END()
ST_ITEM_END()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_BCALL)
ST_MEMBER_BEGIN()
ST_MEMBER("BCALL", ST_DATA_UCHAR, sizeof(unsigned char)) /* BCALL */
ST_MEMBER_END()
ST_ITEM_END()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_ICALL)
ST_MEMBER_BEGIN()
ST_MEMBER("ICALL", ST_DATA_UCHAR, sizeof(unsigned char)) /* BCALL */
ST_MEMBER_END()
ST_ITEM_END()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_ECALL)
ST_MEMBER_BEGIN()
ST_MEMBER("ECALL", ST_DATA_UCHAR, sizeof(unsigned char)) /* BCALL */
ST_MEMBER_END()
ST_ITEM_END()

/*       bit0 ---> slow charge signal
 bit1 ---> KL15 signal
 bit2 ---> quick charge signal
 bit3 ---> RING
 bit4 ---> RTC
 bit5 ---> CAN1
 bit6 ---> ECU upgrade
 bit7 ---> BLE
 bit8 ---> G-SENSOR
 bit9 ---> icall
 bit10---> bcall
 bit11---> ecall
 bit12---> CAN2
 bit13---> CAN3
 bit14��15---> reserved
 */
ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_WAKEUP_SRC)
ST_MEMBER_BEGIN()
ST_MEMBER("WAKEUP SRC", ST_DATA_USHORT, sizeof(unsigned short)) /* WAKEUP SRC */
ST_MEMBER_END()
ST_ITEM_END()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_PM_MODE)
ST_MEMBER_BEGIN()
ST_MEMBER("PM MODE", ST_DATA_UCHAR, sizeof(unsigned char)) /* PM MODE */
ST_MEMBER_END()
ST_ITEM_END()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_SRS_SIG)
ST_MEMBER_BEGIN()
ST_MEMBER("SRS SIG", ST_DATA_UCHAR, sizeof(unsigned char)) /* SRS SIG */
ST_MEMBER_END()
ST_ITEM_END()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_RBT_CNT)
ST_MEMBER_BEGIN()
ST_MEMBER("MCU REBOOT CNT", ST_DATA_UINT, sizeof(unsigned int)) /* mcu reboot count */
ST_MEMBER_END()
ST_ITEM_END()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_TSP_COMM)
ST_MEMBER_BEGIN()
ST_MEMBER("TSP COMM", ST_DATA_UCHAR, sizeof(unsigned char)) /* TSP COMM STATUS */
ST_MEMBER_END()
ST_ITEM_END()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_BAT_VOL)
ST_MEMBER_BEGIN()
ST_MEMBER("BAT Voltage", ST_DATA_USHORT, sizeof(unsigned short)) /* BAT Voltage */
ST_MEMBER_END()
ST_ITEM_END()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_BAT_TEMP)
ST_MEMBER_BEGIN()
ST_MEMBER("BAT Temprature", ST_DATA_INT, sizeof(int)) /* BAT Temprature */
ST_MEMBER_END()
ST_ITEM_END()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_BAT_RST)
ST_MEMBER_BEGIN()
ST_MEMBER("BAT Resistance", ST_DATA_UINT, sizeof(unsigned int)) /* BAT Resistance */
ST_MEMBER_END()
ST_ITEM_END()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_BAT_STATUS)
ST_MEMBER_BEGIN()
ST_MEMBER("BAT Health status", ST_DATA_UCHAR, sizeof(unsigned char)) /* BAT status */
ST_MEMBER_END()
ST_ITEM_END()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_APP_SLEEP)
ST_MEMBER_BEGIN()
ST_MEMBER("APP CAN SLEEP", ST_DATA_UCHAR, sizeof(unsigned char)) /* APP CAN SLEEP STATUS */
ST_MEMBER_END()
ST_ITEM_END()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_CAN1_STATUS)
ST_MEMBER_BEGIN()
ST_MEMBER("CAN1 STATUS", ST_DATA_UCHAR, sizeof(unsigned char)) /* CAN1 STATUS */
ST_MEMBER_END()
ST_ITEM_END()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_CAN2_STATUS)
ST_MEMBER_BEGIN()
ST_MEMBER("CAN2 STATUS", ST_DATA_UCHAR, sizeof(unsigned char)) /* CAN2 STATUS */
ST_MEMBER_END()
ST_ITEM_END()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_CAN3_STATUS)
ST_MEMBER_BEGIN()
ST_MEMBER("CAN3 STATUS", ST_DATA_UCHAR, sizeof(unsigned char)) /* CAN3 STATUS */
ST_MEMBER_END()
ST_ITEM_END()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_CAN4_STATUS)
ST_MEMBER_BEGIN()
ST_MEMBER("CAN4 STATUS", ST_DATA_UCHAR, sizeof(unsigned char)) /* CAN4 STATUS */
ST_MEMBER_END()
ST_ITEM_END()

ST_ITEM_BEGIN()
ST_ITEM_ID(ST_ITEM_UPG_STATUS)
ST_MEMBER_BEGIN()
ST_MEMBER("TBOX UPG STATUS", ST_DATA_UCHAR, sizeof(unsigned char))   /* TBOX UPG STATUS, 0 indicates not upgrading, 1 indicates upgrading */
ST_MEMBER_END()
ST_ITEM_END()

ST_TABLE_END()

